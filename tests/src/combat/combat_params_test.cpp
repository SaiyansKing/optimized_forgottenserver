#include "../all.h"

TEST_SUITE( "Combat Test") {
	TEST_CASE("CombatParam default attributes") {
    CombatParams params;

    CHECK(params.itemId == 0);

    CHECK(params.aggressive);
    CHECK_FALSE(params.useCharges);
    CHECK_FALSE(params.blockedByArmor);
    CHECK_FALSE(params.blockedByShield);
    CHECK_FALSE(params.targetCasterOrTopMost);

    CHECK(params.origin == ORIGIN_SPELL);
    CHECK(params.combatType == COMBAT_NONE);
    CHECK(params.dispelType == CONDITION_NONE);
    CHECK(params.impactEffect == CONST_ME_NONE);
    CHECK(params.distanceEffect == CONST_ANI_NONE);
    CHECK(params.validTargets == COMBAT_TARGET_PARAM_ALL);
  }
}
