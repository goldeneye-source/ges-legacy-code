#include "cbase.h"
#include "../common_test.h"

#include "ge_game_timer.h"

int ge_game_timer_test = 1;

class TimerTest : public CGECommonTest {
protected:
	TimerTest() { }
	virtual ~TimerTest() { }

	virtual void SetUp() {
		// Setup, can throw exceptions
		timer = (CGEGameTimer*) CBaseEntity::Create( "ge_game_timer", vec3_origin, vec3_angle );
	}

	virtual void TearDown() {
		// Cleanup, can throw exceptions
		UTIL_Remove( timer );
	}

	CGEGameTimer *timer;
};

TEST_F(TimerTest, InitialTimer) {
	EXPECT_FALSE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 0 );
}

TEST_F(TimerTest, StartTimer) {
	timer->Start( 100.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 100.0f );
	// Advance time
	AdvanceGameTime( 50.0f );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 50.0f );
}

TEST_F(TimerTest, PauseTimer) {
	// Start timer and immediately pause it
	timer->Start( 100.0f );
	timer->Pause();
	EXPECT_TRUE( timer->IsPaused() );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 100.0f );
	// Advance time
	AdvanceGameTime( 50.0f );
	// Re-check that we have 100 seconds left
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 100.0f );
}

TEST_F(TimerTest, ResumeTimer) {
	// Start timer and immediately pause it
	timer->Start( 100.0f );
	timer->Pause();
	// Advance time
	AdvanceGameTime( 50.0f );
	// Resume
	timer->Resume();
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_TRUE( timer->IsEnabled() );
	// Advance time
	AdvanceGameTime( 50.0f );
	// Check to see we have 50 seconds left
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 50.0f );
}

TEST_F(TimerTest, StopTimer) {
	timer->Start( 100.0f );
	timer->Stop();
	EXPECT_FALSE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 0 );
	// Advance time
	AdvanceGameTime( 50.0f );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 0 );
}

TEST_F(TimerTest, BadStartTimer) {
	timer->Start( 100.0f );
	timer->Start( -1 );
	EXPECT_FALSE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 0 );
}

TEST_F(TimerTest, DualStartTimer) {
	timer->Start( 100.0f );
	timer->Start( 25.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 25.0f );
	// Advance time
	AdvanceGameTime( 50.0f );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 0 );
}

TEST_F(TimerTest, ChangeTimerLengthSame) {
	timer->Start( 100.0f );
	// Advance time
	AdvanceGameTime( 50.0f );
	// Change timer to same value as before
	timer->ChangeLength( 100.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 50.0f );
}

TEST_F(TimerTest, ChangeTimerLengthLess) {
	timer->Start( 100.0f );
	// Advance time
	AdvanceGameTime( 50.0f );
	// Change timer to a value less than it was
	timer->ChangeLength( 25.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 0 );
}

TEST_F(TimerTest, ChangeTimerLengthMore) {
	timer->Start( 100.0f );
	// Advance time
	AdvanceGameTime( 50.0f );
	// Change timer to a value more than it was
	timer->ChangeLength( 150.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 100.0f );
}

TEST_F(TimerTest, ChangeTimerLengthPaused) {
	timer->Start( 100.0f );
	// Advance time
	AdvanceGameTime( 25.0f );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 75.0f );
	// Pause the timer
	timer->Pause();
	// Advance time, remaining time should stay at 75
	AdvanceGameTime( 25.0f );
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 75.0f );
	// Change length to more than it was
	timer->ChangeLength( 150.0f );
	// Advance time
	AdvanceGameTime( 25.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_TRUE( timer->IsPaused() );
	// The remaining time should be 75 + 50 = 125
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 125.0f );
}

TEST_F(TimerTest, ChangeTimerLengthDisabled) {
	timer->Start( 0 );
	EXPECT_FALSE( timer->IsEnabled() );
	// Change length to more than it was
	timer->ChangeLength( 100.0f );
	// Advance time
	AdvanceGameTime( 25.0f );
	EXPECT_TRUE( timer->IsEnabled() );
	EXPECT_FALSE( timer->IsPaused() );
	// The remaining time should be 0 + 100 - 25 = 75
	EXPECT_FLOAT_EQ( timer->GetTimeRemaining(), 75.0f );
}
