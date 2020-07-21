/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2020  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "otpch.h"
#include "tasks.h"

#include "configmanager.h"
#include "protocol.h"
#include "outputmessage.h"
#include "rsa.h"

Protocol::~Protocol()
{
	if (compreesionEnabled) {
		deflateEnd(defStream.get());
	}
}

void Protocol::onSendMessage(const OutputMessage_ptr& msg)
{
	if (!rawMessages) {
		uint32_t _compression = 0;
		if (compreesionEnabled && msg->getLength() >= 128) {
			if (compression(*msg)) {
				_compression = (1U << 31);
			}
		}

		msg->writeMessageLength();

		if (encryptionEnabled) {
      msg->encryptXTEA();
      if (checksumMethod == CanaryLib::CHECKSUM_METHOD_NONE) {
        msg->addCryptoHeader(false, 0);
      } else if (checksumMethod == CanaryLib::CHECKSUM_METHOD_ADLER32) {
        msg->addCryptoHeader(true, NetworkMessage::getChecksum(msg->getOutputBuffer(), msg->getLength()));
      } else if (checksumMethod == CanaryLib::CHECKSUM_METHOD_SEQUENCE) {
        msg->addCryptoHeader(true, _compression | (++serverSequenceNumber));
        if (serverSequenceNumber >= 0x7FFFFFFF) {
          serverSequenceNumber = 0;
        }
      }
		}
  }
}

bool Protocol::onRecvMessage(NetworkMessage& msg)
{
	if (checksumMethod != CanaryLib::CHECKSUM_METHOD_NONE) {
		uint32_t recvChecksum = msg.read<uint32_t>();
		if (checksumMethod == CanaryLib::CHECKSUM_METHOD_SEQUENCE) {
			if (recvChecksum == 0) {
				// checksum 0 indicate that the packet should be connection ping - 0x1C packet header
				// since we don't need that packet skip it
				return false;
			}

			uint32_t checksum = ++clientSequenceNumber;
			if (clientSequenceNumber >= 0x7FFFFFFF) {
				clientSequenceNumber = 0;
			}

			if (recvChecksum != checksum) {
				// incorrect packet - skip it
				return false;
			}
		} else {
			uint32_t checksum;
			int32_t len = msg.getLength() - msg.getBufferPosition();
			if (len > 0) {
				checksum = NetworkMessage::getChecksum(msg.getBuffer() + msg.getBufferPosition(), len);
			} else {
				checksum = 0;
			}

			if (recvChecksum != checksum) {
				// incorrect packet - skip it
				return false;
			}
		}
	}
	if (encryptionEnabled && !msg.decryptXTEA(static_cast<CanaryLib::ChecksumMethods_t>(checksumMethod))) {
		return false;
	}

	using ProtocolWeak_ptr = std::weak_ptr<Protocol>;
	ProtocolWeak_ptr protocolWeak = std::weak_ptr<Protocol>(shared_from_this());

	std::function<void (void)> callback = [protocolWeak, &msg]() {
		if (auto protocol = protocolWeak.lock()) {
			if (auto connection = protocol->getConnection()) {
				protocol->parsePacket(msg);
				connection->resumeWork();
			}
		}
	};
	g_dispatcher().addTask(callback);
	return true;
}

OutputMessage_ptr Protocol::getOutputBuffer(int32_t size)
{
	//dispatcher thread
	if (!outputBuffer) {
		outputBuffer = OutputMessagePool::getOutputMessage();
	} else if ((outputBuffer->getLength() + size) > CanaryLib::MAX_PROTOCOL_BODY_LENGTH) {
		send(outputBuffer);
		outputBuffer = OutputMessagePool::getOutputMessage();
	}
	return outputBuffer;
}

bool Protocol::decryptRSA(NetworkMessage& msg)
{
	if ((msg.getLength() - msg.getBufferPosition()) < 128) {
		return false;
	}

	g_RSA().decrypt(reinterpret_cast<char*>(msg.getBuffer()) + msg.getBufferPosition()); //does not break strict aliasing
	return (msg.readByte() == 0);
}

uint32_t Protocol::getIP() const
{
	if (auto connection = getConnection()) {
		return connection->getIP();
	}

	return 0;
}

void Protocol::enableCompression()
{
	if(!compreesionEnabled)
	{
		int32_t compressionLevel = g_config().getNumber(ConfigManager::COMPRESSION_LEVEL);
		if (compressionLevel != 0) {
			defStream.reset(new z_stream);
			defStream->zalloc = Z_NULL;
			defStream->zfree = Z_NULL;
			defStream->opaque = Z_NULL;
			if (deflateInit2(defStream.get(), compressionLevel, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY) != Z_OK) {
				defStream.reset();
				std::cout << "Zlib deflateInit2 error: " << (defStream->msg ? defStream->msg : "unknown error") << std::endl;
			} else {
				compreesionEnabled = true;
			}
		}
	}
}

bool Protocol::compression(OutputMessage& msg)
{
	static thread_local uint8_t defBuffer[CanaryLib::NETWORKMESSAGE_MAXSIZE];
	defStream->next_in = msg.getOutputBuffer();
	defStream->avail_in = msg.getLength();
	defStream->next_out = defBuffer;
	defStream->avail_out = CanaryLib::NETWORKMESSAGE_MAXSIZE;

	int32_t ret = deflate(defStream.get(), Z_FINISH);
	if (ret != Z_OK && ret != Z_STREAM_END) {
		return false;
	}
	uint32_t totalSize = static_cast<uint32_t>(defStream->total_out);
	deflateReset(defStream.get());
	if (totalSize == 0) {
		return false;
	}

	msg.reset();
	msg.write(reinterpret_cast<const char*>(defBuffer), static_cast<size_t>(totalSize));
	return true;
}
