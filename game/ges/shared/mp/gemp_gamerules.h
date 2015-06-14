///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: gemp_gamerules.h
// Description:
//      GoldenEye game rules Multiplayer version
//
// Created On: 28 Feb 08
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////

#ifndef GEMP_GAMERULES_H
#define GEMP_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "ge_gamerules.h"
#include "ge_round_timer.h"

#ifdef GAME_DLL
	#include "ge_tokenmanager.h"
	#include "ge_loadoutmanager.h"
#endif

// Teamplay desirability
enum GES_TEAMPLAY
{
	TEAMPLAY_NOT = 0,
	TEAMPLAY_ONLY,
	TEAMPLAY_TOGGLE
};

#ifdef CLIENT_DLL
	#define CGEMPRules			C_GEMPRules
	#define CGEMPGameRulesProxy C_GEMPGameRulesProxy
#endif

class CGEMPGameRulesProxy;

class CGEMPRules : public CGERules, public CGEGameplayEventListener
{
public:
	CGEMPRules();
	~CGEMPRules();

	DECLARE_CLASS( CGEMPRules, CGERules );

#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.
#else
	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif

	// ---------------
	// GES Functions
#ifdef GAME_DLL
	// ---------------
	// CGEGameplayEventListener
	virtual void OnMatchStarted();
	virtual void OnRoundRestart();
	virtual void OnRoundStarted();

	// This is used to call functions right before the client spawns into the server
	virtual void ClientActive( CBasePlayer *pPlayer );
	virtual void CalculateCustomDamage( CBasePlayer *pPlayer, CTakeDamageInfo &info, float &health, float &armor );

	bool InRoundRestart();

	float FlArmorRespawnTime( CItem *pItem );

	bool AmmoShouldRespawn();
	bool ArmorShouldRespawn();
	bool ItemShouldRespawn( const char *name );
	bool WeaponShouldRespawn( const char *name );

	CGELoadoutManager *GetLoadoutManager()  { return m_pLoadoutManager; }
	CGETokenManager   *GetTokenManager()	{ return m_pTokenManager;	}

	int GetSpawnPointType( CGEPlayer *pPlayer );
	float GetSpeedMultiplier( CGEPlayer *pPlayer );

	int GetNumActivePlayers();
	int GetNumAlivePlayers();
	int GetNumInRoundPlayers();

	int GetRoundWinner();
	int GetRoundTeamWinner();
	void SetRoundWinner( int winner )		{ m_iPlayerWinner = winner; }
	void SetRoundTeamWinner( int winner )	{ m_iTeamWinner = winner; }

	void ResetPlayerScores( bool resetmatch = false );
	void ResetTeamScores( bool resetmatch = false );

	void BalanceTeams();
	void EnforceTeamplay();
	void EnforceBotCount();
	void RemoveWeaponsFromWorld( const char *szClassName = NULL );

	void SetWeaponSpawnState( bool state )	{ m_bEnableWeaponSpawns = state; }
	void SetAmmoSpawnState( bool state )	{ m_bEnableAmmoSpawns = state; }
	void SetArmorSpawnState( bool state )	{ m_bEnableArmorSpawns = state; }
	void SetIntermission( bool state )		{ m_bInIntermission = state; }

	int  GetRoundTime()				{ return m_iRoundTime; }
	void SetRoundTime( int time_secs );

	void PauseRoundTimer()			{ m_hRoundTimer->PauseTimer(); }
	void ResumeRoundTimer()			{ m_hRoundTimer->ResumeTimer(); }
	void ResetRoundTimer()			{ m_hRoundTimer->SetTimeRemaining( m_iRoundTime ); }

	// These accessors control the use of team spawns (if available and team play enabled)
	bool IsTeamSpawn()				{ return m_bUseTeamSpawns; }
	bool IsTeamSpawnSwapped()		{ return m_bSwappedTeamSpawns; }
	void SwapTeamSpawns()			{ m_bSwappedTeamSpawns = !m_bSwappedTeamSpawns; }
	void SetTeamSpawn( bool state )	{ m_bUseTeamSpawns = state; }

	// Used by Python to set the teamplay mode
	void SetTeamplay( bool state, bool force = false );
	void SetTeamplayMode( int mode ) { m_iTeamplayMode = mode; }

	// Static functions for bots
	static bool CreateBot();
	static bool RemoveBot( const char *name );
#endif

	// -------------------
	// Client-Server Shared
public:
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	float GetMapTimeLeft();
	float GetRoundTimeLeft();
	bool  IsRoundTimePaused();
	bool  IsPlayingRounds();
	int   GetTeamplayMode() { return m_iTeamplayMode; }

	bool IsMultiplayer() { return true; }
	bool IsTeamplay();
	bool IsIntermission();

	// -------------------
	// Server Only
#ifdef GAME_DLL
	virtual void ClientDisconnected( edict_t *pClient );
	virtual void CreateStandardEntities();
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );

	virtual float FlItemRespawnTime( CItem *pItem );
	virtual float FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon );
	virtual bool  FPlayerCanRespawn( CBasePlayer *pPlayer );

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );
	virtual const char *GetGameDescription();
	virtual void		GetTaggedConVarList( KeyValues *pCvarTagList );

	virtual void HandleTimeLimitChange( void );

	virtual void ChangeLevel();
	virtual void SetupChangeLevel( const char *levelname );
	virtual void GoToIntermission( bool forcematchend = false );

	virtual bool OnPlayerSay(CBasePlayer* player, const char* text);
	virtual void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	
	// Used to initialize and shutdown Python
	virtual void LevelInitPreEntity();
	virtual void LevelShutdownPreEntity();

	virtual void FrameUpdatePreEntityThink();
	virtual void Think();

	// Make sure we don't affect the "frag" count (fav weapon)
	bool UseSuicidePenalty() { return false; }
	
	virtual Vector	VecItemRespawnSpot( CItem *pItem );
	virtual QAngle	VecItemRespawnAngles( CItem *pItem );
#endif


private:

#ifdef GAME_DLL
	CGELoadoutManager	*m_pLoadoutManager;
	CGETokenManager		*m_pTokenManager;

	ConVar				*m_pAllTalkVar;

	bool m_bEnableAmmoSpawns;
	bool m_bEnableWeaponSpawns;
	bool m_bEnableArmorSpawns;

	float m_flIntermissionEndTime;
	float m_flNextTeamBalance;
	float m_flNextIntermissionCheck;
	float m_flChangeLevelTime;

	float m_flNextBotCheck;
	CUtlVector<EHANDLE> m_vBotList;

	char  m_szNextLevel[64];
	char  m_szGameDesc[32];

	bool  m_bUseTeamSpawns;
	bool  m_bSwappedTeamSpawns;
	bool  m_bInTeamBalance;

	int	  m_iPlayerWinner;
	int   m_iTeamWinner;

	// Per frame update variables
	// -- updates on first call to the count functions per frame
	// Player counts 
	int   m_iNumAlivePlayers;
	int   m_iNumActivePlayers;
	int   m_iNumInRoundPlayers;
#endif

	CNetworkVar( bool,	m_bTeamPlayDesired );
	CNetworkVar( int,	m_iRoundTime );
	CNetworkVar( int,	m_iTeamplayMode );
	CNetworkVar( bool,	m_bInIntermission );

	CNetworkHandle( CGERoundTimer, m_hRoundTimer );
};


class CGEMPGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CGEMPGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

extern int g_iLastPlayerCount;

inline CGEMPRules* GEMPRules()
{
	return (CGEMPRules*) g_pGameRules;
}

#endif //MC_GEMP_GAMERULES_H
