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

#include "networkmessage.h"

#include "container.h"
#include "creature.h"

Position NetworkMessage::getPosition()
{
	Position pos;
	pos.x = read<uint16_t>();
	pos.y = read<uint16_t>();
	pos.z = readByte();
	return pos;
}

void NetworkMessage::addDouble(double value, uint8_t precision/* = 2*/)
{
	writeByte(precision);
	write<uint32_t>((value * std::pow(static_cast<float>(10), precision)) + std::numeric_limits<int32_t>::max());
}

void NetworkMessage::addPosition(const Position& pos)
{
	write<uint16_t>(pos.x);
	write<uint16_t>(pos.y);
	writeByte(pos.z);
}

void NetworkMessage::addItemId(uint16_t itemId)
{
	write<uint16_t>(Item::items[itemId].clientId);
}
