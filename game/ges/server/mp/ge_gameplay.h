///////////// Copyright © 2013, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_gameplay.h
// Description:
//      Gameplay Manager Definition
//
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////

#ifndef MC_GE_GAMEPLAY_H
#define MC_GE_GAMEPLAY_H
#ifdef _WIN32
#pragma once
#endif

class CGEPlayer;
class CBaseEntity;
class CGEWeapon;
class CGECaptureArea;

enum GEGameState
{
	GAMESTATE_NONE,
	GAMESTATE_STARTING,
	GAMESTATE_RESTART,
	GAMESTATE_DELAY,
	GAMESTATE_PLAYING,
};

// --------------------
// Creation, Shutdown, Reboot
// --------------------
extern void CreateGameplayManager();
extern void ShutdownGameplayManager();


// --------------------
// Base Scenario Linker
// --------------------

class CGEBaseScenario
{
public:
	CGEBaseScenario();

	void LoadConfig();
	void CreateCVar(const char* name, const char* defValue, const char* help);
	void SetIsOfficial(bool state)	{ m_bIsOfficial = state; };
	bool IsOfficial( void )			{ return m_bIsOfficial; };

	virtual void Cleanup();

	virtual const char* GetIdent()=0;
	virtual const char* GetGameDescription()=0;
	virtual const char* GetPrintName()=0;
	virtual int GetHelpString()=0;
	virtual int GetTeamPlay()=0;

	virtual void OnLoadGamePlay()=0;
	virtual void OnUnloadGamePlay()=0;

	virtual void ClientConnect(CGEPlayer *pPlayer)=0;
	virtual void ClientDisconnect(CGEPlayer *pPlayer)=0;

	virtual void OnPlayerSpawn(CGEPlayer *pPlayer)=0;
	virtual void OnPlayerObserver(CGEPlayer *pPlayer)=0;
	virtual void OnPlayerKilled(CGEPlayer *pVictim, CGEPlayer *pKiller, CBaseEntity *pWeapon)=0;
	virtual bool OnPlayerSay(CGEPlayer* pPlayer, const char* text)=0;

	virtual bool CanPlayerRespawn(CGEPlayer *pPlayer)=0;
	virtual bool CanPlayerChangeChar(CGEPlayer* pPlayer, const char* szIdent)=0;
	virtual bool CanPlayerChangeTeam(CGEPlayer *pPlayer, int iOldTeam, int iNewTeam)=0;
	virtual bool CanPlayerHaveItem(CGEPlayer *pPlayer, CBaseEntity *pEntity)=0;
	virtual bool ShouldForcePickup(CGEPlayer *pPlayer, CBaseEntity *pEntity)=0;
	virtual void CalculateCustomDamage(CGEPlayer *pVictim, const CTakeDamageInfo &inputInfo, float &health, float &armor)=0;

	virtual void OnRoundBegin()=0;
	virtual void OnRoundEnd()=0;
	virtual void OnThink()=0;
	virtual void OnCVarChanged(const char* name, const char* oldvalue, const char* newvalue)=0;

	virtual bool CanRoundEnd()=0;
	virtual bool CanMatchEnd()=0;

	virtual void OnCaptureAreaSpawned(CGECaptureArea *pCapture)=0;
	virtual void OnCaptureAreaRemoved(CGECaptureArea *pCapture)=0;
	virtual void OnCaptureAreaEntered(CGECaptureArea *pCapture, CGEPlayer *pPlayer, CGEWeapon *pToken)=0;
	virtual void OnCaptureAreaExited(CGECaptureArea *pCapture, CGEPlayer *pPlayer)=0;

	virtual void OnTokenSpawned(CGEWeapon *pToken)=0;
	virtual void OnTokenRemoved(CGEWeapon *pToken)=0;
	virtual void OnTokenPicked(CGEWeapon *pToken, CGEPlayer *pPlayer)=0;
	virtual void OnTokenDropped(CGEWeapon *pToken, CGEPlayer *pPlayer)=0;
	virtual void OnTokenAttack(CGEWeapon *pToken, CGEPlayer *pPlayer, Vector position, Vector forward)=0;

private:
	bool m_bIsOfficial;
	CUtlVector<ConVar*> m_vCVarList;

	CGEBaseScenario(const CGEBaseScenario &) { }
};


// Class that enables event based messaging of gameplay events
// to all registered listeners. Listeners are registered automatically
// in the CGEGameplayEventListener constructor
class CGEGameplayEventListener
{
public:
	CGEGameplayEventListener();
	~CGEGameplayEventListener();

	// Replace these with inherited versions!
	virtual void OnGameplayLoaded()		{ }		// When gameplay is loaded
	virtual void OnMatchStarted()		{ }		// Before first round of current gameplay on current level
	virtual void OnRoundRestart()		{ }		// Called after the world respawns, but before players
	virtual void OnRoundStarted()		{ }		// After players are spawned in the new round
	virtual void OnRoundEnded()			{ }		// After the round timer ends
	virtual void OnMatchEnded()			{ }		// After the last round's intermission time is over
	virtual void OnGameplayUnloaded()	{ }		// When gameplay is unloaded
};


class CGEBaseGameplayManager
{
public:
	CGEBaseGameplayManager();
	~CGEBaseGameplayManager();

// Abstract methods to access Python Manager implementation
protected:
	// Internal loader for scenarios
	virtual bool DoLoadScenario( const char *ident )=0;
public:
	virtual CGEBaseScenario* GetScenario()=0;
// End Abstract
	
	virtual void Init();
	virtual void Shutdown();

	// Loads the next scenario to play
	bool LoadScenario();
	bool LoadScenario( const char *ident );
	
	void SetIntermission( bool state )	{ m_bRoundIntermission = state; }
	bool IsInRoundIntermission()		{ return m_bRoundIntermission; }
	float GetRemainingIntermission();

	// Check to see if we should end the round or match
	bool CanEndRound()					{ return m_bCanEndRoundCache; }
	bool CanEndMatch()					{ return m_bCanEndMatchCache; }
	bool IsMatchEndBlocked()			{ return m_bMatchBlockedByScenario; }
	
	// Controls for the round (does not check conditions)
	void StartRound();
	void EndRound( bool showreport = true );

	// Controls for the match (does not check conditions)
	void StartMatch();
	void EndMatch();
	
	GEGameState GetState()				{ return m_iGameState; }
	
	void SetRoundLocked( bool state )	{ m_bRoundLocked = state; }
	bool IsRoundLocked()				{ return m_bRoundLocked; }
	bool IsRoundStarted()				{ return m_bInRound; }
	int  GetRoundCount()				{ return m_iRoundCount; }

	void LoadGamePlayList( const char* path );
	void PrintGamePlayList();
	// Merely verifies that the python file exists
	bool IsValidGamePlay( const char *ident );

	// Gameplay cycle management
	void OnLoadGamePlay();
	void OnUnloadGamePlay();
	void OnThink();
	
	// Flow control
	bool SetScenarioOrdered();
	bool SetScenarioRandom();

protected:
	void BroadcastGamePlay();
	void BroadcastRoundStart();
	void BroadcastRoundEnd( bool showreport );
	
	// Return the next scenario to load
	const char *GetNextScenario();

	

	void CalculatePlayerScores();

private:
	void SetState( GEGameState state, bool forcenow = false );
	const char *GetStateName( GEGameState state );

	void CalcCanEndRound();
	void CalcCanEndMatch();

	void LoadScenarioCycle();
	void ResetGameState();

	// Time of next think cycle
	float m_flNextThink;
	
	// Status variables
	bool m_bRoundIntermission;
	bool m_bRoundLocked;
	bool m_bInRound;
	int	 m_iRoundCount;
	float m_flRoundStart;

	// Round and Match end cache
	bool m_bCanEndRoundCache;
	bool m_bCanEndMatchCache;
	bool m_bMatchBlockedByScenario;

	GEGameState m_iGameState;

	CUtlVector<char*> m_vScenarioList;
	CUtlVector<char*> m_vScenarioCycle;

protected:
	// To satisfy Boost::Python requirements of a wrapper
	CGEBaseGameplayManager( const CGEBaseGameplayManager& ) { };
};

extern CGEBaseGameplayManager* GEGameplay();
extern CGEBaseScenario *GetScenario();

#endif //MC_GE_GAMEPLAY_H
