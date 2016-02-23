///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_door_interp.cpp
// Description:
//      Emulates ge_door movement on the client for less jittery elevators.  Does not move at all on the server
//		so make sure anything parented to it is purely visual!
//		
//
// Created On: 2/20/2016
// Created By: Check Github for list of contributors
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "doors.h"
#include "ge_door.h"
#include "entitylist.h"
#include "physics.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
#include "physics_npc_solver.h"
#include "gemp_gamerules.h"

#include "ge_door_interp.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(ge_door_interp, CGEDoorInterp);

BEGIN_DATADESC(CGEDoorInterp)
DEFINE_KEYFIELD(m_sTargetDoor, FIELD_STRING, "TargetDoor"),
DEFINE_KEYFIELD(m_flAccelDampener, FIELD_FLOAT, "AccelDampener"),
DEFINE_KEYFIELD(m_flMoveDelay, FIELD_FLOAT, "MoveDelay"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CGEDoorInterp, DT_GEDoorInterp)
SendPropFloat(SENDINFO(m_flAccelSpeedT)),
SendPropFloat(SENDINFO(m_flMinSpeedT)),
SendPropFloat(SENDINFO(m_flThinkIntervalT)),
SendPropFloat(SENDINFO(m_flDeccelDistT)),
SendPropFloat(SENDINFO(m_flStartMoveTimeT)),
SendPropFloat(SENDINFO(m_flMoveDistanceT)),
SendPropFloat(SENDINFO(m_flMaxSpeedT)),
SendPropFloat(SENDINFO(m_flAccelDampener)),
SendPropFloat(SENDINFO(m_flMoveDelay)),
SendPropVector(SENDINFO(m_vecFinalDestT)),
SendPropVector(SENDINFO(m_vecPosSynch)),
SendPropVector(SENDINFO(m_vecVelSynch)),

SendPropExclude("DT_BaseEntity", "m_vecAbsOrigin"), //Don't tell the client about the location or velocity of this entity as that completely ruins the point.
SendPropExclude("DT_BaseEntity", "m_vecOrigin"),
SendPropExclude("DT_BaseEntity", "m_vecAbsVelocity"),
SendPropExclude("DT_BaseEntity", "m_vecVelocity"),
END_SEND_TABLE()

CGEDoorInterp::CGEDoorInterp()
{
	m_flAccelDampener = 0.8;
	m_flMoveDelay = 0.1;
	m_vecPosSynch = vec3_origin;
}

void CGEDoorInterp::Spawn(void)
{
	m_vecPosSynch = vec3_origin;

	BaseClass::Spawn();

	m_pTargetDoor = GetTarget();

	if (m_pTargetDoor)
	{
		m_pTargetDoor->SetInterpEnt(GetBaseEntity());
	}

	// Our target door may not have spawned and calculated all of its values yet, so we need to wait before copying them.
	SetThink(&CGEDoorInterp::PostSpawnInit);
	SetNextThink(gpGlobals->curtime + 0.1);
}

void CGEDoorInterp::PostSpawnInit(void)
{
	if (m_pTargetDoor)
	{
		m_flAccelSpeedT.Set(m_pTargetDoor->m_flAccelSpeed);
		m_flMinSpeedT.Set(m_pTargetDoor->m_flMinSpeed);
		m_flThinkIntervalT.Set(m_pTargetDoor->m_flThinkInterval);
		m_flMaxSpeedT.Set(m_pTargetDoor->m_flSpeed);

		m_vecPosSynch = m_pTargetDoor->GetAbsOrigin();
		m_vecVelSynch = m_pTargetDoor->GetAbsVelocity();
		SetAbsOrigin(m_vecPosSynch);
		SetAbsVelocity(vec3_origin);

		m_pTargetDoor->SetInterpEnt(GetBaseEntity());
	}
	else
	{
		Warning("ge_door_interp without specified ge_door!\n");
		m_vecPosSynch = GetAbsOrigin();
		m_vecVelSynch = GetAbsVelocity();
	}
}

CGEDoor *CGEDoorInterp::GetTarget() //No need for me to make this twice, it's just a modified version of the ge_door function.
{
	string_t newPartner = m_sTargetDoor;
	CGEDoor *partnerEnt = NULL;

	if (newPartner != NULL_STRING) // Make sure the mapper assigned a partner.
	{
		CBaseEntity *pPartner = gEntList.FindEntityByName(NULL, newPartner, NULL);

		if (pPartner == NULL)
			Msg("Entity %s(%s) has bad target entity %s\n", STRING(GetEntityName()), GetDebugName(), STRING(newPartner));
		else
			partnerEnt = static_cast<CGEDoor*>(pPartner); //Actually return a partner ent.
	}
	return partnerEnt;
}

void CGEDoorInterp::HookMovementValues()
{
	if (!m_pTargetDoor)
		return;

	m_flDeccelDistT.Set(m_pTargetDoor->m_flDeccelDist);
	m_flStartMoveTimeT.Set(m_pTargetDoor->m_flStartMoveTime);
	m_flMoveDistanceT.Set(m_pTargetDoor->m_flMoveDistance);
	m_vecFinalDestT = m_pTargetDoor->m_vecFinalDest;
	m_vecPosSynch = m_pTargetDoor->GetAbsOrigin();
	m_vecVelSynch = m_pTargetDoor->GetAbsVelocity();
}