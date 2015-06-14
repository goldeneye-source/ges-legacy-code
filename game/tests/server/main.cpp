#include "cbase.h"
#include "gtest/gtest.h"

#include "../sdk_listener.h"

using namespace testing;

// We can only run once
static bool g_didRun = false;

bool RunTests()
{
	// We can only run the tests once!
	if ( !g_didRun )
	{
		g_didRun = true;

		int tmp_argc = 0;
		char *tmp_argv = "";

		InitGoogleTest( &tmp_argc, &tmp_argv );

		UnitTest& unit_test = *UnitTest::GetInstance();

		TestEventListeners& listeners = unit_test.listeners();
		delete listeners.Release( listeners.default_result_printer() );
		listeners.Append( new SDKUnitTestListener );

		int res = RUN_ALL_TESTS();

		return true;
	}
	
	return false;
}

// "Pull in" tests here, this is to overcome a bug in VS
//
// Game Timer Test
extern int ge_game_timer_test;
static int test1 = ge_game_timer_test;
// Gamerules Test
extern int ge_gamerules_test;
static int test2 = ge_gamerules_test;
// Gameplay Test
extern int ge_gameplay_test;
static int test3 = ge_gameplay_test;
