#include "cbase.h"
#include "../common_test.h"

#include "ge_gameplay.h"
#include "gemp_gamerules.h"

int ge_gamerules_test = 1;

class GameRulesTest : public CGECommonTest {
protected:
	GameRulesTest() { }
	virtual ~GameRulesTest() { }

	virtual void SetUp() {
		// Create a brand new game rules
		CreateGameRulesObject( "GEMPRules" );
	}

	virtual void TearDown() {
		// Nothing to see here
	}
};
