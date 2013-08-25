#include "cbase.h"
#include "gtest/gtest.h"

#include "baseentity.h"
#include "ge_game_timer.h"

CGEGameTimer *timer = NULL;

class TimerTest : public ::testing::Test {
protected:
	TimerTest() {

	}

	virtual ~TimerTest() {
	// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		timer = (CGEGameTimer*) CBaseEntity::Create( "ge_game_timer", vec3_origin, vec3_angle );
		timer->Start( 100 );
	}

	virtual void TearDown() {
		UTIL_Remove( timer );
		timer = NULL;
	}
};

TEST(TimerTest, StartsTimer) {
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
}
