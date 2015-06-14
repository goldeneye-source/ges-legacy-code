//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team gamerules round timer 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "teamplay_round_timer.h"
#include "ge_gamerules.h"
#include "gemp_gamerules.h"

#ifdef CLIENT_DLL
#include "IClientMode.h"
#include "vgui_controls/AnimationController.h"
#include "c_playerresource.h"
#else
#include "team.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum
{
	RT_THINK_SETUP,
	RT_THINK_NORMAL,	
};

LINK_ENTITY_TO_CLASS( ge_round_timer, CGERoundTimer );

IMPLEMENT_NETWORKCLASS_ALIASED( GERoundTimer, DT_GERoundTimer )

BEGIN_NETWORK_TABLE_NOBASE( CGERoundTimer, DT_GERoundTimer )
#ifdef CLIENT_DLL

	RecvPropBool( RECVINFO( m_bTimerPaused ) ),
	RecvPropTime( RECVINFO( m_flTimeRemaining ) ),
	RecvPropTime( RECVINFO( m_flTimerEndTime ) ),
	RecvPropInt( RECVINFO( m_nTimerMaxLength ) ),
	RecvPropBool( RECVINFO( m_bIsDisabled ) ),
	RecvPropBool( RECVINFO( m_bShowInHUD ) ),
	RecvPropInt( RECVINFO( m_nTimerLength ) ),
	RecvPropInt( RECVINFO( m_nTimerInitialLength ) ),
	RecvPropBool( RECVINFO( m_bAutoCountdown ) ),
	RecvPropInt( RECVINFO( m_nSetupTimeLength ) ),
	RecvPropInt( RECVINFO( m_nState ) ),
	RecvPropBool( RECVINFO( m_bStartPaused ) ),
	RecvPropBool( RECVINFO( m_bStopWatchTimer ) ),
	RecvPropTime( RECVINFO( m_flTotalTime ) ),

#else

	SendPropBool( SENDINFO( m_bTimerPaused ) ),
	SendPropTime( SENDINFO( m_flTimeRemaining ) ),
	SendPropTime( SENDINFO( m_flTimerEndTime ) ),
	SendPropInt( SENDINFO( m_nTimerMaxLength ) ),
	SendPropBool( SENDINFO( m_bIsDisabled ) ),
	SendPropBool( SENDINFO( m_bShowInHUD ) ),
	SendPropInt( SENDINFO( m_nTimerLength ) ),
	SendPropInt( SENDINFO( m_nTimerInitialLength ) ),
	SendPropBool( SENDINFO( m_bAutoCountdown ) ),
	SendPropInt( SENDINFO( m_nSetupTimeLength ) ),
	SendPropInt( SENDINFO( m_nState ) ),
	SendPropBool( SENDINFO( m_bStartPaused ) ),
	SendPropBool( SENDINFO( m_bStopWatchTimer ) ),
	SendPropTime( SENDINFO( m_flTotalTime ) ),

#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
BEGIN_DATADESC(CGERoundTimer)
	DEFINE_KEYFIELD( m_nTimerInitialLength,		FIELD_INTEGER,	"timer_length" ),
	DEFINE_KEYFIELD( m_nTimerMaxLength,			FIELD_INTEGER,	"max_length" ),
	DEFINE_KEYFIELD( m_bShowInHUD,				FIELD_BOOLEAN,	"show_in_hud" ),
	DEFINE_KEYFIELD( m_bIsDisabled,				FIELD_BOOLEAN,	"StartDisabled" ),
	DEFINE_KEYFIELD( m_bAutoCountdown,			FIELD_BOOLEAN,	"auto_countdown" ),
	DEFINE_KEYFIELD( m_nSetupTimeLength,		FIELD_INTEGER,	"setup_length" ),
	DEFINE_KEYFIELD( m_bResetTimeOnRoundStart,	FIELD_BOOLEAN,	"reset_time" ),
	DEFINE_KEYFIELD( m_bStartPaused,			FIELD_BOOLEAN,	"start_paused" ),

	DEFINE_FUNCTION( RoundTimerSetupThink ),
	DEFINE_FUNCTION( RoundTimerThink ),

END_DATADESC();
#endif

#ifndef CLIENT_DLL
#define ROUND_TIMER_THINK			"CGERoundTimerThink"
#define ROUND_TIMER_SETUP_THINK		"CGERoundTimerSetupThink"
#endif

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CGERoundTimer::CGERoundTimer( void )
{
	m_bTimerPaused = false;
	m_flTimeRemaining = 0;
	m_nTimerLength = 0;
	m_nTimerInitialLength = 0;
	m_nTimerMaxLength = 0;
	m_flTimerEndTime = 0;
	m_bIsDisabled = false;
	m_bAutoCountdown = true;
	m_nState.Set( RT_STATE_NORMAL );        // we'll assume no setup time for now
	m_bStartPaused = true;

	m_bStopWatchTimer = false;

	m_flTotalTime = 0.0f;

#ifndef CLIENT_DLL
	m_bPauseDueToWin = false;
	m_bResetTimeOnRoundStart = false;
	m_nTimeToUseAfterSetupFinished = 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::Activate( void )
{
	BaseClass::Activate();

#ifndef CLIENT_DLL
	if ( m_bShowInHUD )
	{
		SetActiveTimer( this );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::Spawn( void )
{
	Precache();

#ifdef GAME_DLL
	int nTimerTime = 0;

	// do we have a setup time?
	if ( m_nSetupTimeLength > 0 )
	{
		nTimerTime = m_nSetupTimeLength;
		SetState( RT_STATE_SETUP );
	}
	else
	{
		nTimerTime = m_nTimerInitialLength;
		SetState( RT_STATE_NORMAL );
	}

	m_nTimeToUseAfterSetupFinished = m_nTimerInitialLength;

	if ( IsDisabled() )  // we need to get the data initialized before actually become disabled
	{
		m_bIsDisabled = false;
		PauseTimer(); // start paused
		SetTimeRemaining( nTimerTime );
		m_bIsDisabled = true;
	}
	else
	{
		PauseTimer(); // start paused
		SetTimeRemaining( nTimerTime );
	}

	m_nTimerLength = nTimerTime;

	BaseClass::Spawn();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CGERoundTimer::ShowInHud( void )
{
	return m_bShowInHUD;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the seconds left on the timer, paused or not.
//-----------------------------------------------------------------------------
float CGERoundTimer::GetTimeRemaining( void )
{
	float flSecondsRemaining;

	if ( IsStopWatchTimer() == true )
	{
		flSecondsRemaining = m_flTotalTime;
	}
	else
	{
		if ( m_bTimerPaused )
		{
			flSecondsRemaining = m_flTimeRemaining;
		}
		else
		{
			flSecondsRemaining = m_flTimerEndTime - gpGlobals->curtime;
		}
	}

	if ( flSecondsRemaining < 0 )
	{
		flSecondsRemaining = 0.0f;
	}

	return flSecondsRemaining;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CGERoundTimer::GetTimerMaxLength( void )
{
	if ( m_nState == RT_STATE_SETUP )
	{
		return m_nSetupTimeLength;
	}
	else
	{
		if ( m_nTimerMaxLength )
			return m_nTimerMaxLength;

		return m_nTimerLength;
	}
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::SetState( int nState )
{
	m_nState = nState;

	if ( nState == RT_STATE_SETUP )
		SetTimerThink( RT_THINK_SETUP );
	else
		SetTimerThink( RT_THINK_NORMAL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::SetTimerThink( int nType )
{
	if ( nType == RT_THINK_SETUP )
	{
		SetContextThink( &CGERoundTimer::RoundTimerSetupThink, gpGlobals->curtime + 0.05, ROUND_TIMER_SETUP_THINK );
		SetContextThink( NULL, 0, ROUND_TIMER_THINK );
	}
	else
	{
		SetContextThink( &CGERoundTimer::RoundTimerThink, gpGlobals->curtime + 0.05, ROUND_TIMER_THINK );
		SetContextThink( NULL, 0, ROUND_TIMER_SETUP_THINK );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::RoundTimerSetupThink( void )
{
	if ( IsDisabled() || m_bTimerPaused )
	{
		SetContextThink( &CGERoundTimer::RoundTimerSetupThink, gpGlobals->curtime + 0.05, ROUND_TIMER_SETUP_THINK );
		return;
	}

	SetContextThink( &CGERoundTimer::RoundTimerSetupThink, gpGlobals->curtime + 0.05, ROUND_TIMER_SETUP_THINK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::RoundTimerThink( void )
{
	if ( IsDisabled() || m_bTimerPaused || gpGlobals->eLoadType == MapLoad_Background )
	{
		SetContextThink( &CGERoundTimer::RoundTimerThink, gpGlobals->curtime + 0.05, ROUND_TIMER_THINK );
		return;
	}

	// Don't do anything when we are in intermission
	if ( GEMPRules()->IsIntermission() )
	{
		// We want to stop timers when the round has been won, but we don't want to 
		// force mapmakers to deal with having to unpause it. This little hack works around that.
		if ( !m_bTimerPaused )
		{
			PauseTimer();
			m_bPauseDueToWin = true;
		}

		SetContextThink( &CGERoundTimer::RoundTimerThink, gpGlobals->curtime + 0.05, ROUND_TIMER_THINK );
		return;
	}
	else if ( m_bPauseDueToWin )
	{
		ResumeTimer();
		m_bPauseDueToWin = false;
	}

	SetContextThink( &CGERoundTimer::RoundTimerThink, gpGlobals->curtime + 0.05, ROUND_TIMER_THINK );
}

//-----------------------------------------------------------------------------
// Purpose: To set the initial timer duration
//-----------------------------------------------------------------------------
void CGERoundTimer::SetTimeRemaining( int iTimerSeconds )
{
	if ( IsDisabled() )
		return;

	// make sure we don't go over our max length
	if ( m_nTimerMaxLength > 0 )
	{
		if ( iTimerSeconds > m_nTimerMaxLength )
		{
			iTimerSeconds = m_nTimerMaxLength;
		}
	}

	m_flTimeRemaining = (float)iTimerSeconds;
	m_flTimerEndTime = gpGlobals->curtime + m_flTimeRemaining;
	m_nTimerLength = iTimerSeconds;
}

//-----------------------------------------------------------------------------
// Purpose: To set the initial timer duration
//-----------------------------------------------------------------------------
void CGERoundTimer::SetStopWatchTimeStamp( void )
{
	if ( IsDisabled() )
		return;

	m_flTotalTime = m_flTotalTime + (gpGlobals->curtime - m_flTimerEndTime);
	m_flTimerEndTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Timer is paused at round end, stops the countdown
//-----------------------------------------------------------------------------
void CGERoundTimer::PauseTimer( void )
{
	if ( IsDisabled() )
		return;

	if ( m_bTimerPaused == false )
	{
		m_bTimerPaused = true;

		m_flTimeRemaining = m_flTimerEndTime - gpGlobals->curtime;
	}

	// Clear pause on win flag, because we've been set by the mapmaker
	m_bPauseDueToWin = false;
}

//-----------------------------------------------------------------------------
// Purpose: To start or re-start the timer after a pause
//-----------------------------------------------------------------------------
void CGERoundTimer::ResumeTimer( void )
{
	if ( IsDisabled() )
		return;

	if ( m_bTimerPaused == true )
	{
		m_bTimerPaused = false;

		if ( IsStopWatchTimer() == true )
		{
			m_flTimerEndTime = gpGlobals->curtime;
		}
		else
		{
			m_flTimerEndTime = gpGlobals->curtime + m_flTimeRemaining;			
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add seconds to the timer while it is running or paused
//-----------------------------------------------------------------------------
void CGERoundTimer::AddTimerSeconds( int iSecondsToAdd, int iTeamResponsible /* = TEAM_UNASSIGNED*/ )
{
	if ( IsDisabled() )
		return;

	if ( m_nTimerMaxLength > 0 )
	{
		// will adding this many seconds push us over our max length?
		if ( GetTimeRemaining() + iSecondsToAdd > m_nTimerMaxLength )
		{
			// adjust to only add up to our max length
			iSecondsToAdd = m_nTimerMaxLength - GetTimeRemaining();
		}
	}

	if ( m_bTimerPaused )
	{
		m_flTimeRemaining += (float)iSecondsToAdd;
	}
	else
	{
		m_flTimerEndTime += (float)iSecondsToAdd;
	}

	m_nTimerLength += iSecondsToAdd;

	if ( ShowInHud() )
	{
		if ( !GEMPRules()->IsIntermission() )
		{
			if ( iTeamResponsible >= LAST_SHARED_TEAM+1 )
			{
				for ( int iTeam = LAST_SHARED_TEAM+1 ; iTeam < GetNumberOfTeams(); iTeam++ )
				{
					if ( iTeam == iTeamResponsible )
						CTeamRecipientFilter filter( iTeam, true );
					else
						CTeamRecipientFilter filter( iTeam, true );
				}
			}
			else
			{
				CReliableBroadcastRecipientFilter filter;
			}
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_timer_time_added" );
		if ( event )
		{
			event->SetInt( "timer", entindex() );
			event->SetInt( "seconds_added", iSecondsToAdd );
			gameeventmanager->FireEvent( event );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: The timer is always transmitted to clients
//-----------------------------------------------------------------------------
int CGERoundTimer::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERoundTimer::SetActiveTimer( CGERoundTimer *pNewlyActive )
{
	CBaseEntity *pChosenTimer = pNewlyActive;	

	// Ensure all other timers are off.
	CBaseEntity *pEntity = NULL;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "ge_round_timer" )) != NULL)
	{
		if ( pEntity == pNewlyActive )
			continue;

		CGERoundTimer *pTimer = assert_cast< CGERoundTimer* >( pEntity );
		if ( !pTimer->IsDisabled() && pTimer->ShowInHud() )
		{
			if ( pChosenTimer )
			{
				// Turn off all other hud timers
				pTimer->SetShowInHud( false );
			}
			else
			{
				// Found a timer. Use it.
				pChosenTimer = pTimer;
			}
		}
	}
}

#endif
