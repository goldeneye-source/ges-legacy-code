//////////  Copyright © 2008, Goldeneye Source. All rights reserved. ///////////
// 
// ge_ammocrate.cpp
//
// Description:
//      An ammo crate that gives a certain amount of ammo of different types
//
// Created On: 3/22/2008 1200
// Created By: KillerMonkey
////////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "ge_armorvest.h"
#include "ge_shareddefs.h"
#include "gemp_gamerules.h"
#include "ge_player.h"
#include "gebot_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( item_armorvest, CGEArmorVest );
LINK_ENTITY_TO_CLASS( item_armorvest_half, CGEArmorVest );
PRECACHE_REGISTER( item_armorvest );
PRECACHE_REGISTER( item_armorvest_half );

BEGIN_DATADESC( CGEArmorVest )
	// Function Pointers
	DEFINE_ENTITYFUNC( ItemTouch ),
	// Inputs
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
END_DATADESC();

#define AliveThinkInterval		1.0f

CGEArmorVest::CGEArmorVest( void )
{
	m_bEnabled = true;
	m_iAmount = MAX_ARMOR;
}

void CGEArmorVest::Spawn( void )
{
	Precache();

	if ( !Q_strcmp(GetClassname(), "item_armorvest_half") )
		m_iAmount = MAX_ARMOR / 2.0f;

	SetModel( "models/weapons/armor/armor.mdl" );

	SetOriginalSpawnOrigin( GetAbsOrigin() );
	SetOriginalSpawnAngles( GetAbsAngles() );

	SetCollisionGroup( COLLISION_GROUP_WEAPON ); // Might as well treat this the same way we do weapons.

	BaseClass::Spawn();

	// So NPC's can "see" us
	AddFlag( FL_OBJECT );

	// Override base's ItemTouch for NPC's
	SetTouch( &CGEArmorVest::ItemTouch );

	// Start thinking like a healthy, alive armorvest.
	SetThink(&CGEArmorVest::AliveThink);
	SetNextThink(gpGlobals->curtime + AliveThinkInterval);
}

void CGEArmorVest::Precache( void )
{
	PrecacheModel( "models/weapons/armor/armor.mdl" );
	PrecacheScriptSound( "ArmorVest.Pickup" );

	BaseClass::Precache();
}

CBaseEntity *CGEArmorVest::Respawn(void)
{
	BaseClass::Respawn();
	m_iSpawnpointsgoal = (int)(120 - 15 * sqrt((float)GEMPRules()->GetNumAlivePlayers()));
	m_iSpawnpoints = 0;
	SetNextThink(gpGlobals->curtime + 1);
	return this;
}

void CGEArmorVest::AliveThink(void)
{
	float distfromspawnersqr = (GetOriginalSpawnOrigin() - GetAbsOrigin()).LengthSqr();

	// If we're too far from our spawn location, instantly respawn.
	if (distfromspawnersqr > 65536)
	{
		// Destroy and remake the vphysics object to prevent oddities.
		VPhysicsDestroyObject();

		UTIL_SetOrigin( this, GetOriginalSpawnOrigin() );
		SetAbsAngles( GetOriginalSpawnAngles() );

		CreateItemVPhysicsObject();
	}

	SetNextThink(gpGlobals->curtime + AliveThinkInterval);
}

void CGEArmorVest::Materialize(void)
{
	// I repurposed the respawn -> materalize loop to demo the dynamic respawn concept.
	// May cause issues, create seperate loop if so.

	m_iSpawnpoints += CalcSpawnProgress();
	if (m_iSpawnpointsgoal < m_iSpawnpoints)
	{
		// Only materialize if we are enabled and allowed
		if (GEMPRules()->ArmorShouldRespawn() && m_bEnabled)
		{
			BaseClass::Materialize();
			// Override base's ItemTouch for NPC's
			SetTouch(&CGEArmorVest::ItemTouch);

			SetThink(&CGEArmorVest::AliveThink);
			SetNextThink(gpGlobals->curtime + AliveThinkInterval);
		}
	}
	else
		SetNextThink(gpGlobals->curtime + 1);
}

void CGEArmorVest::ItemTouch( CBaseEntity *pOther )
{
	if ( pOther->IsNPC() )
	{
		// If the NPC is a Bot hand it off to it's proxy, normal NPC's don't use armor
		CBaseCombatCharacter *pNPC = pOther->MyCombatCharacterPointer();
		CGEBotPlayer *pBot = ((CNPC_GEBase*)pOther)->GetBotPlayer();

		if ( pBot && pNPC )
		{
			// ok, a player is touching this item, but can he have it?
			if ( !g_pGameRules->CanHaveItem( pBot, this ) )
				return;

			if ( MyTouch( pBot ) )
			{
				SetTouch( NULL );
				SetThink( NULL );

				// player grabbed the item. 
				g_pGameRules->PlayerGotItem( pBot, this );
				Respawn();
			}
		}
	}
	else
	{
		BaseClass::ItemTouch( pOther );
	}
}

bool CGEArmorVest::MyTouch( CBasePlayer *pPlayer )
{
	CGEPlayer *pGEPlayer = dynamic_cast<CGEPlayer*>( pPlayer );
	if ( !pGEPlayer )
		return false;

	return pGEPlayer->AddArmor(m_iAmount) || GERules()->ShouldForcePickup(pPlayer, this);
}


// Finds two different values and uses them to compute a point total
// The first is a point value based on how many players are in range and how close they are.
// The second is the distance between the vest and the closest player.

// The closer the closest player is, the slower the armor will respawn.
// However, the weighted average of all the other nearby players can serve to counteract this.
// This means that heavily contested armor spawns still appear fairly frequently.
// A player within 64 units will completely freeze the timer, however.

int CGEArmorVest::CalcSpawnProgress()
{
	float closestlength = 262144;
	float currentlength = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CGEMPPlayer *pPlayer = ToGEMPPlayer(UTIL_PlayerByIndex(i));
		if (pPlayer && pPlayer->IsConnected())
		{
			currentlength = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			if (currentlength < closestlength)
				closestlength = currentlength;
		}
	}

	// Don't stand right on top of the armor spawn, loser.
	if (closestlength < 4096)
		return 4;

	// Ticks up at 6 points at minimum and 10 points at maximum.  Linear falloff.
	return (int)min((sqrt(closestlength)) / 126 + 6, 10);
}

void CGEArmorVest::OnEnabled( void )
{
	// Respawn and wait for materialize
	Respawn();
}

void CGEArmorVest::OnDisabled( void )
{
	// "Respawn", but we won't materialize
	Respawn();
}

void CGEArmorVest::SetEnabled( bool state )
{
	inputdata_t nullData;
	if ( state )
		InputEnable( nullData );
	else
		InputDisable( nullData );
}

void CGEArmorVest::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;
	OnEnabled();
}

void CGEArmorVest::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
	OnDisabled();
}

void CGEArmorVest::InputToggle( inputdata_t &inputdata )
{
	m_bEnabled = !m_bEnabled;
	m_bEnabled ? OnEnabled() : OnDisabled();
}

