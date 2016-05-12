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
		ListenForGameEvent("player_spawn");
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

		// Reset conditions on spawn because only losers die and this acheivement is for winners.
		if (Q_stricmp(event->GetName(), "player_spawn") == 0)
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


// Dart Board:  Kill a player from over 60 feet away with a throwing knife
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

		// If we're using the throwing knives, and the victim is over 60 feet(720 inches) away, we did it!
		if (event->GetInt("weaponid") == WEAPON_KNIFE_THROWING && event->GetInt("dist") > 720)
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
DECLARE_GE_ACHIEVEMENT(CAchPrecisionBombing, ACHIEVEMENT_GES_PRECISION_BOMBING, "GES_PRECISION_BOMBING", 100, GE_ACH_UNLOCKED);


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
DECLARE_GE_ACHIEVEMENT(CAchIndirectFire, ACHIEVEMENT_GES_INDIRECT_FIRE, "GES_INDIRECT_FIRE", 100, GE_ACH_UNLOCKED);


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
DECLARE_GE_ACHIEVEMENT(CAchHuntingParty, ACHIEVEMENT_GES_HUNTING_PARTY, "GES_HUNTING_PARTY", 100, GE_ACH_UNLOCKED);


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
DECLARE_GE_ACHIEVEMENT(CAchPlastiqueSurgery, ACHIEVEMENT_GES_PLASTIQUE_SURGERY, "GES_PLASTIQUE_SURGERY", 100, GE_ACH_UNLOCKED);


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
		if (pAttacker == C_BasePlayer::GetLocalPlayer() && event->GetInt("weaponid") == WEAPON_KLOBB && event->GetInt("weaponskin") == 3 && GEPlayerRes()->GetDevStatus(pVictim->entindex()) == 1)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchDevideAndConquer, ACHIEVEMENT_GES_DEVIDE_AND_CONQUER, "GES_DEVIDE_AND_CONQUER", 100, GE_ACH_UNLOCKED);


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
DECLARE_GE_ACHIEVEMENT(CAchDevidends, ACHIEVEMENT_GES_DEVIDENDS, "GES_DEVIDENDS", 100, GE_ACH_UNLOCKED);

// Proxima Centauri:  Get proximity mine kills from 100 feet away
class CAchProximaCentauri : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(10);
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

		// If we're using the proximity mines, and the victim is less than 100 feet(1200 inches) away, we did it!
		if (event->GetInt("weaponid") == WEAPON_PROXIMITYMINE && event->GetInt("dist") > 1200)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchProximaCentauri, ACHIEVEMENT_GES_PROXIMA_CENTAURI, "GES_PROXIMA_CENTAURI", 100, GE_ACH_UNLOCKED);


// Time To Target:  Get 2 grenade launcher direct hits within a tenth of a second
class CAchTimeToTarget : public CGEAchievement
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
		m_flLastKillTime = 0;
	}
	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
		VarInit();
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		// If this isn't us doing the killing we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID())
			return;

		// If we're using the grenade launcher, and the game registered a headshot, we can consider this a kill.
		if (event->GetInt("weaponid") == WEAPON_GRENADE_LAUNCHER && event->GetBool("headshot"))
		{
			if (gpGlobals->curtime - m_flLastKillTime < 0.1)
				IncrementCount();
			else
				m_flLastKillTime = gpGlobals->curtime;
		}
	}
private:
	float m_flLastKillTime;
};
DECLARE_GE_ACHIEVEMENT(CAchTimeToTarget, ACHIEVEMENT_GES_TIMETOTARGET, "GES_TIMETOTARGET", 100, GE_ACH_UNLOCKED);


// Pitfall: Knock 100 people into pits
class CAchPitfall : public CGEAchievement
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

		// If the victim died to a trap, we did it.
		if ( event->GetInt("weaponid") == WEAPON_TRAP )
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchPitfall, ACHIEVEMENT_GES_PITFALL, "GES_PITFALL", 100, GE_ACH_UNLOCKED);


// Backtracking: Steal 100 levels in Arsenal
class CAchBacktracker : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(100);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("gameplay_event");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		if (!Q_stricmp(event->GetString("name"), "ar_levelsteal"))
		{
			CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
			if (!pPlayer)
				return;

			// If we're the one that triggered the event, we did it!
			if (Q_atoi(event->GetString("value1")) == pPlayer->GetUserID())
				IncrementCount();
		}
	}
};
DECLARE_GE_ACHIEVEMENT(CAchBacktracker, ACHIEVEMENT_GES_BACKTRACKER, "GES_BACKTRACKER", 100, GE_ACH_UNLOCKED);


// One Man Arsenal: Win a round of Arsenal with only the slappers
class CAchOneManArsenal : public CGEAchievement
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
		m_bOnlySlappers = true;
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
		ListenForGameEvent("gameplay_event");
		ListenForGameEvent("round_start");
		VarInit();
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		if (!Q_stricmp(event->GetName(), "player_death"))
		{
			// If this isn't us doing the killing we don't care
			if (event->GetInt("attacker") != pPlayer->GetUserID())
				return;

			// If the victim died to a trap, we did it.
			if (event->GetInt("weaponid") != WEAPON_SLAPPERS)
				m_bOnlySlappers = false;
		}

		else if (!Q_stricmp(event->GetName(), "gameplay_event"))
		{
			if (!Q_stricmp(event->GetString("name"), "ar_completedarsenal"))
			{
				// If we completed the arsenal (no force round end victories), there are more than 3 players, and we only used slappers, we did it!
				if (Q_atoi(event->GetString("value1")) == pPlayer->GetUserID() && m_bOnlySlappers && CalcPlayerCount() > 3)
					IncrementCount();
			}
		}
		else if (!Q_stricmp(event->GetName(), "round_start"))
			m_bOnlySlappers = true; //Reset our status on round start.
	}
private:
	bool m_bOnlySlappers;
};
DECLARE_GE_ACHIEVEMENT(CAchOneManArsenal, ACHIEVEMENT_GES_ONEMANARSENAL, "GES_ONEMANARSENAL", 100, GE_ACH_UNLOCKED);


// Pineapple Express: Kill 10 people with grenades you dropped on death
class CAchPineappleExpress : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(10);
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

		// If the kill was with a grenade, and it has the droppedondeath bitflag set, we did it!
		if (event->GetInt("weaponid") == WEAPON_GRENADE && event->GetInt("custom") & 2)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchPineappleExpress, ACHIEVEMENT_GES_PINEAPPLE_EXPRESS, "GES_PINEAPPLE_EXPRESS", 100, GE_ACH_HIDDEN);


// Pineapple Express: Kill 10 people with grenades you dropped on death
class CAchPinpointPrecision : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(10);
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

		// If this isn't us doing the killing or we killed ourselves we don't care
		if (event->GetInt("attacker") != pPlayer->GetUserID() || event->GetInt("attacker") == event->GetInt("userid"))
			return;

		// If the kill was with a grenade, and it has the inair bitflag set, we did it!
		if (event->GetInt("weaponid") == WEAPON_GRENADE && event->GetInt("custom") & 1)
			IncrementCount();
	}
};
DECLARE_GE_ACHIEVEMENT(CAchPinpointPrecision, ACHIEVEMENT_GES_PINPOINT_PRECISION, "GES_PINPOINT_PRECISION", 100, GE_ACH_UNLOCKED);


// Chopping Block: Eliminate 20 people with slappers in YOLT
class CAchChoppingBlock : public CGEAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(20);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("gameplay_event");
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		if (!Q_stricmp(event->GetString("name"), "yolt_eliminated"))
		{
			CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
			if (!pPlayer)
				return;

			// If we're the one that triggered the event, and the weapon is slappers, we did it!
			if (Q_atoi(event->GetString("value2")) == pPlayer->GetUserID() && !Q_stricmp(event->GetString("value3"), "weapon_slappers"))
				IncrementCount();
		}
	}
};
DECLARE_GE_ACHIEVEMENT(CAchChoppingBlock, ACHIEVEMENT_GES_CHOPPINGBLOCK, "GES_CHOPPINGBLOCK", 100, GE_ACH_UNLOCKED);


// Last man hiding: Be the last man standing in YOLT with only one kill
class CAchLastManHiding : public CGEAchievement
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
		m_iKillCount = 0;
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("player_death");
		ListenForGameEvent("gameplay_event");
		ListenForGameEvent("round_start");
		VarInit();
	}

	virtual void FireGameEvent_Internal(IGameEvent *event)
	{
		CGEPlayer *pPlayer = ToGEPlayer(CBasePlayer::GetLocalPlayer());
		if (!pPlayer)
			return;

		if (!Q_stricmp(event->GetName(), "player_death"))
		{
			// If this isn't us doing the killing we don't care
			if (event->GetInt("attacker") != pPlayer->GetUserID())
				return;

			// But if it is...
			m_iKillCount++;
		}

		else if (!Q_stricmp(event->GetName(), "gameplay_event"))
		{
			if (!Q_stricmp(event->GetString("name"), "yolt_lastmanstanding"))
			{
				Warning("Noticed Victory, value 1 is %s, value 2 is %s, killcount is at %d, and playerID is %d\n", event->GetString("value1"), event->GetString("value2"), m_iKillCount, pPlayer->GetUserID());

				// If we were the last man standing, we had less than 2 kills, and there were more than 3 players in the round, we did it!
				if (Q_atoi(event->GetString("value1")) == pPlayer->GetUserID() && m_iKillCount < 2 && Q_atoi(event->GetString("value2")) > 3)
					IncrementCount();
			}
		}

		else if (!Q_stricmp(event->GetName(), "round_start"))
			m_iKillCount = 0; //Reset our killcount.
	}
private:
	int m_iKillCount;
};
DECLARE_GE_ACHIEVEMENT(CAchLastManHiding, ACHIEVEMENT_GES_LASTMANHIDING, "GES_LASTMANHIDING", 100, GE_ACH_UNLOCKED);
