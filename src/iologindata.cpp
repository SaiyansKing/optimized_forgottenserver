/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2019  Mark Samman <mark.samman@gmail.com>
 * Copyright (C) 2019-2021  Saiyans King
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

#include "databasetasks.h"
#include "iologindata.h"
#include "configmanager.h"
#include "game.h"

extern ConfigManager g_config;
extern Game g_game;

Account IOLoginData::loadAccount(uint32_t accno)
{
	Account account;

	std::stringExtended query(128);
	query << "SELECT `id`, `name`, `password`, `type`, `premdays`, `lastday` FROM `accounts` WHERE `id` = " << accno << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return account;
	}

	account.id = result->getNumber<uint32_t>("id");
	account.name = std::move(result->getString("name"));
	account.accountType = static_cast<AccountType_t>(result->getNumber<int32_t>("type"));
	account.premiumDays = result->getNumber<uint16_t>("premdays");
	account.lastDay = result->getNumber<time_t>("lastday");
	return account;
}

bool IOLoginData::saveAccount(const Account& acc)
{
	std::stringExtended query(128);
	query << "UPDATE `accounts` SET `premdays` = " << acc.premiumDays << ", `lastday` = " << acc.lastDay << " WHERE `id` = " << acc.id;
	return g_database.executeQuery(query);
}

std::string decodeSecret(const std::string& secret)
{
	// simple base32 decoding
	std::string key;
	key.reserve(10);

	uint32_t buffer = 0, left = 0;
	for (const auto& ch : secret) {
		buffer <<= 5;
		if (ch >= 'A' && ch <= 'Z') {
			buffer |= (ch & 0x1F) - 1;
		} else if (ch >= '2' && ch <= '7') {
			buffer |= ch - 24;
		} else {
			// if a key is broken, return empty and the comparison
			// will always be false since the token must not be empty
			return {};
		}

		left += 5;
		if (left >= 8) {
			left -= 8;
			key.push_back(static_cast<char>(buffer >> left));
		}
	}

	return key;
}

bool IOLoginData::loginserverAuthentication(const std::string& name, const std::string& password, Account& account)
{
	const std::string& escapedName = g_database.escapeString(name);
	std::stringExtended query(escapedName.length() + static_cast<size_t>(128));
	query << "SELECT `id`, `name`, `password`, `secret`, `type`, `premdays`, `lastday` FROM `accounts` WHERE `name` = " << escapedName << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return false;
	}

	if (transformToSHA1(password) != result->getString("password")) {
		return false;
	}

	account.id = result->getNumber<uint32_t>("id");
	account.name = std::move(result->getString("name"));
	account.key = std::move(decodeSecret(result->getString("secret")));
	account.accountType = static_cast<AccountType_t>(result->getNumber<int32_t>("type"));
	account.premiumDays = result->getNumber<uint16_t>("premdays");
	account.lastDay = result->getNumber<time_t>("lastday");

	query.clear();
	query << "SELECT `name`, `deletion` FROM `players` WHERE `account_id` = " << account.id;
	result = g_database.storeQuery(query);
	if (result) {
		account.characters.reserve(result->countResults());
		do {
			if (result->getNumber<uint64_t>("deletion") == 0) {
				account.characters.emplace_back(std::move(result->getString("name")));
			}
		} while (result->next());
		std::sort(account.characters.begin(), account.characters.end());
	}
	return true;
}


#if GAME_FEATURE_SESSIONKEY > 0
#if GAME_FEATURE_LOGIN_EMAIL > 0
uint32_t IOLoginData::gameworldAuthentication(const std::string& email, const std::string& password, std::string& characterName, std::string& token, uint32_t tokenTime)
#else
uint32_t IOLoginData::gameworldAuthentication(const std::string& accountName, const std::string& password, std::string& characterName, std::string& token, uint32_t tokenTime)
#endif
#else
uint32_t IOLoginData::gameworldAuthentication(const std::string& accountName, const std::string& password, std::string& characterName)
#endif
{
	#if GAME_FEATURE_LOGIN_EMAIL > 0
	const std::string& escapedEmail = g_database.escapeString(email);
	const std::string& escapedCharacterName = g_database.escapeString(characterName);
	std::stringExtended query(std::max<size_t>(escapedEmail.length(), escapedCharacterName.length()) + static_cast<size_t>(128));
	#else
	const std::string& escapedAccountName = g_database.escapeString(accountName);
	const std::string& escapedCharacterName = g_database.escapeString(characterName);
	std::stringExtended query(std::max<size_t>(escapedAccountName.length(), escapedCharacterName.length()) + static_cast<size_t>(128));
	#endif

	#if GAME_FEATURE_SESSIONKEY > 0
	#if GAME_FEATURE_LOGIN_EMAIL > 0
	query << "SELECT `id`, `password`, `secret` FROM `accounts` WHERE `email` = " << escapedEmail << " LIMIT 1";
	#else
	query << "SELECT `id`, `password`, `secret` FROM `accounts` WHERE `name` = " << escapedAccountName << " LIMIT 1";
	#endif
	#else
	query << "SELECT `id`, `password` FROM `accounts` WHERE `name` = " << escapedAccountName << " LIMIT 1";
	#endif
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return 0;
	}

	#if GAME_FEATURE_SESSIONKEY > 0
	std::string secret = decodeSecret(result->getString("secret"));
	if (!secret.empty()) {
		if (token.empty()) {
			return 0;
		}

		bool tokenValid = token == generateToken(secret, tokenTime) || token == generateToken(secret, tokenTime - 1) || token == generateToken(secret, tokenTime + 1);
		if (!tokenValid) {
			return 0;
		}
	}
	#endif

	if (transformToSHA1(password) != result->getString("password")) {
		return 0;
	}

	uint32_t accountId = result->getNumber<uint32_t>("id");

	query.clear();
	query << "SELECT `account_id`, `name`, `deletion` FROM `players` WHERE `name` = " << escapedCharacterName << " LIMIT 1";
	result = g_database.storeQuery(query);
	if (!result) {
		return 0;
	}

	if (result->getNumber<uint32_t>("account_id") != accountId || result->getNumber<uint64_t>("deletion") != 0) {
		return 0;
	}
	characterName = std::move(result->getString("name"));
	return accountId;
}

AccountType_t IOLoginData::getAccountType(uint32_t accountId)
{
	std::stringExtended query(64);
	query << "SELECT `type` FROM `accounts` WHERE `id` = " << accountId << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return ACCOUNT_TYPE_NORMAL;
	}
	return static_cast<AccountType_t>(result->getNumber<uint16_t>("type"));
}

void IOLoginData::setAccountType(uint32_t accountId, AccountType_t accountType)
{
	std::stringExtended query(128);
	query << "UPDATE `accounts` SET `type` = " << accountType << " WHERE `id` = " << accountId;
	g_database.executeQuery(query);
}

void IOLoginData::updateOnlineStatus(uint32_t guid, bool login)
{
	if (g_config.getBoolean(ConfigManager::ALLOW_CLONES)) {
		return;
	}

	std::stringExtended query(64);
	if (login) {
		query << "INSERT INTO `players_online` VALUES (" << guid << ')';
	} else {
		query << "DELETE FROM `players_online` WHERE `player_id` = " << guid;
	}
	g_database.executeQuery(query);
}

bool IOLoginData::preloadPlayer(Player* player, const std::string& name)
{
	const std::string& escapedName = g_database.escapeString(name);
	std::stringExtended query(escapedName.length() + static_cast<size_t>(280));

	query << "SELECT `id`, `account_id`, `group_id`, `deletion`, (SELECT `type` FROM `accounts` WHERE `accounts`.`id` = `account_id`) AS `account_type`";
	if (!g_config.getBoolean(ConfigManager::FREE_PREMIUM)) {
		query << ", (SELECT `premdays` FROM `accounts` WHERE `accounts`.`id` = `account_id`) AS `premium_days`";
	}
	query << " FROM `players` WHERE `name` = " << escapedName << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return false;
	}

	if (result->getNumber<uint64_t>("deletion") != 0) {
		return false;
	}

	player->setGUID(result->getNumber<uint32_t>("id"));
	Group* group = g_game.groups.getGroup(result->getNumber<uint16_t>("group_id"));
	if (!group) {
		std::cout << "[Error - IOLoginData::preloadPlayer] " << player->name << " has Group ID " << result->getNumber<uint16_t>("group_id") << " which doesn't exist." << std::endl;
		return false;
	}
	player->setGroup(group);
	player->accountNumber = result->getNumber<uint32_t>("account_id");
	player->accountType = static_cast<AccountType_t>(result->getNumber<uint16_t>("account_type"));
	if (!g_config.getBoolean(ConfigManager::FREE_PREMIUM)) {
		player->premiumDays = result->getNumber<uint16_t>("premium_days");
	} else {
		player->premiumDays = std::numeric_limits<uint16_t>::max();
	}

	query.clear();
	query << "SELECT `guild_id`, `rank_id`, `nick` FROM `guild_membership` WHERE `player_id` = " << player->getGUID() << " LIMIT 1";
	if ((result = g_database.storeQuery(query))) {
		uint32_t guildId = result->getNumber<uint32_t>("guild_id");
		uint32_t playerRankId = result->getNumber<uint32_t>("rank_id");
		player->guildNick = std::move(result->getString("nick"));

		Guild* guild = g_game.getGuild(guildId);
		if (!guild) {
			guild = IOGuild::loadGuild(guildId);
		}

		if (guild) {
			g_game.addGuild(guild);

			player->guild = guild;
			const GuildRank* rank = guild->getRankById(playerRankId);
			if (!rank) {
				query.clear();
				query << "SELECT `id`, `name`, `level` FROM `guild_ranks` WHERE `id` = " << playerRankId << " LIMIT 1";
				if ((result = g_database.storeQuery(query))) {
					guild->addRank(result->getNumber<uint32_t>("id"), result->getString("name"), result->getNumber<uint16_t>("level"));
				}

				rank = guild->getRankById(playerRankId);
				if (!rank) {
					player->guild = nullptr;
				}
			}

			player->guildRank = rank;

			IOGuild::getWarList(guildId, player->guildWarVector);

			query.clear();
			query << "SELECT COUNT(*) AS `members` FROM `guild_membership` WHERE `guild_id` = " << guildId << " LIMIT 1";
			if ((result = g_database.storeQuery(query))) {
				guild->setMemberCount(result->getNumber<uint32_t>("members"));
			}
		}
	}
	return true;
}

bool IOLoginData::loadPlayerById(Player* player, uint32_t id)
{
	std::stringExtended query(1024);
	query << "SELECT `id`, `name`, `account_id`, `group_id`, `sex`, `vocation`, `experience`, `level`, `maglevel`, `health`, `healthmax`, `blessings`, `mana`, `manamax`, `manaspent`, `soul`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `lookaddons`, `lookmountbody`, `lookmountfeet`, `lookmounthead`, `lookmountlegs`, `posx`, `posy`, `posz`, `cap`, `lastlogin`, `lastlogout`, `lastip`, `conditions`, `spells`, `storages`, `skulltime`, `skull`, `town_id`, `balance`, `offlinetraining_time`, `offlinetraining_skill`, `stamina`, `skill_fist`, `skill_fist_tries`, `skill_club`, `skill_club_tries`, `skill_sword`, `skill_sword_tries`, `skill_axe`, `skill_axe_tries`, `skill_dist`, `skill_dist_tries`, `skill_shielding`, `skill_shielding_tries`, `skill_fishing`, `skill_fishing_tries`, `direction` FROM `players` WHERE `id` = " << id << " LIMIT 1";
	return loadPlayer(player, g_database.storeQuery(query));
}

bool IOLoginData::loadPlayerByName(Player* player, const std::string& name)
{
	const std::string& escapedName = g_database.escapeString(name);
	std::stringExtended query(escapedName.length() + static_cast<size_t>(1024));
	query << "SELECT `id`, `name`, `account_id`, `group_id`, `sex`, `vocation`, `experience`, `level`, `maglevel`, `health`, `healthmax`, `blessings`, `mana`, `manamax`, `manaspent`, `soul`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `lookaddons`, `lookmountbody`, `lookmountfeet`, `lookmounthead`, `lookmountlegs`, `posx`, `posy`, `posz`, `cap`, `lastlogin`, `lastlogout`, `lastip`, `conditions`, `spells`, `storages`, `skulltime`, `skull`, `town_id`, `balance`, `offlinetraining_time`, `offlinetraining_skill`, `stamina`, `skill_fist`, `skill_fist_tries`, `skill_club`, `skill_club_tries`, `skill_sword`, `skill_sword_tries`, `skill_axe`, `skill_axe_tries`, `skill_dist`, `skill_dist_tries`, `skill_shielding`, `skill_shielding_tries`, `skill_fishing`, `skill_fishing_tries`, `direction` FROM `players` WHERE `name` = " << escapedName << " LIMIT 1";
	return loadPlayer(player, g_database.storeQuery(query));
}

bool IOLoginData::loadContainer(PropStream& propStream, Container* mainContainer)
{
	//Reserve a little space before to avoid massive reallocations
	std::vector<Container*> loadingContainers; loadingContainers.reserve(100);
	loadingContainers.push_back(mainContainer);
	while (!loadingContainers.empty()) {
		StartLoadingContainers:
		Container* container = loadingContainers.back();
		while (container->serializationCount > 0) {
			uint16_t id;
			if (!propStream.read<uint16_t>(id)) {
				std::cout << "[Warning - IOLoginData::loadContainer] Unserialization error for container item: " << container->getID() << std::endl;
				return false;
			}

			Item* item = Item::CreateItem(id);
			if (item) {
				if (item->unserializeAttr(propStream)) {
					Container* c = item->getContainer();
					if (c) {
						--container->serializationCount; // Since we're going out of loop decrease our iterator here
						loadingContainers.push_back(c);
						goto StartLoadingContainers;
					}
					container->internalAddThing(item);
				} else {
					delete item;
					return false;
				}
			}
			--container->serializationCount;
		}
		uint8_t endAttr;
		if (!propStream.read<uint8_t>(endAttr) || endAttr != 0) {
			std::cout << "[Warning - IOLoginData::loadContainer] Unserialization error for container item: " << container->getID() << std::endl;
			return false;
		}
		loadingContainers.pop_back();
		if (!loadingContainers.empty()) {
			loadingContainers.back()->internalAddThing(container);
		}
	}
	return true;
}

void IOLoginData::loadItems(ItemBlockList& itemMap, DBResult_ptr result, PropStream& propStream)
{
	int32_t pid;
	uint16_t id;
	while (propStream.read<int32_t>(pid) && propStream.read<uint16_t>(id)) {
		Item* item = Item::CreateItem(id);
		if (item) {
			if (item->unserializeAttr(propStream)) {
				Container* container = item->getContainer();
				if (container && !loadContainer(propStream, container)) {
					delete item;
					std::cout << "WARNING: Serialize error in IOLoginData::loadItems" << std::endl;
					return;
				}
				itemMap.emplace_back(pid, item);
			} else {
				delete item;
				std::cout << "WARNING: Serialize error in IOLoginData::loadItems" << std::endl;
				return;
			}
		}
	}
}

bool IOLoginData::loadPlayer(Player* player, DBResult_ptr result)
{
	if (!result) {
		return false;
	}

	uint32_t accno = result->getNumber<uint32_t>("account_id");
	Account acc = loadAccount(accno);

	player->setGUID(result->getNumber<uint32_t>("id"));
	player->name = std::move(result->getString("name"));
	player->accountNumber = accno;

	player->accountType = acc.accountType;
	if (g_config.getBoolean(ConfigManager::FREE_PREMIUM)) {
		player->premiumDays = std::numeric_limits<uint16_t>::max();
	} else {
		player->premiumDays = acc.premiumDays;
	}

	Group* group = g_game.groups.getGroup(result->getNumber<uint16_t>("group_id"));
	if (!group) {
		std::cout << "[Error - IOLoginData::loadPlayer] " << player->name << " has Group ID " << result->getNumber<uint16_t>("group_id") << " which doesn't exist" << std::endl;
		return false;
	}
	player->setGroup(group);

	player->bankBalance = result->getNumber<uint64_t>("balance");

	player->setSex(static_cast<PlayerSex_t>(result->getNumber<uint16_t>("sex")));
	player->level = std::max<uint32_t>(1, result->getNumber<uint32_t>("level"));

	uint64_t experience = result->getNumber<uint64_t>("experience");

	uint64_t currExpCount = Player::getExpForLevel(player->level);
	uint64_t nextExpCount = Player::getExpForLevel(player->level + 1);
	if (experience < currExpCount || experience > nextExpCount) {
		experience = currExpCount;
	}

	player->experience = experience;
	if (currExpCount < nextExpCount) {
		player->levelPercent = Player::getPercentLevel(player->experience - currExpCount, nextExpCount - currExpCount);
	} else {
		player->levelPercent = 0;
	}

	player->soul = result->getNumber<uint16_t>("soul");
	player->capacity = result->getNumber<uint32_t>("cap") * 100;
	player->blessings = result->getNumber<uint16_t>("blessings");

	unsigned long conditionsSize;
	const char* conditions = result->getStream("conditions", conditionsSize);
	PropStream propStream;
	propStream.init(conditions, conditionsSize);

	Condition* condition = Condition::createCondition(propStream);
	while (condition) {
		if (condition->unserialize(propStream)) {
			player->storedConditionList.push_back(condition);
		} else {
			delete condition;
		}
		condition = Condition::createCondition(propStream);
	}

	//load spells
	unsigned long attrSize;
	const char* attr = result->getStream("spells", attrSize);
	propStream.init(attr, attrSize);

	std::string spell;
	while (propStream.readString(spell)) {
		player->learnedInstantSpellList.emplace(spell);
	}

	//load storage map
	attr = result->getStream("storages", attrSize);
	propStream.init(attr, attrSize);

	size_t storage_sizes;
	if (propStream.read<size_t>(storage_sizes)) {
		player->storageMap.reserve(storage_sizes);

		uint32_t storage_key;
		int32_t storage_value;
		while (propStream.read<uint32_t>(storage_key) && propStream.read<int32_t>(storage_value)) {
			player->addStorageValue(storage_key, storage_value, true);
		}
	}

	if (!player->setVocation(result->getNumber<uint16_t>("vocation"), true)) {
		std::cout << "[Error - IOLoginData::loadPlayer] " << player->name << " has Vocation ID " << result->getNumber<uint16_t>("vocation") << " which doesn't exist" << std::endl;
		return false;
	}

	player->mana = result->getNumber<uint32_t>("mana");
	player->manaMax = result->getNumber<uint32_t>("manamax");
	player->magLevel = result->getNumber<uint32_t>("maglevel");

	uint64_t nextManaCount = player->vocation->getReqMana(player->magLevel + 1);
	uint64_t manaSpent = result->getNumber<uint64_t>("manaspent");
	if (manaSpent > nextManaCount) {
		manaSpent = 0;
	}

	player->manaSpent = manaSpent;
	player->magLevelPercent = Player::getPercentSkillLevel(player->manaSpent, nextManaCount);

	player->health = result->getNumber<int32_t>("health");
	player->healthMax = result->getNumber<int32_t>("healthmax");

	player->defaultOutfit.lookType = result->getNumber<uint16_t>("looktype");
	player->defaultOutfit.lookHead = result->getNumber<uint16_t>("lookhead");
	player->defaultOutfit.lookBody = result->getNumber<uint16_t>("lookbody");
	player->defaultOutfit.lookLegs = result->getNumber<uint16_t>("looklegs");
	player->defaultOutfit.lookFeet = result->getNumber<uint16_t>("lookfeet");
	player->defaultOutfit.lookAddons = result->getNumber<uint16_t>("lookaddons");
	#if GAME_FEATURE_MOUNT_COLORS > 0
	player->defaultOutfit.lookMountHead = result->getNumber<uint16_t>("lookmounthead");
	player->defaultOutfit.lookMountBody = result->getNumber<uint16_t>("lookmountbody");
	player->defaultOutfit.lookMountLegs = result->getNumber<uint16_t>("lookmountlegs");
	player->defaultOutfit.lookMountFeet = result->getNumber<uint16_t>("lookmountfeet");
	#endif
	player->currentOutfit = player->defaultOutfit;
	player->direction = static_cast<Direction> (result->getNumber<uint16_t>("direction"));
	if (g_game.getWorldType() != WORLD_TYPE_PVP_ENFORCED) {
		const time_t skullSeconds = result->getNumber<time_t>("skulltime") - time(nullptr);
		if (skullSeconds > 0) {
			//ensure that we round up the number of ticks
			player->skullTicks = (skullSeconds + 2);

			uint16_t skull = result->getNumber<uint16_t>("skull");
			if (skull == SKULL_RED) {
				player->skull = SKULL_RED;
			} else if (skull == SKULL_BLACK) {
				player->skull = SKULL_BLACK;
			}
		}
	}

	player->loginPosition.x = result->getNumber<uint16_t>("posx");
	player->loginPosition.y = result->getNumber<uint16_t>("posy");
	player->loginPosition.z = result->getNumber<uint16_t>("posz");

	player->lastLoginSaved = result->getNumber<time_t>("lastlogin");
	player->lastLogout = result->getNumber<time_t>("lastlogout");

	player->offlineTrainingTime = result->getNumber<int32_t>("offlinetraining_time") * 1000;
	player->offlineTrainingSkill = result->getNumber<int32_t>("offlinetraining_skill");

	Town* town = g_game.map.towns.getTown(result->getNumber<uint32_t>("town_id"));
	if (!town) {
		std::cout << "[Error - IOLoginData::loadPlayer] " << player->name << " has Town ID " << result->getNumber<uint32_t>("town_id") << " which doesn't exist" << std::endl;
		return false;
	}

	player->town = town;

	const Position& loginPos = player->loginPosition;
	if (loginPos.x == 0 && loginPos.y == 0 && loginPos.z == 0) {
		player->loginPosition = player->getTemplePosition();
	}

	player->staminaMinutes = result->getNumber<uint16_t>("stamina");

	static const std::string skillNames[] = {"skill_fist", "skill_club", "skill_sword", "skill_axe", "skill_dist", "skill_shielding", "skill_fishing"};
	static const std::string skillNameTries[] = {"skill_fist_tries", "skill_club_tries", "skill_sword_tries", "skill_axe_tries", "skill_dist_tries", "skill_shielding_tries", "skill_fishing_tries"};
	static constexpr size_t size = sizeof(skillNames) / sizeof(std::string);
	for (uint8_t i = 0; i < size; ++i) {
		uint16_t skillLevel = result->getNumber<uint16_t>(skillNames[i]);
		uint64_t skillTries = result->getNumber<uint64_t>(skillNameTries[i]);
		uint64_t nextSkillTries = player->vocation->getReqSkillTries(i, skillLevel + 1);
		if (skillTries > nextSkillTries) {
			skillTries = 0;
		}

		player->skills[i].level = skillLevel;
		player->skills[i].tries = skillTries;
		player->skills[i].percent = Player::getPercentSkillLevel(skillTries, nextSkillTries);
	}

	//load inventory items
	ItemBlockList itemMap;

	std::stringExtended query(128);
	query << "SELECT `items` FROM `players` WHERE `id` = " << player->getGUID() << " LIMIT 1";
	if ((result = g_database.storeQuery(query))) {
		attr = result->getStream("items", attrSize);
		propStream.init(attr, attrSize);
		loadItems(itemMap, result, propStream);
		for (const auto& it : itemMap) {
			Item* item = it.second;
			uint32_t pid = static_cast<uint32_t>(it.first);
			#if GAME_FEATURE_STORE_INBOX > 0 || GAME_FEATURE_PURSE_SLOT > 0
			if (pid >= 1 && pid <= 11) {
			#else
			if (pid >= 1 && pid <= 10) {
			#endif
				player->internalAddThing(pid, item);
				item->startDecaying();
			}
		}
	}

	#if GAME_FEATURE_STORE_INBOX > 0
	if (!player->inventory[CONST_SLOT_STORE_INBOX]) {
		player->internalAddThing(CONST_SLOT_STORE_INBOX, Item::CreateItem(ITEM_STORE_INBOX));
	}
	#elif GAME_FEATURE_PURSE_SLOT > 0
	if (!player->inventory[CONST_SLOT_PURSE]) {
		player->internalAddThing(CONST_SLOT_PURSE, Item::CreateItem(ITEM_PURSE));
	}
	#endif

	//load depot locker items
	itemMap.clear();

	query.clear();
	query << "SELECT `depotlockeritems` FROM `players` WHERE `id` = " << player->getGUID() << " LIMIT 1";
	if ((result = g_database.storeQuery(query))) {
		attr = result->getStream("depotlockeritems", attrSize);
		propStream.init(attr, attrSize);
		loadItems(itemMap, result, propStream);
		for (const auto& it : itemMap) {
			Item* item = it.second;
			uint32_t pid = static_cast<uint32_t>(it.first);
			if (pid >= 0 && pid < 100) {
				DepotLocker* depotLocker = player->getDepotLocker(pid);
				if (depotLocker) {
					depotLocker->internalAddThing(item);
					item->startDecaying();
				} else {
					std::cout << "[Error - IOLoginData::loadPlayer " << item->getID() << "] Cannot load depot locker " << pid << " for player " << player->name << std::endl;
				}
			}
		}
	}

	//load depot items
	itemMap.clear();

	query.clear();
	query << "SELECT `depotitems` FROM `players` WHERE `id` = " << player->getGUID() << " LIMIT 1";
	if ((result = g_database.storeQuery(query))) {
		attr = result->getStream("depotitems", attrSize);
		propStream.init(attr, attrSize);
		loadItems(itemMap, result, propStream);
		for (const auto& it : itemMap) {
			Item* item = it.second;
			uint32_t pid = static_cast<uint32_t>(it.first);
			if (pid >= 0 && pid < 100) {
				DepotChest* depotChest = player->getDepotChest(pid, true);
				if (depotChest) {
					depotChest->internalAddThing(item);
					item->startDecaying();
				} else {
					std::cout << "[Error - IOLoginData::loadPlayer " << item->getID() << "] Cannot load depot " << pid << " for player " << player->name << std::endl;
				}
			}
		}
	}

	#if GAME_FEATURE_MARKET > 0
	//load inbox items
	itemMap.clear();

	query.clear();
	query << "SELECT `inboxitems` FROM `players` WHERE `id` = " << player->getGUID() << " LIMIT 1";
	if ((result = g_database.storeQuery(query))) {
		attr = result->getStream("inboxitems", attrSize);
		propStream.init(attr, attrSize);
		loadItems(itemMap, result, propStream);
		for (const auto& it : itemMap) {
			Item* item = it.second;
			player->getInbox()->internalAddThing(item);
			item->startDecaying();
		}
	}
	#endif

	#if GAME_FEATURE_STASH > 0
	//load stash items
	query.clear();
	query << "SELECT `supplystash` FROM `players` WHERE `id` = " << player->getGUID() << " LIMIT 1";
	if ((result = g_database.storeQuery(query))) {
		attr = result->getStream("supplystash", attrSize);
		propStream.init(attr, attrSize);

		uint16_t itemId;
		uint32_t itemCount;
		while (propStream.read<uint16_t>(itemId) && propStream.read<uint32_t>(itemCount)) {
			player->stashItems[itemId] = itemCount;
		}
	}
	#endif

	//load vip
	query.clear();
	query << "SELECT `player_id` FROM `account_viplist` WHERE `account_id` = " << player->getAccount();
	if ((result = g_database.storeQuery(query))) {
		do {
			player->addVIPInternal(result->getNumber<uint32_t>("player_id"));
		} while (result->next());
	}

	player->updateBaseSpeed();
	player->updateInventoryWeight();
	player->updateItemsLight(true);
	return true;
}

void IOLoginData::saveItem(PropWriteStream& stream, const Item* item)
{
	const Container* container = item->getContainer();
	if (!container) {
		// Write ID & props
		stream.write<uint16_t>(item->getID());
		item->serializeAttr(stream);
		stream.write<uint8_t>(0x00); // attr end
		return;
	}

	// Write ID & props
	stream.write<uint16_t>(item->getID());
	item->serializeAttr(stream);

	// Hack our way into the attributes
	stream.write<uint8_t>(ATTR_CONTAINER_ITEMS);
	stream.write<uint32_t>(container->size());

	//Reserve a little space before to avoid massive reallocations
	std::vector<std::pair<const Container*, ItemDeque::const_reverse_iterator>> savingContainers; savingContainers.reserve(100);
	savingContainers.emplace_back(container, container->getReversedItems());
	while (!savingContainers.empty()) {
		StartSavingContainers:
		container = savingContainers.back().first;
		ItemDeque::const_reverse_iterator& it = savingContainers.back().second;
		for (auto end = container->getReversedEnd(); it != end; ++it) {
			item = (*it);
			container = item->getContainer();
			if (!container) {
				// Write ID & props
				stream.write<uint16_t>(item->getID());
				item->serializeAttr(stream);
				stream.write<uint8_t>(0x00); // attr end
			} else {
				// Write ID & props
				stream.write<uint16_t>(item->getID());
				item->serializeAttr(stream);

				// Hack our way into the attributes
				stream.write<uint8_t>(ATTR_CONTAINER_ITEMS);
				stream.write<uint32_t>(container->size());

				++it; // Since we're going out of loop increase our iterator here
				savingContainers.emplace_back(container, container->getReversedItems());
				goto StartSavingContainers;
			}
		}
		stream.write<uint8_t>(0x00); // attr end
		savingContainers.pop_back();
	}
}

bool IOLoginData::saveItems(const Player* player, const ItemBlockList& itemList, std::stringExtended& query, PropWriteStream& propWriteStream, const std::string& table)
{
	for (const auto& it : itemList) {
		int32_t pid = it.first;
		Item* item = it.second;

		propWriteStream.write<int32_t>(pid);
		saveItem(propWriteStream, item);
	}

	size_t attributesSize;
	const char* attributes = propWriteStream.getStream(attributesSize);
	if (attributesSize > 0) {
		query << "UPDATE `players` SET `" << table << "` = " << g_database.escapeBlob(attributes, attributesSize) << " WHERE `id` = " << player->getGUID();
		if (!g_database.executeQuery(query)) {
			return false;
		}
	} else {
		query << "UPDATE `players` SET `" << table << "` = NULL WHERE `id` = " << player->getGUID();
		if (!g_database.executeQuery(query)) {
			return false;
		}
	}
	return true;
}

bool IOLoginData::savePlayer(Player* player)
{
	if (player->getHealth() <= 0) {
		player->changeHealth(1);
	}

	std::stringExtended query(2048);
	query << "SELECT `save` FROM `players` WHERE `id` = " << player->getGUID() << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return false;
	}

	if (result->getNumber<uint16_t>("save") == 0) {
		query.clear();
		query << "UPDATE `players` SET `lastlogin` = " << player->lastLoginSaved << ", `lastip` = " << player->lastIP << " WHERE `id` = " << player->getGUID();
		return g_database.executeQuery(query);
	}

	//First, an UPDATE query to write the player itself
	query.clear();
	query << "UPDATE `players` SET ";
	query << "`level` = " << player->level;
	query << ",`group_id` = " << player->group->id;
	query << ",`vocation` = " << player->getVocationId();
	query << ",`health` = " << player->health;
	query << ",`healthmax` = " << player->healthMax;
	query << ",`experience` = " << player->experience;
	query << ",`lookbody` = " << player->defaultOutfit.lookBody;
	query << ",`lookfeet` = " << player->defaultOutfit.lookFeet;
	query << ",`lookhead` = " << player->defaultOutfit.lookHead;
	query << ",`looklegs` = " << player->defaultOutfit.lookLegs;
	query << ",`looktype` = " << player->defaultOutfit.lookType;
	query << ",`lookaddons` = " << player->defaultOutfit.lookAddons;
	#if GAME_FEATURE_MOUNT_COLORS > 0
	query << ",`lookmountbody` = " << player->defaultOutfit.lookMountBody;
	query << ",`lookmountfeet` = " << player->defaultOutfit.lookMountFeet;
	query << ",`lookmounthead` = " << player->defaultOutfit.lookMountHead;
	query << ",`lookmountlegs` = " << player->defaultOutfit.lookMountLegs;
	#endif
	query << ",`maglevel` = " << player->magLevel;
	query << ",`mana` = " << player->mana;
	query << ",`manamax` = " << player->manaMax;
	query << ",`manaspent` = " << player->manaSpent;
	query << ",`soul` = " << player->soul;
	query << ",`town_id` = " << player->town->getID();

	const Position& loginPosition = player->getLoginPosition();
	query << ",`posx` = " << loginPosition.getX();
	query << ",`posy` = " << loginPosition.getY();
	query << ",`posz` = " << loginPosition.getZ();

	query << ",`cap` = " << (player->capacity / 100);
	query << ",`sex` = " << player->sex;
	if (player->lastLoginSaved != 0) {
		query << ",`lastlogin` = " << player->lastLoginSaved;
	}

	if (player->lastIP != 0) {
		query << ",`lastip` = " << player->lastIP;
	}

	//serialize conditions
	PropWriteStream propWriteStream;
	for (Condition* condition : player->conditions) {
		if (condition->isPersistent()) {
			condition->serialize(propWriteStream);
			propWriteStream.write<uint8_t>(CONDITIONATTR_END);
		}
	}

	size_t attributesSize;
	const char* attributes = propWriteStream.getStream(attributesSize);

	query << ",`conditions` = " << g_database.escapeBlob(attributes, attributesSize);

	// learned spells
	propWriteStream.clear();
	for (const auto& learnedSpell : player->learnedInstantSpellList) {
		propWriteStream.writeString(learnedSpell);
	}

	attributes = propWriteStream.getStream(attributesSize);
	if (attributesSize > 0) {
		query << ",`spells` = " << g_database.escapeBlob(attributes, attributesSize);
	} else {
		query << ",`spells` = NULL";
	}

	// storages
	player->genReservedStorageRange();
	propWriteStream.clear();
	propWriteStream.write<size_t>(player->storageMap.size());
	for (const auto& it : player->storageMap) {
		propWriteStream.write<uint32_t>(it.first);
		propWriteStream.write<int32_t>(it.second);
	}

	attributes = propWriteStream.getStream(attributesSize);
	if (attributesSize > 0) {
		query << ",`storages` = " << g_database.escapeBlob(attributes, attributesSize);
	} else {
		query << ",`storages` = NULL";
	}

	if (g_game.getWorldType() != WORLD_TYPE_PVP_ENFORCED) {
		int64_t skullTime = 0;
		if (player->skullTicks > 0) {
			skullTime = time(nullptr) + player->skullTicks;
		}
		query << ",`skulltime` = " << skullTime;

		Skulls_t skull = SKULL_NONE;
		if (player->skull == SKULL_RED || player->skull == SKULL_BLACK) {
			skull = player->skull;
		}
		query << ",`skull` = " << skull;
	}

	query << ",`lastlogout` = " << player->getLastLogout();
	query << ",`balance` = " << player->bankBalance;
	query << ",`offlinetraining_time` = " << (player->getOfflineTrainingTime() / 1000);
	query << ",`offlinetraining_skill` = " << player->getOfflineTrainingSkill();
	query << ",`stamina` = " << player->getStaminaMinutes();

	query << ",`skill_fist` = " << player->skills[SKILL_FIST].level;
	query << ",`skill_fist_tries` = " << player->skills[SKILL_FIST].tries;
	query << ",`skill_club` = " << player->skills[SKILL_CLUB].level;
	query << ",`skill_club_tries` = " << player->skills[SKILL_CLUB].tries;
	query << ",`skill_sword` = " << player->skills[SKILL_SWORD].level;
	query << ",`skill_sword_tries` = " << player->skills[SKILL_SWORD].tries;
	query << ",`skill_axe` = " << player->skills[SKILL_AXE].level;
	query << ",`skill_axe_tries` = " << player->skills[SKILL_AXE].tries;
	query << ",`skill_dist` = " << player->skills[SKILL_DISTANCE].level;
	query << ",`skill_dist_tries` = " << player->skills[SKILL_DISTANCE].tries;
	query << ",`skill_shielding` = " << player->skills[SKILL_SHIELD].level;
	query << ",`skill_shielding_tries` = " << player->skills[SKILL_SHIELD].tries;
	query << ",`skill_fishing` = " << player->skills[SKILL_FISHING].level;
	query << ",`skill_fishing_tries` = " << player->skills[SKILL_FISHING].tries;
	query << ",`direction` = " << player->getDirection();
	if (!player->isOffline()) {
		query << ",`onlinetime` = `onlinetime` + " << (time(nullptr) - player->lastLoginSaved);
	}
	query << ",`blessings` = " << player->blessings;
	query << " WHERE `id` = " << player->getGUID();

	DBTransaction transaction(&g_database);
	if (!transaction.begin()) {
		return false;
	}

	if (!g_database.executeQuery(query)) {
		return false;
	}

	//item saving
	query.clear();
	ItemBlockList itemList;
	#if GAME_FEATURE_STORE_INBOX > 0 || GAME_FEATURE_PURSE_SLOT > 0
	for (int32_t slotId = 1; slotId <= 11; ++slotId) {
	#else
	for (int32_t slotId = 1; slotId <= 10; ++slotId) {
	#endif
		Item* item = player->inventory[slotId];
		if (item) {
			itemList.emplace_back(slotId, item);
		}
	}

	propWriteStream.clear();
	if (!saveItems(player, itemList, query, propWriteStream, "items")) {
		return false;
	}

	if (player->lastDepotId != -1) {
		//save depot lockers
		query.clear();
		itemList.clear();
		for (const auto& it : player->depotLockerMap) {
			DepotLocker* depotLocker = it.second;
			for (auto item = depotLocker->getReversedItems(), end = depotLocker->getReversedEnd(); item != end; ++item) {
				uint16_t itemId = (*item)->getID();
				#if GAME_FEATURE_STASH > 0
				if (itemId == ITEM_DEPOT || itemId == ITEM_INBOX || itemId == ITEM_MARKET || itemId == ITEM_SUPPLY_STASH) {
				#elif GAME_FEATURE_MARKET > 0
				if (itemId == ITEM_DEPOT || itemId == ITEM_INBOX || itemId == ITEM_MARKET) {
				#else
				if (itemId == ITEM_DEPOT) {
				#endif
					continue;
				}
				itemList.emplace_back(static_cast<int32_t>(it.first), *item);
			}
		}

		propWriteStream.clear();
		if (!saveItems(player, itemList, query, propWriteStream, "depotlockeritems")) {
			return false;
		}

		//save depot items
		query.clear();
		itemList.clear();
		for (const auto& it : player->depotChests) {
			DepotChest* depotChest = it.second;
			for (auto item = depotChest->getReversedItems(), end = depotChest->getReversedEnd(); item != end; ++item) {
				itemList.emplace_back(static_cast<int32_t>(it.first), *item);
			}
		}

		propWriteStream.clear();
		if (!saveItems(player, itemList, query, propWriteStream, "depotitems")) {
			return false;
		}
	}

	#if GAME_FEATURE_MARKET > 0
	//save inbox items
	query.clear();
	itemList.clear();
	for (auto item = player->getInbox()->getReversedItems(), end = player->getInbox()->getReversedEnd(); item != end; ++item) {
		itemList.emplace_back(0, *item);
	}

	propWriteStream.clear();
	if (!saveItems(player, itemList, query, propWriteStream, "inboxitems")) {
		return false;
	}
	#endif

	#if GAME_FEATURE_STASH > 0
	//save stash items
	propWriteStream.clear();
	for (const auto& it : player->stashItems) {
		propWriteStream.write<uint16_t>(it.first);
		propWriteStream.write<uint32_t>(it.second);
	}

	attributes = propWriteStream.getStream(attributesSize);
	query.clear();
	if (attributesSize > 0) {
		query << "UPDATE `players` SET `supplystash` = " << g_database.escapeBlob(attributes, attributesSize);
	} else {
		query << "UPDATE `players` SET `supplystash` = NULL";
	}
	query << " WHERE `id` = " << player->getGUID();

	if (!g_database.executeQuery(query)) {
		return false;
	}
	#endif

	//End the transaction
	return transaction.commit();
}

std::string IOLoginData::getNameByGuid(uint32_t guid)
{
	std::stringExtended query(64);
	query << "SELECT `name` FROM `players` WHERE `id` = " << guid << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return std::string();
	}
	return result->getString("name");
}

uint32_t IOLoginData::getGuidByName(const std::string& name)
{
	const std::string& escapedName = g_database.escapeString(name);
	std::stringExtended query(escapedName.length() + static_cast<size_t>(64));
	query << "SELECT `id` FROM `players` WHERE `name` = " << escapedName << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return 0;
	}
	return result->getNumber<uint32_t>("id");
}

bool IOLoginData::getGuidByNameEx(uint32_t& guid, bool& specialVip, std::string& name)
{
	const std::string& escapedName = g_database.escapeString(name);
	std::stringExtended query(escapedName.length() + static_cast<size_t>(128));
	query << "SELECT `name`, `id`, `group_id`, `account_id` FROM `players` WHERE `name` = " << escapedName << " LIMIT 1";
	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return false;
	}

	name = std::move(result->getString("name"));
	guid = result->getNumber<uint32_t>("id");
	Group* group = g_game.groups.getGroup(result->getNumber<uint16_t>("group_id"));

	uint64_t flags;
	if (group) {
		flags = group->flags;
	} else {
		flags = 0;
	}

	specialVip = (flags & PlayerFlag_SpecialVIP) != 0;
	return true;
}

bool IOLoginData::formatPlayerName(std::string& name)
{
	const std::string& escapedName = g_database.escapeString(name);
	std::stringExtended query(escapedName.length() + static_cast<size_t>(64));
	query << "SELECT `name` FROM `players` WHERE `name` = " << escapedName << " LIMIT 1";

	DBResult_ptr result = g_database.storeQuery(query);
	if (!result) {
		return false;
	}

	name = std::move(result->getString("name"));
	return true;
}

void IOLoginData::increaseBankBalance(uint32_t guid, uint64_t bankBalance)
{
	std::stringExtended query(128);
	query << "UPDATE `players` SET `balance` = `balance` + " << bankBalance << " WHERE `id` = " << guid;
	g_database.executeQuery(query);
}

bool IOLoginData::hasBiddedOnHouse(uint32_t guid)
{
	std::stringExtended query(128);
	query << "SELECT `id` FROM `houses` WHERE `highest_bidder` = " << guid << " LIMIT 1";
	return g_database.storeQuery(query).get() != nullptr;
}

void IOLoginData::addVIPEntry(uint32_t accountId, uint32_t guid, const std::string& description, uint32_t icon, bool notify)
{
	const std::string& escapedDescription = g_database.escapeString(description);
	std::stringExtended query(escapedDescription.length() + static_cast<size_t>(256));
	query << "INSERT IGNORE INTO `account_viplist` (`account_id`, `player_id`, `description`, `icon`, `notify`) VALUES (" << accountId << ',' << guid << ',';
	query << escapedDescription << ',' << icon << ',' << (notify ? "1" : "0") << ')';
	g_databaseTasks.addTask(std::move(static_cast<std::string&>(query)));
}

void IOLoginData::editVIPEntry(uint32_t accountId, uint32_t guid, const std::string& description, uint32_t icon, bool notify)
{
	const std::string& escapedDescription = g_database.escapeString(description);
	std::stringExtended query(escapedDescription.length() + static_cast<size_t>(256));
	query << "UPDATE `account_viplist` SET `description` = " << escapedDescription << ", `icon` = " << icon << ", `notify` = " << (notify ? "1" : "0");
	query << " WHERE `account_id` = " << accountId << " AND `player_id` = " << guid;
	g_databaseTasks.addTask(std::move(static_cast<std::string&>(query)));
}

void IOLoginData::removeVIPEntry(uint32_t accountId, uint32_t guid)
{
	std::stringExtended query(128);
	query << "DELETE FROM `account_viplist` WHERE `account_id` = " << accountId << " AND `player_id` = " << guid;
	g_databaseTasks.addTask(std::move(static_cast<std::string&>(query)));
}

void IOLoginData::addPremiumDays(uint32_t accountId, int32_t addDays)
{
	std::stringExtended query(128);
	query << "UPDATE `accounts` SET `premdays` = `premdays` + " << addDays << " WHERE `id` = " << accountId;
	g_database.executeQuery(query);
}

void IOLoginData::removePremiumDays(uint32_t accountId, int32_t removeDays)
{
	std::stringExtended query(128);
	query << "UPDATE `accounts` SET `premdays` = `premdays` - " << removeDays << " WHERE `id` = " << accountId;
	g_database.executeQuery(query);
}
