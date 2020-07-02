#include "../all.h"

Vocation voc(1);

TEST_SUITE( "CombatTest - canDoTargetCombat" ) {
	TEST_CASE("Can attack when attacker is null") {
    g_vocations().addVocation(voc);
    CHECK(Combat::canDoTargetCombat(nullptr, &monsterB, CombatParams()) == RETURNVALUE_NOERROR);
  }

	TEST_CASE("Can't attacker when isTargetValid is false (target null)") {
    CHECK(Combat::canDoTargetCombat(&monsterA, nullptr, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE);
  }

	TEST_CASE("Target is player - can't attack players with cannotBeAttacked flag") {
    Group group;
    group.flags = PlayerFlag_CannotBeAttacked;

    Player targetPlayer(nullptr);
    targetPlayer.setGroup(&group);

    CHECK(Combat::canDoTargetCombat(&monsterA, &targetPlayer, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);
  }

	TEST_CASE("PvP - player with cannotAttackPlayer flag can't attack players") {
    Group group;
    group.flags = PlayerFlag_CannotAttackPlayer;

    Player targetPlayer(nullptr);
    targetPlayer.setGroup(&group);
    CHECK(Combat::canDoTargetCombat(&targetPlayer, &targetPlayer, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);
  }

	TEST_CASE("PvP - protected players can't attack/be attacked") {
    g_config().setNumber(ConfigManager::PROTECTION_LEVEL, 5);
    CHECK(Combat::canDoTargetCombat(&player, &player, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);
  }

	TEST_CASE("PvP - no PvP tiles should be respected") {
    g_config().setNumber(ConfigManager::PROTECTION_LEVEL, 0);

    Player targetPlayer(nullptr);

    // set player vocation
    targetPlayer.setVocation(voc.getId());

    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    targetPlayer.setParent(&tile);

    // Can't attack non PvP tile
    tile.setFlag(TILESTATE_NOPVPZONE);
    CHECK(Combat::canDoTargetCombat(&targetPlayer, &targetPlayer, CombatParams()) == RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE);
    
    // Can attack PvP tile
    tile.resetFlag(TILESTATE_NOPVPZONE);
    CHECK(Combat::canDoTargetCombat(&targetPlayer, &targetPlayer, CombatParams()) == RETURNVALUE_NOERROR);
  }

	TEST_CASE("PvP - can't attack on no PVP world") {
    g_config().setNumber(ConfigManager::PROTECTION_LEVEL, 0);

    Player targetPlayer(nullptr);

    // set player vocation
    targetPlayer.setVocation(voc.getId());

    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    targetPlayer.setParent(&tile);

    g_game().setWorldType(WORLD_TYPE_NO_PVP);
    CHECK(Combat::canDoTargetCombat(&targetPlayer, &targetPlayer, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);
    g_game().setWorldType(WORLD_TYPE_PVP);
  }

	TEST_CASE("PvP - [summon attacker] master has cannotAttackPlayer flag") {
    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    player.setParent(&tile);

    // set group
    Group group;
    group.flags = PlayerFlag_CannotAttackPlayer;
    player.setGroup(&group);

    // create summon
    Monster summon(&type);
    summon.setMaster(&player);

    CHECK(Combat::canDoTargetCombat(&summon, &player, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);

    player.setGroup(nullptr);
    player.setParent(nullptr);
  }

	TEST_CASE("PvP - [summon attacker] is no pvp tile") {
    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    player.setParent(&tile);

    // create summon
    Monster summon(&type);
    summon.setMaster(&player);

    tile.setFlag(TILESTATE_NOPVPZONE);
    CHECK(Combat::canDoTargetCombat(&summon, &player, CombatParams()) == RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE);

    player.setParent(nullptr);
  }

	TEST_CASE("PvP - [summon attacker] is player protected") {
    g_config().setNumber(ConfigManager::PROTECTION_LEVEL, 5);

    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    player.setParent(&tile);

    // create summon
    Monster summon(&type);
    summon.setMaster(&player);

    CHECK(Combat::canDoTargetCombat(&summon, &player, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);

    player.setParent(nullptr);
  }

	TEST_CASE("PvP - [summon attacker] can attack regular players") {
    g_config().setNumber(ConfigManager::PROTECTION_LEVEL, 0);

    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    player.setParent(&tile);

    // create summon
    Monster summon(&type);
    summon.setMaster(&player);

    // set player vocation
    player.setVocation(voc.getId());

    CHECK(Combat::canDoTargetCombat(&summon, &player, CombatParams()) == RETURNVALUE_NOERROR);

    player.setParent(nullptr);
  }

	TEST_CASE("PvP - [summon attacker] can't attack on no PVP world") {
    g_config().setNumber(ConfigManager::PROTECTION_LEVEL, 0);

    // create no PvP tile and set to player
    StaticTile tile = StaticTile(1, 1, 1);
    player.setParent(&tile);

    // create summon
    Monster summon(&type);
    summon.setMaster(&player);
    summon.setParent(&tile);

    // set player vocation
    player.setVocation(voc.getId());

    g_game().setWorldType(WORLD_TYPE_NO_PVP);
    CHECK(Combat::canDoTargetCombat(&summon, &player, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER);
    g_game().setWorldType(WORLD_TYPE_PVP);
    
    player.setParent(nullptr);
  }

	TEST_CASE("Monster cannot attack monster") {
    CHECK(Combat::canDoTargetCombat(&monsterA, &monsterB, CombatParams()) == RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE);
  }

	TEST_CASE("Monster can attack common player") {
    CHECK(Combat::canDoTargetCombat(&monsterA, &player, CombatParams()) == RETURNVALUE_NOERROR);
  }

	TEST_CASE("Player can attack monster") {
    CHECK(Combat::canDoTargetCombat(&player, &monsterA, CombatParams()) == RETURNVALUE_NOERROR);
  }
} 