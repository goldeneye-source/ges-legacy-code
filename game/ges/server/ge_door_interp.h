///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_door_interp.h
// Description:
//      Func ge_door that moves independantly on client and server to prevent jittery elevators.
//		
//
// Created On: 2/20/2016
// Created By: Check Github for list of contributors
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"

class CGEDoorInterp : public CBaseEntity
{
public:
	DECLARE_CLASS(CGEDoorInterp, CBaseEntity);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CGEDoorInterp();

	string_t m_sTargetDoor; // Name of the door this entity mimics

	void Spawn();
	virtual void HookMovementValues();
	virtual void PostSpawnInit();

	CGEDoor *GetTarget();

	CGEDoor *m_pTargetDoor; // Pointer to target door

	//Copies of the relevant ge_door info to send to clients.
	CNetworkVar(float, m_flAccelSpeedT);
	CNetworkVar(float, m_flMinSpeedT);
	CNetworkVar(float, m_flThinkIntervalT);
	CNetworkVar(float, m_flDeccelDistT);
	CNetworkVar(float, m_flStartMoveTimeT);
	CNetworkVar(float, m_flMoveDistanceT);
	CNetworkVar(float, m_flMaxSpeedT);
	CNetworkVector(m_vecFinalDestT);
	CNetworkVector(m_vecPosSynch);
	CNetworkVector(m_vecVelSynch);

	CNetworkVar(float, m_flAccelDampener);
	CNetworkVar(float, m_flMoveDelay);
};