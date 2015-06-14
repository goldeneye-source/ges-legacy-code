//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Round timer for team gamerules
//
//=============================================================================//

#ifndef GE_ROUND_TIMER_H
#define GE_ROUND_TIMER_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CGERoundTimer C_GERoundTimer
#endif

class CGERoundTimer : public CBaseEntity
{
public:
	DECLARE_CLASS( CGERoundTimer, CBaseEntity );
	DECLARE_NETWORKCLASS();

	CGERoundTimer();

	virtual void Spawn( void );
	virtual void Activate( void );

	// Returns seconds to display.
	// When paused shows amount of time left once the timer is resumed
	virtual float GetTimeRemaining( void );
	virtual int   GetTimerMaxLength( void );
	virtual bool  ShowInHud( void );
	virtual bool  StartPaused( void )	{ return m_bStartPaused; }

	bool IsDisabled( void )		{ return m_bIsDisabled; }
	int  GetTimerState( void )	{ return m_nState; }

	bool IsTimerPaused( void )	{ return m_bTimerPaused; }
	
#ifdef CLIENT_DLL
	void InternalSetPaused( bool bPaused ) { m_bTimerPaused = bPaused; }
#else
	DECLARE_DATADESC();

	void		 SetStopWatchTimeStamp( void );
	virtual void SetTimeRemaining( int iTimerSeconds ); // Set the initial length of the timer
	virtual void AddTimerSeconds( int iSecondsToAdd, int iTeamResponsible = TEAM_UNASSIGNED ); // Add time to an already running ( or paused ) timer
	virtual void PauseTimer( void );
	virtual void ResumeTimer( void );
	virtual void SetAutoCountdown( bool bAuto ){ m_bAutoCountdown = bAuto; }

	void		 SetShowInHud( bool bShowInHUD ) { m_bShowInHUD = bShowInHUD; }

	int UpdateTransmitState();
#endif

	void SetStopWatch( bool bState ) { m_bStopWatchTimer = bState; }
	bool IsStopWatchTimer( void ) { return m_bStopWatchTimer; }
	float GetStopWatchTotalTime( void ) { return m_flTotalTime; }

private:

#ifdef GAME_DLL
	void SetState( int nState );
	void SetTimerThink( int nType );
	void RoundTimerThink( void );
	void RoundTimerSetupThink( void );

	static void SetActiveTimer( CGERoundTimer *pNewlyActive );
#endif

private:
	CNetworkVar( bool, m_bTimerPaused );
	CNetworkVar( float, m_flTimeRemaining );
	CNetworkVar( float, m_flTimerEndTime );	
	CNetworkVar( bool, m_bIsDisabled );
	CNetworkVar( bool, m_bShowInHUD );
	CNetworkVar( int, m_nTimerLength );			// current timer's length (used in the timer panel if no max length is set)
	CNetworkVar( int, m_nTimerInitialLength );	// initial length of the timer
	CNetworkVar( int, m_nTimerMaxLength );		// max time the timer can have (0 is no max)
	CNetworkVar( bool, m_bAutoCountdown );		// automatically count down the end of a round
	CNetworkVar( int, m_nSetupTimeLength );		// current timer's setup time length (setup time is the time before the round begins)
	CNetworkVar( int, m_nState );				// RT_STATE_SETUP or RT_STATE_NORMAL
	CNetworkVar( bool, m_bStartPaused );		// start the timer paused when it spawns
	CNetworkVar( float, m_flTotalTime );
	CNetworkVar( bool, m_bStopWatchTimer );

#ifdef GAME_DLL
	bool			m_bPauseDueToWin;
	bool			m_bResetTimeOnRoundStart;
	int				m_nTimeToUseAfterSetupFinished;
#endif 
};

#ifdef CLIENT_DLL
extern CGERoundTimer *g_GERoundTimer;
#endif

#endif	//GE_ROUND_TIMER_H