///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_bot.h
// Description:
//      Basic Bot for testing
//
// Created On: 22 Nov 08
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////
#ifndef GE_BOT_H
#define GE_BOT_H
#ifdef _WIN32
#pragma once
#endif

// If iTeam or iClass is -1, then a team or class is randomly chosen.
CBasePlayer *BotPutInServer( bool bFrozen, int iTeam );

#endif // GE_BOT_H

