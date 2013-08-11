///////////// Copyright © 2013, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_gameplay.cpp
// Description:
//      Gameplay Manager Definition
//
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"

#include "ge_gameplay.h"
#include "gemp_player.h"
#include "gemp_gamerules.h"
#include "ge_utils.h"

#include "ge_playerresource.h"
#include "ge_radarresource.h"
#include "ge_gameplayresource.h"
#include "ge_tokenmanager.h"

#include "team.h"
#include "script_parser.h"
#include "filesystem.h"
#include "viewport_panel_names.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool g_bInGameplayReload;

CGEBaseGameplayManager *g_GamePlay = NULL;
CGEBaseGameplayManager *GEGameplay()
{
	return g_GamePlay;
}

CGEBaseScenario *GetScenario()
{
	assert( g_GamePlay );
	return g_GamePlay->GetScenario();
}

void GEGameplay_Callback( IConVar *var, const char *pOldString, float flOldValue )
{
	if ( !GEMPRules() && !g_bInGameplayReload )
		return;

	ConVar *cVar = static_cast<ConVar*>(var);
	GEGameplay()->LoadScenario( cVar->GetString() );
}

void GEGPCVar_Callback( IConVar *var, const char *pOldString, float flOldValue )
{
	if ( !GEGameplay() || !GEGameplay()->GetScenario() )
		return;

	ConVar *cVar = static_cast<ConVar*>(var);
	GEGameplay()->GetScenario()->OnCVarChanged( cVar->GetName(), pOldString, cVar->GetString() );
}

ConVar ge_gp_cyclefile( "ge_gp_cyclefile", "gameplaycycle.txt", FCVAR_GAMEDLL, "The gameplay cycle to use for random gameplay or ordered gameplay" );
ConVar ge_autoteam( "ge_autoteam", "0", FCVAR_REPLICATED|FCVAR_NOTIFY, "Automatically toggles teamplay based on the player count (supplied value) [4-32]",  true, 0, true, MAX_PLAYERS );

ConVar ge_gameplay_mode( "ge_gameplay_mode", "0", FCVAR_GAMEDLL, "Mode to choose next gameplay: \n\t0=Same as last map, \n\t1=Random from Gameplay Cycle file, \n\t2=Ordered from Gameplay Cycle file" );
ConVar ge_gameplay( "ge_gameplay", "DeathMatch", FCVAR_GAMEDLL, "Sets the current gameplay mode.\nDefault is 'deathmatch'", GEGameplay_Callback );

extern ConVar ge_rounddelay;
extern ConVar ge_teamplay;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CGEBaseScenario::CGEBaseScenario()
{
	m_bIsOfficial = false;
}

void CGEBaseScenario::LoadConfig()
{
	char szCommand[255];

	Msg( "Executing gamemode [%s.cfg] config file\n", GetIdent() );
	Q_snprintf( szCommand,sizeof(szCommand), "exec %s.cfg\n", GetIdent() );
	engine->ServerCommand( szCommand );
}

void CGEBaseScenario::CreateCVar( const char* name, const char* defValue, const char* help )
{
	if ( !name || !defValue || !help )
	{
		Warning( "[GES GP] Failed to create ConVar due to invalid parameters!\n" );
		return;
	}

	ConVar *var = g_pCVar->FindVar( name );
	if ( var )
	{
		Warning( "[GES GP] Attempting to create CVAR %s twice!\n", name );
		m_vCVarList.FindAndRemove( var );
		g_pCVar->UnregisterConCommand( var );
	}

	// Create the CVar (registers it) and set the value to invoke OnCVarChanged
	var = new ConVar( name, defValue, FCVAR_GAMEDLL|FCVAR_NOTIFY, help, GEGPCVar_Callback );
	var->SetValue( defValue );
	m_vCVarList.AddToTail( var );
}

void CGEBaseScenario::Cleanup()
{	
	// Unregister each ConVar we are tracking
	for( int i=0; i < m_vCVarList.Size(); i++ )
		g_pCVar->UnregisterConCommand( m_vCVarList[i] );

	m_vCVarList.PurgeAndDeleteElements();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Main container for GP Event Listeners
CUtlVector<CGEGameplayEventListener*> g_vGPEventListeners;

// Convienance functions
#ifdef _DEBUG
#define GP_EVENT( func ) \
	for ( int i=0; i < g_vGPEventListeners.Count(); i++ ) { \
		g_vGPEventListeners[i]->func(); \
	}
#else
#define GP_EVENT( func ) \
	for ( int i=0; i < g_vGPEventListeners.Count(); i++ ) { \
		try { \
			g_vGPEventListeners[i]->func(); \
		} catch (...) { \
			DevWarning( "[GPEventListener] Event called on invalid listener!\n" ); \
			g_vGPEventListeners.Remove( i-- ); \
		} \
	}
#endif

CGEGameplayEventListener::CGEGameplayEventListener()
{
	if ( !g_vGPEventListeners.HasElement( this ) )
	{
		g_vGPEventListeners.AddToTail( this );
		return;
	}

	DevWarning( "[GPEventListener] Event listener attempted double registration!\n" );
}

CGEGameplayEventListener::~CGEGameplayEventListener()
{
	int idx = g_vGPEventListeners.Find( this );
	if ( idx != -1 )
		g_vGPEventListeners.Remove( idx );
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int g_iScenarioIndex = -1;

CGEBaseGameplayManager::CGEBaseGameplayManager()
{
	m_iGameState = GAMESTATE_NONE;
	m_flNextThink = 0;
}

CGEBaseGameplayManager::~CGEBaseGameplayManager()
{
	m_vScenarioCycle.PurgeAndDeleteElements();
	m_vScenarioList.PurgeAndDeleteElements();
}

void CGEBaseGameplayManager::Init()
{
	m_bInRound = false;
	m_flRoundStart = 0;
	m_flNextThink = 0;

	m_bCanEndRoundCache = false;
	m_bCanEndMatchCache = false;
	m_bMatchBlockedByScenario = false;

	LoadScenarioCycle();
	
	LoadScenario();
}

void CGEBaseGameplayManager::Shutdown()
{
	// TODO: Shouldn't this be "UnloadScenario()"??
	// Unload the scenario
	OnUnloadGamePlay();
}

void CGEBaseGameplayManager::BroadcastGamePlay( void )
{
	// Let all the clients know what game mode we are in now
	IGameEvent* pEvent = gameeventmanager->CreateEvent("gamemode_change");
	if ( pEvent )
	{
		pEvent->SetString( "ident", GetScenario()->GetIdent() );
		pEvent->SetBool( "official", GetScenario()->IsOfficial() );
		gameeventmanager->FireEvent(pEvent);
	}
}

void CGEBaseGameplayManager::BroadcastRoundStart( void )
{
	// Let all the clients know that the round started
	IGameEvent* pEvent = gameeventmanager->CreateEvent("round_start");
	if ( pEvent )
	{
		pEvent->SetString( "gameplay", GetScenario()->GetIdent() );
		pEvent->SetBool( "teamplay", GEMPRules()->IsTeamplay() );
		pEvent->SetInt( "roundcount", m_iRoundCount );
		gameeventmanager->FireEvent(pEvent);
	}
}

void CGEBaseGameplayManager::BroadcastRoundEnd( bool showreport )
{
	// Let all the clients know that the round ended
	IGameEvent* pEvent = gameeventmanager->CreateEvent("round_end");
	if ( pEvent )
	{
		pEvent->SetInt( "winnerid", GEMPRules()->GetRoundWinner() );
		pEvent->SetInt( "teamid", GEMPRules()->GetRoundTeamWinner() );
		pEvent->SetBool( "isfinal", g_fGameOver );
		pEvent->SetBool( "showreport", showreport );
		pEvent->SetInt( "roundlength", gpGlobals->curtime - m_flRoundStart );
		pEvent->SetInt( "roundcount", m_iRoundCount );
		GEStats()->SetAwardsInEvent( pEvent );
		gameeventmanager->FireEvent(pEvent);
	}
}

bool CGEBaseGameplayManager::LoadScenario()
{
	return LoadScenario( GetNextScenario() );
}

bool CGEBaseGameplayManager::LoadScenario( const char *ident )
{
	// TODO: Maybe add robust fail checks here
	if ( !DoLoadScenario( ident ) )
		return false;

	// Set up the world and notify Python
	OnLoadGamePlay();

	// Start the match
	StartMatch();

	return true;
}

const char *CGEBaseGameplayManager::GetNextScenario()
{
	int mode = ge_gameplay_mode.GetInt();
	int count = m_vScenarioCycle.Count();

	if ( mode == 1 )
	{
		// Random game mode, find a scenario in our list to load
		if ( count > 0 )
		{
			int cur = g_iScenarioIndex;
			do {
				g_iScenarioIndex = GERandom<int>( count );
			} while ( count > 1 && g_iScenarioIndex != cur );

			return m_vScenarioCycle[ g_iScenarioIndex ];
		}
	}
	else if ( mode == 2 )
	{
		// Ordered game mode, get the next scenario in our list
		if ( count > 0 )
		{
			// Increment our index, rolling over if we exceed our count
			if ( ++g_iScenarioIndex  >= count )
				g_iScenarioIndex = 0;

			return m_vScenarioCycle[ g_iScenarioIndex ];
		}
	}

	// Static game mode, load the scenario set in our ConVar
	return ge_gameplay.GetString();
}

void CGEBaseGameplayManager::OnLoadGamePlay( void )
{
	ResetGameState();

	// Let everyone know what we are playing
	UTIL_ClientPrintAll( HUD_PRINTTALK, "#GES_Gameplay_Changed", GetScenario()->GetPrintName() );

	// Allow us to precache stuff in OnLoadGameplay
	bool precache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Call into the Python OnLoadGamePlay
	GetScenario()->OnLoadGamePlay();
	GetScenario()->LoadConfig();

	CBaseEntity::SetAllowPrecache( precache );

	// Call our listeners
	GP_EVENT( OnGameplayLoaded );

	// Call into reconnect each player again to run initializations
	FOR_EACH_MPPLAYER(i, pPlayer)		
		GetScenario()->ClientConnect(pPlayer);
		pPlayer->SetInitialSpawn( true );
		pPlayer->HintMessage( "", 0 );
	END_OF_PLAYER_LOOP()
}

void CGEBaseGameplayManager::OnUnloadGamePlay( void )
{
	// Remove any tokens we may have had
	if ( GEMPRules() && GEMPRules()->GetTokenManager() )
		GEMPRules()->GetTokenManager()->Reset();

	FOR_EACH_PLAYER(i, pPlayer)		
		GetScenario()->ClientDisconnect( pPlayer );	
	END_OF_PLAYER_LOOP()
		
	GetScenario()->OnUnloadGamePlay();

	GP_EVENT( OnGameplayUnloaded );

	GetScenario()->Cleanup();
}

void CGEBaseGameplayManager::OnThink()
{
	// If we aren't ready to think don't do it
	if( m_flNextThink > gpGlobals->curtime )
		return;

	switch ( GetState() )
	{
	case GAMESTATE_NONE:
		m_flNextThink = gpGlobals->curtime + 0.5f;
		return;
	
	case GAMESTATE_RESTART:
		// If map time will run out within 30 seconds, ignore a round restart
		if ( mp_timelimit.GetBool() && GEMPRules()->GetMatchTimeRemaining() <= 30.0f && GetScenario()->CanMatchEnd() )
		{
			Msg( "[MATCH END] Aborting round restart due to insufficient map time, %0.2f\n", gpGlobals->curtime );
			// TODO: this arrangement is really awkward...
			EndMatch();
			SetState( GAMESTATE_NONE );
			return;
		}

		if ( m_iRoundCount == 0 )
			StartMatch();

		StartRound();
		
		SetState( GAMESTATE_PLAYING );
		break;

	case GAMESTATE_DELAY:
		if ( g_fGameOver )
			EndMatch();
		else
			EndRound();

		SetState( GAMESTATE_RESTART );
		return;

	case GAMESTATE_PLAYING:
		CalcCanEndRound();
		CalcCanEndMatch();

		// Check if we should end the round
		if ( CanEndRound() )
			SetState( GAMESTATE_DELAY );
		else
			GetScenario()->OnThink();
		break;
	}

	m_flNextThink = gpGlobals->curtime + 0.1f;
}

void CGEBaseGameplayManager::StartMatch()
{
	// Reset
	m_bMatchBlockedByScenario = false;

	// Let our listeners know we are starting a new match
	GP_EVENT( OnMatchStarted );

	// Let our clients know what game mode we are playing
	BroadcastGamePlay();

	// Show scenario help
	FOR_EACH_MPPLAYER( i, pPlayer )
		pPlayer->ShowScenarioHelp();
	END_OF_PLAYER_LOOP()
}

void CGEBaseGameplayManager::StartRound()
{
	// Increment our round count
	m_iRoundCount++;

	m_flRoundStart = gpGlobals->curtime;
	m_bRoundIntermission = false;

	// TODO: Shouldn't this be in game rules???
	// Set teamplay here so we can show a proper round report if we go from team to non-team gameplay
	GEMPRules()->SetTeamplayMode( GetScenario()->GetTeamPlay() );

	if ( ge_autoteam.GetInt() > 0 )
	{
		// If we had 1 or more people than ge_autoteam on the last map, enforce teamplay immediately
		// If we have the requisite active players, enforce teamplay
		// If we don't, and ge_teamplay > 1 (ie not set by the player), then deactivate teamplay
		if ( g_iLastPlayerCount >= (ge_autoteam.GetInt() + 1) || GEMPRules()->GetNumActivePlayers() >= ge_autoteam.GetInt() )
			ge_teamplay.SetValue(2);
		else if ( ge_teamplay.GetInt() > 1 )
			ge_teamplay.SetValue(0);
	}

	GEMPRules()->SetRoundWinner( 0 );
	GEMPRules()->SetRoundTeamWinner( TEAM_UNASSIGNED );

	FOR_EACH_PLAYER(i, pPlayer)
		pPlayer->SetScoreBoardColor( SB_COLOR_NORMAL );
	END_OF_PLAYER_LOOP()

	// Reload the world sparing only level designer placed entities
	// Keep this here to ensure it happens BEFORE OnRoundRestart
	GEMPRules()->WorldReload();

	GP_EVENT( OnRoundRestart );

	UTIL_ClientPrintAll( HUD_PRINTTALK, "#GES_RoundRestart" );

	BroadcastRoundStart();

	// Call into our gameplay "OnRoundBegin"
	GetScenario()->OnRoundBegin();
	
	// Keep this here to ensure it happens BEFORE OnRoundStarted
	GEMPRules()->SpawnPlayers();

	m_bInRound = true;

	GP_EVENT( OnRoundStarted );
}

void CGEBaseGameplayManager::EndRound( bool showreport /*=true*/ )
{
	// Call into python to do post round cleanup and score setting
	GetScenario()->OnRoundEnd();

	// Freeze players
	FOR_EACH_MPPLAYER(i, pPlayer)
		pPlayer->FreezePlayer();
	END_OF_PLAYER_LOOP()

	// Calculate the player's favorite weapons and set them
	GEStats()->SetFavoriteWeapons();

	// Calculate the scores for the active players
	CalculatePlayerScores();

	// Tell players we finished the round
	BroadcastRoundEnd( showreport );

	GP_EVENT( OnRoundEnded );
	UTIL_ClientPrintAll( HUD_PRINTTALK, "#GES_RoundEnd" );

	m_bRoundIntermission = true;
	// TODO: This should be OBE'd
	GEMPRules()->SetIntermission(true);

	if ( showreport )
		// Delay enough so that the scores don't get reset before the round report is visible
		m_flNextThink = gpGlobals->curtime + max( ge_rounddelay.GetInt(), 0.5f );
	else
		// Only give 3 second delay if we didn't count this round
		m_flNextThink = gpGlobals->curtime + 3.0f;

	Msg( "[ROUND END] We are playing rounds, the round has ended, %0.2f\n", gpGlobals->curtime );
}

void CGEBaseGameplayManager::EndMatch()
{
	// TODO: This needs to go away!
	// Tell the GameRules to go into intermission
	GEMPRules()->GoToIntermission();

	// Call into python to do post round cleanup and score setting
	GetScenario()->OnRoundEnd();

	// Freeze players
	FOR_EACH_MPPLAYER(i, pPlayer)
		pPlayer->FreezePlayer();
	END_OF_PLAYER_LOOP()

	// Calculate the player's favorite weapons and set them
	GEStats()->SetFavoriteWeapons();

	// Calculate the scores for the active players
	// TODO: We only calculate the scores if we never did it in a round report
	CalculatePlayerScores();

	// Tell clients we finished the round
	BroadcastRoundEnd( true );

	GP_EVENT( OnMatchEnded );
	UTIL_ClientPrintAll( HUD_PRINTTALK, "#GES_MatchEnd" );

	m_bRoundIntermission = true;
	// TODO: Do we need to differentiate the intermissions??
	// TODO: This should be OBE'd
	GEMPRules()->SetIntermission(true);

	Msg( "[MATCH END] The match has ended, %0.2f\n", gpGlobals->curtime );
}

void CGEBaseGameplayManager::CalculatePlayerScores()
{
	FOR_EACH_MPPLAYER(i, pPlayer)
		// HACK HACK: Fake a kill so we can record their inning time and weapon held time
		GEStats()->Event_PlayerKilled( pPlayer, CTakeDamageInfo() );

		// Add the player's round scores to their match scores
		pPlayer->AddMatchScore( pPlayer->GetRoundScore() );
		pPlayer->AddMatchDeaths( pPlayer->DeathCount() );
	END_OF_PLAYER_LOOP()

	// Add the team round scores to the match scores
	for ( int i = FIRST_GAME_TEAM; i < MAX_GE_TEAMS; i++ )
	{
		CTeam *team = GetGlobalTeam( i );
		if ( team )
			team->AddMatchScore( team->GetRoundScore() );
	}

	// Make sure we capture the latest scores and send them to the clients
	if ( g_pPlayerResource )
		g_pPlayerResource->UpdatePlayerData();
}

float CGEBaseGameplayManager::GetRemainingIntermission()
{
	return max( m_flNextThink - gpGlobals->curtime, 0 );
}

// -----------------------------------------------
// RESET GAME STATE - Called when a new game mode is loaded to make sure we 
//   start with unchanged Python settings
void CGEBaseGameplayManager::ResetGameState( void )
{
	FOR_EACH_PLAYER(i, pPlayer)
		pPlayer->SetDamageMultiplier( 1.0f );
		pPlayer->SetSpeedMultiplier( 1.0f );
		pPlayer->SetMaxHealth( MAX_HEALTH );
		pPlayer->SetMaxArmor( MAX_ARMOR );
		pPlayer->SetScoreBoardColor( SB_COLOR_NORMAL );
	END_OF_PLAYER_LOOP()

	GEMPRules()->SetAmmoSpawnState( true );
	GEMPRules()->SetWeaponSpawnState( true );
	GEMPRules()->SetArmorSpawnState( true );
	GEMPRules()->SetTeamSpawn( true );

	SetRoundLocked( false );

	// Reset our radar state
	if ( g_pRadarResource )
	{
		g_pRadarResource->SetForceRadar( false );
		g_pRadarResource->DropAllContacts();
	}

	// Reset the excluded characters
	g_pGameplayResource->SetCharacterExclusion( "" );

	// Remove any tokens we may have had
	if ( GEMPRules()->GetTokenManager() )
		GEMPRules()->GetTokenManager()->Reset();

	// This delays resetting scores until AFTER the round delay
	m_iRoundCount = 0;
	m_bInRound = false;
}

void CGEBaseGameplayManager::SetState( GEGameState state, bool force_now /*= false*/ )
{
	if ( m_iGameState != state )
	{
		DevMsg( 2, "[GES GP] Changing state from %s to %s, %s\n", GetStateName( m_iGameState ), GetStateName( state ), force_now ? "FORCED" : "NOT FORCED" );

		m_iGameState = state;
		if ( force_now )
		{
			m_flNextThink = gpGlobals->curtime;
			OnThink();
		}
	}
}

void CGEBaseGameplayManager::CalcCanEndRound()
{
	// Default assumption is that we cannot end
	m_bCanEndRoundCache = false;

	// We must be playing rounds to end a round!
	if ( !GEMPRules()->IsRoundTimeEnabled() )
		return;

	// Check time constraints
	if ( GEMPRules()->GetRoundTimeRemaining() <= 0 )
	{
		// We ran out of time and our scenario says we can end
		if ( GetScenario()->CanRoundEnd() )
			m_bCanEndRoundCache = true;
	}
}

void CGEBaseGameplayManager::CalcCanEndMatch()
{
	// Default assumption is that we cannot end
	m_bCanEndMatchCache = false;

	// We must be able to end our round to end the match
	if ( !CanEndRound() )
		return;

	// Check time constraints
	if ( (CanEndRound() && GEMPRules()->IsMatchTimeEnabled() && GEMPRules()->GetMatchTimeRemaining() <= 0) || m_bMatchBlockedByScenario )
	{
		// We ran out of time and our scenario says we can end
		if ( GetScenario()->CanMatchEnd() )
			m_bCanEndMatchCache = true;
		else
			m_bMatchBlockedByScenario = true;
	}
}

extern void StripChar(char *szBuffer, const char cWhiteSpace );
void CGEBaseGameplayManager::LoadScenarioCycle()
{
	if ( m_vScenarioCycle.Count() > 0 )
		return;

	const char *cfile = ge_gp_cyclefile.GetString();
	Assert( cfile != NULL );

	// Check the time of the mapcycle file and re-populate the list of level names if the file has been modified
	const int nCycleTimeStamp = filesystem->GetPathTime( cfile, "GAME" );

	if ( 0 == nCycleTimeStamp )
	{
		// cycle file does not exist, make a list containing only the current gameplay
		char *szCurrentGameplay = new char[32];
		Q_strncpy( szCurrentGameplay, GetScenario()->GetIdent(), 32 );
		m_vScenarioCycle.AddToTail( szCurrentGameplay );
	}
	else
	{
		int nFileLength;
		char *aFileList = (char*)UTIL_LoadFileForMe( cfile, &nFileLength );

		const char* curMode = ge_gameplay.GetString();

		if ( aFileList && nFileLength )
		{
			CUtlVector<char*> vList;
			V_SplitString( aFileList, "\n", vList );

			for ( int i = 0; i < vList.Count(); i++ )
			{
				bool bIgnore = false;

				// Strip out the spaces in the name
				StripChar( vList[i] , '\r');
				StripChar( vList[i] , ' ');
				
				if ( !IsValidGamePlay( vList[i] ) )
				{
					bIgnore = true;

					char szWarningMessage[MAX_PATH];
					V_snprintf( szWarningMessage, MAX_PATH, "Invalid scenario '%s' included in gameplay cycle file. Ignored.\n", vList[i] );
					Warning( szWarningMessage );
				}
				else if ( !Q_strncmp( vList[i], "//", 2 ) )
				{
					bIgnore = true;
				}

				if ( !bIgnore )
				{
					m_vScenarioCycle.AddToTail(vList[i]);
					vList[i] = NULL;
				}
			}

			// Only resolve our gameplay index if we are out of bounds
			if ( g_iScenarioIndex < 0 || g_iScenarioIndex >= m_vScenarioCycle.Count() )
			{
				for (int i=0; i<m_vScenarioCycle.Size(); i++)
				{
					// Find the first match for our current game mode if it exists in the list
					if ( curMode && Q_stricmp(m_vScenarioCycle[i], curMode) == 0 )
						g_iScenarioIndex = i;
				}
			}

			for ( int i = 0; i < vList.Count(); i++ )
			{
				if ( vList[i] )
					delete [] vList[i];
			}

			vList.Purge();

			UTIL_FreeFile( (byte *)aFileList );
		}
	}

	// If somehow we have no scenarios in the list then add the current one
	if ( m_vScenarioCycle.Count() == 0 )
	{
		char *ident = new char[32];
		Q_strncpy( ident, GetScenario()->GetIdent(), 32 );
		m_vScenarioCycle.AddToTail( ident );
	}
}

bool CGEBaseGameplayManager::IsValidGamePlay( const char *ident )
{
	if ( !ident )
		return false;

	for ( int i=0; i < m_vScenarioList.Count(); i++ ) 
	{
		if ( !Q_stricmp( ident, m_vScenarioList[i] ) )
			return true;
	}

	return false;
}

void CGEBaseGameplayManager::PrintGamePlayList()
{
	Msg("Available Game Modes: \n");
	for (int i=0; i < m_vScenarioList.Count(); i++) 
	{
		Msg( "%s\n", m_vScenarioList[i] );
	}
}

void CGEBaseGameplayManager::LoadGamePlayList(const char* path)
{
	for ( int i=0; i < m_vScenarioList.Count(); i++ )
		delete [] m_vScenarioList[i];
	m_vScenarioList.RemoveAll();

	// TODO: This is on a fast-track to deletion. This stuff should be handled in Python...
	FileFindHandle_t finder;
	const char *fileName = filesystem->FindFirstEx( path, "MOD", &finder );
	while ( fileName )
	{
		if ( Q_strncmp( fileName, "__", 2 ) )
		{
			char *fileNameNoExt = new char[64];
			Q_StripExtension( fileName, fileNameNoExt, 64 );
			m_vScenarioList.AddToTail( fileNameNoExt );
		}

		fileName = filesystem->FindNext( finder );
	}
	filesystem->FindClose( finder );
}

const char *CGEBaseGameplayManager::GetStateName( GEGameState state )
{
	switch ( state )
	{
	case GAMESTATE_NONE:
		return "None";
	case GAMESTATE_DELAY:
		return "Delay";
	case GAMESTATE_PLAYING:
		return "Playing";
	case GAMESTATE_STARTING:
		return "Starting";
	case GAMESTATE_RESTART:
		return "Restarting";
	default:
		return "INVALID";
	}
}

extern ConVar nextlevel;
static CUtlVector<char*> gMapList;
static void LoadMapList() {
	FileFindHandle_t findHandle; // note: FileFINDHandle
	char *file;

	const char *pFilename = filesystem->FindFirstEx( "maps\\*.bsp", "MOD", &findHandle );
	while ( pFilename )
	{
		file = new char[32];
		Q_strncpy( file, pFilename, min( Q_strlen(pFilename) - 3, 32 ) );
		gMapList.AddToTail( file );

		pFilename = filesystem->FindNext( findHandle );
	}

	filesystem->FindClose( findHandle );
}

// Map auto completion for ge_endmatch entries
static int MapAutoComplete( char const *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	int i, k;
	CUtlVector<char*> tokens;
	Q_SplitString( partial, " ", tokens );
	if ( tokens.Count() < 2 )
		return 0;

	if ( gMapList.Count() == 0 )
		LoadMapList();

	for ( i=0, k=0; (i < gMapList.Count() && k < COMMAND_COMPLETION_MAXITEMS); i++ )
	{
		if ( StringHasPrefix( gMapList[i], tokens[1] ) ) {
			Q_strncpy( commands[k], tokens[0], COMMAND_COMPLETION_ITEM_LENGTH );
			Q_strncat( commands[k], " ", COMMAND_COMPLETION_ITEM_LENGTH );
			Q_strncat( commands[k], gMapList[i], COMMAND_COMPLETION_ITEM_LENGTH );
			k++;
		}
	}

	ClearStringVector( tokens );

	return k; // number of entries
}

// Override for changelevel, this calls ge_endmatch
CON_COMMAND_F_COMPLETION(__ovr_changelevel, "Change the current level after ending the current round/match. Use `changelevel [mapname] 0` to change immediately.", 0, MapAutoComplete)
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	if ( !engine->IsMapValid(args[1]) )
	{
		Warning( "Invalid map name supplied to changelevel\n" );
		return;
	}

	if ( !GEMPRules() || g_fGameOver )
	{
		// Change the level if we supplied 2 arguments or are in intermission
		engine->ServerCommand( UTIL_VarArgs( "__real_changelevel %s\n", args[1] ) );
	}
	else if ( args.ArgC() > 2 )
	{
		GEMPRules()->SetupChangeLevel( args[1] );
	}
	else
	{
		// Issue the ge_endmatch command otherwise
		engine->ServerCommand( UTIL_VarArgs( "ge_endmatch %s\n", args[1] ) );
	}
}

CON_COMMAND(ge_gameplaylistrefresh, "Refreshes the list of known gameplays, useful for servers who add gameplays while the server is still running")
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() || !GEGameplay() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	// Reload the gameplay without using the cache
	GEGameplay()->LoadGamePlayList( "scripts/python/GamePlay/*.py" );
}

// WARNING: Deprecated! Use ge_endround instead!
CON_COMMAND(ge_restartround, "Restart the current round showing scores always")
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() || !GEGameplay() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	Msg( "Warning! This command is deprecated, use ge_endround instead!\n" );
	engine->ServerCommand( args.ArgS() );
}

CON_COMMAND(ge_endround, "End the current round, use `ge_endround 0` to skip scores" )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() || !GEGameplay() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	// Ignore if we are not in a playing state
	if ( !GEGameplay()->IsRoundStarted() )
		return;

	bool showreport = true;
	if ( args.ArgC() > 1 && args[1][0] == '0' )
		showreport = false;

	GEGameplay()->EndRound( showreport );
}

CON_COMMAND(ge_endround_keepweapons, "End the current round but keep the same weapon set even if randomized.")
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() || !GEGameplay() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	// Ignore if we are not in a playing state
	if ( GEGameplay()->GetState() != GAMESTATE_PLAYING )
		return;

	// This is where the magic happens
	GEMPRules()->GetLoadoutManager()->KeepLoadoutOnNextChange();

	bool showreport = true;
	if ( args.ArgC() > 1 && args[1][0] == '0' )
		showreport = false;

	GEGameplay()->EndRound( showreport );
}

CON_COMMAND_F_COMPLETION(ge_endmatch, "Ends the match loading the next map or one specified (eg ge_endmatch [mapname]).", 0, MapAutoComplete)
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() || !GEMPRules() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	if ( args.ArgC() > 1 && engine->IsMapValid( args[1] ) )
		nextlevel.SetValue( args[1] );

	GEGameplay()->EndMatch();
}

CON_COMMAND(ge_gameplaylist, "Lists the possible gameplay selections.")
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	GEGameplay()->PrintGamePlayList();
}

#ifdef _DEBUG
CON_COMMAND( ge_cyclegameplay, "Simulates a map transition" )
{
	GEGameplay()->LoadScenario();
}
#endif
