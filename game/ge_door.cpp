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
#include "entitylist.h"
#include "physics.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
#include "physics_npc_solver.h"

#ifdef HL1_DLL
#include "filters.h"
#endif

#ifdef CSTRIKE_DLL
#include "KeyValues.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CGEDoor : public CBaseDoor
{
public:
	DECLARE_CLASS(CGEDoor, CBaseDoor);
	DECLARE_DATADESC();

	CGEDoor();

	float m_flacceltime; // Time it takes the door to accelerate to max speed
	float m_flAccelSpeed; // Overrides acceltime when used.  Speed that the door accelerates per second.
	float m_flMinSpeed; // Minimum speed the door will travel when decelerating.
	int m_iUseLimit; // How many times the door can be activated mid-motion before denying any further inputs.
	string_t m_sPartner; // Name of the door that opens and closes with this one
	CGEDoor *m_pPartnerEnt; // Actual entity that opens and closes with this one

	void Spawn();
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void SetPartner(string_t newPartner);
	void DoorGoUp();
	void DoorGoDown();
	void Blocked(CBaseEntity *pOther);
	int DoorActivate();
	virtual void MoveThink();
	void CalcMovementValues(Vector startpos, Vector endpos);

	void InputClose(inputdata_t &inputdata);
	void InputOpen(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);

	COutputEvent m_FirstClose;		// Triggered only on the initial close input.
	COutputEvent m_FirstOpen;		// Triggered only on the initial open input.

private:
	float m_flTargetDist; //Distance the door must move to reach the goal posistion
	float m_flDeccelDist; //Distance within the end that the door must start decelerating.  Might not be the same as acceldist.

	Vector m_vecAccelDir; //Direction the door accelerates

	float m_flLastMoveCalc; //Time that the door last moved

	CBaseEntity *m_pLastActivator; //Last person to use the door.  This is used to prevent stuff like someone standing on the other side of a door and closing it every time it gets opened effectively shutting anyone else out.
	float m_flStartMoveTime; // Time that the current movecycle was started.  Used to check if the door can be closed by someone other than the one who opened it.
	int m_iCurrentUses; // How many times the door has been used this movecycle.

	QAngle m_angFinalDest; // For rotating doors:  Angle the door is moving to.
	QAngle m_angAccelDir; // For rotating doors:  Angle the door accelerates on.
};

LINK_ENTITY_TO_CLASS(func_ge_door, CGEDoor);

BEGIN_DATADESC(CGEDoor)

DEFINE_KEYFIELD(m_flacceltime, FIELD_FLOAT, "AccelerationTime"),
DEFINE_KEYFIELD(m_flAccelSpeed, FIELD_FLOAT, "AccelerationSpeed"),
DEFINE_KEYFIELD(m_flMinSpeed, FIELD_FLOAT, "MinimumSpeed"),
DEFINE_KEYFIELD(m_iUseLimit, FIELD_INTEGER, "UseLimit"),
DEFINE_KEYFIELD(m_sPartner, FIELD_STRING, "PartnerDoor"),
DEFINE_OUTPUT(m_FirstOpen, "OnFirstOpen"),
DEFINE_OUTPUT(m_FirstClose, "OnFirstClose"),

END_DATADESC()

// Pretty much integrates every time.  If a way to directly alter displacement could be found while retaining the pushing
// behavior a better integrator can be used and this can be increased substancially.  RK2 could work but there are probably better options.
#define DOOR_THINKTIME 0.01f

// Boost to the doors accel speed when it's changing direction
#define DOOR_DECCELBOOST 1.15f

CGEDoor::CGEDoor()
{
	// Set these values here incase they were not assigned by the mapper somehow.
	m_flacceltime = 1;
	m_iUseLimit = 3;
	m_flMinSpeed = 5;

	m_flAccelSpeed = 0;
	m_flDeccelDist = 0;
	m_flStartMoveTime = 0;
}

void CGEDoor::Spawn(void)
{
	BaseClass::Spawn();
	SetPartner(m_sPartner);

	//Calculate kinematic constants essential for acceleration
	if (m_flAccelSpeed == 0)
		m_flAccelSpeed = m_flSpeed / m_flacceltime; //by default speed is calculated from time
	else
		m_flacceltime = m_flSpeed / m_flAccelSpeed; //but if the user sets speed then that will be used instead.

	m_flStartMoveTime = 0;
	m_flLastMoveCalc = 0;
	m_pLastActivator = 0;
	m_iCurrentUses = 0;
}

void CGEDoor::CalcMovementValues(Vector startpos, Vector endpos)
{
	float traveldistance,  travelmetric, speed;

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

	//We are currently traveling in the opposite direction, meaning we will actually start accelerating toward the endpos from further away than expected.
	if (travelmetric < 0)
	{
		traveldistance += speed*speed / (2 * m_flAccelSpeed * DOOR_DECCELBOOST);
		DevMsg("Moving in opposite direction, new traveldistance calculated to be %f \n", traveldistance);
	}
	else if (travelmetric > 0)
	{
		// Even if moving in the same direction we can add to travel distance because we can pretend
		// the door started moving at 0 velocity from a posistion past the true posistion.
		traveldistance += speed*speed / (2 * m_flAccelSpeed); //No deccelboost this time though
		DevMsg("Moving in same direction, new traveldistance calculated to be %f \n", traveldistance);
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
	CBaseEntity *pPartner = gEntList.FindEntityByName(NULL, newPartner, NULL);

	if (newPartner != NULL_STRING && pPartner == NULL)
		Msg("Entity %s(%s) has bad partner %s\n", STRING(m_iClassname), GetDebugName(), STRING(newPartner));
	else if (pPartner != NULL && Q_stricmp(pPartner->GetClassname(), "CGEDoor"))
		m_pPartnerEnt = static_cast<CGEDoor*>(pPartner);
	else
		Msg("Entity %s(%s) tried to adopt partner %s but they are not a ge_door!\n", STRING(m_iClassname), GetDebugName(), STRING(newPartner));
}

//-----------------------------------------------------------------------------
// Purpose: When fired door and partner will start moving
// Output : int
//-----------------------------------------------------------------------------
int CGEDoor::DoorActivate()
{
	if (m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP)
	{
		// door should close
		DoorGoDown();
		if (m_pPartnerEnt)
			m_pPartnerEnt->DoorGoDown();
	}
	else
	{
		// open door
		DoorGoUp();
		if (m_pPartnerEnt)
			m_pPartnerEnt->DoorGoUp();
	}

	return 1;
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

				Vector activatorToOrigin;
				AngleVectors(m_hActivator->EyeAngles(), &activatorToOrigin);
				activatorToOrigin.z = 0;

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
	float framespeed, remainingdist, directionmetric, speedmetric;

	// The velocity and destinations of rotating doors come from different places
	if (IsRotatingDoor())
	{
		frameangularvelocity = GetLocalAngularVelocity();
		remainingangle = m_angFinalDest - GetLocalAngles();

		framespeed = frameangularvelocity.Length();
		remainingdist = remainingangle.Length();

		directionmetric = (remainingangle.x + remainingangle.y + remainingangle.z) * (m_angAccelDir.x + m_angAccelDir.y + m_angAccelDir.z);
		speedmetric = (frameangularvelocity.x + frameangularvelocity.y + frameangularvelocity.z) * (m_angAccelDir.x + m_angAccelDir.y + m_angAccelDir.z);
	}
	else
	{
		framevelocity = GetLocalVelocity();
		remainingvector = m_vecFinalDest - GetLocalOrigin();

		framespeed = framevelocity.Length();
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
		accelspeed *= DOOR_DECCELBOOST; //Give accelspeed a slight boost so the door can change directions a bit faster.
	}

	if (remainingdist < m_flDeccelDist) //DeccelDist should always be checked first because it can sometimes overlap with acceltime.
		framespeed = max(framespeed - accelspeed *calctime, m_flMinSpeed); //Cap it to make sure the door doesn't start accelerating in the other direction or get stuck.
	else
		framespeed = min(framespeed + accelspeed *calctime, m_flSpeed); //Cap it so the door will move at max speed once it hits it.

	if (remainingdist > 0) // Still has distance to move so keep integrating
	{
		if (IsRotatingDoor())
			SetLocalAngularVelocity(framespeed * m_angAccelDir);
		else
			SetLocalVelocity(framespeed * m_vecAccelDir);

		SetNextThink(gpGlobals->curtime + DOOR_THINKTIME);
	}
	else // Has hit or gone past the ending posistion so finalize the transistion and stop moving
	{
		if (IsRotatingDoor())
		{
			SetLocalAngularVelocity( QAngle(0, 0, 0) );
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

	// If the door has been used more than allowed, ignore the input.
	if (m_iCurrentUses >= m_iUseLimit)
		return;

	// Don't let another person instantly close the door after it gets opened.
	if (m_pLastActivator != pActivator && gpGlobals->curtime - m_flStartMoveTime < 0.5)
		return;

	m_pLastActivator = pActivator;
	m_iCurrentUses += 1;

	if (m_pPartnerEnt)
		m_pPartnerEnt->m_iCurrentUses += 1;

	if (m_bLocked)
		m_OnLockedUse.FireOutput(pActivator, pCaller);
	else
		DoorActivate();
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame when the door is blocked while opening or closing.
// Input  : pOther - The blocking entity.
//-----------------------------------------------------------------------------
void CGEDoor::Blocked(CBaseEntity *pOther)
{
	SetLocalVelocity(0);
	SetLocalAngularVelocity(QAngle(0, 0, 0));

	if (m_pPartnerEnt)
	{
		m_pPartnerEnt->SetLocalVelocity(0);
		m_pPartnerEnt->SetLocalAngularVelocity(QAngle(0, 0, 0));
	}

	BaseClass::Blocked(pOther);

	if (m_pPartnerEnt)
	{
		if (m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP)
			m_pPartnerEnt->DoorGoUp();
		else
			m_pPartnerEnt->DoorGoDown();
	}
}

void CGEDoor::InputOpen(inputdata_t &inputdata)
{
	if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_GOING_UP)
	{
		// I'm locked, can't open
		if (m_bLocked)
			return;

		// Play door unlock sounds.
		DoorGoUp();
		if (m_pPartnerEnt)
			m_pPartnerEnt->DoorGoUp();
	}
}

void CGEDoor::InputClose(inputdata_t &inputdata)
{
	if (m_toggle_state != TS_AT_BOTTOM)
	{
		DoorGoDown();
		if (m_pPartnerEnt)
			m_pPartnerEnt->DoorGoDown();
	}
}

void CGEDoor::InputToggle(inputdata_t &inputdata)
{
	// I'm locked, can't open
	if (m_bLocked)
		return;

	if (m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN)
	{
		DoorGoUp();
		if (m_pPartnerEnt)
			m_pPartnerEnt->DoorGoUp();
	}
	else
	{
		DoorGoDown();
		if (m_pPartnerEnt)
			m_pPartnerEnt->DoorGoDown();
	}
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
