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

#ifndef FS_NETWORKMESSAGE_H_B853CFED58D1413A87ACED07B2926E03
#define FS_NETWORKMESSAGE_H_B853CFED58D1413A87ACED07B2926E03

#include "const.h"
#include "item.h"
#include "position.h"

class Item;
struct Position;

class NetworkMessage : public CanaryLib::NetworkMessage
{
	public:
		NetworkMessage() = default;

		// TODO: migrate that to lib in a generic way. needs to update client too.
		Position getPosition();
		void addDouble(double value, uint8_t precision = 2);
		void addItemId(uint16_t itemId);
		void addPosition(const Position& pos);
};

#endif // #ifndef __NETWORK_MESSAGE_H__
