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

	void InputChangeCollideGroup(inputdata_t &inputdata);
};

LINK_ENTITY_TO_CLASS(func_ge_brush, CGEBrush);

BEGIN_DATADESC(CGEBrush)

DEFINE_KEYFIELD(m_iCustomCollideGroup, FIELD_INTEGER, "CollisionGroup"),

DEFINE_INPUTFUNC(FIELD_INTEGER, "SetCollideGroup", InputChangeCollideGroup),

END_DATADESC()


CGEBrush::CGEBrush()
{
	// Set these values here incase they were not assigned by the mapper somehow.
	m_iCustomCollideGroup = COLLISION_GROUP_NONE;
}

void CGEBrush::Spawn(void)
{
	BaseClass::Spawn();

	SetCollisionGroup(m_iCustomCollideGroup);
}

void CGEBrush::InputChangeCollideGroup(inputdata_t &inputdata)
{
	m_iCustomCollideGroup = inputdata.value.Int();
	SetCollisionGroup(m_iCustomCollideGroup);
}