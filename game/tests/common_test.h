#ifndef GE_COMMON_TEST_H
#define GE_COMMON_TEST_H

#include "edict.h"
#include "gtest/gtest.h"

class CGECommonTest : public ::testing::Test {
protected:
	CGECommonTest() {
		// Capture the current time
		_curtime = gpGlobals->curtime;
	}

	virtual ~CGECommonTest() {
		// Restore the time
		gpGlobals->curtime = _curtime;
	}

	// Advance time by the given interval (seconds)
	void AdvanceGameTime( float seconds ) {
		gpGlobals->curtime += seconds;
	}

	// Set the time to the given moment (seconds)
	void SetGameTime( float abs_seconds ) {
		gpGlobals->curtime = abs_seconds;
	}

private:
	float _curtime;
};

#endif
