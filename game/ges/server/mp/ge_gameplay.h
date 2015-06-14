///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : Server
//   File        : ge_gameplay.h
//   Description :
//      [TODO: Write the purpose of ge_gameplay.h.]
//
//   Created On: 8/31/2009 9:37:45 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef MC_GE_GAMEPLAY_H
#define MC_GE_GAMEPLAY_H
#ifdef _WIN32
#pragma once
#endif

class CGEPlayer;
class CBaseEntity;
class CGEWeapon;
class CGECaptureArea;

enum GES_GAMESTATE
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

	virtual bool SetGamePlay( const char* szIdent )=0;
	virtual CGEBaseScenario* GetScenario( void )=0;

	virtual void Init();
	virtual void Shutdown();

	virtual void BroadcastGamePlay( void );
	virtual void BroadcastRoundStart( void );
	virtual void BroadcastRoundEnd( void );

	virtual void SetIntermission(bool state)	{ m_bRoundIntermission = state; }
	virtual bool IsInRoundIntermission( void )	{ return m_bRoundIntermission; }
	virtual float GetRemainingIntermission( void );

	void SetFirstLoad( bool state )			{ m_bFirstLoad = state; }
	
	void SetState(GES_GAMESTATE iState, bool forcenow = false);
	bool IsInState( GES_GAMESTATE iState )	{ return iState == m_iGameState; }
	GES_GAMESTATE GetState()				{ return m_iGameState; }
	
	void SetRoundLocked( bool state )	{ m_bRoundLocked = state; }
	bool IsRoundLocked()				{ return m_bRoundLocked; }
	bool IsRoundStarted()				{ return m_bInRound; }
	int  GetNumRounds()					{ return m_iRoundCount; }

	void DisableRoundScoring()			{ m_bDoRoundScores = false; }

	void LoadGamePlayList( const char* path );
	void PrintGamePlayList();
	// Merely verifies that the python file exists
	bool IsValidGamePlay( const char *ident );

	// Gameplay cycle management
	void OnLoadGamePlay( void );
	void OnUnloadGamePlay( void );
	void OnThink();

	void PreRoundBegin();
	void PostRoundBegin();

	// Event management
	static void ClearEventListeners();

	// Flow control
	bool SetGamePlayOrdered();
	bool SetGamePlayRandom();

protected:
	CGEBaseGameplayManager( const CGEBaseGameplayManager& ) { };

	void LoadGamePlayCycle();
	void ResetGameState( void );
	bool ShouldEndRound( void );

	float m_flNextThink;

	bool m_bFirstLoad;
	bool m_bInReload;
	bool m_bRoundIntermission;
	bool m_bDoRoundScores;
	bool m_bRoundLocked;
	bool m_bInRound;
	int	 m_iRoundCount;
	float m_flRoundStart;

	GES_GAMESTATE m_iGameState;

	CUtlVector<char*> m_vGamePlayList;
	CUtlVector<char*> m_vCycleList;
};

extern CGEBaseGameplayManager* GEGameplay();

#endif //MC_GE_GAMEPLAY_H
