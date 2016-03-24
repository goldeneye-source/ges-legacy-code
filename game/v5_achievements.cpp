///////////// Copyright © 2016, Goldeneye: Source. All rights reserved. /////////////
// 
// File: v5_achievements.cpp
// Description:
//      Goldeneye Achievements for Version 5.0
//
// Created On: 01 Oct 09
// Check github for contributors.
/////////////////////////////////////////////////////////////////////////////
#include "cbase.h"
#include "achievementmgr.h"
#include "util_shared.h"
#include "ge_achievement.h"

// Define our client's manager so that they can recieve updates when they achieve things
//#ifdef CLIENT_DLL

#include "clientmode_shared.h"
#include "c_ge_player.h"
#include "c_gemp_player.h"
#include "c_ge_playerresource.h"
#include "c_ge_gameplayresource.h"
#include "ge_weapon.h"
#include "gemp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Double Down:  Kill 2 players with one shotgun blast.
class CAchDoubleDown : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(5);
		VarInit();
	}
	virtual void VarInit()
	{
		m_flLastKillTime = 0;
	}
	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
		VarInit();
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CBasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
		if (!pPlayer)
			return;

		// Only care about our kills
		if ( event->GetInt("attacker") == pPlayer->GetUserID() && event->GetInt("weaponid") == WEAPON_SHOTGUN )
		{
			if (gpGlobals->curtime <= m_flLastKillTime + 0.2f)
			{
				// If it's not LTK just give it to them instantly.
				if (!IsScenario("ltk"))
				{
					SetCount(GetGoal() - 1);
					IncrementCount();
				}
				else
					IncrementCount();
			}

			m_flLastKillTime = gpGlobals->curtime;
		}
	}

private:
	float m_flLastKillTime;
};
DECLARE_GE_ACHIEVEMENT(CAchDoubleDown, ACHIEVEMENT_GES_DOUBLE_DOWN, "GES_DOUBLE_DOWN", 100, GE_ACH_HIDDEN);

// There's The Armor:  Take 8 gauges of damage in a single life
class CAchTheresTheArmor : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
		VarInit();
	}
	virtual void VarInit()
	{
		m_iDmgTaken = 0;
	}
	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_hurt");
		ListenForGameEvent("player_death");
		VarInit();
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us getting hurt we don't care
		if (event->GetInt("userid") != pPlayer->GetUserID())
			return;

		// Reset conditions on death because only losers die and this acheivement is for winners.
		if (Q_stricmp(event->GetName(), "player_death") == 0)
		{
			m_iDmgTaken = 0;
			return;
		}

		// If we made it this far then the event is just the damage event, so we should add the damage we took to our total.
		// Provided that we're still alive, anyway.
		if (event->GetInt("health") > 0)
		{
			m_iDmgTaken += event->GetInt("damage");

			if (m_iDmgTaken >= 2560) //160 * 8 = 1280
				IncrementCount();
		}
		else
			m_iDmgTaken = 0;

	}
private:
	int m_iDmgTaken;
};
DECLARE_GE_ACHIEVEMENT(CAchTheresTheArmor, ACHIEVEMENT_GES_THERES_THE_ARMOR, "GES_THERES_THE_ARMOR", 100, GE_ACH_HIDDEN);

// Remote Delivery:  Kill a player that you can't see with remote mines
class CAchRemoteDelivery : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(25);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the remote mines and the victim isn't within our PVS, we did it!
		if (event->GetInt("weaponid") == WEAPON_REMOTEMINE && !event->GetBool("InPVS"))
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchRemoteDelivery, ACHIEVEMENT_GES_REMOTE_DELIVERY, "GES_REMOTE_DELIVERY", 100, GE_ACH_UNLOCKED);

// You're Fired:  Kill a player with a direct rocket launcher hit
class CAchYoureFired : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(100);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the rocket launcher and the victim registered a direct hit, we did it!
		if (event->GetInt("weaponid") == WEAPON_ROCKET_LAUNCHER && event->GetBool("headshot"))
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchYoureFired, ACHIEVEMENT_GES_YOURE_FIRED, "GES_YOURE_FIRED", 100, GE_ACH_UNLOCKED);


// Shell Shocked:  Kill a player with a direct grenade launcher hit
class CAchShellShocked : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(50);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the grenade launcher, the victim isn't within our PVS, and the game registered a headshot, we did it!
		if (event->GetInt("weaponid") == WEAPON_GRENADE_LAUNCHER && event->GetBool("headshot"))
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchShellShocked, ACHIEVEMENT_GES_SHELL_SHOCKED, "GES_SHELL_SHOCKED", 100, GE_ACH_UNLOCKED);


// Dart Board:  Kill a player from over 80 feet away with a throwing knife
class CAchDartBoard : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(5);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the throwing knives, and the victim is over 80 feet(960 inches) away, we did it!
		if (event->GetInt("weaponid") == WEAPON_KNIFE_THROWING && event->GetInt("dist") > 960)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchDartBoard, ACHIEVEMENT_GES_DART_BOARD, "GES_DART_BOARD", 100, GE_ACH_UNLOCKED);


// Precision Bombing:  Kill a player from over 100 feet away with a grenade launcher direct hit
class CAchPrecisionBombing : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(3);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the grenade launcher, got a direct hit, and the victim is over 100 feet(1200 inches) away, we did it!
		if (event->GetInt("weaponid") == WEAPON_GRENADE_LAUNCHER && event->GetBool("headshot") && event->GetInt("dist") > 1200)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchPrecisionBombing, ACHIEVEMENT_PRECISION_BOMBING, "GES_PRECISION_BOMBING", 100, GE_ACH_UNLOCKED);


// Indirect Fire:  Kill a player you cannot see with a direct grenade launcher hit
class CAchIndirectFire : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(3);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the grenade launcher, the victim isn't within our PVS, and the game registered a headshot, we did it!
		if (event->GetInt("weaponid") == WEAPON_GRENADE_LAUNCHER && event->GetBool("headshot") && !event->GetBool("InPVS"))
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchIndirectFire, ACHIEVEMENT_INDIRECT_FIRE, "GES_INDIRECT_FIRE", 100, GE_ACH_UNLOCKED);


// Hunting Party:  Get point blank shotgun kills
class CAchHuntingParty : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(100);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the shotgun, and the victim is less than 8 feet(96 inches) away, we did it!
		if (event->GetInt("weaponid") == WEAPON_SHOTGUN && event->GetInt("dist") < 60)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchHuntingParty, ACHIEVEMENT_HUNTING_PARTY, "GES_HUNTING_PARTY", 100, GE_ACH_UNLOCKED);


// Plastique Surgery:  Get envionmental explosive kills
class CAchPlastiqueSurgery : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(50);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing or we killed ourselves, we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID() || event->GetInt("attacker") == event->GetInt("userid"))
			return;

		// If we killed them with an envionmental explosion, we did it!
		if (event->GetInt("weaponid") == WEAPON_EXPLOSION)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchPlastiqueSurgery, ACHIEVEMENT_PLASTIQUE_SURGERY, "GES_PLASTIQUE_SURGERY", 100, GE_ACH_UNLOCKED);


// Devide and Conquer:  Kill a developer with the developer klobb
class CAchDevideAndConquer : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void Event_EntityKilled(CBaseEntity *pVictim, CBaseEntity *pAttacker, CBaseEntity *pInflictor, IGameEvent *event)
	{
		if (!pVictim)
			return;

		// If the attacker is us, we're using the developer klobb, and the victim is a developer, we did it!
		if (pAttacker == C_BasePlayer::GetLocalPlayer() && event->GetInt("weaponid") == WEAPON_KLOBB && event->GetInt("weaponskin") == 2 && GEPlayerRes()->GetDevStatus(pVictim->entindex()) == 1)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchDevideAndConquer, ACHIEVEMENT_DEVIDE_AND_CONQUER, "GES_DEVIDE_AND_CONQUER", 100, GE_ACH_UNLOCKED);


// Devidends:  Get 20 kills with the developer or BT klobb
class CAchDevidends : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(20);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing, we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we killed them with the klobb, and the klobb has one of the dev skins, we did it!
		if (event->GetInt("weaponid") == WEAPON_KLOBB && event->GetInt("weaponskin") > 0)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchDevidends, ACHIEVEMENT_DEVIDENDS, "GES_DEVIDENDS", 100, GE_ACH_UNLOCKED);
