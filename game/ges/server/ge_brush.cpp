///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_door.cpp
// Description:
//      func_brush that assigns itself whatever collision group the mapper wants.
//		
//
// Created On: 9/30/2015
// Created By: Check Github for list of contributors
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "modelentities.h"
#include "entitylist.h"
#include "ge_armorvest.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CGEBrush : public CFuncBrush
{
public:
	DECLARE_CLASS(CGEBrush, CFuncBrush);
	DECLARE_DATADESC();

	CGEBrush();

	int m_iCustomCollideGroup; // Collision group of the brush

	void Spawn();
	virtual void	RemoveTouch(CBaseEntity *pOther);

	void InputChangeCollideGroup(inputdata_t &inputdata);
};

LINK_ENTITY_TO_CLASS(func_ge_brush, CGEBrush);

BEGIN_DATADESC(CGEBrush)

DEFINE_KEYFIELD(m_iCustomCollideGroup, FIELD_INTEGER, "CollisionGroup"),
DEFINE_ENTITYFUNC(RemoveTouch),

DEFINE_INPUTFUNC(FIELD_INTEGER, "SetCollideGroup", InputChangeCollideGroup),

END_DATADESC()


#define SF_REMOVE_TOUCHING 4

CGEBrush::CGEBrush()
{
	// Set these values here incase they were not assigned by the mapper somehow.
	m_iCustomCollideGroup = COLLISION_GROUP_NONE;
}

void CGEBrush::Spawn(void)
{
	BaseClass::Spawn();

	SetCollisionGroup(m_iCustomCollideGroup);

	if (HasSpawnFlags(SF_REMOVE_TOUCHING))
		SetTouch(&CGEBrush::RemoveTouch);
	else
		SetTouch(NULL);
}

void CGEBrush::RemoveTouch(CBaseEntity *pOther)
{
	if (!pOther || pOther->IsPlayer() || pOther->IsNPC()) // Don't remove players or bots.
		return;

	if (!strncmp(pOther->GetClassname(), "item_armorvest", 14)) // We shouldn't remove armor, just force it to respawn.
		((CGEArmorVest*)pOther)->Respawn();
	else
		UTIL_Remove(pOther);
}

void CGEBrush::InputChangeCollideGroup(inputdata_t &inputdata)
{
	m_iCustomCollideGroup = inputdata.value.Int();
	SetCollisionGroup(m_iCustomCollideGroup);
}