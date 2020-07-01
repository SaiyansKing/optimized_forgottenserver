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

#ifndef FS_STRING_H_C5003BEA4242FA1BF0EB01601538A14ABE32BBEF0EF9666214578864
#define FS_STRING_H_C5003BEA4242FA1BF0EB01601538A14ABE32BBEF0EF9666214578864

#include <string>
#include <cstring>

constexpr char ext_digits[] =
"0001020304050607080910111213141516171819"
"2021222324252627282930313233343536373839"
"4041424344454647484950515253545556575859"
"6061626364656667686970717273747576777879"
"8081828384858687888990919293949596979899";

namespace std
{
	class stringExtended : public string
	{
		public:
			stringExtended() : string() { }
			stringExtended(size_t reserveSize) : string() { this->reserve(reserveSize); }

			inline stringExtended& append(const string& str) {
				string::append(str);
				return (*this);
			}
			inline stringExtended& append(const string& str, size_t subpos, size_t sublen) {
				string::append(str, subpos, sublen);
				return (*this);
			}
			inline stringExtended& append(const char* str) {
				string::append(str);
				return (*this);
			}
			inline stringExtended& append(const char* str, size_t n) {
				string::append(str, n);
				return (*this);
			}
			inline stringExtended& append(size_t n, char c) {
				string::append(n, c);
				return (*this);
			}

			stringExtended& appendInt(uint64_t value) {
				char str_buffer[22]; // Should be able to contain uint64_t max value + sign
				char* ptrBufferEnd = str_buffer + 21;
				char* ptrBuffer = ptrBufferEnd;

				while (value >= 100) {
					uint32_t index = static_cast<uint32_t>((value % 100) * 2);
					value /= 100;

					ptrBuffer -= 2;
					memcpy(ptrBuffer, ext_digits + index, 2);
				}
				if (value < 10) {
					*--ptrBuffer = static_cast<char>('0' + value);
					string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
					return (*this);
				}

				uint32_t index = static_cast<uint32_t>(value * 2);
				ptrBuffer -= 2;
				memcpy(ptrBuffer, ext_digits + index, 2);
				string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
				return (*this);
			}
		
			stringExtended& appendInt(int64_t value) {
				uint64_t abs_value = static_cast<uint64_t>(value);
				bool neg = (value < 0);
				if (neg) {
					abs_value = 0 - abs_value;
				}

				char str_buffer[22]; // Should be able to contain uint64_t max value + sign
				char* ptrBufferEnd = str_buffer + 21;
				char* ptrBuffer = ptrBufferEnd;

				while (abs_value >= 100) {
					uint32_t index = static_cast<uint32_t>((abs_value % 100) * 2);
					abs_value /= 100;

					ptrBuffer -= 2;
					memcpy(ptrBuffer, ext_digits + index, 2);
				}
				if (abs_value < 10) {
					*--ptrBuffer = static_cast<char>('0' + abs_value);
					if (neg) {
						*--ptrBuffer = '-';
					}
					string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
					return (*this);
				}

				uint32_t index = static_cast<uint32_t>(abs_value * 2);
				ptrBuffer -= 2;
				memcpy(ptrBuffer, ext_digits + index, 2);
				if (neg) {
					*--ptrBuffer = '-';
				}
				string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
				return (*this);
			}

			inline stringExtended& appendInt(uint32_t value) { return appendInt(static_cast<uint64_t>(value)); }
			inline stringExtended& appendInt(uint16_t value) { return appendInt(static_cast<uint64_t>(value)); }
			inline stringExtended& appendInt(uint8_t value) { return appendInt(static_cast<uint64_t>(value)); }
			inline stringExtended& appendInt(int32_t value) { return appendInt(static_cast<int64_t>(value)); }
			inline stringExtended& appendInt(int16_t value) { return appendInt(static_cast<int64_t>(value)); }
			inline stringExtended& appendInt(int8_t value) { return appendInt(static_cast<int64_t>(value)); }
			
			stringExtended& appendIntShowPos(uint64_t value) {
				char str_buffer[22]; // Should be able to contain uint64_t max value + sign
				char* ptrBufferEnd = str_buffer + 21;
				char* ptrBuffer = ptrBufferEnd;

				while (value >= 100) {
					uint32_t index = static_cast<uint32_t>((value % 100) * 2);
					value /= 100;

					ptrBuffer -= 2;
					memcpy(ptrBuffer, ext_digits + index, 2);
				}
				if (value < 10) {
					*--ptrBuffer = static_cast<char>('0' + value);
					*--ptrBuffer = '+';
					string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
					return (*this);
				}

				uint32_t index = static_cast<uint32_t>(value * 2);
				ptrBuffer -= 2;
				memcpy(ptrBuffer, ext_digits + index, 2);
				*--ptrBuffer = '+';
				string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
				return (*this);
			}
			
			stringExtended& appendIntShowPos(int64_t value) {
				uint64_t abs_value = static_cast<uint64_t>(value);
				bool neg = (value < 0);
				if (neg) {
					abs_value = 0 - abs_value;
				}

				char str_buffer[22]; // Should be able to contain uint64_t max value + sign
				char* ptrBufferEnd = str_buffer + 21;
				char* ptrBuffer = ptrBufferEnd;

				while (abs_value >= 100) {
					uint32_t index = static_cast<uint32_t>((abs_value % 100) * 2);
					abs_value /= 100;

					ptrBuffer -= 2;
					memcpy(ptrBuffer, ext_digits + index, 2);
				}
				if (abs_value < 10) {
					*--ptrBuffer = static_cast<char>('0' + abs_value);
					if (neg) {
						*--ptrBuffer = '-';
					} else {
						*--ptrBuffer = '+';
					}
					string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
					return (*this);
				}

				uint32_t index = static_cast<uint32_t>(abs_value * 2);
				ptrBuffer -= 2;
				memcpy(ptrBuffer, ext_digits + index, 2);
				if (neg) {
					*--ptrBuffer = '-';
				} else {
					*--ptrBuffer = '+';
				}
				string::append(ptrBuffer, std::distance(ptrBuffer, ptrBufferEnd));
				return (*this);
			}

			inline stringExtended& appendIntShowPos(uint32_t value) { return appendIntShowPos(static_cast<uint64_t>(value)); }
			inline stringExtended& appendIntShowPos(uint16_t value) { return appendIntShowPos(static_cast<uint64_t>(value)); }
			inline stringExtended& appendIntShowPos(uint8_t value) { return appendIntShowPos(static_cast<uint64_t>(value)); }
			inline stringExtended& appendIntShowPos(int32_t value) { return appendIntShowPos(static_cast<int64_t>(value)); }
			inline stringExtended& appendIntShowPos(int16_t value) { return appendIntShowPos(static_cast<int64_t>(value)); }
			inline stringExtended& appendIntShowPos(int8_t value) { return appendIntShowPos(static_cast<int64_t>(value)); }

			inline stringExtended& operator<< (const string& str) { return append(str); }
			inline stringExtended& operator<< (const char* str) { return append(str); }
			inline stringExtended& operator<< (const char c) { return append(1, c); }

			inline stringExtended& operator<< (uint64_t value) { return appendInt(value); }
			inline stringExtended& operator<< (uint32_t value) { return appendInt(value); }
			inline stringExtended& operator<< (uint16_t value) { return appendInt(value); }
			inline stringExtended& operator<< (uint8_t value) { return appendInt(value); }
			inline stringExtended& operator<< (int32_t value) { return appendInt(value); }
			inline stringExtended& operator<< (int16_t value) { return appendInt(value); }
			inline stringExtended& operator<< (int8_t value) { return appendInt(value); }

			inline stringExtended& operator<<= (uint64_t value) { return appendIntShowPos(value); }
			inline stringExtended& operator<<= (uint32_t value) { return appendIntShowPos(value); }
			inline stringExtended& operator<<= (uint16_t value) { return appendIntShowPos(value); }
			inline stringExtended& operator<<= (uint8_t value) { return appendIntShowPos(value); }
			inline stringExtended& operator<<= (int32_t value) { return appendIntShowPos(value); }
			inline stringExtended& operator<<= (int16_t value) { return appendIntShowPos(value); }
			inline stringExtended& operator<<= (int8_t value) { return appendIntShowPos(value); }
	};
};

#endif
