///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_door_interp.h
// Description:
//      Func ge_door that moves independantly on client and server to prevent jittery elevators.
//		This entity is absurdly hacked together and only meant to serve one purpose.
//		It doesn't justify the amount of time which would be required for the overhaul neccecery for a sensible fix to elevator
//		jitter.
//
// Created On: 2/20/2016
// Created By: Check Github for list of contributors
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_GEDoorInterp : public C_BaseEntity
{
	DECLARE_CLASS(C_GEDoorInterp, C_BaseEntity);

public:
	DECLARE_CLIENTCLASS();

	C_GEDoorInterp();

	void Spawn();
	void PostDataUpdate(DataUpdateType_t updateType);
	void MoveThink();
	void Simulate();

	//Copies of the relevant ge_door info to send to clients.
	float m_flAccelSpeedT;
	float m_flMinSpeedT;
	float m_flThinkIntervalT;
	float m_flDeccelDistT;
	float m_flStartMoveTimeT;
	float m_flMoveDistanceT;
	float m_flMaxSpeedT;
	float m_flMoveDelay;
	float m_flAccelDampener;
	Vector m_vecFinalDestT;
	Vector m_vecPosSynch;
	Vector m_vecVelSynch;

private:
	float m_flAccelSpeed;
	float m_flMoveStartTime;
	float m_flLastMoveCalc;
	float m_flLastPosCalc;
	float m_flNextSimThink;
	Vector m_vecAccelDir;
	Vector m_vecSimPos;
	Vector m_vecSimVel;
	Vector m_vecCurrentDest;
	bool m_bIsMoving;
	bool m_bNeedsSpawnData;
};

IMPLEMENT_CLIENTCLASS_DT(C_GEDoorInterp, DT_GEDoorInterp, CGEDoorInterp)
	RecvPropFloat(RECVINFO(m_flAccelSpeedT)),
	RecvPropFloat(RECVINFO(m_flMinSpeedT)),
	RecvPropFloat(RECVINFO(m_flThinkIntervalT)),
	RecvPropFloat(RECVINFO(m_flDeccelDistT)),
	RecvPropFloat(RECVINFO(m_flStartMoveTimeT)),
	RecvPropFloat(RECVINFO(m_flMoveDistanceT)),
	RecvPropFloat(RECVINFO(m_flMaxSpeedT)),
	RecvPropFloat(RECVINFO(m_flAccelDampener)),
	RecvPropFloat(RECVINFO(m_flMoveDelay)),
	RecvPropVector(RECVINFO(m_vecFinalDestT)),
	RecvPropVector(RECVINFO(m_vecPosSynch)),
	RecvPropVector(RECVINFO(m_vecVelSynch)),
END_RECV_TABLE()

C_GEDoorInterp::C_GEDoorInterp()
{
	m_bIsMoving = false;
	m_bNeedsSpawnData = true;
	m_vecPosSynch = vec3_origin;
	m_vecVelSynch = vec3_origin;
	m_flAccelDampener = 0.8;
	m_flMoveDelay = 0.1;
	m_flNextSimThink = 0;
	m_flMoveStartTime = -1;
	m_flAccelSpeedT = 0;
}

void C_GEDoorInterp::Spawn(void)
{
	m_bIsMoving = false;
	m_bNeedsSpawnData = true;
	m_vecPosSynch = vec3_origin;
	m_vecVelSynch = vec3_origin;

	BaseClass::Spawn();

	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

void C_GEDoorInterp::PostDataUpdate(DataUpdateType_t updateType)
{
	BaseClass::PostDataUpdate(updateType);

	if (m_bNeedsSpawnData)
	{
		SetAbsOrigin(m_vecPosSynch);
	}

	if (m_vecFinalDestT != m_vecCurrentDest)
	{
		m_bNeedsSpawnData = false;
		m_vecSimVel = m_vecVelSynch;
		m_vecSimPos = m_vecPosSynch;
		SetAbsOrigin(m_vecSimPos);
		SetAbsVelocity(vec3_origin);
		m_vecAccelDir = (m_vecFinalDestT - m_vecSimPos) / (m_vecFinalDestT - m_vecSimPos).Length();
		m_flNextSimThink = gpGlobals->curtime;
		m_vecCurrentDest = m_vecFinalDestT;
		m_flMoveStartTime = gpGlobals->curtime + m_flMoveDelay;
	}
}

void C_GEDoorInterp::Simulate()
{
	if (m_bIsMoving && IsAbsQueriesValid())
	{
		// However, we want to use roughly the same velocity integration intervals as the server so we don't desynch from the actual door due to integrator error.
//		if (m_flNextSimThink < gpGlobals->curtime)
		MoveThink();

		//float calcTime = gpGlobals->curtime - m_flLastPosCalc;
		//m_flLastPosCalc = gpGlobals->curtime;

		// We need to integrate posistion every frame for smooth movement
//		m_vecSimPos = m_vecSimPos + m_vecSimVel * calcTime;
		
//		SetAbsOrigin(m_vecSimPos);


		if (IsPlayerSimulated())
			Warning("We are player simulated!\n");

		if (ShouldPredict())
			Warning("We are predicted!\n");

		if (ShouldInterpolate())
			Warning("We are Interpolated!\n");
	}
	else if (m_flMoveStartTime != -1 && m_flMoveStartTime < gpGlobals->curtime)
	{
		m_flLastPosCalc = m_flLastMoveCalc = gpGlobals->curtime;
		m_bIsMoving = true;
		m_flMoveStartTime = -1;
	}
}

void C_GEDoorInterp::MoveThink(void)
{
	Vector framevelocity, remainingvector;
	QAngle frameangularvelocity, remainingangle;
	float framespeed, remainingdist, directionmetric, speedmetric, covereddist;

	framevelocity = m_vecSimVel;
	remainingvector = m_vecFinalDestT - m_vecSimPos;

	framespeed = framevelocity.Length();
	remainingdist = remainingvector.Length();

	directionmetric = (remainingvector.x + remainingvector.y + remainingvector.z) * (m_vecAccelDir.x + m_vecAccelDir.y + m_vecAccelDir.z);
	speedmetric = (framevelocity.x + framevelocity.y + framevelocity.z) * (m_vecAccelDir.x + m_vecAccelDir.y + m_vecAccelDir.z);

	float calctime = (gpGlobals->curtime - m_flLastMoveCalc);
	m_flLastMoveCalc = gpGlobals->curtime;
	float accelspeed = m_flAccelSpeedT * m_flAccelDampener;
	covereddist = max(m_flMoveDistanceT - remainingdist, 0);

	// If direction metric is negative then the two vectors point in opposite directions.  Only works because there are only two possible directions for them to point in.
	// If they point in opposite directions then we have moved past our objective.
	if (directionmetric < 0)
		remainingdist *= -1;

	//If speedmetric is negative the door is moving in the direction opposite its acceleration vector
	if (speedmetric < 0)
	{
		framespeed *= -1;
	}

	// Now calculate our speed for the next interval.

	//DeccelDist should always be checked first because it can sometimes overlap with acceltime.  We use a kinematic equation
	//for this instead of the integrator to reduce final deceleration error as much as possible.
	//Cap the minimum at minspeed so mappers can have their doors slam shut if they want them to.
	if (remainingdist < m_flDeccelDistT && framespeed >= 0) //Make sure we're actually moving towards the destination while also being within range of it.
		framespeed = clamp(sqrt(2 * remainingdist * accelspeed), m_flMinSpeedT, framespeed + accelspeed *calctime); //Make sure we don't suddenly speed up or pass our minimum speed.
	else if (covereddist < m_flDeccelDistT && framespeed < -accelspeed * calctime) //The opposite case, where we're moving towards the other end posistion and risk going past it.  Use an equation here too to prevent that.
		framespeed = max(framespeed + accelspeed * calctime, -sqrt(2 * covereddist * accelspeed)); //Make sure we can't randomly speed up.  Both values are negative here so we use max instead of min.
	else
		framespeed = min(framespeed + accelspeed *calctime, m_flMaxSpeedT); //Cap it so the door will move at max speed once it hits it.

	if (remainingdist > 0) // Still has distance to move so keep integrating
	{
		m_vecSimVel = framespeed * m_vecAccelDir;

		m_vecSimPos = m_vecSimPos + m_vecSimVel * calctime;

		SetAbsOrigin(m_vecSimPos);
		SetAbsVelocity(vec3_origin);

		// If the distance the door will move in the next interval is greater than the remaining distance, the door is about to move past the end point and then teleport back!
		if (framespeed * m_flThinkIntervalT > remainingdist)
			m_flNextSimThink = gpGlobals->curtime + remainingdist / framespeed; //Correct for this by adjusting the interval to match up with the end time.
		else if (-framespeed * m_flThinkIntervalT > covereddist) // We're about to move past the other endpos while changing directions!
			m_flNextSimThink = gpGlobals->curtime + covereddist / -framespeed;
		else
			m_flNextSimThink = gpGlobals->curtime + m_flThinkIntervalT;
	}
	else // Has hit or gone past the ending posistion so finalize the transistion and stop moving
	{
		m_vecSimVel = vec3_origin;
		m_vecSimPos = m_vecFinalDestT;
		SetAbsVelocity(m_vecSimVel);
		SetAbsOrigin(m_vecSimPos);
		m_bIsMoving = false;
	}
}