#include "cbase.h"
#include "gtest/gtest.h"

using namespace testing;

/*
class TestEngine : public IVEngineServer {
private:
	IVEngineServer *_engine;

public:
	// Constructor
	TestEngine( IVEngineServer *_engine ) { this->_engine = _engine; }

	// Overrides for testing
	virtual void ChangeLevel( const char *s1, const char *s2 ) {
		Msg( "[TestEngine] Change Level Issued (%s)\n", s1 );
	}
	
	// Pass-throughs
	virtual int		IsMapValid( const char *filename ) { _engine->IsMapValid( filename ); }
	virtual bool	IsDedicatedServer( void ) { _engine->IsDedicatedServer(); }
	virtual int		IsInEditMode( void ) { _engine->IsInEditMode(); }
	virtual int		PrecacheModel( const char *s, bool preload = false ) { _engine->PrecacheModel(s, preload); }
	virtual int		PrecacheSentenceFile( const char *s, bool preload = false ) { _engine->PrecacheSentenceFile(s, preload); }
	virtual int		PrecacheDecal( const char *name, bool preload = false ) { _engine->PrecacheDecal(name, preload); }
	virtual int		PrecacheGeneric( const char *s, bool preload = false ) { _engine->PrecacheGeneric(s, preload); }
	virtual bool	IsModelPrecached( char const *s ) const { _engine->IsModelPrecached(s); }
	virtual bool	IsDecalPrecached( char const *s ) const { _engine->IsDecalPrecached(s); }
	virtual bool	IsGenericPrecached( char const *s ) const { _engine->IsGenericPrecached(s); }
	virtual int		GetClusterForOrigin( const Vector &org ) { _engine->GetClusterForOrigin( org ); }
	virtual int		GetPVSForCluster( int cluster, int outputpvslength, unsigned char *outputpvs ) { _engine->GetPVSForCluster(cluster, outputpvslength); }
	virtual bool	CheckOriginInPVS( const Vector &org, const unsigned char *checkpvs, int checkpvssize ) { _engine->CheckOriginInPVS(org, checkpvs, checkpvssize); }
	virtual bool	CheckBoxInPVS( const Vector &mins, const Vector &maxs, const unsigned char *checkpvs, int checkpvssize ) { _engine->CheckBoxInPVS(mins, maxs, checkpvs, checkpvssize); }
	virtual int		GetPlayerUserId( const edict_t *e ) { _engine->GetPlayerUserId(e); }
	virtual const char	*GetPlayerNetworkIDString( const edict_t *e ) { _engine->GetPlayerNetworkIDString(e); }
	virtual int		GetEntityCount( void ) { _engine->GetEntityCount(); }
	virtual int		IndexOfEdict( const edict_t *pEdict ) { _engine->IndexOfEdict(pEdict); }
	virtual edict_t	*PEntityOfEntIndex( int iEntIndex ) { _engine->PEntityOfEntIndex(iEntIndex); }
	virtual INetChannelInfo* GetPlayerNetInfo( int playerIndex ) { _engine->GetPlayerNetInfo(playerIndex); }
	virtual edict_t		*CreateEdict( int iForceEdictIndex = -1 ) { _engine->CreateEdict(iForceEdictIndex); }
	virtual void		RemoveEdict( edict_t *e ) { _engine->RemoveEdict(e); }
	virtual void		*PvAllocEntPrivateData( long cb ) { _engine->PvAllocEntPrivateData(cb); }
	virtual void		FreeEntPrivateData( void *pEntity ) { _engine->FreeEntPrivateData(pEntity); }
	virtual void		*SaveAllocMemory( size_t num, size_t size ) { _engine->SaveAllocMemory(num, size); }
	virtual void		SaveFreeMemory( void *pSaveMem ) { _engine->SaveFreeMemory(pSaveMem); }
	virtual void		EmitAmbientSound( int entindex, const Vector &pos, const char *samp, float vol, soundlevel_t soundlevel, int fFlags, int pitch, float delay = 0.0f ) { _engine->
	virtual void        FadeClientVolume( const edict_t *pEdict, float fadePercent, float fadeOutSeconds, float holdTime, float fadeInSeconds ) { _engine->
	virtual int			SentenceGroupPick( int groupIndex, char *name, int nameBufLen ) { _engine->
	virtual int			SentenceGroupPickSequential( int groupIndex, char *name, int nameBufLen, int sentenceIndex, int reset ) { _engine->
	virtual int			SentenceIndexFromName( const char *pSentenceName ) { _engine->
	virtual const char *SentenceNameFromIndex( int sentenceIndex ) { _engine->
	virtual int			SentenceGroupIndexFromName( const char *pGroupName ) { _engine->
	virtual const char *SentenceGroupNameFromIndex( int groupIndex ) { _engine->
	virtual float		SentenceLength( int sentenceIndex ) { _engine->

	// Issue a command to the command parser as if it was typed at the server console.	
	virtual void		ServerCommand( const char *str ) { _engine->
	// Execute any commands currently in the command parser immediately (instead of once per frame)
	virtual void		ServerExecute( void ) { _engine->
	// Issue the specified command to the specified client (mimics that client typing the command at the console).
	virtual void		ClientCommand( edict_t *pEdict, const char *szFmt, ... ) { _engine->

	// Set the lightstyle to the specified value and network the change to any connected clients.  Note that val must not 
	//  change place in memory (use MAKE_STRING) for anything that's not compiled into your mod.
	virtual void		LightStyle( int style, const char *val ) { _engine->

	// Project a static decal onto the specified entity / model (for level placed decals in the .bsp)
	virtual void		StaticDecal( const Vector &originInEntitySpace, int decalIndex, int entityIndex, int modelIndex, bool lowpriority ) { _engine->
	
	// Given the current PVS(or PAS) and origin, determine which players should hear/receive the message
	virtual void		Message_DetermineMulticastRecipients( bool usepas, const Vector& origin, CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits ) { _engine->

	// Begin a message from a server side entity to its client side counterpart (func_breakable glass, e.g.)
	virtual bf_write	*EntityMessageBegin( int ent_index, ServerClass * ent_class, bool reliable ) { _engine->
	// Begin a usermessage from the server to the client .dll
	virtual bf_write	*UserMessageBegin( IRecipientFilter *filter, int msg_type ) { _engine->
	// Finish the Entity or UserMessage and dispatch to network layer
	virtual void		MessageEnd( void ) { _engine->

	// Print szMsg to the client console.
	virtual void		ClientPrintf( edict_t *pEdict, const char *szMsg ) { _engine->

	// SINGLE PLAYER/LISTEN SERVER ONLY (just matching the client .dll api for this)
	// Prints the formatted string to the notification area of the screen ( down the right hand edge
	//  numbered lines starting at position 0
	virtual void		Con_NPrintf( int pos, const char *fmt, ... ) { _engine->
	// SINGLE PLAYER/LISTEN SERVER ONLY(just matching the client .dll api for this)
	// Similar to Con_NPrintf, but allows specifying custom text color and duration information
	virtual void		Con_NXPrintf( const struct con_nprint_s *info, const char *fmt, ... ) { _engine->

	// Change a specified player's "view entity" (i.e., use the view entity position/orientation for rendering the client view)
	virtual void		SetView( const edict_t *pClient, const edict_t *pViewent ) { _engine->

	// Get a high precision timer for doing profiling work
	virtual float		Time( void ) { _engine->

	// Set the player's crosshair angle
	virtual void		CrosshairAngle( const edict_t *pClient, float pitch, float yaw ) { _engine->

	// Get the current game directory (hl2, tf2, hl1, cstrike, etc.)
	virtual void        GetGameDir( char *szGetGameDir, int maxlength ) { _engine->

	// Used by AI node graph code to determine if .bsp and .ain files are out of date
	virtual int 		CompareFileTime( const char *filename1, const char *filename2, int *iCompare ) { _engine->

	// Locks/unlocks the network string tables (.e.g, when adding bots to server, this needs to happen).
	// Be sure to reset the lock after executing your code!!!
	virtual bool		LockNetworkStringTables( bool lock ) { _engine->

	// Create a bot with the given name.  Returns NULL if fake client can't be created
	virtual edict_t		*CreateFakeClient( const char *netname ) { _engine->	

	// Get a convar keyvalue for s specified client
	virtual const char	*GetClientConVarValue( int clientIndex, const char *name ) { _engine->
	
	// Parse a token from a file
	virtual const char	*ParseFile( const char *data, char *token, int maxlen ) { _engine->
	// Copies a file
	virtual bool		CopyFile( const char *source, const char *destination ) { _engine->

	// Reset the pvs, pvssize is the size in bytes of the buffer pointed to by pvs.
	// This should be called right before any calls to AddOriginToPVS
	virtual void		ResetPVS( byte *pvs, int pvssize ) { _engine->
	// Merge the pvs bits into the current accumulated pvs based on the specified origin ( not that each pvs origin has an 8 world unit fudge factor )
	virtual void		AddOriginToPVS( const Vector &origin ) { _engine->
	
	// Mark a specified area portal as open/closed.
	// Use SetAreaPortalStates if you want to set a bunch of them at a time.
	virtual void		SetAreaPortalState( int portalNumber, int isOpen ) { _engine->
	
	// Queue a temp entity for transmission
	virtual void		PlaybackTempEntity( IRecipientFilter& filter, float delay, const void *pSender, const SendTable *pST, int classID  ) { _engine->
	// Given a node number and the specified PVS, return with the node is in the PVS
	virtual int			CheckHeadnodeVisible( int nodenum, const byte *pvs, int vissize ) { _engine->
	// Using area bits, cheeck whether area1 flows into area2 and vice versa (depends on area portal state)
	virtual int			CheckAreasConnected( int area1, int area2 ) { _engine->
	// Given an origin, determine which area index the origin is within
	virtual int			GetArea( const Vector &origin ) { _engine->
	// Get area portal bit set
	virtual void		GetAreaBits( int area, unsigned char *bits, int buflen ) { _engine->
	// Given a view origin (which tells us the area to start looking in) and a portal key,
	// fill in the plane that leads out of this area (it points into whatever area it leads to).
	virtual bool		GetAreaPortalPlane( Vector const &vViewOrigin, int portalKey, VPlane *pPlane ) { _engine->

	// Save/restore wrapper - FIXME:  At some point we should move this to it's own interface
	virtual bool		LoadGameState( char const *pMapName, bool createPlayers ) { _engine->
	virtual void		LoadAdjacentEnts( const char *pOldLevel, const char *pLandmarkName ) { _engine->
	virtual void		ClearSaveDir() { _engine->

	// Get the pristine map entity lump string.  (e.g., used by CS to reload the map entities when restarting a round.)
	virtual const char*	GetMapEntitiesString() { _engine->

	// Text message system -- lookup the text message of the specified name
	virtual client_textmessage_t *TextMessageGet( const char *pName ) { _engine->

	// Print a message to the server log file
	virtual void		LogPrint( const char *msg ) { _engine->

	// Builds PVS information for an entity
	virtual void		BuildEntityClusterList( edict_t *pEdict, PVSInfo_t *pPVSInfo ) { _engine->

	// A solid entity moved, update spatial partition
	virtual void SolidMoved( edict_t *pSolidEnt, ICollideable *pSolidCollide, const Vector* pPrevAbsOrigin, bool testSurroundingBoundsOnly ) { _engine->
	// A trigger entity moved, update spatial partition
	virtual void TriggerMoved( edict_t *pTriggerEnt, bool testSurroundingBoundsOnly ) { _engine->
	
	// Create/destroy a custom spatial partition
	virtual ISpatialPartition *CreateSpatialPartition( const Vector& worldmin, const Vector& worldmax ) { _engine->
	virtual void 		DestroySpatialPartition( ISpatialPartition * ) { _engine->

	// Draw the brush geometry in the map into the scratch pad.
	// Flags is currently unused.
	virtual void		DrawMapToScratchPad( IScratchPad3D *pPad, unsigned long iFlags ) { _engine->

	// This returns which entities, to the best of the server's knowledge, the client currently knows about.
	// This is really which entities were in the snapshot that this client last acked.
	// This returns a bit vector with one bit for each entity.
	//
	// USE WITH CARE. Whatever tick the client is really currently on is subject to timing and
	// ordering differences, so you should account for about a quarter-second discrepancy in here.
	// Also, this will return NULL if the client doesn't exist or if this client hasn't acked any frames yet.
	// 
	// iClientIndex is the CLIENT index, so if you use pPlayer->entindex(), subtract 1.
	virtual const CBitVec<MAX_EDICTS>* GetEntityTransmitBitsForClient( int iClientIndex ) { _engine->
	
	// Is the game paused?
	virtual bool		IsPaused() { _engine->
	
	// Marks the filename for consistency checking.  This should be called after precaching the file.
	virtual void		ForceExactFile( const char *s ) { _engine->
	virtual void		ForceModelBounds( const char *s, const Vector &mins, const Vector &maxs ) { _engine->
	virtual void		ClearSaveDirAfterClientLoad() { _engine->

	// Sets a USERINFO client ConVar for a fakeclient
	virtual void		SetFakeClientConVarValue( edict_t *pEntity, const char *cvar, const char *value ) { _engine->
	
	// Marks the material (vmt file) for consistency checking.  If the client and server have different
	// contents for the file, the client's vmt can only use the VertexLitGeneric shader, and can only
	// contain $baseTexture and $bumpmap vars.
	virtual void		ForceSimpleMaterial( const char *s ) { _engine->

	// Is the engine in Commentary mode?
	virtual int			IsInCommentaryMode( void ) { _engine->
	

	// Mark some area portals as open/closed. It's more efficient to use this
	// than a bunch of individual SetAreaPortalState calls.
	virtual void		SetAreaPortalStates( const int *portalNumbers, const int *isOpen, int nPortals ) { _engine->

	// Called when relevant edict state flags change.
	virtual void		NotifyEdictFlagsChange( int iEdict ) { _engine->
	
	// Only valid during CheckTransmit. Also, only the PVS, networked areas, and
	// m_pTransmitInfo are valid in the returned strucutre.
	virtual const CCheckTransmitInfo* GetPrevCheckTransmitInfo( edict_t *pPlayerEdict ) { _engine->
	
	virtual CSharedEdictChangeInfo* GetSharedEdictChangeInfo() { _engine->

	// Tells the engine we can immdiately re-use all edict indices
	// even though we may not have waited enough time
	virtual void			AllowImmediateEdictReuse( ) { _engine->

	// Returns true if the engine is an internal build. i.e. is using the internal bugreporter.
	virtual bool		IsInternalBuild( void ) { _engine->

	virtual IChangeInfoAccessor *GetChangeAccessor( const edict_t *pEdict ) { _engine->	

	// Name of most recently load .sav file
	virtual char const *GetMostRecentlyLoadedFileName() { _engine->
	virtual char const *GetSaveFileName() { _engine->

	// Matchmaking
	virtual void MultiplayerEndGame() { _engine->
	virtual void ChangeTeam( const char *pTeamName ) { _engine->

	// Cleans up the cluster list
	virtual void CleanUpEntityClusterList( PVSInfo_t *pPVSInfo ) { _engine->

	virtual void SetAchievementMgr( IAchievementMgr *pAchievementMgr ) =0;
	virtual IAchievementMgr *GetAchievementMgr() { _engine->

	virtual int	GetAppID() { _engine->
	
	virtual bool IsLowViolence() { _engine->
	
	// Call this to find out the value of a cvar on the client.
	//
	// It is an asynchronous query, and it will call IServerGameDLL::OnQueryCvarValueFinished when
	// the value comes in from the client.
	//
	// Store the return value if you want to match this specific query to the OnQueryCvarValueFinished call.
	// Returns InvalidQueryCvarCookie if the entity is invalid.
	virtual QueryCvarCookie_t StartQueryCvarValue( edict_t *pPlayerEntity, const char *pName ) { _engine->

	virtual void InsertServerCommand( const char *str ) { _engine->

	// Fill in the player info structure for the specified player index (name, model, etc.)
	virtual bool GetPlayerInfo( int ent_num, player_info_t *pinfo ) { _engine->

	// Returns true if this client has been fully authenticated by Steam
	virtual bool IsClientFullyAuthenticated( edict_t *pEdict ) { _engine->

	// This makes the host run 1 tick per frame instead of checking the system timer to see how many ticks to run in a certain frame.
	// i.e. it does the same thing timedemo does.
	virtual void SetDedicatedServerBenchmarkMode( bool bBenchmarkMode ) { _engine->

	// Methods to set/get a gamestats data container so client & server running in same process can send combined data
	virtual void SetGamestatsData( CGamestatsData *pGamestatsData ) { _engine->
	virtual CGamestatsData *GetGamestatsData() { _engine->

	// Returns the SteamID of the specified player. It'll be NULL if the player hasn't authenticated yet.
	virtual const CSteamID	*GetClientSteamID( edict_t *pPlayerEdict ) { _engine->
};

class SDKEnvironment : public Environment {
public:
	// Override this to define how to set up the environment.
	virtual void SetUp() {
		orig_engine = engine;
		engine = new TestEngine( engine );
	}

	// Override this to define how to tear down the environment.
	virtual void TearDown() {
		engine = orig_engine;
	}

	IVEngineServer *orig_engine;
};
*/
