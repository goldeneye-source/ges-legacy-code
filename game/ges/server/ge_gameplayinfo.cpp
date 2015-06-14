//////////  Copyright © 2011, Goldeneye Source. All rights reserved. ///////////
// 
// ge_gameplayinfo.cpp
//
// Description:
//     See Header
//
// Created On: 6/11/2011
// Created By: Killer Monkey
////////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "gemp_gamerules.h"
#include "ge_gameplay.h"

class CGEGameplayInfo : public CPointEntity, public CGEGameplayEventListener
{
	DECLARE_CLASS( CGEGameplayInfo, CPointEntity );

public:
	CGEGameplayInfo();
	DECLARE_DATADESC();

	void OnRoundRestart();
	void OnRoundEnded();

	void InputGetPlayerCount( inputdata_t &inputdata );
	void InputGetRoundCount( inputdata_t &inputdata );

	COutputInt m_outPlayerCount;
	COutputInt m_outRoundCount;
	COutputEvent m_outTeamplayOn;
	COutputEvent m_outTeamplayOff;
	COutputEvent m_outRoundStart;
	COutputEvent m_outRoundEnd;
};

LINK_ENTITY_TO_CLASS( ge_gameplayinfo, CGEGameplayInfo );

BEGIN_DATADESC( CGEGameplayInfo )
	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "GetPlayerCount", InputGetPlayerCount ),
	DEFINE_INPUTFUNC( FIELD_VOID, "GetRoundCount", InputGetRoundCount ),

	// Outputs
	DEFINE_OUTPUT( m_outPlayerCount, "PlayerCount"),
	DEFINE_OUTPUT( m_outRoundCount,  "RoundCount" ),
	DEFINE_OUTPUT( m_outTeamplayOn,	 "TeamplayOn" ),
	DEFINE_OUTPUT( m_outTeamplayOff, "TeamplayOff"),
	DEFINE_OUTPUT( m_outRoundStart, "RoundStart"),
	DEFINE_OUTPUT( m_outRoundEnd, "RoundEnd"),
END_DATADESC()

CGEGameplayInfo::CGEGameplayInfo()
{

}

// Called BEFORE players spawn
void CGEGameplayInfo::OnRoundRestart()
{
	// Call teamplay output
	if ( GEMPRules()->IsTeamplay() )
		m_outTeamplayOn.FireOutput( this, this );
	else
		m_outTeamplayOff.FireOutput( this, this );

	// Call players output
	m_outPlayerCount.Set( GEMPRules()->GetNumActivePlayers(), this, this );

	// Call num rounds output
	m_outRoundCount.Set( GEGameplay()->GetNumRounds(), this, this );

	// Call round start output
	m_outRoundStart.FireOutput( this, this );
}

// Called when the round timer ends
void CGEGameplayInfo::OnRoundEnded()
{
	m_outRoundEnd.FireOutput( this, this );
}

void CGEGameplayInfo::InputGetPlayerCount( inputdata_t &inputdata )
{
	m_outPlayerCount.Set( GEMPRules()->GetNumActivePlayers(), inputdata.pActivator, this );
}

void CGEGameplayInfo::InputGetRoundCount( inputdata_t &inputdata )
{
	m_outRoundCount.Set( GEGameplay()->GetNumRounds(), inputdata.pActivator, this );
}
