///////////// Copyright © 2009, Goldeneye: Source. All rights reserved. /////////////
// 
// File: weapon_grenade_launcher.cpp
// Description:
//      Implements the Grenade Launcher weapon
//
// Created On: 4/5/2009
// Created By: Jonathan White <killermonkey01@gmail.com>
/////////////////////////////////////////////////////////////////////////////
#include "cbase.h"
#include "in_buttons.h"
#include "ge_weapon.h"
#include "ge_shareddefs.h"

#if defined( CLIENT_DLL )
	#include "c_ge_player.h"
	#include "c_gemp_player.h"
	#include "c_te_legacytempents.h"
#else
	#include "util.h"
	#include "gemp_player.h"
	#include "grenade_shell.h"
	#include "npc_gebase.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined( CLIENT_DLL )
	#define CGEWeaponGrenadeLauncher	C_GEWeaponGrenadeLauncher
#endif

//------------------------------------------------------------------------------------------
//
// GE Grenade Launcher weapon class
//
class CGEWeaponGrenadeLauncher : public CGEWeapon
{
public:
	DECLARE_CLASS( CGEWeaponGrenadeLauncher, CGEWeapon );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

	DECLARE_ACTTABLE();
	
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	CGEWeaponGrenadeLauncher( void );

	virtual GEWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_LAUNCHER; }
	
	virtual bool IsExplosiveWeapon() { return true; }
	virtual void OnReloadOffscreen( void );

	virtual void PrimaryAttack( void );
	virtual void ItemPostFrame( void );

	virtual void AddViewKick( void );

//	virtual void OnFireAnimReset(); //Fires when the firing animation loops around.
	virtual void CalculateShellVis( bool fillclip ); // Calculates how many shells to have in the launcher on draw/reload
	virtual void PushShells( bool fireshell ); // Shifts the bodygroups for shells over one on fire.
	virtual void AssignShellVis(); // Reassigns bodygroups based on m_iShellVisArray

#ifdef GAME_DLL
	virtual int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	// NPC Functions
	virtual void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
#endif

	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy(void);
	virtual void Precache( void );
	virtual void FinishReload(void);

private:
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckLaunchPosition( const Vector &vecEye, Vector &vecSrc );
	void	LaunchGrenade( void );
	int		m_iShellBodyGroup[6];
	bool	m_iShellVisArray[6];
	float	m_bPushShells;

	CNetworkVar( bool, m_bPreLaunch );
	CNetworkVar( float, m_flGrenadeSpawnTime );

private:
	CGEWeaponGrenadeLauncher( const CGEWeaponGrenadeLauncher & );
};

acttable_t	CGEWeaponGrenadeLauncher::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_GES_IDLE_AUTOSHOTGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SHOTGUN,				false },

	{ ACT_MP_RUN,						ACT_GES_RUN_AUTOSHOTGUN,					false },
	{ ACT_MP_WALK,						ACT_GES_WALK_AUTOSHOTGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SHOTGUN,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_GES_GESTURE_RANGE_ATTACK_AUTOSHOTGUN,	false },
	{ ACT_GES_ATTACK_RUN_PRIMARYFIRE,	ACT_GES_GESTURE_RANGE_ATTACK_AUTOSHOTGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_GES_GESTURE_RANGE_ATTACK_AUTOSHOTGUN,	false },

	{ ACT_GES_DRAW,						ACT_GES_GESTURE_DRAW_RIFLE,					false },

	{ ACT_MP_RELOAD_STAND,				ACT_GES_GESTURE_RELOAD_AUTOSHOTGUN,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_GES_GESTURE_RELOAD_AUTOSHOTGUN,			false },

	{ ACT_MP_JUMP,						ACT_GES_JUMP_AUTOSHOTGUN,					false },
};
IMPLEMENT_ACTTABLE(CGEWeaponGrenadeLauncher);

IMPLEMENT_NETWORKCLASS_ALIASED( GEWeaponGrenadeLauncher, DT_GEWeaponGrenadeLauncher )

BEGIN_NETWORK_TABLE( CGEWeaponGrenadeLauncher, DT_GEWeaponGrenadeLauncher )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bPreLaunch ) ),
	RecvPropFloat( RECVINFO( m_flGrenadeSpawnTime ) ),
#else
	SendPropBool( SENDINFO( m_bPreLaunch ) ),
	SendPropFloat( SENDINFO( m_flGrenadeSpawnTime ) ),
#endif
END_NETWORK_TABLE()

#if !defined( CLIENT_DLL )
BEGIN_DATADESC( CGEWeaponGrenadeLauncher )
	DEFINE_FIELD( m_bPreLaunch, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flGrenadeSpawnTime, FIELD_FLOAT ),

//	DEFINE_THINKFUNC( OnFireAnimReset ),
END_DATADESC()
#endif

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CGEWeaponGrenadeLauncher )
	DEFINE_PRED_FIELD( m_bPreLaunch, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flGrenadeSpawnTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD_TOL( m_flGrenadeSpawnTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_grenade_launcher, CGEWeaponGrenadeLauncher );
PRECACHE_WEAPON_REGISTER(weapon_grenade_launcher);

CGEWeaponGrenadeLauncher::CGEWeaponGrenadeLauncher( void )
{
	// NPC Ranging
	m_fMinRange1 = 250;
	m_fMaxRange1 = 3000;

	m_iShellBodyGroup[0] = { -1 };
	m_bPushShells = false;
}

bool CGEWeaponGrenadeLauncher::Deploy( void )
{
	m_bPreLaunch = false;
	m_flGrenadeSpawnTime = -1;
	bool success = BaseClass::Deploy();

	// We have to do this here because "GetBodygroupFromName" relies on the weapon having a viewmodel.
	// It's debatable if this is really how we should go about it, instead of just a table of integers for the bodygroups.
	// but this does protect us from someone recompiling the model with a different order of bodygroups.

	if (m_iShellBodyGroup[0] == -1) // We only need to check shell1 because we assign all of them at once.
	{
		m_iShellBodyGroup[0] = GetBodygroupFromName("shell1");
		m_iShellBodyGroup[1] = GetBodygroupFromName("shell2");
		m_iShellBodyGroup[2] = GetBodygroupFromName("shell3");
		m_iShellBodyGroup[3] = GetBodygroupFromName("shell4");
		m_iShellBodyGroup[4] = GetBodygroupFromName("shell5");
		m_iShellBodyGroup[5] = GetBodygroupFromName("shell6");


		// Also calculate shell visibility on the very first draw because we haven't reloaded yet and need to know how many to have in the chambers.
		CalculateShellVis(false);
	}

	AssignShellVis(); // If it's not the first draw we don't recalculate here because we only reorginize shell posistions on reload.

	return success;
}

bool CGEWeaponGrenadeLauncher::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_bPushShells = false; //Prevent a shell push if we abort firing.

	return BaseClass::Holster(pSwitchingTo);
}

void CGEWeaponGrenadeLauncher::OnReloadOffscreen(void)
{
	BaseClass::OnReloadOffscreen();

	CalculateShellVis(true); // Check shell count when we reload, but adjust for the amount of ammo we're about to take out of the clip.
	AssignShellVis(); // Also reassign vis because this is the one time it goes offscreen.
}

// We have to calculate shell vis here too, due to passive reload.  It might cause a small issue if someone picks up ammo during the second
// half of their reload but the only way to really fix that is to have the actual reload finish halway through the animation instead of at the end.
void CGEWeaponGrenadeLauncher::FinishReload(void)
{
	// Since FinishReload doesn't actually tell us if it reloaded anything so we need to figure that out here.
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return;

	int reserveammo = pOwner->GetAmmoCount(m_iPrimaryAmmoType);

	if (m_iClip1 < GetMaxClip1() && reserveammo > 0) //If our clip needs more ammo and we have the ammo to give it, we reloaded!
	{
		CalculateShellVis(true);

		if (pOwner->GetActiveWeapon() == this) // If we have the weapon out we might as well try and correct any issues caused by picking up extra ammo during reload now.
			AssignShellVis(); 
	}

	BaseClass::FinishReload();
}

void CGEWeaponGrenadeLauncher::Precache( void )
{
	PrecacheModel("models/weapons/gl/grenadeprojectile.mdl");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Prepare the launcher to fire
//-----------------------------------------------------------------------------
void CGEWeaponGrenadeLauncher::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}

	// Bring us back to center view
	pPlayer->ViewPunchReset();

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_bPreLaunch = true;
	SendWeaponAnim( GetPrimaryAttackActivity() );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	ToGEPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	m_flGrenadeSpawnTime = gpGlobals->curtime + GetFireDelay();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_bPushShells = true;
}

void CGEWeaponGrenadeLauncher::ItemPostFrame( void )
{
	if( m_bPreLaunch && m_flGrenadeSpawnTime < gpGlobals->curtime )
	{
		LaunchGrenade();
		m_bPreLaunch = false;
	}

	if (m_bPushShells && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		m_bPushShells = false;
		PushShells(false);
	}

	BaseClass::ItemPostFrame();
}

#ifdef GAME_DLL
void CGEWeaponGrenadeLauncher::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	LaunchGrenade();
}
#endif

// Make sure we aren't going to throw the grenade INTO something!
void CGEWeaponGrenadeLauncher::CheckLaunchPosition( const Vector &vecEye, Vector &vecSrc )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return;

	trace_t tr;
	UTIL_TraceHull( vecEye, vecSrc, -Vector(6,6,6), Vector(6,6,6), pOwner->PhysicsSolidMaskForEntity(), pOwner, pOwner->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
		vecSrc = tr.endpos;
}

//-----------------------------------------------------------------------------
// Purpose: Launch a bounce/impact grenade
//-----------------------------------------------------------------------------
void CGEWeaponGrenadeLauncher::LaunchGrenade( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return;

#ifndef CLIENT_DLL
	Vector	vForward, vRight, vUp;
	AngleVectors( pOwner->EyeAngles(), &vForward, &vRight, &vUp );

	Vector vecSrc = pOwner->Weapon_ShootPosition();

	if ( pOwner->IsPlayer() )
	{
		VectorMA( vecSrc, 3.5f, vRight, vecSrc );
		VectorMA( vecSrc, 9.0f, vForward, vecSrc );
		VectorMA( vecSrc, -6.0f, vUp, vecSrc );
	}

	CheckLaunchPosition( pOwner->EyePosition(), vecSrc );

	Vector vecThrow;
	pOwner->GetVelocity( &vecThrow, NULL );
	VectorMA( vecThrow, 1050.0f, vForward, vecThrow );
	VectorMA( vecThrow, 100.0f, vUp, vecThrow );

	QAngle angAiming;
	VectorAngles( vecThrow, angAiming );

	// Convert us into a bot player :-D
	if ( pOwner->IsNPC() )
	{
		CNPC_GEBase *pNPC = (CNPC_GEBase*) pOwner;
		if ( pNPC->GetBotPlayer() )
			pOwner = pNPC->GetBotPlayer();
	}

	CGEShell *pShell = (CGEShell *)CBaseEntity::Create( "npc_shell", vecSrc, angAiming, NULL );

	if ( pShell )
	{
		pShell->SetThrower( pOwner );
		pShell->SetOwnerEntity( pOwner );
		pShell->SetSourceWeapon(this);
		pShell->SetVelocity( vecThrow, NULL );

		pShell->SetDamage( GetGEWpnData().m_iDamage );
		pShell->SetDamageRadius( GetGEWpnData().m_flDamageRadius );

		// Tell the owner what we threw to implement anti-spamming
		if ( pOwner->IsPlayer() )
			ToGEMPPlayer( pOwner )->AddThrownObject( pShell );
	}
#endif

	// Remove the grenade from our ammo pool
	m_iClip1 -= 1;
	WeaponSound( SINGLE );

	//Add our view kick in
	AddViewKick();
}

void CGEWeaponGrenadeLauncher::CalculateShellVis( bool fillclip )
{
	// Shells on the shotgun viewmodel will be removed if the reserve ammo is less than 5.  
	// Each time we draw and reload, we check to see if this is the case, and if it is we change bodygroups accordingly.

	CBaseCombatCharacter *pOwner = GetOwner();

	

	if (!pOwner)
		return;

	int reserveammo = pOwner->GetAmmoCount(m_iPrimaryAmmoType);
	int clipammo = m_iClip1;

	if (fillclip) //If we need to fill the clip, just add on whatever reserve ammo we have.
		clipammo += reserveammo; //This can be over 6, it won't change the comparision results.

	// We don't actually change bodygroup visibility here so passive reload doesn't cause bodygroup changes on other weapons.
	for (int i = 5; i >= 0; i--)
	{
		if (clipammo > i)
			m_iShellVisArray[i] = true;
		else
			m_iShellVisArray[i] = false;
	}
}

void CGEWeaponGrenadeLauncher::PushShells( bool fireshell )
{
	bool shiftedArray[6];

	if (fireshell)
		m_iShellVisArray[0] = false; //shell 1 just fire fired out of the chamber.  In just a moment it will shift to shell 6.

	// Shift all the shells over one so we're ready for them to reset.
	for (int i = 5; i >= 0; i--)
	{
		shiftedArray[i] = m_iShellVisArray[(i + 1) % 6]; //be sure to wrap around and make slot 6 slot 1
	}

	// We have to do 2 loops or a number of other fancy things to prevent overwriting one of the values.
	for (int i = 5; i >= 0; i--)
	{
		m_iShellVisArray[i] = shiftedArray[i];
	}

	// Now assign the visibility
	AssignShellVis();
}

void CGEWeaponGrenadeLauncher::AssignShellVis()
{
	for (int i = 5; i >= 0; i--)
	{
		if (m_iShellVisArray[i])
			SwitchBodygroup(m_iShellBodyGroup[i], 0);
		else
			SwitchBodygroup(m_iShellBodyGroup[i], 1);
	}
}

void CGEWeaponGrenadeLauncher::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

	viewPunch.x = SharedRandomFloat( "gelauncherpax", GetGEWpnData().Kick.x_min, GetGEWpnData().Kick.x_max );
	viewPunch.y = SharedRandomFloat( "gelauncherpay", GetGEWpnData().Kick.y_min, GetGEWpnData().Kick.y_max );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}
