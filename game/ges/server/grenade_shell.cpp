///////////// Copyright © 2009, Goldeneye: Source. All rights reserved. /////////////
// 
// File: grenade_shell.cpp
// Description:
//      This is what makes the shells go BOOM
//
// Created On: 9/29/2008
// Created By: Jonathan White <killermonkey01@gmail.com>
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "grenade_shell.h"
#include "explode.h"
#include "ge_utils.h"
#include "gebot_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GE_SHELL_FUSETIME  3.8f

LINK_ENTITY_TO_CLASS( npc_shell, CGEShell );

BEGIN_DATADESC( CGEShell )
	// Function Pointers
	DEFINE_ENTITYFUNC( PlayerTouch ),
	DEFINE_THINKFUNC( ExplodeThink ),
END_DATADESC()


CGEShell::~CGEShell( void )
{
}

void CGEShell::Spawn( void )
{
	Precache();
	BaseClass::Spawn();

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;
	m_iBounceCount	= 0;

	// Default Damages they should be modified by the thrower
	SetDamage( 320 );
	SetDamageRadius( 260 );

	SetModel( "models/weapons/gl/grenadeprojectile.mdl" );
	SetSize( -Vector(3,3,3), Vector(3,3,3) );
	
	// So NPC's can "see" us
	AddFlag( FL_OBJECT );

	// Init our physics definition
	VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags() | FSOLID_TRIGGER, false );
	SetMoveType( MOVETYPE_VPHYSICS );
	SetCollisionGroup( COLLISION_GROUP_GRENADE );

	// Detonate after a set amount of time
	SetThink( &CGEShell::ExplodeThink );
	SetNextThink( gpGlobals->curtime + GE_SHELL_FUSETIME );

	// Instantly Explode when we hit a player
	SetTouch( &CGEShell::PlayerTouch );

	// Give us a little ass juice
	CreateSmokeTrail();

	// No air-drag, lower friction (NOTE: Cannot set specific gravity for HAVOK physics)
	VPhysicsGetObject()->EnableDrag( false );
	float flDamping = 0.0f;
	float flAngDamping = 0.4f;
	VPhysicsGetObject()->SetDamping( &flDamping, &flAngDamping );

	m_flFloorTimeout = gpGlobals->curtime + 0.75f;

	AddSolidFlags( FSOLID_NOT_STANDABLE );
}

void CGEShell::Precache( void )
{
	PrecacheModel("models/weapons/gl/grenadeprojectile.mdl");
	BaseClass::Precache();
}

void CGEShell::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->SetVelocityInstantaneous( &velocity, &angVelocity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGEShell::CreateSmokeTrail( void )
{
	if ( m_hSmokeTrail )
		return;

	// Smoke trail.
	if ( (m_hSmokeTrail = SmokeTrail::CreateSmokeTrail()) != NULL )
	{
		m_hSmokeTrail->m_Opacity = 0.25f;
		m_hSmokeTrail->m_SpawnRate = 70;
		m_hSmokeTrail->m_ParticleLifetime = 0.9f;
		m_hSmokeTrail->m_StartColor.Init( 0.5f, 0.5f , 0.5f );
		m_hSmokeTrail->m_EndColor.Init( 0, 0, 0 );
		m_hSmokeTrail->m_StartSize = 10;
		m_hSmokeTrail->m_EndSize = 22;
		m_hSmokeTrail->m_SpawnRadius = 4;
		m_hSmokeTrail->m_MinSpeed = 4;
		m_hSmokeTrail->m_MaxSpeed = 24;
		
		m_hSmokeTrail->SetLifetime( 10.0f );
		m_hSmokeTrail->FollowEntity( this, "smoke" );
	}
}

void CGEShell::ExplodeThink() 
{
	if( m_hSmokeTrail )
	{
		m_hSmokeTrail->SetLifetime( 0.5f );
		m_hSmokeTrail = NULL;
	}

	Explode();
}

void CGEShell::PlayerTouch( CBaseEntity *pOther )
{
	// This catches self hits
	if ( !PassServerEntityFilter( this, pOther) )
		return;

	// Always explode immediately upon hitting another player
	if ( pOther->IsPlayer() || pOther->IsNPC() )
		SetNextThink( gpGlobals->curtime );
}

// This function is in replacement of "Touch" since we are using the VPhysics
// and that does not stimulate "Touch" responses if we collide with the world
void CGEShell::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	// Call the baseclass first so we don't interfere with the normal running of things
	BaseClass::VPhysicsCollision( index, pEvent );

	// Grab what we hit
	CBaseEntity *pOther = pEvent->pEntities[!index];

	if ( pOther->IsWorld() )
	{
		surfacedata_t *phit = physprops->GetSurfaceData( pEvent->surfaceProps[!index] );
		if ( phit->game.material == 'X' )
		{
			// Game Over, we hit the sky box, remove the rocket from the world (no explosion)
			PhysCallbackRemove( GetNetworkable() );
			return;
		}
	}

	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if ( !PassServerEntityFilter( this, pOther) )
		return;

	if ( !g_pGameRules->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) )
		return;

	trace_t tr;
	CollisionEventToTrace( index, pEvent, tr );

	// If we hit another player or hit the floor after a wall/ceiling bounce, instantly explode
	if ( pOther->IsPlayer() || pOther->IsNPC() || ( (m_iBounceCount > 0 || m_flFloorTimeout < gpGlobals->curtime) && tr.plane.normal.z < -0.6f ) )
	{
		SetNextThink( gpGlobals->curtime );
		return;
	}

	// Only lose 20% speed
	Vector vecFinalVelocity = pEvent->postVelocity[index];
	vecFinalVelocity.NormalizeInPlace();
	if ( !m_iBounceCount )
		vecFinalVelocity *= pEvent->collisionSpeed * 0.8f;
	else
		vecFinalVelocity *= pEvent->collisionSpeed;
	PhysCallbackSetVelocity( pEvent->pObjects[index], vecFinalVelocity );
	
	++m_iBounceCount;
}

int CGEShell::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	if ( m_takedamage == DAMAGE_NO )
		return 0;

	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage.
	if( inputInfo.GetDamageType() & DMG_BLAST )
	{
		m_iHealth -= inputInfo.GetDamage();
		if ( m_iHealth <= 0 )
			ExplodeThink();

		return inputInfo.GetDamage();
	}

	return 0;
}
