///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_hudroundtimer.cpp
// Description:
//      Shows the current round time (placer until the Gameplay Info Panel is up and running)
//
// Created On: 11/14/2008
// Created By: Killer Monkey
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "hud.h"
#include "iclientmode.h"
#include "view.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/AnimationController.h>
#include "vgui/ISurface.h"
#include "IVRenderView.h"
#include "in_buttons.h"
#include "hudelement.h"

#include "ge_gamerules.h"
#include "gemp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CGEHudRoundTimer : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CGEHudRoundTimer, CHudNumericDisplay );

public:
	CGEHudRoundTimer( const char *pElementName );

	virtual void ResetColor( void );
	virtual void Reset( void );
	virtual void Think( void );
	virtual bool ShouldDraw( void );

	CPanelAnimationVar( Color, m_NormColor, "fontcolor", "255 255 255 255" );
	CPanelAnimationVar( Color, m_SpecColor, "speccolor", "0 0 0 235" );

private:
	float m_flNextAnim;
	float m_flNextThink;
};

DECLARE_HUDELEMENT( CGEHudRoundTimer );

CGEHudRoundTimer::CGEHudRoundTimer( const char *pElementName ) : 
	BaseClass( NULL, "GERoundTimer" ), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetIsTime(true);
	m_flNextAnim = 0;
	m_flNextThink = 0;
}

void CGEHudRoundTimer::Reset( void )
{
	m_flNextThink = 0;
}

bool CGEHudRoundTimer::ShouldDraw( void )
{
	// Don't show if we are in intermission time!
	if ( GEMPRules() && GEMPRules()->IsIntermission() )
	{
		m_flNextAnim = 0;
		return false;
	}

	return CHudElement::ShouldDraw();
}

void CGEHudRoundTimer::Think( void )
{
	if ( !GEMPRules() || gpGlobals->curtime < m_flNextThink )
		return;

	if ( GEMPRules()->IsRoundTimePaused() || GEMPRules()->GetRoundTimeLeft() <= 0 )
	{
		ResetColor();
		SetDisplayValue( -1 );
		return;
	}

	float timeleft = GEMPRules()->GetRoundTimeLeft();
	SetDisplayValue( timeleft );

	// Don't bother with this if we have hardly any time left or no time at all
	if ( timeleft > 0.5f )
	{
		// If we only have 10 seconds left start going beserk!
		if ( timeleft <= 10.0f && m_flNextAnim < gpGlobals->curtime )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("RoundTimeCritical");
			m_flNextAnim = gpGlobals->curtime + g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("RoundTimeCritical");
		}
		// If we only have 30 seconds left start notifying the player!
		else if ( timeleft <= 30.0f && m_flNextAnim < gpGlobals->curtime )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("RoundTimeExpiring");
			m_flNextAnim = gpGlobals->curtime + 5.0f; //Do this every 5 seconds
		}
		else if ( m_flNextAnim == 0 )
		{	
			ResetColor();
		}
	}
	else
	{
		m_flNextAnim = 0;
		ResetColor();
	}

	m_flNextThink = gpGlobals->curtime + 0.1f;
}

void CGEHudRoundTimer::ResetColor( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( pPlayer && (pPlayer->IsObserver() || pPlayer->GetTeamNumber() == TEAM_SPECTATOR) )
		SetFgColor( m_SpecColor );
	else
		SetFgColor( m_NormColor );

	m_flBlur = 0;
}
