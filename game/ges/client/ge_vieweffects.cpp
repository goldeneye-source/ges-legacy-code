///////////// Copyright © 2010, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_vieweffects.cpp
// Description:
//      Adds client view effects for GE (namely breathing effect)
//
// Created On: 12 Jan 2010
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "ivieweffects.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "viewrender.h"

#include "ge_shareddefs.h"
#include "ge_vieweffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CGEViewEffects g_ViewEffects;
IViewEffects *vieweffects = ( IViewEffects * )&g_ViewEffects;

#define ATTR_TIMEOUT 3.0f
#define ATTR_ACCELTIME ATTR_TIMEOUT * 0.35f
#define ATTR_DEACCELTIME ATTR_TIMEOUT * 0.75f

CGEViewEffects::CGEViewEffects( void )
{
	ResetBreath();
}

void CGEViewEffects::LevelInit( void )
{
	ResetBreath();
	m_flPixelScale = XRES( 28.0f );

	CViewEffects::LevelInit();
}

void CGEViewEffects::ResetBreath( void )
{
	m_iNextAttractor = 0;
	m_flAttractorTimeout = 0;

	for ( int i=0; i < NUM_ATTRACTORS; i++ )
		GenerateRandomAttractor( m_vAttractors[i] );

	m_vBO = vec3_origin;
	m_vOldBO = vec3_origin;
}

void CGEViewEffects::ApplyBreath( void )
{
	if ( gpGlobals->curtime >= m_flAttractorTimeout )
	{
		GenerateRandomAttractor( m_vAttractors[m_iNextAttractor] );
		m_iNextAttractor = m_iNextAttractor == (NUM_ATTRACTORS-1) ? 0 : ++m_iNextAttractor;
		m_flAttractorTimeout = m_vAttractors[m_iNextAttractor].spawntime + ATTR_TIMEOUT;
	}

	m_vOldBO = m_vBO;

	float pull, timealive, influence;
	Vector newBO = m_vBO, dir;
	for ( int i=0; i < NUM_ATTRACTORS; i++ )
	{
		dir = m_vAttractors[i].attr - m_vBO;
		timealive = gpGlobals->curtime - m_vAttractors[i].spawntime;

		// Figure out our influence (deaccelerate after half of our lifetime
		if ( timealive >= ATTR_DEACCELTIME )
			influence = RemapValClamped( timealive, ATTR_DEACCELTIME, ATTR_TIMEOUT, 1.15f, 0.4f );
		else if ( timealive <= ATTR_ACCELTIME )
			influence = RemapValClamped( timealive, 0, ATTR_ACCELTIME, 0.1f, 1.15f );
		else
			influence = 1.15f;

		// Calculate our pull by taking an influenced step 
		pull = influence * m_vAttractors[i].stepsize * dir.NormalizeInPlace();
		
		VectorMA( newBO, pull, dir, newBO );
	}

	m_vBO = newBO;
}

void CGEViewEffects::BreathMouseMove( float *x, float *y )
{
	C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();

	if ( pLocal && !pLocal->IsObserver() && pLocal->IsAlive() )
	{
		// Simply take the distance of m_vBO from the center of the screen and add it to x and y :-D
		*y += (m_vBO[PITCH] - m_vOldBO[PITCH]) * m_flPixelScale;
		*x += (m_vBO[YAW] - m_vOldBO[YAW]) * m_flPixelScale;
	}
}

void CGEViewEffects::GenerateRandomAttractor( attractor_t &attractor )
{
	float radius = GERandom<float>( 0.64f );
	float angle = GERandom<float>( M_TWOPI );

	attractor.attr[PITCH] = radius * cos( angle );
	attractor.attr[YAW] = 1.3333 * radius * sin( angle );
	attractor.attr[ROLL] = 0;

	attractor.spawntime = gpGlobals->curtime;
	attractor.stepsize = (m_vBO.DistTo( attractor.attr ) / ATTR_TIMEOUT) * gpGlobals->frametime;
}
