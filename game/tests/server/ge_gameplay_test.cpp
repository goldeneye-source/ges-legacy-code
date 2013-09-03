#include "cbase.h"
#include "../common_test.h"

#include "ge_gameplay.h"
#include "gemp_gamerules.h"

int ge_gameplay_test = 1;

class CGEScenarioTest : public CGEBaseScenario
{
public:
	CGEScenarioTest() {
		teamplay = TEAMPLAY_NOT;
		can_round_end = can_match_end = true;
	}

	virtual void Init() {
		Msg( "[PYGP] Called OnLoadGamePlay()\n" );
		Msg( "[PYGP] Called GetScenarioHelp()\n" );
	}
	virtual void Shutdown() { Msg( "[PYGP] Called OnUnloadGamePlay()\n" ); }

	const char* GetIdent() { return "TestScenario"; }
	virtual const char* GetGameDescription() { return "Test Scenario Description"; }
	virtual const char* GetPrintName() { return "Test Scenario"; }
	virtual int GetHelpString() { return 0; }

	virtual void ClientConnect( CGEPlayer *pPlayer ) { Msg( "[PYGP] Called OnPlayerConnect()\n" ); }
	virtual void ClientDisconnect( CGEPlayer *pPlayer ) { Msg( "[PYGP] Called OnPlayerDisconnect()\n" ); }

	virtual void OnThink() { Msg( "[PYGP] Called OnThink()\n" ); }

	virtual void OnRoundBegin() { Msg( "[PYGP] Called OnRoundBegin()\n" ); }
	virtual void OnRoundEnd() { Msg( "[PYGP] Called OnRoundEnd()\n" ); }

	virtual void OnCVarChanged(const char* name, const char* oldvalue, const char* newvalue) { 
		Msg( "[PYGP] Called OnCVarChanged()\n" );
	}

	virtual bool CanPlayerRespawn(CGEPlayer *pPlayer) { 
		Msg( "[PYGP] Called CanPlayerRespawn()\n" ); 
		return true;
	}
	virtual bool CanPlayerHaveItem(CGEPlayer *pPlayer, CBaseEntity *pEntity) { 
		Msg( "[PYGP] Called CanPlayerHaveItem()\n" );
		return true;
	}
	virtual bool CanPlayerChangeChar(CGEPlayer* pPlayer, const char* szIdent) { 
		Msg( "[PYGP] Called CanPlayerChangeChar()\n" ); 
		return true;
	}
	virtual bool CanPlayerChangeTeam(CGEPlayer *pPlayer, int iOldTeam, int iNewTeam) { 
		Msg( "[PYGP] Called CanPlayerChangeTeam()\n" ); 
		return true;
	}
	virtual void CalculateCustomDamage(CGEPlayer *pVictim, const CTakeDamageInfo &inputInfo, float &health, float &armor) { 
		Msg( "[PYGP] Called CalculateCustomDamage()\n" );
	}
	virtual bool ShouldForcePickup(CGEPlayer *pPlayer, CBaseEntity *pEntity) { 
		Msg( "[PYGP] Called ShouldForcePickup()\n" ); 
		return false;
	}
	
	virtual void OnPlayerSpawn(CGEPlayer *pPlayer) { Msg( "[PYGP] Called OnPlayerSpawn()\n" ); }
	virtual void OnPlayerObserver(CGEPlayer *pPlayer) { Msg( "[PYGP] Called OnPlayerObserver()\n" ); }
	virtual void OnPlayerKilled(CGEPlayer *pVictim, CGEPlayer *pKiller, CBaseEntity *pWeapon) { Msg( "[PYGP] Called OnPlayerKilled()\n" ); }
	virtual bool OnPlayerSay(CGEPlayer* pPlayer, const char* text) { 
		Msg( "[PYGP] Called OnPlayerSay()\n" ); 
		return false;
	}

	virtual void OnCaptureAreaSpawned( CGECaptureArea *pCapture ) { 
		Msg( "[PYGP] Called OnCaptureAreaSpawned()\n" );
	}
	virtual void OnCaptureAreaRemoved( CGECaptureArea *pCapture ) { 
		Msg( "[PYGP] Called OnCaptureAreaRemoved()\n" );
	}
	virtual void OnCaptureAreaEntered( CGECaptureArea *pCapture, CGEPlayer *pPlayer, CGEWeapon *pToken ) { 
		Msg( "[PYGP] Called OnCaptureAreaEntered()\n" );
	}
	virtual void OnCaptureAreaExited( CGECaptureArea *pCapture, CGEPlayer *pPlayer ) { 
		Msg( "[PYGP] Called OnCaptureAreaExited()\n" );
	}

	virtual void OnTokenSpawned( CGEWeapon *pToken ) { Msg( "[PYGP] Called OnTokenSpawned()\n" ); }
	virtual void OnTokenRemoved( CGEWeapon *pToken ) { Msg( "[PYGP] Called OnTokenRemoved()\n" ); }
	virtual void OnTokenPicked( CGEWeapon *pToken, CGEPlayer *pPlayer ) { Msg( "[PYGP] Called OnTokenPicked()\n" ); }
	virtual void OnTokenDropped( CGEWeapon *pToken, CGEPlayer *pPlayer ) { Msg( "[PYGP] Called OnTokenDropped()\n" ); }
	virtual void OnTokenAttack( CGEWeapon *pToken, CGEPlayer *pPlayer, Vector position, Vector forward ) { Msg( "[PYGP] Called OnTokenAttack()\n" ); }

	virtual bool CanRoundEnd() { 
		Msg( "[PYGP] Called CanRoundEnd()\n" ); 
		return can_round_end;
	}
	virtual bool CanMatchEnd() { 
		Msg( "[PYGP] Called CanMatchEnd()\n" ); 
		return can_match_end;
	}

	virtual int GetTeamPlay() {
		return teamplay;
	}

	int teamplay;
	bool can_round_end;
	bool can_match_end;
};

class CGEGameplayTestClass : public CGEBaseGameplayManager
{
public:
	CGEGameplayTestClass() : m_Scenario(NULL) { }

	virtual CGEBaseScenario* GetScenario() { return m_Scenario; }
	CGEScenarioTest *GetTestScenario() { return m_Scenario; }

protected:
	// Internal loader for scenarios
	virtual bool DoLoadScenario( const char *ident ) {
		m_Scenario = new CGEScenarioTest();
		return true;
	}

	virtual bool IsValidScenario() { return m_Scenario != NULL; }

private:
	CGEScenarioTest *m_Scenario;
};

extern ConVar ge_roundcount;
extern ConVar ge_roundtime;
extern ConVar mp_timelimit;

class GameplayTest : public CGECommonTest {
protected:
	GameplayTest() { }
	virtual ~GameplayTest() { }

	virtual void SetUp() {
		// Store convar values
		_roundcount = ge_roundcount.GetInt();
		_roundtime = ge_roundtime.GetFloat();
		_matchtime = mp_timelimit.GetFloat();

		// Disable round count for the tests
		ge_roundcount.SetValue( -1 );

		// Set the round (30 secs) and match (5 min) timers
		SetRoundTime( 30.0f );
		SetMatchTime( 5 );

		// Create a brand new gameplay
		gameplay = new CGEGameplayTestClass();
		gameplay->Init();
	}

	virtual void TearDown() {
		// Remove our custom gameplay instance
		delete gameplay;
		gameplay = NULL;

		// Reset our timers
		ge_roundcount.SetValue( _roundcount );
		SetRoundTime( _roundtime );
		SetMatchTime( _matchtime );
	}

	void SetMatchTime( int minutes ) {
		mp_timelimit.SetValue( minutes );
	}

	void SetRoundTime( int seconds ) {
		ge_roundtime.SetValue( seconds );
	}

	void DisableRoundTime() {
		SetRoundTime( 0 );
	}

	void DisableMatchTime() {
		SetMatchTime( 0 );
	}

	int _roundcount;
	float _roundtime;
	float _matchtime;
	CGEGameplayTestClass *gameplay;
};

TEST_F( GameplayTest, StartRound ) {
	// Grab the length of the round
	int round_time = GEMPRules()->GetRoundTimeRemaining();

	// Start the round
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// Check that we are actually in the round
	EXPECT_TRUE( gameplay->IsInRound() );
	EXPECT_FALSE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, EndRound_ByCall ) {
	// Start the round
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// Force round end
	AdvanceGameTime( 1.0f );
	gameplay->EndRound();
	
	// Make sure we are in a round intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_TRUE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, EndRound_ByTime ) {
	// Grab the length of the round
	int round_time = GEMPRules()->GetRoundTimeRemaining();

	// Start the round
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// Move us past the end of the round
	AdvanceGameTime( round_time );
	gameplay->OnThink();

	// Make sure we are in a round intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_TRUE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, EndMatch_ByCall_OneRound ) {
	// Start round
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// End the match
	gameplay->EndMatch();

	// Check for final intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_FALSE( gameplay->IsInRoundIntermission() );
	EXPECT_TRUE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, EndMatch_ByCall_MultiRound ) {
	// Start round 1
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// End round 1
	gameplay->EndRound();

	// Start round 2
	AdvanceGameTime( gameplay->GetRemainingIntermission() + 1.0f );
	gameplay->OnThink();

	// End the match
	gameplay->EndMatch();

	// Check for round intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_TRUE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );

	// Exit round intermission
	AdvanceGameTime( gameplay->GetRemainingIntermission() + 1.0f );
	gameplay->OnThink();

	// Check for final intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_FALSE( gameplay->IsInRoundIntermission() );
	EXPECT_TRUE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, EndMatch_ByTime ) {
	// Grab the length of the round
	int round_time = GEMPRules()->GetRoundTimeRemaining();
	// Grab the length of the match
	int match_time = GEMPRules()->GetMatchTimeRemaining();
	
	// Start round 1
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// End round 1
	AdvanceGameTime( round_time );
	gameplay->OnThink();

	// Start round 2
	AdvanceGameTime( gameplay->GetRemainingIntermission() + 1.0f );
	gameplay->OnThink();

	// End round 2 and the match all at once
	AdvanceGameTime( match_time );
	gameplay->OnThink();

	// Check for round intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_TRUE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );

	// Exit round intermission
	AdvanceGameTime( gameplay->GetRemainingIntermission() + 1.0f );
	gameplay->OnThink();

	// Check for final intermission
	EXPECT_FALSE( gameplay->IsInRound() );
	EXPECT_FALSE( gameplay->IsInRoundIntermission() );
	EXPECT_TRUE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, CanEndRound_ByScenario ) {
	// Disable round ending
	gameplay->GetTestScenario()->can_round_end = false;
	
	// Grab the length of the round
	int round_time = GEMPRules()->GetRoundTimeRemaining();
	
	// Start the round
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// Move us past the end of the round
	AdvanceGameTime( round_time );
	gameplay->OnThink();

	// Check still in round
	EXPECT_TRUE( gameplay->IsInRound() );
	EXPECT_FALSE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );
}

TEST_F( GameplayTest, CanEndMatch_ByScenario ) {
	// Disable match ending
	gameplay->GetTestScenario()->can_match_end = false;
	
	// Grab the length of the match
	int match_time = GEMPRules()->GetMatchTimeRemaining();
	
	// Start the round
	AdvanceGameTime( 1.0f );
	gameplay->OnThink();

	// Move us past the end of the match
	AdvanceGameTime( match_time );
	gameplay->OnThink();

	// Move us past round intermission (should start a new round)
	AdvanceGameTime( gameplay->GetRemainingIntermission() + 1.0f );
	gameplay->OnThink();

	// Check still in round (ie, the match did not end)
	EXPECT_TRUE( gameplay->IsInRound() );
	EXPECT_FALSE( gameplay->IsInRoundIntermission() );
	EXPECT_FALSE( gameplay->IsInFinalIntermission() );
}

// TEST: CanEndRound_ByTimer (Disable round timer)
// TEST: CanEndMatch_ByTimer (Disable match timer)
// TEST: RoundLocked
// TEST: Scenario loading (invalid identity, shutdown/init, etc)
