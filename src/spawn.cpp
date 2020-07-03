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

#include "spawn.h"
#include "game.h"
#include "monster.h"
#include "configmanager.h"

#include "pugicast.h"

static constexpr int32_t MINSPAWN_INTERVAL = 1000;

bool Spawns::loadFromXml(const std::string& filename)
{
	if (loaded) {
		return true;
	}

	npcList.reserve(1000);
	spawnList.reserve(1000);

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str());
	if (!result) {
		printXMLError("Error - Spawns::loadFromXml", filename, result);
		return false;
	}

	this->filename = filename;
	loaded = true;

	for (auto spawnNode : doc.child("spawns").children()) {
		Position centerPos(
			pugi::cast<uint16_t>(spawnNode.attribute("centerx").value()),
			pugi::cast<uint16_t>(spawnNode.attribute("centery").value()),
			pugi::cast<uint16_t>(spawnNode.attribute("centerz").value())
		);

		int32_t radius;
		pugi::xml_attribute radiusAttribute = spawnNode.attribute("radius");
		if (radiusAttribute) {
			radius = pugi::cast<int32_t>(radiusAttribute.value());
		} else {
			radius = -1;
		}

		spawnList.emplace_back(centerPos);
		Spawn& spawn = spawnList.front();

		for (auto childNode : spawnNode.children()) {
			if (strcasecmp(childNode.name(), "monster") == 0) {
				pugi::xml_attribute nameAttribute = childNode.attribute("name");
				if (!nameAttribute) {
					continue;
				}

				Direction dir;

				pugi::xml_attribute directionAttribute = childNode.attribute("direction");
				if (directionAttribute) {
					dir = static_cast<Direction>(pugi::cast<uint16_t>(directionAttribute.value()));
				} else {
					dir = DIRECTION_NORTH;
				}

				Position pos(
					centerPos.x + pugi::cast<uint16_t>(childNode.attribute("x").value()),
					centerPos.y + pugi::cast<uint16_t>(childNode.attribute("y").value()),
					centerPos.z
				);
				uint32_t interval = pugi::cast<uint32_t>(childNode.attribute("spawntime").value()) * 1000;
				if (interval > MINSPAWN_INTERVAL) {
					spawn.addMonster(nameAttribute.as_string(), pos, dir, interval);
				} else {
					std::cout << "[Warning - Spawns::loadFromXml] " << nameAttribute.as_string() << ' ' << pos << " spawntime can not be less than " << MINSPAWN_INTERVAL / 1000 << " seconds." << std::endl;
				}
			} else if (strcasecmp(childNode.name(), "npc") == 0) {
				pugi::xml_attribute nameAttribute = childNode.attribute("name");
				if (!nameAttribute) {
					continue;
				}

				Npc* npc = Npc::createNpc(nameAttribute.as_string());
				if (!npc) {
					continue;
				}

				pugi::xml_attribute directionAttribute = childNode.attribute("direction");
				if (directionAttribute) {
					npc->setDirection(static_cast<Direction>(pugi::cast<uint16_t>(directionAttribute.value())));
				}

				npc->setMasterPos(Position(
					centerPos.x + pugi::cast<uint16_t>(childNode.attribute("x").value()),
					centerPos.y + pugi::cast<uint16_t>(childNode.attribute("y").value()),
					centerPos.z
				), radius);
				npcList.push_back(npc);
			}
		}
	}
	return true;
}

void Spawns::startup()
{
	if (!loaded || isStarted()) {
		return;
	}

	for (Npc* npc : npcList) {
		g_game().placeCreature(npc, npc->getMasterPos(), false, true);
	}
	npcList.clear();
	npcList.shrink_to_fit();

	for (Spawn& spawn : spawnList) {
		spawn.startup();
	}

	started = true;
}

void Spawns::clear()
{
	for (Spawn& spawn : spawnList) {
		spawn.stopEvent();
	}
	spawnList.clear();
	spawnList.shrink_to_fit();

	loaded = false;
	started = false;
	filename.clear();
}

bool Spawns::isInZone(const Position& centerPos, int32_t radius, const Position& pos)
{
	if (radius == -1) {
		return true;
	}

	return ((pos.getX() >= centerPos.getX() - radius) && (pos.getX() <= centerPos.getX() + radius) &&
	        (pos.getY() >= centerPos.getY() - radius) && (pos.getY() <= centerPos.getY() + radius));
}

void Spawn::startSpawnCheck()
{
	if (checkSpawnEvent == 0) {
		checkSpawnEvent = g_dispatcher().addEvent(getInterval(), std::bind(&Spawn::checkSpawn, this));
	}
}

Spawn::~Spawn()
{
	for (const spawnBlock_t& sb : spawnMap) {
		if (sb.monster) {
			sb.monster->setSpawn(nullptr);
			sb.monster->decrementReferenceCounter();
		}
	}
}

bool Spawn::findPlayer(const Position& pos)
{
	SpectatorVector spectators;
	g_game().map.getSpectators(spectators, pos, false, true);
	for (Creature* spectator : spectators) {
		if (!spectator->getPlayer()->hasFlag(PlayerFlag_IgnoredByMonsters)) {
			return true;
		}
	}
	return false;
}

bool Spawn::spawnMonster(spawnBlock_t& sb, bool startup /*= false*/)
{
	std::unique_ptr<Monster> monster_ptr(new Monster(sb.mType));
	monster_ptr->setDirection(sb.direction);
	if (startup) {
		//No need to send out events to the surrounding since there is no one out there to listen!
		if (!g_game().internalPlaceCreature(monster_ptr.get(), sb.pos, true)) {
			return false;
		}
	} else {
		if (!g_game().placeCreature(monster_ptr.get(), sb.pos, false, true)) {
			return false;
		}
	}

	sb.monster = monster_ptr.release();
	sb.monster->setSpawn(this);
	sb.monster->setMasterPos(sb.pos);
	#if GAME_FEATURE_NEWSPEED_LAW > 0
	sb.monster->cacheSpeed();
	#endif
	sb.monster->incrementReferenceCounter();
	sb.lastSpawn = OTSYS_TIME();
	return true;
}

void Spawn::startup()
{
	spawnMap.shrink_to_fit();
	for (spawnBlock_t& sb : spawnMap) {
		spawnMonster(sb, true);
	}
}

void Spawn::checkSpawn()
{
	checkSpawnEvent = 0;

	int32_t spawnCount = 0;
	size_t spawnedCount = 0;
	for (uint32_t i = 0, end = static_cast<uint32_t>(spawnMap.size()); i < end; ++i) {
		if (++spawnIndex >= end) {
			spawnIndex = 0;
		}

		spawnBlock_t& sb = spawnMap[spawnIndex];
		if (sb.monster) {
			if (sb.monster->isRemoved()) {
				sb.lastSpawn = OTSYS_TIME();
				sb.monster->decrementReferenceCounter();
				sb.monster = nullptr;
			} else {
				++spawnedCount;
			}
			continue;
		}

		if (OTSYS_TIME() >= sb.lastSpawn + sb.interval) {
			if (sb.mType->info.isBlockable && findPlayer(sb.pos)) {
				sb.lastSpawn = OTSYS_TIME();
				continue;
			}

			if (sb.mType->info.isBlockable) {
				if (spawnMonster(sb)) {
					++spawnedCount;
					if (++spawnCount >= g_config().getNumber(ConfigManager::RATE_SPAWN)) {
						break;
					}
				}
			} else {
				scheduleSpawn(spawnIndex, 3 * 1500);
				++spawnedCount;
				if (++spawnCount >= g_config().getNumber(ConfigManager::RATE_SPAWN)) {
					break;
				}
			}
		}
	}

	if (spawnedCount < spawnMap.size()) {
		checkSpawnEvent = g_dispatcher().addEvent(getInterval(), std::bind(&Spawn::checkSpawn, this));
	}
}

void Spawn::scheduleSpawn(uint32_t spawnId, int32_t interval)
{
	if (spawnId >= static_cast<uint32_t>(spawnMap.size())) {
		return;
	}

	spawnBlock_t& sb = spawnMap[spawnId];
	if (interval <= 0) {
		if (!spawnMonster(sb)) {
			sb.lastSpawn = OTSYS_TIME();
			startSpawnCheck();
		}
	} else {
		g_game().addMagicEffect(sb.pos, CONST_ME_TELEPORT);
		g_dispatcher().addEvent(1500, std::bind(&Spawn::scheduleSpawn, this, spawnId, interval - 1500));
	}
}

bool Spawn::addMonster(const std::string& name, const Position& pos, Direction dir, uint32_t interval)
{
	MonsterType* mType = g_monsters().getMonsterType(name);
	if (!mType) {
		std::cout << "[Spawn::addMonster] Can not find " << name << std::endl;
		return false;
	}

	this->interval = std::min<uint32_t>(this->interval, interval);
	spawnMap.emplace_back(mType, interval, pos, dir);
	return true;
}

void Spawn::removeMonster(Monster* monster)
{
	for (spawnBlock_t& sb : spawnMap) {
		if (sb.monster == monster) {
			monster->decrementReferenceCounter();
			sb.lastSpawn = OTSYS_TIME();
			sb.monster = nullptr;
			return;
		}
	}
}

void Spawn::stopEvent()
{
	if (checkSpawnEvent != 0) {
		g_dispatcher().stopEvent(checkSpawnEvent);
		checkSpawnEvent = 0;
	}
}
