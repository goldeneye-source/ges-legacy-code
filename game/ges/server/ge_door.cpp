///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_door.cpp
// Description:
//      Fancy door that accelerates and can change direction during motion.  Also supports paired doors to
//		avoid a lot of the nonsense caused by double door entity systems.
//
// Created On: 9/30/2015
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

#ifdef HL1_DLL
#include "filters.h"
#endif

#ifdef CSTRIKE_DLL
#include "KeyValues.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(func_ge_door, CGEDoor);

BEGIN_DATADESC(CGEDoor)

DEFINE_KEYFIELD(m_flacceltime, FIELD_FLOAT, "AccelerationTime"),
DEFINE_KEYFIELD(m_flAccelSpeed, FIELD_FLOAT, "AccelerationSpeed"),
DEFINE_KEYFIELD(m_flMinSpeed, FIELD_FLOAT, "MinimumSpeed"),
DEFINE_KEYFIELD(m_flTriggerThreshold, FIELD_FLOAT, "TriggerThreshold"),
DEFINE_KEYFIELD(m_iUseLimit, FIELD_INTEGER, "UseLimit"),
DEFINE_KEYFIELD(m_sPartner, FIELD_STRING, "PartnerDoor"),
DEFINE_KEYFIELD(m_flThinkInterval, FIELD_FLOAT, "ThinkInterval"),
DEFINE_INPUTFUNC(FIELD_VOID, "ForceToggle", InputForceToggle),
DEFINE_OUTPUT(m_FirstOpen, "OnFirstOpen"),
DEFINE_OUTPUT(m_FirstClose, "OnFirstClose"),
DEFINE_OUTPUT(m_PassThreshold, "OnCrossThreshold"),
DEFINE_OUTPUT(m_FarThreshold, "OnCrossToThresholdFarSide"),
DEFINE_OUTPUT(m_NearThreshold, "OnCrossToThresholdNearSide"),

END_DATADESC()

// Boost to the doors accel speed when it's changing direction
#define DOOR_DECCELBOOST 1.1f

CGEDoor::CGEDoor()
{
	// Set these values here incase they were not assigned by the mapper somehow.
	m_flacceltime = 1;
	m_flTriggerThreshold = 0.5;
	m_iUseLimit = 3;
	m_flMinSpeed = 5;
	m_flThinkInterval = 0.1;
	m_flDeccelBoost = 1.0f;

	m_flAccelSpeed = 0;
	m_flDeccelDist = 0;
	m_flStartMoveTime = 0;
}

void CGEDoor::Spawn(void)
{
	BaseClass::Spawn();
	BuildPartnerList();

	//Calculate kinematic constants essential for acceleration
	if (m_flAccelSpeed == 0)
		m_flAccelSpeed = m_flSpeed / m_flacceltime; //by default speed is calculated from time
	else
		m_flacceltime = m_flSpeed / m_flAccelSpeed; //but if the user sets speed then that will be used instead.

	if (!IsRotatingDoor())
		m_flMoveDistance = (m_vecPosition2 - m_vecPosition1).Length();


	m_flTriggerDistance = m_flMoveDistance * (1 - m_flTriggerThreshold);

	//If this door can do damage, add it to the trap list.
	if (m_flBlockDamage != 0)
		GEMPRules()->AddTrapToList(this);

	m_flStartMoveTime = 0;
	m_flLastMoveCalc = 0;
	m_pLastActivator = NULL;
	m_iCurrentUses = 0;
	m_bPassedThreshold = false;
}

void CGEDoor::CalcMovementValues(Vector startpos, Vector endpos)
{
	float traveldistance, travelmetric, speed;

	Vector travelvector = endpos - startpos;
	Vector velocity = GetLocalVelocity();
	QAngle angvelocity = GetLocalAngularVelocity();
	QAngle moveAngle;

	// The kinematic calculations are the same, but rotating doors require different data gathering.
	if (IsRotatingDoor())
	{
		moveAngle = m_angFinalDest - GetLocalAngles();
		traveldistance = moveAngle.Length();
		m_angAccelDir = moveAngle / traveldistance;
		m_vecFinalDest = GetLocalOrigin();
		speed = angvelocity.Length();

		travelmetric = (angvelocity.x + angvelocity.y + angvelocity.z)*(moveAngle.x + moveAngle.y + moveAngle.z);
	}
	else
	{
		traveldistance = travelvector.Length();
		m_vecAccelDir = travelvector / traveldistance;
		m_vecFinalDest = endpos;
		speed = velocity.Length();

		travelmetric = (velocity.x + velocity.y + velocity.z)*(travelvector.x + travelvector.y + travelvector.z);
	}

	//Basic kinematics to solve for the distance the door will travel while accelerating in order to determine when to start decelerating.
	m_flDeccelDist = 0.5 * m_flAccelSpeed * m_flacceltime * m_flacceltime;

	// The global error of euler integrators is proportional to stepsize.
	// we need to make sure tha the door can't go through its frame when changing directions while near it due to moving
	// faster than the equations predict.
	// To accomplish this we can just boost the deceleration speed by a calculated value.
	// however i'm kind of pressed for time so i'll settle for this fudged value for now
	// it might be worth just using a differential equation for the other bound of the door instead of bothering with this.

	m_flDeccelBoost = 1 + m_flThinkInterval * speed / m_flacceltime;

	//We are currently traveling in the opposite direction, meaning we will actually start accelerating toward the endpos from further away than expected.
	if (travelmetric < 0)
	{
		traveldistance += speed*speed / (2 * m_flAccelSpeed * m_flDeccelBoost);
	}
	else if (travelmetric > 0)
	{
		// Even if moving in the same direction we can add to travel distance because we can pretend
		// the door started moving at 0 velocity from a posistion past the true posistion.
		traveldistance += speed*speed / (2 * m_flAccelSpeed); //No deccelboost this time though
	}

	if (m_flDeccelDist > traveldistance / 2)
	{
		m_flDeccelDist = traveldistance / 2;
	}

	DevMsg("DeccelDist calculated to be %f \n", m_flDeccelDist);
	m_flTargetDist = traveldistance;
	m_flStartMoveTime = gpGlobals->curtime;

	if (m_flLastMoveCalc == 0)
		m_flLastMoveCalc = gpGlobals->curtime;
}

void CGEDoor::SetPartner(string_t newPartner)
{
	m_sPartner = newPartner;

	BuildPartnerList();
}

CGEDoor *CGEDoor::GetPartner()
{
	string_t newPartner = m_sPartner;
	CGEDoor *partnerEnt = NULL;

	if (newPartner != NULL_STRING) // Make sure the mapper assigned a partner.
	{
		CBaseEntity *pPartner = gEntList.FindEntityByName(NULL, newPartner, NULL);

		if (pPartner == NULL)
			Msg("Entity %s(%s) has bad partner entity %s\n", STRING(GetEntityName()), GetDebugName(), STRING(newPartner));
		else if (pPartner != NULL && Q_stricmp(pPartner->GetClassname(), "CGEDoor"))
			partnerEnt = static_cast<CGEDoor*>(pPartner); //Actually return a partner ent.
		else
			Msg("Entity %s(%s) tried to adopt partner %s but they are not a ge_door!\n", STRING(GetEntityName()), GetDebugName(), STRING(newPartner));
	}
	return partnerEnt;
}

void CGEDoor::BuildPartnerList()
{
	m_pPartnerEnts.RemoveAll();
	CGEDoor *curdoor = this;

	for (int i = 0; i < 128; i++)
	{
		curdoor = curdoor->GetPartner();
		if (curdoor != NULL && curdoor != this && !m_pPartnerEnts.HasElement(curdoor))
			m_pPartnerEnts.AddToTail(curdoor);
		else
			break;
	}


	DevMsg("Partnerlist count for door is %d \n", m_pPartnerEnts.Count());

	Vector totalpos = GetAbsOrigin();

	for (int i = 0; i < m_pPartnerEnts.Count(); i++)
		totalpos += m_pPartnerEnts[i]->GetAbsOrigin();

	m_vecGroupCenter = totalpos / (m_pPartnerEnts.Count() + 1);
}

//-----------------------------------------------------------------------------
// Purpose: When fired door and partner will start moving
// Output : int
//-----------------------------------------------------------------------------
int CGEDoor::DoorActivate()
{
	if (m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP) // door should close
		DoorGroupGoDown(true);
	else // open door
		DoorGroupGoUp(true);

	return 1;
}

void CGEDoor::DoorGroupGoUp(bool triggerself)
{
	if (triggerself)
		DoorGoUp();

	for (int i = 0; i < m_pPartnerEnts.Count(); i++)
	{
		m_pPartnerEnts[i]->m_hActivator = m_hActivator;
		m_pPartnerEnts[i]->DoorGoUp();
	}
}

void CGEDoor::DoorGroupGoDown(bool triggerself)
{
	if (triggerself)
		DoorGoDown();

	for (int i = 0; i < m_pPartnerEnts.Count(); i++)
	{
		m_pPartnerEnts[i]->m_hActivator = m_hActivator;
		m_pPartnerEnts[i]->DoorGoDown();
	}
}

void CGEDoor::DoorGoUp(void)
{
	edict_t	*pevActivator;

	UpdateAreaPortals(true);

	// emit door moving and stop sounds on CHAN_STATIC so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		// If we're not moving already, start the moving noise
		if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN)
		{
			StartMovingSound();
		}
	}

	if (m_toggle_state != TS_GOING_DOWN && m_toggle_state != TS_GOING_UP)
		m_FirstOpen.FireOutput(this, this);

	m_toggle_state = TS_GOING_UP;

	if (IsRotatingDoor())
	{
		float	sign = 1.0f;

		// We are opening from a non-closed posistion which means we need to reverse direction without concern for the player's orientation.
		if (m_vecAngle1 != GetLocalAngles())
		{
			if ((m_vecAngle2 - GetLocalAngles()).Length() > m_flMoveDistance)
				sign *= -1;
		}

		else if (m_hActivator != NULL)
		{
			pevActivator = m_hActivator->edict();

			if (!HasSpawnFlags(SF_DOOR_ONEWAY) && m_vecMoveAng.y) 		// Y axis rotation, move away from the player
			{
				// Positive is CCW, negative is CW, so make 'sign' 1 or -1 based on which way we want to open.
				// Important note:  All doors face East at all times, and twist their local angle to open.
				//					So you can't look at the door's facing to determine which way to open.

				Vector nearestPoint;
				Vector activatorToNearestPoint = GetAbsOrigin() - m_hActivator->GetAbsOrigin();
				activatorToNearestPoint.z = 0;
				VectorNormalize(activatorToNearestPoint);

				Vector activatorToOrigin;

				if (GetAbsOrigin() == m_vecGroupCenter)
					AngleVectors(m_hActivator->EyeAngles(), &activatorToOrigin);
				else
					activatorToOrigin = m_vecGroupCenter - m_hActivator->GetAbsOrigin();

				activatorToOrigin.z = 0;
				VectorNormalize(activatorToOrigin);

				Vector cross = activatorToOrigin.Cross(activatorToNearestPoint);

				if (cross.z < 0.0f)
				{
					sign = -1.0f;
				}
			}
		}
		m_angFinalDest = m_vecAngle2*sign;

		// The function will detect that this is a rotating door and use m_angFinalDest to calculate the angles, so we just pass null paramters to it here.
		CalcMovementValues(Vector(0, 0, 0), Vector(0, 0, 0));
		m_movementType = 2;
	}
	else
	{
		CalcMovementValues(GetLocalOrigin(), m_vecPosition2);
		m_movementType = 1;
	}

	SetMoveDoneTime((m_flacceltime + m_flTargetDist / m_flSpeed) * 5); //This has to be greater than 0 or the door won't move.  Just set it to a time that will always be longer than the movetime.

	SetThink(&CGEDoor::MoveThink);
	MoveThink();

	//Fire our open ouput
	m_OnOpen.FireOutput(this, this);
}

void CGEDoor::DoorGoDown(void)
{
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		// If we're not moving already, start the moving noise
		if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN)
		{
			StartMovingSound();
		}
	}

	if (m_toggle_state != TS_GOING_DOWN && m_toggle_state != TS_GOING_UP)
		m_FirstClose.FireOutput(this, this);

	m_toggle_state = TS_GOING_DOWN;

	if (IsRotatingDoor())
	{
		m_angFinalDest = m_vecAngle1;
		m_movementType = 2;
		CalcMovementValues(Vector(0, 0, 0), Vector(0, 0, 0));
	}
	else
	{
		m_movementType = 1;
		CalcMovementValues(GetLocalOrigin(), m_vecPosition1);
	}

	SetMoveDoneTime((m_flacceltime + m_flTargetDist / m_flSpeed) * 5);

	SetThink(&CGEDoor::MoveThink);
	MoveThink();

	//Fire our closed output
	m_OnClose.FireOutput(this, this);
}

void CGEDoor::MoveThink(void)
{
	Vector framevelocity, remainingvector;
	QAngle frameangularvelocity, remainingangle;
	float prevframespeed, framespeed, remainingdist, directionmetric, speedmetric, threshdist;

	// The velocity and destinations of rotating doors come from different places
	if (IsRotatingDoor())
	{
		frameangularvelocity = GetLocalAngularVelocity();
		remainingangle = m_angFinalDest - GetLocalAngles();

		framespeed = frameangularvelocity.Length();
		prevframespeed = framespeed;
		remainingdist = remainingangle.Length();

		directionmetric = (remainingangle.x + remainingangle.y + remainingangle.z) * (m_angAccelDir.x + m_angAccelDir.y + m_angAccelDir.z);
		speedmetric = (frameangularvelocity.x + frameangularvelocity.y + frameangularvelocity.z) * (m_angAccelDir.x + m_angAccelDir.y + m_angAccelDir.z);
	}
	else
	{
		framevelocity = GetLocalVelocity();
		remainingvector = m_vecFinalDest - GetLocalOrigin();

		framespeed = framevelocity.Length();
		prevframespeed = framespeed;
		remainingdist = remainingvector.Length();

		directionmetric = (remainingvector.x + remainingvector.y + remainingvector.z) * (m_vecAccelDir.x + m_vecAccelDir.y + m_vecAccelDir.z);
		speedmetric = (framevelocity.x + framevelocity.y + framevelocity.z) * (m_vecAccelDir.x + m_vecAccelDir.y + m_vecAccelDir.z);
	}

	float calctime = (gpGlobals->curtime - m_flLastMoveCalc);
	m_flLastMoveCalc = gpGlobals->curtime;
	float accelspeed = m_flAccelSpeed;

	// If direction metric is negative then the two vectors point in opposite directions.  Only works because there are only two possible directions for them to point in.
	// If they point in opposite directions then we have moved past our objective.
	if (directionmetric < 0)
		remainingdist *= -1;

	//If speedmetric is negative the door is moving in the direction opposite its acceleration vector
	if (speedmetric < 0)
	{
		framespeed *= -1;
		accelspeed +=  m_flDeccelBoost; //Give it our deccelboost to make sure it doesn't go through a wall or its partner as it changes directions.
	}


	// Calculate distance to threshold
	if (m_toggle_state == TS_GOING_DOWN)
		threshdist = m_flMoveDistance - remainingdist;
	else
		threshdist = remainingdist;

	// Fire output when door passes threshold
	if ((threshdist < m_flTriggerDistance && !m_bPassedThreshold) || (threshdist > m_flTriggerDistance && m_bPassedThreshold))
	{
		m_PassThreshold.FireOutput(this, this);

		if (m_bPassedThreshold)
			m_NearThreshold.FireOutput(this, this);
		else
			m_FarThreshold.FireOutput(this, this);

		m_bPassedThreshold = !m_bPassedThreshold;
	}


	// Now calculate our speed for the next interval.

	//DeccelDist should always be checked first because it can sometimes overlap with acceltime.  We use a kinematic equation
	//for this instead of the integrator to reduce final deceleration error as much as possible.
	//Cap the minimum at minspeed so mappers can have their doors slam shut if they want them to.
	if (remainingdist < m_flDeccelDist && framespeed >= 0) //Make sure we're actually moving towards the destination while also being within range of it.
		framespeed = max(sqrt(2 * remainingdist * accelspeed), m_flMinSpeed);
	else
		framespeed = min(framespeed + accelspeed *calctime, m_flSpeed); //Cap it so the door will move at max speed once it hits it.

	if (remainingdist > 0) // Still has distance to move so keep integrating
	{
		if (framespeed != prevframespeed) //Only update velocity if our velocity is different.
		{
			if (IsRotatingDoor())
				SetLocalAngularVelocity(framespeed * m_angAccelDir);
			else
				SetLocalVelocity(framespeed * m_vecAccelDir);
		}
		// If the distance the door will move in the next interval is greater than the remaining distance, the door is about to move past the end point and then teleport back!
		if (framespeed * m_flThinkInterval > remainingdist)
			SetNextThink(gpGlobals->curtime + remainingdist / framespeed); //Correct for this by adjusting the interval to match up with the end time.
		else
			SetNextThink(gpGlobals->curtime + m_flThinkInterval);
	}
	else // Has hit or gone past the ending posistion so finalize the transistion and stop moving
	{
		if (IsRotatingDoor())
		{
			SetLocalAngularVelocity(QAngle(0, 0, 0));
			SetLocalAngles(m_angFinalDest);
		}
		else
		{
			SetLocalVelocity(0);
			SetLocalOrigin(m_vecFinalDest);
		}

		m_movementType = 0;
		SetMoveDoneTime(-1);
		m_flLastMoveCalc = 0;
		m_iCurrentUses = 0;

		if (m_toggle_state == TS_GOING_DOWN)
		{
			DoorHitBottom();
		}
		else
		{
			DoorHitTop();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the player uses the door.
// Input  : pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CGEDoor::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_hActivator = pActivator;

	// We can't +use this if it can't be +used
	if (m_hActivator != NULL && m_hActivator->IsPlayer() && HasSpawnFlags(SF_DOOR_PUSE) == false)
		return;

	// Check how many times the door has been used, and increment the counter if it still has uses left.
	if (!CheckUse(pActivator))
		return;

	if (m_bLocked)
		m_OnLockedUse.FireOutput(pActivator, pCaller);
	else
		DoorActivate();
}

// Used to check if the door has been used too many times. Will return false if it has.
bool CGEDoor::CheckUse(CBaseEntity *pActivator)
{
	// If the uselimit is 0 or lower then always open, otherwise if the door has been used more than allowed, ignore the input.
	if (m_iUseLimit > 0 && m_iCurrentUses >= m_iUseLimit)
		return false;

	// Don't let someone instantly close the door after it gets opened.
	if (gpGlobals->curtime < m_flStartMoveTime + 0.25)
		return false;

	// Don't let another person quickly close the door after it gets opened.
	if (m_pLastActivator != pActivator && gpGlobals->curtime < m_flStartMoveTime + 1.0)
		return false;

	m_pLastActivator = pActivator;
	m_iCurrentUses += 1;

	for (int i = 0; i < m_pPartnerEnts.Count(); i++)
	{
		m_pPartnerEnts[i]->m_iCurrentUses += 1;
		m_pPartnerEnts[i]->m_pLastActivator = pActivator;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame when the door is blocked while opening or closing.
// Input  : pOther - The blocking entity.
//-----------------------------------------------------------------------------
void CGEDoor::Blocked(CBaseEntity *pOther)
{
	SetLocalVelocity(0);
	SetLocalAngularVelocity(QAngle(0, 0, 0));

	for (int i = 0; i < m_pPartnerEnts.Count(); i++)
	{
		m_pPartnerEnts[i]->SetLocalVelocity(0);
		m_pPartnerEnts[i]->SetLocalAngularVelocity(QAngle(0, 0, 0));
	}

	BaseClass::Blocked(pOther);

	if (m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP)
		DoorGroupGoUp(false);
	else
		DoorGroupGoDown(false);
}

void CGEDoor::InputOpen(inputdata_t &inputdata)
{
	if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_GOING_UP)
	{
		// I'm locked, can't open
		if (m_bLocked)
			return;

		// Play door unlock sounds.
		DoorGroupGoUp(true);

		if (inputdata.pActivator)
			m_pLastActivator = inputdata.pActivator;
	}
}

void CGEDoor::InputClose(inputdata_t &inputdata)
{
	if (m_toggle_state != TS_AT_BOTTOM && m_toggle_state != TS_GOING_DOWN)
	{
		DoorGroupGoDown(true);

		if (inputdata.pActivator)
			m_pLastActivator = inputdata.pActivator;
	}
}

void CGEDoor::InputToggle(inputdata_t &inputdata)
{
	// I'm locked, can't open
	if (m_bLocked)
		return;

	// Toggle will also check uses because mappers like to have door controling buttons that could also be abused.
	if (!CheckUse(inputdata.pActivator))
		return;

	if (m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN)
		DoorGroupGoUp(true);
	else
		DoorGroupGoDown(true);
}

// Just the toggle input but ignores the uselimit and locked conditions.
void CGEDoor::InputForceToggle(inputdata_t &inputdata)
{
	if (m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN)
		DoorGroupGoUp(true);
	else
		DoorGroupGoDown(true);

	if (inputdata.pActivator)
		m_pLastActivator = inputdata.pActivator;
}



//==================================================
// CGERotDoor:  Basically just the code for CRotDoor but inheriting from CGEDoor instead
//==================================================

class CGERotDoor : public CGEDoor
{
public:
	DECLARE_CLASS(CGERotDoor, CGEDoor);

	void Spawn(void);
	bool CreateVPhysics();
	// This is ONLY used by the node graph to test movement through a door
	virtual void SetToggleState(int state);
	virtual bool IsRotatingDoor() { return true; }

	bool m_bSolidBsp;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(func_ge_door_rotating, CGERotDoor);

BEGIN_DATADESC(CGERotDoor)
DEFINE_KEYFIELD(m_bSolidBsp, FIELD_BOOLEAN, "solidbsp"),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGERotDoor::Spawn(void)
{
	BaseClass::Spawn();

	// set the axis of rotation
	CBaseToggle::AxisDir();

	// check for clockwise rotation
	if (HasSpawnFlags(SF_DOOR_ROTATE_BACKWARDS))
		m_vecMoveAng = m_vecMoveAng * -1;

	m_vecAngle1 = GetLocalAngles();
	m_vecAngle2 = GetLocalAngles() + m_vecMoveAng * m_flMoveDistance;

	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating door start/end positions are equal\n");

	// Starting open allows a func_door to be lighted in the closed position but
	// spawn in the open position
	//
	// SF_DOOR_START_OPEN_OBSOLETE is an old broken way of spawning open that has
	// been deprecated.
	if (HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE))
	{
		// swap pos1 and pos2, put door at pos2, invert movement direction
		QAngle vecNewAngles = m_vecAngle2;
		m_vecAngle2 = m_vecAngle1;
		m_vecAngle1 = vecNewAngles;
		m_vecMoveAng = -m_vecMoveAng;

		// We've already had our physics setup in BaseClass::Spawn, so teleport to our
		// current position. If we don't do this, our vphysics shadow will not update.
		Teleport(NULL, &m_vecAngle1, NULL);

		m_toggle_state = TS_AT_BOTTOM;
	}
	else if (m_eSpawnPosition == FUNC_DOOR_SPAWN_OPEN)
	{
		// We've already had our physics setup in BaseClass::Spawn, so teleport to our
		// current position. If we don't do this, our vphysics shadow will not update.
		Teleport(NULL, &m_vecAngle2, NULL);
		m_toggle_state = TS_AT_TOP;
	}
	else
	{
		m_toggle_state = TS_AT_BOTTOM;
	}

	// Slam the object back to solid - if we really want it to be solid.
	if (m_bSolidBsp)
	{
		SetSolid(SOLID_BSP);
	}
}

//-----------------------------------------------------------------------------

bool CGERotDoor::CreateVPhysics()
{
	if (!IsSolidFlagSet(FSOLID_NOT_SOLID))
	{
		VPhysicsInitShadow(false, false);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
// This is ONLY used by the node graph to test movement through a door
void CGERotDoor::SetToggleState(int state)
{
	if (state == TS_AT_TOP)
		SetLocalAngles(m_vecAngle2);
	else
		SetLocalAngles(m_vecAngle1);
}
