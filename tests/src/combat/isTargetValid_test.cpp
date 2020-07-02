#include "../all.h"

LuaEnvironment g_luaEnvironment;

CombatParams params;
MonsterType type;
Player player(nullptr);
Monster monsterA(&type);
Monster monsterB(&type);
Npc npc;

TEST_SUITE( "CombatTest - isTargetValid" ) {
	TEST_CASE("Null target is always false") {
    CHECK_FALSE(Combat::isTargetValid(nullptr, params));
  }

	TEST_CASE("Default params must attack any creature") {
    CHECK(Combat::isTargetValid(&player, params));
    CHECK(Combat::isTargetValid(&monsterB, params));
    CHECK(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_PLAYERS must attack players only") {
    params.validTargets = COMBAT_TARGET_PARAM_PLAYERS;
    CHECK(Combat::isTargetValid(&player, params));
    CHECK_FALSE(Combat::isTargetValid(&monsterB, params));
    CHECK_FALSE(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_MONSTERS must attack monsters only") {
    params.validTargets = COMBAT_TARGET_PARAM_MONSTERS;
    CHECK_FALSE(Combat::isTargetValid(&player, params));
    CHECK(Combat::isTargetValid(&monsterB, params));
    CHECK_FALSE(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_PLAYERSANDMONSTERS must attack players and monsters") {
    params.validTargets = COMBAT_TARGET_PARAM_PLAYERSANDMONSTERS;
    CHECK(Combat::isTargetValid(&player, params));
    CHECK(Combat::isTargetValid(&monsterB, params));
    CHECK_FALSE(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_NPCS must attack npcs only") {
    params.validTargets = COMBAT_TARGET_PARAM_NPCS;
    CHECK_FALSE(Combat::isTargetValid(&player, params));
    CHECK_FALSE(Combat::isTargetValid(&monsterB, params));
    CHECK(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_PLAYERSANDNPCS must attack players and npcs") {
    params.validTargets = COMBAT_TARGET_PARAM_PLAYERSANDNPCS;
    CHECK(Combat::isTargetValid(&player, params));
    CHECK_FALSE(Combat::isTargetValid(&monsterB, params));
    CHECK(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_MONSTERSANDNPCS must attack monsters and npcs") {
    params.validTargets = COMBAT_TARGET_PARAM_MONSTERSANDNPCS;
    CHECK_FALSE(Combat::isTargetValid(&player, params));
    CHECK(Combat::isTargetValid(&monsterB, params));
    CHECK(Combat::isTargetValid(&npc, params));
  }

	TEST_CASE("CombatParams validTarget = COMBAT_TARGET_PARAM_ALL must attack any creature") {
    params.validTargets = COMBAT_TARGET_PARAM_ALL;
    CHECK(Combat::isTargetValid(&player, params));
    CHECK(Combat::isTargetValid(&monsterB, params));
    CHECK(Combat::isTargetValid(&npc, params));
  }
} 