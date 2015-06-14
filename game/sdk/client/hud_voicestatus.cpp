//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/ImageList.h>
#include "c_baseplayer.h"
#include "voice_status.h"
#include "clientmode_shared.h"
#include "c_playerresource.h"
#include "voice_common.h"
#include <igameresources.h>

#include "vgui_avatarimage.h"

#ifdef GE_DLL
#include "c_ge_playerresource.h"
#include "ge_shareddefs.h"
#include "gemp_gamerules.h"
#include "ge_utils.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar *sv_alltalk = NULL;

using namespace vgui;
//=============================================================================
// Icon for the local player using voice
//=============================================================================
class CHudVoiceSelfStatus : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudVoiceSelfStatus, vgui::Panel );

	CHudVoiceSelfStatus( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();
	virtual void VidInit();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CHudTexture *m_pVoiceIcon;


	Color	m_clrIcon;
};


DECLARE_HUDELEMENT( CHudVoiceSelfStatus );


CHudVoiceSelfStatus::CHudVoiceSelfStatus( const char *pName ) :
	vgui::Panel( NULL, "HudVoiceSelfStatus" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );

	m_pVoiceIcon = NULL;

	SetHiddenBits( 0 );

	m_clrIcon = Color(255,255,255,255);
}

	
void CHudVoiceSelfStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

#ifdef HL2MP
	SetBgColor( Color( 0, 0, 0, 0 ) );
#endif

	SetPaintBackgroundEnabled( false );
}

void CHudVoiceSelfStatus::VidInit( void )
{
	m_pVoiceIcon = gHUD.GetIcon( "voice_self" );
}

bool CHudVoiceSelfStatus::ShouldDraw()
{
	return GetClientVoiceMgr()->IsLocalPlayerSpeaking();
}

void CHudVoiceSelfStatus::Paint()
{
   if( !m_pVoiceIcon )
		return;
	
	int x, y, w, h;
	GetBounds( x, y, w, h );

	m_pVoiceIcon->DrawSelf( 0, 0, w, h, m_clrIcon );
}


//=============================================================================
// Icons for other players using voice
//=============================================================================
class CHudVoiceStatus : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudVoiceStatus, vgui::Panel );

	CHudVoiceStatus( const char *name );
	~CHudVoiceStatus();

	virtual void PostApplySchemeSettings( vgui::IScheme *pScheme );
	virtual bool ShouldDraw();	
	virtual void Paint();
	virtual void VidInit();
	virtual void Init();
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:

	//Tony; display avatars instead of an icon.
	vgui::ImageList				*m_pImageList;
	int							m_iImageAvatars[MAX_PLAYERS+1];

	int							m_iPlayerAvatar[MAX_PLAYERS+1];
	CUtlMap<int,int>			m_mapAvatarsToImageList;
	int m_iDeadImageID;

	Color	m_clrIcon;

	CUtlLinkedList< int > m_SpeakingList;
	
#ifdef GE_DLL
	CPanelAnimationVar( vgui::HFont, m_NameFont, "font", "Default" );
	CPanelAnimationVar( vgui::HFont, m_NameFont_Small, "font_small", "Default" );
#else
	CPanelAnimationVar( vgui::HFont, m_NameFont, "Default", "Default" );
#endif

	CPanelAnimationVarAliasType( float, item_tall, "item_tall", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, item_wide, "item_wide", "100", "proportional_float" );

	CPanelAnimationVarAliasType( float, item_spacing, "item_spacing", "2", "proportional_float" );

	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_tall, "icon_tall", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_wide, "icon_wide", "32", "proportional_float" );

	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "32", "proportional_float" );
};


DECLARE_HUDELEMENT( CHudVoiceStatus );

extern bool AvatarIndexLessFunc( const int &lhs, const int &rhs );
CHudVoiceStatus::CHudVoiceStatus( const char *pName ) : vgui::Panel( NULL, "HudVoiceStatus" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pImageList = NULL;

	m_mapAvatarsToImageList.SetLessFunc( AvatarIndexLessFunc );
	m_mapAvatarsToImageList.RemoveAll();
	memset( &m_iImageAvatars, 0, sizeof(int) * (MAX_PLAYERS+1) );

	SetHiddenBits( 0 );

	m_clrIcon = Color(255,255,255,255);

#ifdef GE_DLL
	m_iDeadImageID = surface()->DrawGetTextureId( "VGUI/scoreboard/player_dead" );
#else
	m_iDeadImageID = surface()->DrawGetTextureId( "hud/leaderboard_dead" );
#endif
	if ( m_iDeadImageID == -1 ) // we didn't find it, so create a new one
	{
		m_iDeadImageID = surface()->CreateNewTextureID();	
	}

#ifdef GE_DLL
	surface()->DrawSetTextureFile( m_iDeadImageID, "VGUI/scoreboard/player_dead", true, false );
#else
	surface()->DrawSetTextureFile( m_iDeadImageID, "hud/leaderboard_dead", true, false );
#endif
}

CHudVoiceStatus::~CHudVoiceStatus()
{
	if ( m_pImageList )
		delete m_pImageList;
}

void CHudVoiceStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{

	if ( m_pImageList )
		delete m_pImageList;
	m_pImageList = new ImageList( true );

	m_mapAvatarsToImageList.RemoveAll();
	memset( &m_iImageAvatars, 0, sizeof(int) * (MAX_PLAYERS+1) );

	BaseClass::ApplySchemeSettings( pScheme );

#ifdef HL2MP
	SetBgColor( Color( 0, 0, 0, 0 ) );
#endif

	SetPaintBackgroundEnabled( false );
}
void CHudVoiceStatus::PostApplySchemeSettings( vgui::IScheme *pScheme )
{
	// resize the images to our resolution
	for (int i = 0; i < m_pImageList->GetImageCount(); i++ )
	{
		int wide, tall;
		m_pImageList->GetImage(i)->GetSize(wide, tall);
		m_pImageList->GetImage(i)->SetSize(scheme()->GetProportionalScaledValueEx( GetScheme(),wide), scheme()->GetProportionalScaledValueEx( GetScheme(),tall));
	}
}

void CHudVoiceStatus::Init( void )
{
	m_SpeakingList.RemoveAll();
}

void CHudVoiceStatus::VidInit( void )
{
}

void CHudVoiceStatus::OnThink( void )
{
	//Tony; don't update if local player aint here!
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (!player)
		return;

	for( int i=1;i<=gpGlobals->maxClients;i++ )
	{
		bool bTalking = GetClientVoiceMgr()->IsPlayerSpeaking(i);
		
		//Tony; update avatars.
		if ( steamapicontext->SteamFriends() && steamapicontext->SteamUtils() )
		{
			player_info_t pi;
			if ( engine->GetPlayerInfo( i, &pi ) )
			{
				if ( pi.friendsID )
				{
					CSteamID steamIDForPlayer( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );

					// See if the avatar's changed
					int iAvatar = steamapicontext->SteamFriends()->GetFriendAvatar( steamIDForPlayer );
					if ( m_iImageAvatars[i] != iAvatar )
					{
						m_iImageAvatars[i] = iAvatar;

						// Now see if we already have that avatar in our list
						int iIndex = m_mapAvatarsToImageList.Find( iAvatar );
						if ( iIndex == m_mapAvatarsToImageList.InvalidIndex() )
						{
							CAvatarImage *pImage = new CAvatarImage();
							pImage->SetAvatarSteamID( steamIDForPlayer );
							pImage->SetAvatarSize( 32, 32 );	// Deliberately non scaling
							int iImageIndex = m_pImageList->AddImage( pImage );

							m_mapAvatarsToImageList.Insert( iAvatar, iImageIndex );
						}
					}

					int iIndex = m_mapAvatarsToImageList.Find( iAvatar );

					m_iPlayerAvatar[i] = -1; //Default it.
					if ( iIndex != m_mapAvatarsToImageList.InvalidIndex() )
					{
						//Tony; copy the avatar over.
						m_iPlayerAvatar[i] = m_mapAvatarsToImageList[iIndex];
					}
				}
			}
		}

		// if they are in the list and not talking, remove them
		if( !bTalking )
		{
			// remove them if they are in the list
			m_SpeakingList.FindAndRemove(i);
		}
		else
		{
			// if they are talking and not in the list, add them to the end
			if( m_SpeakingList.Find(i) == m_SpeakingList.InvalidIndex() )
			{
				m_SpeakingList.AddToTail(i);
			}
		}
	}

#ifdef _DEBUG
	// FOR DEBUGGING SO WE CAN SEE OUR OWN BOX!
	if ( GetClientVoiceMgr()->IsLocalPlayerSpeaking() )
	{
		if( m_SpeakingList.Find(player->entindex()) == m_SpeakingList.InvalidIndex() )
		{
			m_SpeakingList.AddToTail(player->entindex());
		}
	}
	else
	{
		m_SpeakingList.FindAndRemove(player->entindex());
	}
#endif
}

bool CHudVoiceStatus::ShouldDraw()
{
	if ( IsInFreezeCam() == true )
		return false;

	return true;
}

#ifdef GE_DLL
void CHudVoiceStatus::Paint()
{
	if ( !g_PR )
		return;

	int x, y, w, h;
	GetBounds( x, y, w, h );

	// Heights to draw the current voice item at
	int xpos = 0;
	int ypos = h - item_tall;

	int iFontHeight = 0;

	if ( !sv_alltalk )
		sv_alltalk = cvar->FindVar( "sv_alltalk" );

	//draw everyone in the list!
	for( unsigned short i = m_SpeakingList.Head(); i != m_SpeakingList.InvalidIndex(); i = m_SpeakingList.Next(i) )
	{
		int playerIndex = m_SpeakingList.Element(i);
		bool bIsAlive = g_PR->IsAlive( playerIndex );
		// Added for GE so spectators show their icon, not the skull
		int iTeam = g_PR->GetTeam( playerIndex );

		vgui::IScheme *pClientScheme = vgui::scheme()->GetIScheme( vgui::scheme()->GetScheme( "ClientScheme" ) );
		Color c;
		switch ( iTeam )
		{
			case TEAM_JANUS:
				c = pClientScheme->GetColor( "GEJanusChatColor", g_PR->GetTeamColor(TEAM_JANUS) );
				break;
			case TEAM_MI6:
				c = pClientScheme->GetColor( "GEMI6ChatColor", g_PR->GetTeamColor(TEAM_MI6) );
				break;
			default:
				c = pClientScheme->GetColor( "GENoTeamChatColor", g_PR->GetTeamColor(TEAM_UNASSIGNED) );
				break;
		}
		c[3] = 200;
	
		const char *pName = g_PR->GetPlayerName(playerIndex);
		char cName[64];
		Q_strncpy( cName, pName, 64 );

		wchar_t szconverted[64];
		g_pVGuiLocalize->ConvertANSIToUnicode( GEUTIL_RemoveColorHints(cName), szconverted, sizeof(szconverted)  );

		// Draw the item background
		surface()->DrawSetColor( c );
		surface()->DrawFilledRect( xpos, ypos, xpos + item_wide, ypos + item_tall );
	
		int iDeathIconWidth = 0;
		int imageIndex = m_iPlayerAvatar[playerIndex];
		int avatarWidth = 0, avatarHeight = 0;

		// For GE, don't show the dead symbol if we are spectating
		if ( bIsAlive == false && m_iDeadImageID != -1 && iTeam != TEAM_SPECTATOR )
		{
			Vertex_t vert[4];	
			float uv1 = 0.0f;
			float uv2 = 1.0f;

			// Draw the dead material
			surface()->DrawSetTexture( m_iDeadImageID );

			vert[0].Init( Vector2D( xpos, ypos ), Vector2D( uv1, uv1 ) );
			vert[1].Init( Vector2D( xpos + icon_wide, ypos ), Vector2D( uv2, uv1 ) );
			vert[2].Init( Vector2D( xpos + icon_wide, ypos + icon_tall ), Vector2D( uv2, uv2 ) );				
			vert[3].Init( Vector2D( xpos, ypos + icon_tall ), Vector2D( uv1, uv2 ) );

			surface()->DrawSetColor( Color(255,255,255,255) );

			surface()->DrawTexturedPolygon( 4, vert );

			iDeathIconWidth = icon_wide;
		}
		// Only draw death icon OR the avatar so as not to overcrowd the player's name
		else if ( m_pImageList->IsValidIndex(imageIndex) )
		{
			// 0 is always the blank image
			if (imageIndex > 0)
			{
				IImage *image = m_pImageList->GetImage(imageIndex);
				if (image)
				{
					image->SetPos( xpos + icon_xpos, ypos + icon_ypos );
					image->SetSize(icon_wide, icon_tall);
					// Populate these variables as our width / height changes if we are friends with this person
					image->GetSize( avatarWidth, avatarHeight );
					image->Paint();
				}
			}
		}

		// Figure out if we will fit in the box with our normal font, if not use the small font
		int text_x = xpos + icon_xpos + iDeathIconWidth + avatarWidth;
		int iTextWidth, iTextSpace = item_wide - text_x;

		surface()->GetTextSize( m_NameFont, szconverted, iTextWidth, iFontHeight );
		if ( iTextWidth > iTextSpace )
		{
			surface()->DrawSetTextFont( m_NameFont_Small );
			surface()->GetTextSize( m_NameFont_Small, szconverted, iTextWidth, iFontHeight );
		}
		else
			surface()->DrawSetTextFont( m_NameFont );

		// Draw the player's name
		surface()->DrawSetTextPos( xpos + text_x, ypos + ( item_tall / 2 ) - ( iFontHeight / 2 ) );

		IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
		surface()->DrawSetTextColor( pScheme->GetColor( "GEVoiceChat", Color(255,255,255) ) );
		surface()->DrawPrintText( szconverted, wcslen(szconverted) );
			
		ypos -= ( item_spacing + item_tall );
	}
}
#endif
