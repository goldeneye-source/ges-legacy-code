///////////// Copyright © 2013, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_round_timer.cpp
// Description:
//      Round timer for GE:S
//
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////
#include "cbase.h"
#include "ge_game_timer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( ge_game_timer, CGEGameTimer );

IMPLEMENT_NETWORKCLASS_ALIASED( GEGameTimer, DT_GEGameTimer )

BEGIN_NETWORK_TABLE_NOBASE( CGEGameTimer, DT_GEGameTimer )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bEnabled ) ),
	RecvPropBool( RECVINFO( m_bPaused ) ),
	RecvPropTime( RECVINFO( m_flPauseTimeRemaining ) ),
	RecvPropTime( RECVINFO( m_flLength ) ),
	RecvPropTime( RECVINFO( m_flEndTime ) ),
#else
	SendPropBool( SENDINFO( m_bEnabled ) ),
	SendPropBool( SENDINFO( m_bPaused ) ),
	SendPropTime( SENDINFO( m_flPauseTimeRemaining ) ),
	SendPropTime( SENDINFO( m_flLength ) ),
	SendPropTime( SENDINFO( m_flEndTime ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CGEGameTimer::CGEGameTimer()
{
#ifdef GAME_DLL
	// Reset our variables
	Stop();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Gets the seconds left on the timer, paused or not.
//-----------------------------------------------------------------------------
float CGEGameTimer::GetTimeRemaining()
{
	// If we are not started return 0
	if ( !m_bEnabled )
		return 0;

	float seconds_remaining;
	
	if ( m_bPaused )
		seconds_remaining = m_flPauseTimeRemaining;
	else
		seconds_remaining = m_flEndTime - gpGlobals->curtime;

	return max( seconds_remaining, 0 );
}

#ifdef GAME_DLL
// Starts the timer if length_seconds is greater than 0, otherwise stops the timer
// If the timer is already started, this will adjust the end time accordingly
void CGEGameTimer::Start( float length_seconds )
{
	if ( length_seconds <= 0 ) {
		// Stop the timer if called with Start(0)
		Stop();
	} else {
		// Start the timer
		m_bEnabled = true;
		m_flLength = length_seconds;
		m_flEndTime = gpGlobals->curtime + m_flLength;

		// Reset any pause state
		m_bPaused = false;
		m_flPauseTimeRemaining = 0;
	}
}

void CGEGameTimer::Stop()
{
	// Stop the timer
	m_bEnabled = false;
	m_flLength = 0;
	m_flEndTime = 0;

	// Reset any pause state
	m_bPaused = false;
	m_flPauseTimeRemaining = 0;
}

void CGEGameTimer::ChangeLength( float length_seconds )
{
	// Stop the timer if we change to 0 length
	if ( length_seconds <= 0 ) {
		Stop();
		return;
	}

	// Start the timer if we were not started before
	if ( !IsEnabled() ) {
		Start( length_seconds );
		return;
	}

	// If paused, resume the time to capture our real end time
	bool was_paused = IsPaused();
	if ( was_paused )
		Resume();

	// Find time diff and modify the end time
	int time_diff = length_seconds - m_flLength;
	m_flEndTime += time_diff;

	// If we were paused, put as back in that state
	if ( was_paused )
		Pause();
}

//-----------------------------------------------------------------------------
// Purpose: Timer is paused at round end, stops the countdown
//-----------------------------------------------------------------------------
void CGEGameTimer::Pause()
{
	if ( !m_bPaused )
	{
		// Store our pause time remaining to our currently remaining time
		m_flPauseTimeRemaining = GetTimeRemaining();
		m_bPaused = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: To start or re-start the timer after a pause
//-----------------------------------------------------------------------------
void CGEGameTimer::Resume()
{
	if ( m_bPaused )
	{
		// Set our new end time to our Resume Time plus our Remaining Time
		m_flEndTime = gpGlobals->curtime + GetTimeRemaining();
		m_flPauseTimeRemaining = 0;
		m_bPaused = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: The timer is always transmitted to clients
//-----------------------------------------------------------------------------
int CGEGameTimer::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

#endif
