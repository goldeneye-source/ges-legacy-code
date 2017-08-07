///////////// Copyright © 2017, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_blacklist.h
//
// Logic for blacklisting servers
//
/////////////////////////////////////////////////////////////////////////////
#ifndef GE_BLACKLIST_H
#define GE_BLACKLIST_H

#ifdef _WIN32
#pragma once
#endif

bool InvestigateGEServer( const char *address, bool abortOnInvalidIP );

#endif