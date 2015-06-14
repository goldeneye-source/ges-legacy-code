///////////// Copyright © 2008, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_utils.h
// Description:
//      Various utilities that valve "forgot" to make
//
// Created On: 08 Sep 08
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////'
#ifndef GE_UTILS_H
#define GE_UTILS_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/IceKey.H"
#include "filesystem.h"

#define FOR_EACH_DICT( dictName, iterName ) \
	for ( int iterName = dictName.First(); iterName != dictName.InvalidIndex(); iterName = dictName.Next(iterName) )

void GEUTIL_EncryptICE( unsigned char * buffer, int size, const unsigned char *key );
bool GEUTIL_DEncryptFile(const char* filename, const unsigned char* key, bool bEncrypt = true, bool bChangeExt = false);

void GEUTIL_OverrideCommand( const char *real_name, const char *new_name, const char *override_name, const char *override_desc = "" );

const unsigned char *GetHash( void );
void ClearStringVector( CUtlVector<char*> &vec );
void UTIL_ColorToString( Color col, char *str, int size );

const char *Q_SkipSpaces( const char *in, int offset = 0 );
const char *Q_SkipData( const char *in, int offset = 0 );
int Q_ExtractData( const char *in, CUtlVector<char*> &out );

#ifdef CLIENT_DLL
// Returns true on successful parsing, false if it isn't localizable or fails parsing
void GEUTIL_ParseLocalization( wchar_t *out, int size, const char *input );

void GEUTIL_GetTextSize( const wchar_t *utext, vgui::HFont font, int &wide, int &tall );
void GEUTIL_GetTextSize( const char *text, vgui::HFont font, int &wide, int &tall );

wchar_t *GEUTIL_GetGameplayName( wchar_t *out, int byte_size );
#else
#include "ge_player.h"
// Shared python functions that can be used in C++ as well
void pyShowPopupHelp( CGEPlayer *pPlayer, const char *title, const char *msg, const char *img = "", float holdtime = 5.0f, bool canArchive = false );
void pyShowPopupHelpTeam( int team, const char *title, const char *msg, const char *img = NULL, float holdtime = 5.0f, bool canArchive = false );
#endif

bool IsValidColorHint( int ch );

char *GEUTIL_RemoveColorHints( char *src );
wchar_t *GEUTIL_RemoveColorHints( wchar_t *src );

void GEUTIL_DelayRemove( CBaseEntity *pEnt, float delay );

bool GEUTIL_GetVersion( string_t &text, int &major, int &minor, int &client, int &server );
#endif
