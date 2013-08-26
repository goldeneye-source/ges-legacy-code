#include "cbase.h"
#include "gtest/gtest.h"

#include "gemp_gamerules.h"

int ge_gamerules_test = 1;

class GameRulesTest : public ::testing::Test {
protected:
	GameRulesTest() {
		// Setup, NO exceptions
	}

	virtual void SetUp() {
		// Create a brand new game rules
		CreateGameRulesObject( "GEMPRules" );
	}

	virtual void TearDown() {
		// Nothing to see here
	}

	virtual ~GameRulesTest() {
		// Cleanup, NO exceptions
	}
};

TEST_F(GameRulesTest, 