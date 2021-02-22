TRUE = true
FALSE = false

result.getDataInt = result.getNumber
result.getDataLong = result.getNumber
result.getDataString = result.getString
result.getDataStream = result.getStream

LUA_ERROR = false
LUA_NO_ERROR = true
LUA_NULL = nil

STACKPOS_GROUND = 0
STACKPOS_FIRST_ITEM_ABOVE_GROUNDTILE = 1
STACKPOS_SECOND_ITEM_ABOVE_GROUNDTILE = 2
STACKPOS_THIRD_ITEM_ABOVE_GROUNDTILE = 3
STACKPOS_FOURTH_ITEM_ABOVE_GROUNDTILE = 4
STACKPOS_FIFTH_ITEM_ABOVE_GROUNDTILE = 5
STACKPOS_TOP_CREATURE = 253
STACKPOS_TOP_FIELD = 254
STACKPOS_TOP_MOVEABLE_ITEM_OR_CREATURE = 255

THING_TYPE_PLAYER = CREATURETYPE_PLAYER + 1
THING_TYPE_MONSTER = CREATURETYPE_MONSTER + 1
THING_TYPE_NPC = CREATURETYPE_NPC + 1

COMBAT_POISONDAMAGE = COMBAT_EARTHDAMAGE
CONDITION_EXHAUST = CONDITION_EXHAUST_WEAPON
TALKTYPE_ORANGE_1 = TALKTYPE_MONSTER_SAY
TALKTYPE_ORANGE_2 = TALKTYPE_MONSTER_YELL

NORTH = DIRECTION_NORTH
EAST = DIRECTION_EAST
SOUTH = DIRECTION_SOUTH
WEST = DIRECTION_WEST
SOUTHWEST = DIRECTION_SOUTHWEST
SOUTHEAST = DIRECTION_SOUTHEAST
NORTHWEST = DIRECTION_NORTHWEST
NORTHEAST = DIRECTION_NORTHEAST

do
	local function CreatureIndex(self, key)
		local methods = getmetatable(self)
		if key == "uid" then
			return methods.getId(self)
		elseif key == "type" then
			local creatureType = 0
			if methods.isPlayer(self) then
				creatureType = THING_TYPE_PLAYER
			elseif methods.isMonster(self) then
				creatureType = THING_TYPE_MONSTER
			elseif methods.isNpc(self) then
				creatureType = THING_TYPE_NPC
			end
			return creatureType
		elseif key == "itemid" then
			return 1
		elseif key == "actionid" then
			return 0
		end
		return methods[key]
	end
	rawgetmetatable("Player").__index = CreatureIndex
	rawgetmetatable("Monster").__index = CreatureIndex
	rawgetmetatable("Npc").__index = CreatureIndex
end

do
	local function ItemIndex(self, key)
		local methods = getmetatable(self)
		if key == "itemid" then
			return methods.getId(self)
		elseif key == "actionid" then
			return methods.getActionId(self)
		elseif key == "uid" then
			return methods.getUniqueId(self)
		elseif key == "type" then
			return methods.getSubType(self)
		end
		return methods[key]
	end
	rawgetmetatable("Item").__index = ItemIndex
	rawgetmetatable("Container").__index = ItemIndex
	rawgetmetatable("Teleport").__index = ItemIndex
end

do
	local function ActionNewIndex(self, key, value)
		if key == "onUse" then
			self:onUse(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("Action").__newindex = ActionNewIndex
end

do
	local function TalkActionNewIndex(self, key, value)
		if key == "onSay" then
			self:onSay(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("TalkAction").__newindex = TalkActionNewIndex
end

do
	local function CreatureEventNewIndex(self, key, value)
		if key == "onLogin" then
			self:type("login")
			self:onLogin(value)
			return
		elseif key == "onLogout" then
			self:type("logout")
			self:onLogout(value)
			return
		elseif key == "onThink" then
			self:type("think")
			self:onThink(value)
			return
		elseif key == "onPrepareDeath" then
			self:type("preparedeath")
			self:onPrepareDeath(value)
			return
		elseif key == "onDeath" then
			self:type("death")
			self:onDeath(value)
			return
		elseif key == "onKill" then
			self:type("kill")
			self:onKill(value)
			return
		elseif key == "onAdvance" then
			self:type("advance")
			self:onAdvance(value)
			return
		elseif key == "onModalWindow" then
			self:type("modalwindow")
			self:onModalWindow(value)
			return
		elseif key == "onTextEdit" then
			self:type("textedit")
			self:onTextEdit(value)
			return
		elseif key == "onHealthChange" then
			self:type("healthchange")
			self:onHealthChange(value)
			return
		elseif key == "onManaChange" then
			self:type("manachange")
			self:onManaChange(value)
			return
		elseif key == "onExtendedOpcode" then
			self:type("extendedopcode")
			self:onExtendedOpcode(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("CreatureEvent").__newindex = CreatureEventNewIndex
end

do
	local function MoveEventNewIndex(self, key, value)
		if key == "onEquip" then
			self:type("equip")
			self:onEquip(value)
			return
		elseif key == "onDeEquip" then
			self:type("deequip")
			self:onDeEquip(value)
			return
		elseif key == "onAddItem" then
			self:type("additem")
			self:onAddItem(value)
			return
		elseif key == "onRemoveItem" then
			self:type("removeitem")
			self:onRemoveItem(value)
			return
		elseif key == "onStepIn" then
			self:type("stepin")
			self:onStepIn(value)
			return
		elseif key == "onStepOut" then
			self:type("stepout")
			self:onStepOut(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("MoveEvent").__newindex = MoveEventNewIndex
end

do
	local function GlobalEventNewIndex(self, key, value)
		if key == "onThink" then
			self:onThink(value)
			return
		elseif key == "onTime" then
			self:onTime(value)
			return
		elseif key == "onStartup" then
			self:type("startup")
			self:onStartup(value)
			return
		elseif key == "onShutdown" then
			self:type("shutdown")
			self:onShutdown(value)
			return
		elseif key == "onRecord" then
			self:type("record")
			self:onRecord(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("GlobalEvent").__newindex = GlobalEventNewIndex
end

do
	local function WeaponNewIndex(self, key, value)
		if key == "onUseWeapon" then
			self:onUseWeapon(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("Weapon").__newindex = WeaponNewIndex
end

do
	local function SpellNewIndex(self, key, value)
		if key == "onCastSpell" then
			self:onCastSpell(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("Spell").__newindex = SpellNewIndex
end

do
	local function MonsterTypeNewIndex(self, key, value)
		if key == "onThink" then
			self:eventType(MONSTERS_EVENT_THINK)
			self:onThink(value)
			return
		elseif key == "onAppear" then
			self:eventType(MONSTERS_EVENT_APPEAR)
			self:onAppear(value)
			return
		elseif key == "onDisappear" then
			self:eventType(MONSTERS_EVENT_DISAPPEAR)
			self:onDisappear(value)
			return
		elseif key == "onMove" then
			self:eventType(MONSTERS_EVENT_MOVE)
			self:onMove(value)
			return
		elseif key == "onSay" then
			self:eventType(MONSTERS_EVENT_SAY)
			self:onSay(value)
			return
		end
		rawset(self, key, value)
	end
	rawgetmetatable("MonsterType").__newindex = MonsterTypeNewIndex
end

function pushThing(thing)
	local t = {uid = 0, itemid = 0, type = 0, actionid = 0}
	if thing then
		if thing:isItem() then
			t.uid = thing:getUniqueId()
			t.itemid = thing:getId()
			if ItemType(t.itemid):hasSubType() then
				t.type = thing:getSubType()
			end
			t.actionid = thing:getActionId()
		elseif thing:isCreature() then
			t.uid = thing:getId()
			t.itemid = 1
			if thing:isPlayer() then
				t.type = THING_TYPE_PLAYER
			elseif thing:isMonster() then
				t.type = THING_TYPE_MONSTER
			else
				t.type = THING_TYPE_NPC
			end
		end
	end
	return t
end

createCombatObject = Combat
addCombatCondition = Combat.addCondition
setCombatArea = Combat.setArea
setCombatCallback = Combat.setCallback
setCombatFormula = Combat.setFormula
setCombatParam = Combat.setParameter

Combat.setCondition = function(...)
	Combat.addCondition(...)
end

setCombatCondition = function(...)
	Combat.addCondition(...)
end

function doTargetCombatHealth(...) return doTargetCombat(...) end
function doAreaCombatHealth(...) return doAreaCombat(...) end
function doTargetCombatMana(cid, target, min, max, effect) return doTargetCombat(cid, target, COMBAT_MANADRAIN, min, max, effect) end
function doAreaCombatMana(cid, pos, area, min, max, effect) return doAreaCombat(cid, COMBAT_MANADRAIN, pos, area, min, max, effect) end

createConditionObject = Condition
setConditionParam = Condition.setParameter
setConditionFormula = Condition.setFormula
addDamageCondition = Condition.addDamage
addOutfitCondition = Condition.setOutfit

doCombatAreaHealth = doAreaCombatHealth
doCombatAreaMana = doAreaCombatMana
doCombatAreaCondition = doAreaCombatCondition
doCombatAreaDispel = doAreaCombatDispel

function doCombat(cid, combat, var) return combat:execute(cid, var) end

function isCreature(cid) return Creature(cid) end
function isPlayer(cid) return Player(cid) end
function isMonster(cid) return Monster(cid) end
function isSummon(cid) local c = Creature(cid) return c and c:getMaster() end
function isNpc(cid) return Npc(cid) end
function isItem(uid) return Item(uid) end
function isContainer(uid) return Container(uid) end

function getCreatureName(cid) local c = Creature(cid) return c and c:getName() or false end
function getCreatureHealth(cid) local c = Creature(cid) return c and c:getHealth() or false end
function getCreatureMaxHealth(cid, ignoreModifiers) local c = Creature(cid) return c and (ignoreModifiers and c:getBaseMaxHealth() or c:getMaxHealth()) or false end
function getCreaturePosition(cid) local c = Creature(cid) return c and c:getPosition() or false end
getCreaturePos = getCreaturePosition
creatureGetPosition = getCreaturePosition
function getCreatureOutfit(cid) local c = Creature(cid) return c and c:getOutfit() or false end
function getCreatureSpeed(cid) local c = Creature(cid) return c and c:getSpeed() or false end
function getCreatureBaseSpeed(cid) local c = Creature(cid) return c and c:getBaseSpeed() or false end
function getCreatureLookDirection(cid) local c = Creature(cid) return c and c:getDirection() or false end
getCreatureLookDir = getCreatureLookDirection
function getCreatureByName(name) local c = Creature(name) return c and c:getId() or false end
function getCreatureHideHealth(cid) local c = Creature(cid) return c and c:isHealthHidden() or false end
function getCreatureSkullType(cid) local c = Creature(cid) return c and c:getSkull() or false end
getCreatureSkull = getCreatureSkullType
function getCreatureSpeakType() debugPrint("Deprecated function(getCreatureSpeakType).") return true end
function getCreatureLastPosition() debugPrint("Deprecated function(getCreatureLastPosition).") return true end
function getCreatureStorageList() debugPrint("Deprecated function(getCreatureStorageList).") return true end
function getCreatureGuildEmblem() debugPrint("Deprecated function(getCreatureGuildEmblem).") return true end
function getCreaturePartyShield() debugPrint("Deprecated function(getCreaturePartyShield).") return true end

function getCreatureDamageRatio(cid, target)
	local c, t = Creature(cid), Creature(target)
	if c and t then
		local totalDamage = 0
		local attackerDamage = 0
		local dmgMap = c:getDamageMap()
		for tid, damage in next, dmgMap, nil do
			totalDamage = totalDamage + damage.total
			if tid == t:getId() then
				attackerDamage = attackerDamage + damage.total
			end
		end
		return totalDamage > 0 and (attackerDamage / totalDamage) or 0
	end
	return false
end

function getCreatureTarget(cid)
	local c = Creature(cid)
	if c then
		local target = c:getTarget()
		return target and target:getId() or 0
	end
	return false
end

function getCreatureMaster(cid)
	local c = Creature(cid)
	if c then
		local master = c:getMaster()
		return master and master:getId() or c:getId()
	end
	return false
end

function getCreatureSummons(cid)
	local c = Creature(cid)
	if c == nil then
		return false
	end

	local result = {}
	local summons = c:getSummons()
	for i = 1, #summons do
		local summon = summons[i]
		result[#result + 1] = summon:getId()
	end
	return result
end

function setCreatureMaxHealth(cid, health) local c = Creature(cid) return c and c:setMaxHealth(health) or false end
function doCreatureAddHealth(cid, health) local c = Creature(cid) return c and c:addHealth(health) or false end
function doRemoveCreature(cid) local c = Creature(cid) return c and c:remove() or false end
function doCreatureSetLookDir(cid, direction) local c = Creature(cid) return c and c:setDirection(direction) or false end
doSetCreatureDirection = doCreatureSetLookDir
doCreatureSetLookDirection = doCreatureSetLookDir
function doCreatureSetSkullType(cid, skull) local c = Creature(cid) return c and c:setSkull(skull) or false end
function doCreatureSetHideHealth(cid, hide) local c = Creature(cid) return c and c:setHiddenHealth(hide) or false end
function doCreatureSay(cid, text, type, ...) local c = Creature(cid) return c and c:say(text, type, ...) or false end
doPlayerSay = doCreatureSay
function doCreatureChangeOutfit(cid, outfit) local c = Creature(cid) return c and c:setOutfit(outfit) or false end
function doSetCreatureDropLoot(cid, doDrop) local c = Creature(cid) return c and c:setDropLoot(doDrop) or false end
doCreatureSetDropLoot = doSetCreatureDropLoot
function doChangeSpeed(cid, delta) local c = Creature(cid) return c and c:changeSpeed(delta) or false end
function doAddCondition(cid, conditionId) local c = Creature(cid) return c and c:addCondition(conditionId) or false end
function doRemoveCondition(cid, conditionType, ...) local c = Creature(cid) return c and (c:removeCondition(conditionType, CONDITIONID_COMBAT, ...) or c:removeCondition(conditionType, CONDITIONID_DEFAULT, ...) or true) end
function doRemoveConditions() debugPrint("Deprecated function(doRemoveConditions).") return true end
function doCreatureSetSpeakType() debugPrint("Deprecated function(doCreatureSetSpeakType).") return true end
function doCreatureSetGuildEmblem() debugPrint("Deprecated function(doCreatureSetGuildEmblem).") return true end
function doCreatureSetPartyShield() debugPrint("Deprecated function(doCreatureSetPartyShield).") return true end

function getCreatureCondition(cid, type, ...) local c = Creature(cid) return c and c:hasCondition(type, ...) or false end
hasCondition = getCreatureCondition
hasCreatureCondition = getCreatureCondition

function registerCreatureEvent(cid, name) local c = Creature(cid) return c and c:registerEvent(name) or false end
function unregisterCreatureEvent(cid, name) local c = Creature(cid) return c and c:unregisterEvent(name) or false end

function getPlayerByName(name) local p = Player(name) return p and p:getId() or false end
function getIPByPlayerName(name) local p = Player(name) return p and p:getIp() or false end
function getPlayerGUID(cid) local p = Player(cid) return p and p:getGuid() or false end
function getPlayerNameDescription(cid, distance) local p = Player(cid) return p and p:getDescription(distance) or false end
function getPlayerSpecialDescription() debugPrint("Deprecated function(getPlayerSpecialDescription).") return true end
function getPlayerIp(cid) local p = Player(cid) return p and p:getIp() or false end
function getPlayerAccountId(cid) local p = Player(cid) return p and p:getAccountId() or false end
function getPlayerAccountType(cid) local p = Player(cid) return p and p:getAccountType() or false end
function getPlayerAccount(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local resultId = db.storeQuery("SELECT `name` FROM `accounts` WHERE `id` = " .. player:getAccountId() .. " LIMIT 1")
	if resultId ~= false then
		local name = result.getString(resultId, "name")
		result.free(resultId)
		return name
	end
	return false
end
function getPlayerByGUID(guid) local p = Player(guid) return p and p:getId() or nil end
function getPlayerByNameWildcard(name, ret) local p = Player(name) return ret and (p and RETURNVALUE_NOERROR or RETURNVALUE_PLAYERWITHTHISNAMEISNOTONLINE) or (p and p:getId() or nil) end
function getPlayerSkullEnd(cid) local p = Player(cid) return p and p:getSkullTime() + os.time() or false end
function getPlayerLastLoginSaved(cid) local p = Player(cid) return p and p:getLastLoginSaved() or false end
getPlayerLastLogin = getPlayerLastLoginSaved
function getPlayerName(cid) local p = Player(cid) return p and p:getName() or false end
function getPlayerFreeCap(cid) local p = Player(cid) return p and (p:getFreeCapacity() / 100) or false end
function getPlayerPosition(cid) local p = Player(cid) return p and p:getPosition() or false end
function getPlayerMagLevel(cid, ignoreModifiers) local p = Player(cid) return p and (ignoreModifiers and p:getBaseMagicLevel() or p:getMagicLevel()) or false end
function getPlayerAccess(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end
	return player:getGroup():getAccess() and 1 or 0
end
function getPlayerGhostAccess(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end
	return (player:getGroup():hasFlag(PlayerFlag_CanSenseInvisibility) or player:getGroup():getAccess()) and 1 or 0
end
function getPlayerWeapon(cid, ignoreAmmo)
	local player = Player(cid)
	if player == nil then
		return false
	end

	for i = CONST_SLOT_RIGHT, CONST_SLOT_LEFT do
		local inventoryItem = player:getSlotItem(i)
		if inventoryItem then
			local it = ItemType(inventoryItem:getId())
			local weaponType = it:getWeaponType()
			if weaponType ~= WEAPON_NONE and weaponType ~= WEAPON_SHIELD and weaponType ~= WEAPON_AMMO then
				if not(ignoreAmmo) and weaponType == WEAPON_DISTANCE then
					local ammoType = it:getAmmoType()
					if ammoType ~= AMMO_NONE then
						local inventoryAmmo = player:getSlotItem(CONST_SLOT_AMMO)
						if inventoryAmmo and ammoType == ItemType(inventoryAmmo:getId()):getAmmoType() then
							return pushThing(inventoryAmmo)
						end
					end
				else
					return pushThing(inventoryItem)
				end
			end
		end
	end
	return pushThing(nil)
end
function getPlayerSkill(cid, skillId) local p = Player(cid) return p and p:getSkillLevel(skillId) or false end
function getPlayerSkillTries(cid, skillId) local p = Player(cid) return p and p:getSkillTries(skillId) or false end
getPlayerSkillLevel = getPlayerSkill
function getPlayerMana(cid) local p = Player(cid) return p and p:getMana() or false end
function getPlayerMaxMana(cid, ignoreModifiers) local p = Player(cid) return p and (ignoreModifiers and p:getBaseMaxMana() or p:getMaxMana()) or false end
getCreatureMana = getPlayerMana
getCreatureMaxMana = getPlayerMaxMana
function getPlayerLevel(cid) local p = Player(cid) return p and p:getLevel() or false end
function getPlayerExperience(cid) local p = Player(cid) return p and p:getExperience() or false end
function getPlayerFightMode(cid) local p = Player(cid) return p and p:getFightMode() or false end
function getPlayerSecureMode(cid) local p = Player(cid) return p and p:hasSecureMode() or false end
function getPlayerSpentMana(cid) local p = Player(cid) return p and p:getManaSpent() or false end
function getPlayerRequiredMana(cid, magicLevel) local p = Player(cid) return p and p:getVocation():getRequiredManaSpent(magicLevel) or false end
function getPlayerRequiredSkillTries(cid, skillId, skillLevel) local p = Player(cid) return p and p:getVocation():getRequiredSkillTries(skillId, skillLevel) or false end
function getPlayerTown(cid) local p = Player(cid) return p and p:getTown():getId() or false end
function getPlayerVocation(cid) local p = Player(cid) return p and p:getVocation():getId() or false end
function getPlayerSoul(cid) local p = Player(cid) return p and p:getSoul() or false end
function getPlayerSex(cid) local p = Player(cid) return p and p:getSex() or false end
function getPlayerStorageValue(cid, key) local p = Player(cid) return p and p:getStorageValue(key) or false end
getCreatureStorage = getPlayerStorageValue
function getPlayerBalance(cid) local p = Player(cid) return p and p:getBankBalance() or false end
function getPlayerMoney(cid) local p = Player(cid) return p and p:getMoney() or false end
function getPlayerStamina(cid) local p = Player(cid) return p and p:getStamina() or false end
function getPlayerGroupId(cid) local p = Player(cid) return p and p:getGroup():getId() or false end
function getPlayerLookDir(cid) local p = Player(cid) return p and p:getDirection() or false end
getPlayerLookDirection = getPlayerLookDir
function getPlayerLight(cid) local p = Player(cid) return p and p:getLight() or false end
function getPlayerDepotItems(cid, depotId) local p = Player(cid) return p and p:getDepotItems(depotId) or false end
function getPlayerSkullType(cid) local p = Player(cid) return p and p:getSkull() or false end
function getPlayerLossPercent(cid) local p = Player(cid) return p and p:getDeathPenalty() or false end
function getPlayerMount(cid, mountId) local p = Player(cid) return p and p:hasMount(mountId) or false end
function getPlayerPremiumDays(cid) local p = Player(cid) return p and p:getPremiumDays() or false end
function getPlayerBlessing(cid, blessing) local p = Player(cid) return p and p:hasBlessing(blessing) or false end
function getPlayerFlagValue(cid, flag) local p = Player(cid) return p ~= nil and p:hasFlag(flag) or false end
function getPlayerCustomFlagValue() return false end
function getPlayerParty(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local party = player:getParty()
	if party == nil then
		return nil
	end
	return party:getLeader():getId()
end
function getPlayerGuildId(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local guild = player:getGuild()
	if guild == nil then
		return false
	end
	return guild:getId()
end
function getPlayerGuildLevel(cid) local p = Player(cid) return p and p:getGuildLevel() or false end
function getPlayerGuildName(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local guild = player:getGuild()
	if guild == nil then
		return false
	end
	return guild:getName()
end
function getPlayerGuildRankId(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local guild = player:getGuild()
	if guild == nil then
		return false
	end

	local rank = guild:getRankByLevel(player:getGuildLevel())
	return rank and rank.id or false
end
function getPlayerGuildRank(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local guild = player:getGuild()
	if guild == nil then
		return false
	end

	local rank = guild:getRankByLevel(player:getGuildLevel())
	return rank and rank.name or false
end
function getPlayerGuildNick(cid) local p = Player(cid) return p and p:getGuildNick() or false end
function getPlayerMasterPos(cid) local p = Player(cid) return p and p:getTown():getTemplePosition() or false end
function getPlayerItemCount(cid, itemId, ...) local p = Player(cid) return p and p:getItemCount(itemId, ...) or false end
function getPlayerSlotItem(cid, slot)
	local player = Player(cid)
	if player == nil then
		return pushThing(nil)
	end
	return pushThing(player:getSlotItem(slot))
end
function getPlayerItemById(cid, deepSearch, itemId, ...)
	local player = Player(cid)
	if player == nil then
		return pushThing(nil)
	end
	return pushThing(player:getItemById(itemId, deepSearch, ...))
end
function getPlayerFood(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end
	local c = player:getCondition(CONDITION_REGENERATION, CONDITIONID_DEFAULT) return c and math.floor(c:getTicks() / 1000) or 0
end
function canPlayerLearnInstantSpell(cid, name) local p = Player(cid) return p and p:canLearnSpell(name) or false end
function getPlayerLearnedInstantSpell(cid, name) local p = Player(cid) return p and p:hasLearnedSpell(name) or false end
function isPlayerGhost(cid) local p = Player(cid) return p and p:isInGhostMode() or false end
function isPlayerPzLocked(cid) local p = Player(cid) return p and p:isPzLocked() or false end
function isPremium(cid) local p = Player(cid) return p and p:isPremium() or false end
function getPlayersByIPAddress(ip, mask)
	if mask == nil then mask = 0xFFFFFFFF end
	local masked = bit.band(ip, mask)
	local result = {}
	local players = Game.getPlayers()
	for i = 1, #players do
		local player = players[i]
		if bit.band(player:getIp(), mask) == masked then
			result[#result + 1] = player:getId()
		end
	end
	return result
end
function getPlayersByAccountId(accId)
	local result = {}
	local players = Game.getPlayers()
	for i = 1, #players do
		local player = players[i]
		if player:getAccountId() == accId then
			result[#result + 1] = player:getId()
		end
	end
	return result
end
getPlayersByAccountNumber = getPlayersByAccountId
function getOnlinePlayers()
	local result = {}
	local players = Game.getPlayers()
	for i = 1, #players do
		local player = players[i]
		result[#result + 1] = player:getName()
	end
	return result
end
getPlayersOnlineEx = getOnlinePlayers
function getPlayersOnline()
	local result = {}
	local players = Game.getPlayers()
	for i = 1, #players do
		local player = players[i]
		result[#result + 1] = player:getId()
	end
	return result
end
function getPlayerGUIDByName(name)
	local player = Player(name)
	if player then
		return player:getGuid()
	end

	local resultId = db.storeQuery("SELECT `id` FROM `players` WHERE `name` = " .. db.escapeString(name) .. " LIMIT 1")
	if resultId ~= false then
		local guid = result.getNumber(resultId, "id")
		result.free(resultId)
		return guid
	end
	return 0
end
function getAccountNumberByPlayerName(name)
	local player = Player(name)
	if player then
		return player:getAccountId()
	end

	local resultId = db.storeQuery("SELECT `account_id` FROM `players` WHERE `name` = " .. db.escapeString(name) .. " LIMIT 1")
	if resultId ~= false then
		local accountId = result.getNumber(resultId, "account_id")
		result.free(resultId)
		return accountId
	end
	return 0
end

getPlayerAccountBalance = getPlayerBalance
getPlayersByIPNumber = getPlayersByIPAddress
getPlayersByIp = getPlayersByIPAddress
getPlayersByIP = getPlayersByIPAddress
getIpByName = getIPByPlayerName
getIPByName = getIPByPlayerName

function setPlayerStorageValue(cid, key, value) local p = Player(cid) return p and p:setStorageValue(key, value) or false end
doCreatureSetStorage = setPlayerStorageValue
doPlayerSetStorageValue = setPlayerStorageValue
function doPlayerSetNameDescription() debugPrint("Deprecated function(doPlayerSetNameDescription).") return true end
function doPlayerSendChannelMessage(cid, author, message, type, channel) local p = Player(cid) return p and p:sendChannelMessage(author, message, type, channel) or false end
function doPlayerSetMaxCapacity(cid, cap) local p = Player(cid) return p and p:setCapacity(cap) or false end
function doPlayerSetSpecialDescription() debugPrint("Deprecated function(doPlayerSetSpecialDescription).") return true end
function doPlayerSetBalance(cid, balance) local p = Player(cid) return p and p:setBankBalance(balance) or false end
setPlayerBalance = doPlayerSetBalance
function doPlayerSetPromotionLevel(cid, level) local p = Player(cid) return p and p:setVocation(p:getVocation():getPromotion()) or false end
setPlayerPromotionLevel = doPlayerSetPromotionLevel
function doPlayerSetStamina(cid, minutes) local p = Player(cid) return p and p:setStamina(minutes) or false end
setPlayerStamina = doPlayerSetStamina
function doPlayerAddStamina(cid, minutes) local p = Player(cid) return p and p:setStamina(p:getStamina() + minutes) or false end
function doPlayerAddMoney(cid, money) local p = Player(cid) return p and p:addMoney(money) or false end
function doPlayerRemoveMoney(cid, money) local p = Player(cid) return p and p:removeMoney(money) or false end
pay = doPlayerRemoveMoney
function doPlayerSave(cid) local p = Player(cid) return p and p:save() or false end
function doPlayerAddSoul(cid, soul) local p = Player(cid) return p and p:addSoul(soul) or false end
function doPlayerSetVocation(cid, vocation) local p = Player(cid) return p and p:setVocation(Vocation(vocation)) or false end
function doPlayerSetTown(cid, town) local p = Player(cid) return p and p:setTown(Town(town)) or false end
function setPlayerGroupId(cid, groupId) local p = Player(cid) return p and p:setGroup(Group(groupId)) or false end
doPlayerSetGroupId = setPlayerGroupId
function doPlayerSetSex(cid, sex) local p = Player(cid) return p and p:setSex(sex) or false end
function doPlayerSetGuildLevel(cid, level) local p = Player(cid) return p and p:setGuildLevel(level) or false end
function doPlayerSetGuildNick(cid, nick) local p = Player(cid) return p and p:setGuildNick(nick) or false end
function doPlayerSetOfflineTrainingSkill(cid, skillId) local p = Player(cid) return p and p:setOfflineTrainingSkill(skillId) or false end
function doPlayerSetSkullEnd(cid, time, type) local p = Player(cid) return p and (p:setSkull(type) and p:setSkullEnd(time - os.time())) or false end
function doShowTextDialog(cid, itemId, text) local p = Player(cid) return p and p:showTextDialog(itemId, text) or false end
function doPlayerAddItemEx(cid, uid, ...) local p = Player(cid) return p and p:addItemEx(Item(uid), ...) or false end
function doPlayerRemoveItem(cid, itemid, count, ...) local p = Player(cid) return p and p:removeItem(itemid, count, ...) or false end
function doPlayerAddPremiumDays(cid, days) local p = Player(cid) return p and p:addPremiumDays(days) or false end
function doPlayerRemovePremiumDays(cid, days) local p = Player(cid) return p and p:removePremiumDays(days) or false end
function doPlayerAddBlessing(cid, blessing) local p = Player(cid) return p and p:addBlessing(blessing) or false end
function doPlayerAddOutfit(cid, lookType, addons) local p = Player(cid) return p and p:addOutfitAddon(lookType, addons) or false end
function doPlayerRemOutfit(cid, lookType, addons)
	local player = Player(cid)
	if player == nil then
		return false
	end
	if addons == 255 then
		return player:removeOutfit(lookType)
	else
		return player:removeOutfitAddon(lookType, addons)
	end
end
doPlayerRemoveOutfit = doPlayerRemOutfit
function canPlayerWearOutfit(cid, lookType, addons) local p = Player(cid) return p and p:hasOutfit(lookType, addons) or false end
function doPlayerLearnInstantSpell(cid, name) local p = Player(cid) return p and p:learnSpell(name) or false end
function doPlayerUnlearnInstantSpell(cid, name) local p = Player(cid) return p and p:forgetSpell(name) or false end
function doPlayerAddMount(cid, mountId) local p = Player(cid) return p and p:addMount(mountId) or false end
function doPlayerRemoveMount(cid, mountId) local p = Player(cid) return p and p:removeMount(mountId) or false end
function doPlayerSendOutfitWindow(cid) local p = Player(cid) return p and p:sendOutfitWindow() or false end
function doPlayerSendCancel(cid, text) local p = Player(cid) return p and p:sendCancelMessage(text) or false end
function doPlayerFeed(cid, food) local p = Player(cid) return p and p:feed(food) or false end
playerLearnInstantSpell = doPlayerLearnInstantSpell
function doPlayerPopupFYI(cid, message) local p = Player(cid) return p and p:popupFYI(message) or false end
function doSendTutorial(cid, tutorialId) local p = Player(cid) return p and p:sendTutorial(tutorialId) or false end
doPlayerSendTutorial = doSendTutorial
function doAddMapMark(cid, pos, type, description) local p = Player(cid) return p and p:addMapMark(pos, type, description or "") or false end
doPlayerAddMapMark = doAddMapMark
function doPlayerSendTextMessage(cid, type, text, ...) local p = Player(cid) return p and p:sendTextMessage(type, text, ...) or false end
if(CLIENT_VERSION < 900) then
function doSendAnimatedText(pos, text, color, player) Game.sendAnimatedText(text, pos, color, player) return true end
else
function doSendAnimatedText() debugPrint("Deprecated function(doSendAnimatedText).") return true end
end
function doPlayerAddExp(cid, exp, useMult, ...)
	local player = Player(cid)
	if player == nil then
		return false
	end

	if useMult then
		exp = exp * Game.getExperienceStage(player:getLevel())
	end
	return player:addExperience(exp, ...)
end
function doPlayerAddManaSpent(cid, mana) local p = Player(cid) return p and p:addManaSpent(mana) or false end
doPlayerAddSpentMana = doPlayerAddManaSpent
function doPlayerAddSkillTry(cid, skillid, n) local p = Player(cid) return p and p:addSkillTries(skillid, n) or false end
function setPlayerMaxMana(cid, mana) local p = Player(cid) return p and p:setMaxMana(mana) or false end
setCreatureMaxMana = setPlayerMaxMana
function doPlayerAddMana(cid, mana, ...) local p = Player(cid) return p and p:addMana(mana, ...) or false end
doCreatureAddMana = doPlayerAddMana
function doPlayerJoinParty(cid, leaderId)
	local player = Player(cid)
	if player == nil then
		return false
	end

	if player:getParty() then
		player:sendTextMessage(MESSAGE_INFO_DESCR, "You are already in a party.")
		return true
	end

	local leader = Player(leaderId)
	if leader == nil then
		return false
	end

	local party = leader:getParty()
	if party == nil or party:getLeader() ~= leader then
		return true
	end

	local invitees = party:getInvitees()
	for i = 1, #invitees do
		local invitee = invitees[i]
		if player ~= invitee then
			return true
		end
	end

	party:addMember(player)
	return true
end
function getPartyMembers(cid)
	local player = Player(cid)
	if player == nil then
		return false
	end

	local party = player:getParty()
	if party == nil then
		return false
	end

	local result = {party:getLeader():getId()}
	local members = party:getMembers()
	for i = 1, #members do
		local member = members[i]
		result[#result + 1] = member:getId()
	end
	return result
end

doPlayerSendDefaultCancel = doPlayerSendCancel

function getMonsterTargetList(cid)
	local monster = Monster(cid)
	if monster == nil then
		return false
	end

	local result = {}
	local targetCreatures = monster:getTargetList()
	for i = 1, #targetCreatures do
		local creature = targetCreatures[i]
		if monster:isTarget(creature) then
			result[#result + 1] = creature:getId()
		end
	end
	return result
end
function getMonsterFriendList(cid)
	local monster = Monster(cid)
	if monster == nil then
		return false
	end

	local z = monster:getPosition().z

	local result = {}
	local friendCreatures = monster:getFriendList()
	for i = 1, #friendCreatures do
		local creature = friendCreatures[i]
		if not creature:isRemoved() and creature:getPosition().z == z then
			result[#result + 1] = creature:getId()
		end
	end
	return result
end
function doSetMonsterTarget(cid, target)
	local monster = Monster(cid)
	if monster == nil then
		return false
	end

	if monster:getMaster() then
		return true
	end

	local target = Creature(cid)
	if target == nil then
		return false
	end

	monster:selectTarget(target)
	return true
end
doMonsterSetTarget = doSetMonsterTarget
function doMonsterChangeTarget(cid)
	local monster = Monster(cid)
	if monster == nil then
		return false
	end

	if monster:getMaster() then
		return true
	end

	monster:searchTarget(1)
	return true
end
function doCreateNpc(name, pos, ...) local n = Game.createNpc(name, pos, ...) return n and n:setMasterPos(pos) or false end
function doCreateMonster(name, pos, ...) local m = Game.createMonster(name, pos, ...) return m and m:getId() or false end
function doSummonCreature(name, pos, ...)
	local m = Game.createMonster(name, pos, ...)
	if m then
		return m:getId()
	else
		local n = Game.createNpc(name, pos, ...)
		if n then
			n:setMasterPos(pos)
			return n:getId()
		end
	end
	return false
end
function doConvinceCreature(cid, target)
	local creature = Creature(cid)
	if creature == nil then
		return false
	end

	local targetCreature = Creature(target)
	if targetCreature == nil then
		return false
	end

	creature:addSummon(targetCreature)
	return true
end

function getTownId(townName) local t = Town(townName) return t and t:getId() or false end
function getTownName(townId) local t = Town(townId) return t and t:getName() or false end
function getTownTemplePosition(townId) local t = Town(townId) return t and t:getTemplePosition() or false end

function getItemParent(uid)
	local item = Item(uid)
	if item == nil then
		return false
	end

	local parent = item:getParent()
	if parent == nil then
		return pushThing(nil)
	end

	if parent:isTile() then
		local ground = parent:getGround()
		if ground and ground ~= item then
			return pushThing(ground)
		end
		return pushThing(nil)
	end
	return pushThing(parent)
end

function doSetItemActionId(uid, actionId) local i = Item(uid) return i and i:setActionId(actionId) or false end
doItemSetActionId = doSetItemActionId
function doTransformItem(uid, newItemId, ...) local i = Item(uid) return i and i:transform(newItemId, ...) or false end
function doChangeTypeItem(uid, newType) local i = Item(uid) return i and i:transform(i:getId(), newType) or false end
function doRemoveItem(uid, ...) local i = Item(uid) return i and i:remove(...) or false end
function getItemAttribute(uid, key) local i = Item(uid) return i and i:getAttribute(key) or false end
function doItemSetAttribute(uid, key, value) local i = Item(uid) return i and i:setAttribute(key, value) or false end
function doItemEraseAttribute(uid, key) local i = Item(uid) return i and i:removeAttribute(key) or false end

function getContainerSize(uid) local c = Container(uid) return c and c:getSize() or false end
function getContainerCap(uid) local c = Container(uid) return c and c:getCapacity() or false end
function getContainerItem(uid, slot)
	local container = Container(uid)
	if container == nil then
		return pushThing(nil)
	end
	return pushThing(container:getItem(slot))
end

function doAddContainerItemEx(uid, virtualId)
	local container = Container(uid)
	if container == nil then
		return false
	end

	local res = container:addItemEx(Item(virtualId))
	if res == nil then
		return false
	end
	return res
end

function doSendMagicEffect(pos, magicEffect, ...) return Position(pos):sendMagicEffect(magicEffect, ...) end
function doSendDistanceShoot(fromPos, toPos, distanceEffect, ...) return Position(fromPos):sendDistanceEffect(toPos, distanceEffect, ...) end
function isSightClear(fromPos, toPos, floorCheck) return Position(fromPos):isSightClear(toPos, floorCheck) end

function getPromotedVocation(vocationId)
	local vocation = Vocation(vocationId)
	if vocation == nil then
		return 0
	end

	local promotedVocation = vocation:getPromotion()
	if promotedVocation == nil then
		return 0
	end
	return promotedVocation:getId()
end
getPlayerPromotionLevel = getPromotedVocation

function getGuildId(guildName)
	local resultId = db.storeQuery("SELECT `id` FROM `guilds` WHERE `name` = " .. db.escapeString(guildName) .. " LIMIT 1")
	if resultId == false then
		return false
	end

	local guildId = result.getNumber(resultId, "id")
	result.free(resultId)
	return guildId
end

function getHouseName(houseId) local h = House(houseId) return h and h:getName() or false end
function getHouseOwner(houseId) local h = House(houseId) return h and h:getOwnerGuid() or false end
function getHouseEntry(houseId) local h = House(houseId) return h and h:getExitPosition() or false end
function getHouseTown(houseId) local h = House(houseId) if h == nil then return false end local t = h:getTown() return t and t:getId() or false end
function getHouseTilesSize(houseId) local h = House(houseId) return h and h:getTileCount() or false end

function isItemStackable(itemId) return ItemType(itemId):isStackable() end
function isItemRune(itemId) return ItemType(itemId):isRune() end
function isItemDoor(itemId) return ItemType(itemId):isDoor() end
function isItemContainer(itemId) return ItemType(itemId):isContainer() end
function isItemFluidContainer(itemId) return ItemType(itemId):isFluidContainer() end
function isItemMovable(itemId) return ItemType(itemId):isMovable() end
function isCorpse(uid) local i = Item(uid) return i and ItemType(i:getId()):isCorpse() or false end

isItemMoveable = isItemMovable
isMoveable = isMovable

function getItemName(itemId) return ItemType(itemId):getName() end
getItemNameById = getItemName
function getItemWeight(itemId, ...) return ItemType(itemId):getWeight(...) / 100 end
function getItemDescriptions(itemId)
	local itemType = ItemType(itemId)
	return {
		name = itemType:getName(),
		plural = itemType:getPluralName(),
		article = itemType:getArticle(),
		description = itemType:getDescription()
	}
end
function getItemIdByName(name)
	local id = ItemType(name):getId()
	if id == 0 then
		return false
	end
	return id
end
function getItemWeightByUID(uid, ...)
	local item = Item(uid)
	if item == nil then
		return false
	end

	local itemType = ItemType(item:getId())
	return itemType:isStackable() and (itemType:getWeight(item:getCount(), ...) / 100) or (itemType:getWeight(1, ...) / 100)
end
function getItemRWInfo(uid)
	local item = Item(uid)
	if item == nil then
		return false
	end

	local rwFlags = 0
	local itemType = ItemType(item:getId())
	if itemType:isReadable() then
		rwFlags = bit.bor(rwFlags, 1)
	end

	if itemType:isWritable() then
		rwFlags = bit.bor(rwFlags, 2)
	end
	return rwFlags
end
function getContainerCapById(itemId) return ItemType(itemId):getCapacity() end
function getFluidSourceType(itemId) local it = ItemType(itemId) return it.id ~= 0 and it:getFluidSource() or false end
function hasProperty(uid, prop)
	local item = Item(uid)
	if item == nil then
		return false
	end

	local parent = item:getParent()
	if parent:isTile() and item == parent:getGround() then
		return parent:hasProperty(prop)
	else
		return item:hasProperty(prop)
	end
end
hasItemProperty = hasProperty

function doSetItemText(uid, text)
	local item = Item(uid)
	if item == nil then
		return false
	end

	if text ~= "" then
		item:setAttribute(ITEM_ATTRIBUTE_TEXT, text)
	else
		item:removeAttribute(ITEM_ATTRIBUTE_TEXT)
	end
	return true
end
function doSetItemSpecialDescription(uid, desc)
	local item = Item(uid)
	if item == nil then
		return false
	end

	if desc ~= "" then
		item:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, desc)
	else
		item:removeAttribute(ITEM_ATTRIBUTE_DESCRIPTION)
	end
	return true
end
function doDecayItem(uid) local i = Item(uid) return i and i:decay() or false end

function setHouseOwner(id, guid) local h = House(id) return h and h:setOwnerGuid(guid) or false end
function getHouseRent(id) local h = House(id) return h and h:getRent() or nil end
function getHouseAccessList(id, listId) local h = House(id) return h and h:getAccessList(listId) or nil end
function setHouseAccessList(id, listId, listText) local h = House(id) return h and h:setAccessList(listId, listText) or false end

function getHouseByPlayerGUID(playerGUID)
	local houses = Game.getHouses()
	for i = 1, #houses do
		local house = houses[i]
		if house:getOwnerGuid() == playerGUID then
			return house:getId()
		end
	end
	return nil
end

function getTileHouseInfo(pos)
	local t = Tile(pos)
	if t == nil then
		return false
	end
	local h = t:getHouse()
	return h and h:getId() or false
end

function getTilePzInfo(position)
	local t = Tile(position)
	if t == nil then
		return false
	end
	return t:hasFlag(TILESTATE_PROTECTIONZONE)
end

function getTileInfo(position)
	local t = Tile(position)
	if t == nil then
		return false
	end

	local ret = pushThing(t:getGround())
	ret.protection = t:hasFlag(TILESTATE_PROTECTIONZONE)
	ret.nopz = ret.protection
	ret.blockcast = t:hasFlag(TILESTATE_BLOCKPROJECTILE)
	ret.blocksolid = t:hasFlag(TILESTATE_BLOCKSOLID)
	ret.nopvp = t:hasFlag(TILESTATE_NOPVPZONE)
	ret.nologout = t:hasFlag(TILESTATE_NOLOGOUT)
	ret.pvp = t:hasFlag(TILESTATE_PVPZONE)
	ret.refresh = t:hasFlag(TILESTATE_REFRESH)
	ret.house = t:getHouse() ~= nil
	ret.bed = t:hasFlag(TILESTATE_BED)
	ret.depot = t:hasFlag(TILESTATE_DEPOT)

	ret.things = t:getThingCount()
	ret.creatures = t:getCreatureCount()
	ret.items = t:getItemCount()
	ret.topItems = t:getTopItemCount()
	ret.downItems = t:getDownItemCount()
	return ret
end

function getTileItemByType(position, itemType)
	local t = Tile(position)
	if t == nil then
		return pushThing(nil)
	end
	return pushThing(t:getItemByType(itemType))
end
getTileItemsByType = getTileItemByType

function getTileItemById(position, itemId, ...)
	local t = Tile(position)
	if t == nil then
		return pushThing(nil)
	end
	return pushThing(t:getItemById(itemId, ...))
end

function getTileThingByPos(position)
	local t = Tile(position)
	if t == nil then
		if position.stackpos == -1 then
			return -1
		end
		return pushThing(nil)
	end

	if position.stackpos == -1 then
		return t:getThingCount()
	end
	return pushThing(t:getThing(position.stackpos))
end

function getTileThingByTopOrder(position, topOrder)
	local t = Tile(position)
	if t == nil then
		return pushThing(nil)
	end
	return pushThing(t:getItemByTopOrder(topOrder))
end

function getTopCreature(position)
	local t = Tile(position)
	if t == nil then
		return pushThing(nil)
	end
	return pushThing(t:getTopCreature())
end

function queryTileAddThing(thing, position, ...) local t = Tile(position) return t and t:queryAdd(thing, ...) or false end
doTileQueryAdd = queryTileAddThing

function doTeleportThing(uid, dest, pushMovement)
	if type(uid) == "userdata" then
		if uid:isCreature() then
			return uid:teleportTo(dest, pushMovement or false)
		else
			return uid:moveTo(dest)
		end
	else
		if uid >= 0x10000000 then
			local creature = Creature(uid)
			if creature then
				return creature:teleportTo(dest, pushMovement or false)
			end
		else
			local item = Item(uid)
			if item then
				return item:moveTo(dest)
			end
		end
	end
	return false
end

function getThingPos(uid)
	local thing
	if type(uid) ~= "userdata" then
		if uid >= 0x10000000 then
			thing = Creature(uid)
		else
			thing = Item(uid)
		end
	else
		thing = uid
	end

	if thing == nil then
		return false
	end

	local stackpos = 0
	local tile = thing:getTile()
	if tile then
		stackpos = tile:getThingIndex(thing)
	end

	local position = thing:getPosition()
	position.stackpos = stackpos
	return position
end
getThingPosition = getThingPos

function getThingfromPos(pos)
	local tile = Tile(pos)
	if tile == nil then
		return pushThing(nil)
	end

	local thing
	local stackpos = pos.stackpos or 0
	if stackpos == STACKPOS_TOP_MOVEABLE_ITEM_OR_CREATURE then
		thing = tile:getTopCreature()
		if thing == nil then
			local item = tile:getTopDownItem()
			if item and item:getType():isMovable() then
				thing = item
			end
		end
	elseif stackpos == STACKPOS_TOP_FIELD then
		thing = tile:getFieldItem()
	elseif stackpos == STACKPOS_TOP_CREATURE then
		thing = tile:getTopCreature()
	else
		thing = tile:getThing(stackpos)
	end
	return pushThing(thing)
end
getThingFromPos = getThingfromPos
getThingFromPosition = getThingfromPos

function doRelocate(fromPos, toPos, creatures)
	if fromPos == toPos then
		return false
	end

	local fromTile = Tile(fromPos)
	if fromTile == nil then
		return false
	end

	if Tile(toPos) == nil then
		return false
	end

	if creatures == nil then
		creatures = true
	end

	for i = fromTile:getThingCount() - 1, 0, -1 do
		local thing = fromTile:getThing(i)
		if thing then
			if thing:isItem() then
				if ItemType(thing:getId()):isMovable() then
					thing:moveTo(toPos)
				end
			elseif creatures and thing:isCreature() then
				thing:teleportTo(toPos)
			end
		end
	end
	return true
end

function getThing(uid)
	return uid >= 0x10000000 and pushThing(Creature(uid)) or pushThing(Item(uid))
end

function getConfigInfo(info)
	if type(info) ~= "string" then
		return nil
	end
	dofile('config.lua')
	return _G[info]
end
getConfigValue = getConfigInfo

function getWorldCreatures(type)
	if type == 0 then
		return Game.getPlayerCount()
	elseif type == 1 then
		return Game.getMonsterCount()
	elseif type == 2 then
		return Game.getNpcCount()
	end
	return Game.getPlayerCount() + Game.getMonsterCount() + Game.getNpcCount()
end

saveData = saveServer
savePlayers = saveServer
doSaveServer = saveServer
doCleanMap = cleanMap

function getGlobalStorageValue(key)
	return Game.getStorageValue(key) or -1
end
function setGlobalStorageValue(key, value)
	Game.setStorageValue(key, value)
	return true
end
getStorage = getGlobalStorageValue
doSetStorage = setGlobalStorageValue

getWorldType = Game.getWorldType
setWorldType = Game.setWorldType
getGameState = Game.getGameState
doSetGameState = Game.setGameState
doExecuteRaid = Game.startRaid
executeRaid = Game.startRaid

numberToVariant = Variant
stringToVariant = Variant
positionToVariant = Variant

function targetPositionToVariant(position)
	local variant = Variant(position)
	variant.type = VARIANT_TARGETPOSITION
	return variant
end

variantToNumber = Variant.getNumber
variantToString = Variant.getString
variantToPosition = Variant.getPosition

function doCreateTeleport(itemId, destination, position)
	local item = Game.createItem(itemId, 1, position)
	if not item:isTeleport() then
		item:remove()
		return false
	end
	item:setDestination(destination)
	return item:getUniqueId()
end

function getSpectators(centerPos, rangex, rangey, multifloor, onlyPlayers)
	local result = Game.getSpectators(centerPos, multifloor or false, onlyPlayers or false, rangex, rangex, rangey, rangey)
	if #result == 0 then
		return nil
	end

	for i = 1, #result do
		local sid = result[i]:getId()
		result[i] = sid
	end
	return result
end

function broadcastMessage(message, messageType)
	Game.broadcastMessage(message, messageType)
	print("> Broadcasted message: \"" .. message .. "\".")
end
doBroadcastMessage = broadcastMessage

function Guild.addMember(self, player)
	return player:setGuild(guild)
end
function Guild.removeMember(self, player)
	return player:getGuild() == self and player:setGuild(nil)
end

function getPlayerInstantSpellCount(cid) local p = Player(cid) return p and #p:getInstantSpells() end
function getPlayerInstantSpellInfo(cid, spellId)
	local player = Player(cid)
	if not player then
		return false
	end

	local spell = Spell(spellId)
	if not spell or not player:canCast(spell) then
		return false
	end
	return spell
end

function doSetItemOutfit(cid, item, time) local c = Creature(cid) return c and c:setItemOutfit(item, time) end
function doSetMonsterOutfit(cid, name, time) local c = Creature(cid) return c and c:setMonsterOutfit(name, time) end
function doSetCreatureOutfit(cid, outfit, time)
	local creature = Creature(cid)
	if not creature then
		return false
	end

	local condition = Condition(CONDITION_OUTFIT)
	condition:setOutfit(outfit)
	condition:setTicks(time)
	creature:addCondition(condition)
	return true
end

function doTileAddItemEx(pos, uid, ...)
	local tile = Tile(pos)
	if not tile then
		return false
	end

	local item = Item(uid)
	if item then
		return tile:addItemEx(item, ...)
	end
	return false
end

function isInArray(array, value) return table.contains(array, value) end

function doCreateItem(itemid, count, pos)
	if pos == nil then
		pos = count
		count = 1
	end

	local tile = Tile(pos)
	if not tile then
		return false
	end

	local item = Game.createItem(itemid, count, pos)
	if item then
		return item:getUniqueId()
	end
	return false
end

function doCreateItemEx(itemid, count)
	local item = Game.createItem(itemid, count)
	if item then
		return item:getUniqueId()
	end
	return false
end

function doMoveCreature(cid, direction) local c = Creature(cid) return c ~= nil and c:move(direction) end

function createFunctions(class)
	local exclude = {[2] = {"is"}, [3] = {"get", "set", "add", "can"}, [4] = {"need"}}
	local temp = {}
	for name, func in next, class, nil do
		local add = true
		for strLen, strTable in next, exclude, nil do
			if table.contains(strTable, name:sub(1, strLen)) then
				add = false
			end
		end
		if add then
			local str = name:sub(1, 1):upper() .. name:sub(2)
			local getFunc = function(self) return func(self) end
			local setFunc = function(self, ...) return func(self, ...) end
			local get = "get" .. str
			local set = "set" .. str
			if not (rawget(class, get) and rawget(class, set)) then
				table.insert(temp, {set, setFunc, get, getFunc})
			end
		end
	end
	for i = 1, #temp do
		local func = temp[i]
		rawset(class, func[1], func[2])
		rawset(class, func[3], func[4])
	end
end

function doPlayerTakeItem(cid, itemid, count) local p = Player(cid) return p and p:removeItem(itemid, count) end
function doPlayerAddLevel(cid, amount, round) local p = Player(cid) return p and p:addLevel(amount, round) end
function getExperienceForLevel(level)
	return math.floor((((level - 6) * level + 17) * level - 12) / 6) * 100
end
doPlayerAddExperience = doPlayerAddExp

function doSummonMonster(cid, name)
	local creature = Creature(cid)
	if creature == nil then
		return false
	end

	local summon = Game.createMonster(name, creature:getPosition(), true, false, creature)
	return summon and summon:getId() or false
end

getAccountIdByName = getAccountNumberByPlayerName
getAccountNumberByName = getAccountNumberByPlayerName

function getAccountByName(name)
	local player = Player(name)
	if player then
		return getPlayerAccount(player)
	end

	local resultId = db.storeQuery("SELECT `name` FROM `accounts` WHERE `id` = (SELECT `account_id` FROM `players` WHERE `name` = " .. db.escapeString(name) .. " LIMIT 1) LIMIT 1")
	if resultId ~= false then
		local name = result.getString(resultId, "name")
		result.free(resultId)
		return name
	end
	return false
end

function getAccountIdByAccount(name)
	local resultId = db.storeQuery("SELECT `id` FROM `accounts` WHERE `name` = " .. db.escapeString(name) .. " LIMIT 1")
	if resultId ~= false then
		local id = result.getNumber(resultId, "id")
		result.free(resultId)
		return id
	end
	return false
end

function getAccountByAccountId(id)
	local resultId = db.storeQuery("SELECT `name` FROM `accounts` WHERE `id` = " .. id .. " LIMIT 1")
	if resultId ~= false then
		local name = result.getString(resultId, "name")
		result.free(resultId)
		return name
	end
	return false
end
