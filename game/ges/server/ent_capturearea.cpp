///////////// Copyright © 2011, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ent_capturearea.cpp
// Description:
//      See header
//
// Created On: 01 Aug 11
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////
#include "cbase.h"
#include "ent_capturearea.h"
#include "gemp_gamerules.h"
#include "ge_gameplay.h"
#include "npc_gebase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CAPTURE_MODEL "models/gameplay/capturepoint.mdl"

LINK_ENTITY_TO_CLASS( ge_capturearea, CGECaptureArea );

PRECACHE_REGISTER( ge_capturearea );

// Start of our data description for the class
BEGIN_DATADESC( CGECaptureArea )
//	DEFINE_ENTITYFUNC( CaptureTouch ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CGECaptureArea, DT_GECaptureArea )
	SendPropBool( SENDINFO(m_bEnableGlow) ),
	SendPropInt( SENDINFO(m_GlowColor), 32, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO(m_GlowDist) ),
END_SEND_TABLE()

CGECaptureArea::CGECaptureArea()
{
	m_szGroupName[0] = '\0';
	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
}

CGECaptureArea::~CGECaptureArea()
{

}

void CGECaptureArea::Spawn( void )
{
	Precache();
	BaseClass::Spawn();

	// Do a trace from the origin of the object straight down to find the floor
	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector(0,0,-1024), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( !tr.startsolid && tr.fraction < 1.0 )
		SetAbsOrigin( tr.endpos );
	
	AddFlag( FL_OBJECT );

	VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false );
	SetBlocksLOS( false );
	SetMoveType( MOVETYPE_NONE );
	SetCollisionGroup( COLLISION_GROUP_CAPAREA );

	SetRadius( 32.0f );
	
	PhysicsTouchTriggers();
	ClearTouchingEntities();

	GEMPRules()->GetTokenManager()->OnCaptureAreaSpawned( this );
}

void CGECaptureArea::Precache( void )
{
	PrecacheModel( CAPTURE_MODEL );
	BaseClass::Precache();
}

void CGECaptureArea::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();
	if ( GEMPRules() )
		GEMPRules()->GetTokenManager()->OnCaptureAreaRemoved( this );
}

void CGECaptureArea::SetGroupName( const char* name )
{
	Q_strncpy( m_szGroupName, name, 32 );
}

void CGECaptureArea::SetRadius( float radius )
{
	CollisionProp()->UseTriggerBounds( true, min( radius, 127.0f) );
}

void CGECaptureArea::SetupGlow( bool state, Color glowColor /*=Color(255,255,255)*/, float glowDist /*=250.0f*/ )
{
	m_GlowColor.Set( glowColor.GetRawColor() );
	m_GlowDist.Set( glowDist );
	m_bEnableGlow = state;
}

void CGECaptureArea::ClearTouchingEntities( void )
{
	m_hTouchingEntities.RemoveAll();
}

void CGECaptureArea::StartTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() && !pOther->IsNPC() )
		return;

	if ( GEMPRules()->GetTokenManager()->CanTouchCaptureArea( m_szGroupName, pOther ) )
	{
		// Check if we are touching us already, if not add us to the list
		EHANDLE hPlayer = pOther->GetRefEHandle();
		if ( m_hTouchingEntities.Find( hPlayer ) == m_hTouchingEntities.InvalidIndex() )
		{
			m_hTouchingEntities.AddToTail( hPlayer );

			// Resolve our held token (note we must use the pOther)
			const char *szToken = GEMPRules()->GetTokenManager()->GetCaptureAreaToken(m_szGroupName);
			CGEWeapon *pToken = (CGEWeapon*) pOther->MyCombatCharacterPointer()->Weapon_OwnsThisType( szToken );

			// Resolve the player (or bot proxy)
			CGEPlayer *pPlayer = ToGEMPPlayer( pOther );
			if ( !pPlayer )
				pPlayer = ToGEBotPlayer( pOther );

			// Notify the Gameplay
			GEGameplay()->GetScenario()->OnCaptureAreaEntered( this, pPlayer, pToken );
		}
	}
}

void CGECaptureArea::EndTouch( CBaseEntity *pOther )
{
	EHANDLE hPlayer = pOther->GetRefEHandle();
	if ( m_hTouchingEntities.FindAndRemove( hPlayer ) )
	{
		// Resolve the player (or bot proxy)
		CGEPlayer *pPlayer = ToGEMPPlayer( pOther );
		if ( !pPlayer )
			pPlayer = ToGEBotPlayer( pOther );

		if ( GEGameplay() )
			GEGameplay()->GetScenario()->OnCaptureAreaExited( this, ToGEMPPlayer(pOther) );
	}
}
