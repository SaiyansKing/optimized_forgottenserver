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

#define FS_OTPCH_H_F00C737DA6CA4C8D90F57430C614367F

#include <boost/asio.hpp>

// Definitions should be global.
#include "definitions.h"

#include "pugixml/src/pugixml.hpp"
#include "features.h"

// Canary Lib
#include "../canary-lib/include/include.hpp"

#if GAME_FEATURE_ROBINHOOD_HASH_MAP > 0
#include "robin_hood.h"
#endif
