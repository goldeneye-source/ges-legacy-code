///////////// Copyright © 2008, Anthony Iacono. All rights reserved. /////////////
// 
// File: ge_hudcrosshair.cpp
// Description:
//      I can has crosshairs?
//
// Created On: 03/05/2008
// Created By: Anthony Iacono
/////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "ge_hudcrosshair.h"
#include "c_ge_player.h"
#include "view.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar cl_ge_draw3dhud;

DECLARE_HUDELEMENT( CGEHudCrosshair );

#define CROSSHAIR_SCALE	0.7f

CGEHudCrosshair::CGEHudCrosshair( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "GEHudCrosshair" )
{
	m_p3DCrosshair = NULL;
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
}

CGEHudCrosshair::~CGEHudCrosshair()
{
	if ( m_p3DCrosshair )
		m_p3DCrosshair->DecrementReferenceCount();
}

void CGEHudCrosshair::VidInit()
{
	m_pCrosshair = gHUD.GetIcon("ge_crosshair");
	if ( m_pCrosshair && !m_p3DCrosshair )
	{
		m_p3DCrosshair = materials->FindMaterial( m_pCrosshair->szTextureFile, TEXTURE_GROUP_VGUI );
		m_p3DCrosshair->IncrementReferenceCount();
	}
}

void CGEHudCrosshair::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );
    SetSize( ScreenWidth(), ScreenHeight() );
}

void CGEHudCrosshair::DrawCrosshair()
{
	// Special call to draw a 3D mesh crosshair in front of our view
	if ( ShouldDraw() && ShouldDrawCrosshair() )
	{
		C_GEPlayer *pPlayer = ToGEPlayer( C_BasePlayer::GetLocalPlayer() );
		if ( !pPlayer )
			return;

		// Base distance from camera
		float dist = 1200.0f;

		// Adjust distance for zoom
		float zoom = pPlayer->GetZoom() + pPlayer->GetDefaultFOV();
		if (zoom < 90.0f )
			dist *= 1 / tan( DEG2RAD(zoom) / 2.0f );
		
		// Move the camera origin out the calc'd distance
		Vector vOrigin = CurrentViewOrigin();
		VectorMA( vOrigin, dist, CurrentViewForward(), vOrigin );

		// Vectors for drawing
		Vector vUp = CurrentViewUp();
		Vector vRight = CurrentViewRight();
		vRight.z = 0;
		VectorNormalize( vRight );

		float flSize = m_pCrosshair->EffectiveWidth( CROSSHAIR_SCALE );

		CMatRenderContextPtr pRenderContext( materials );
		pRenderContext->Bind( m_p3DCrosshair );

		IMesh *pMesh = pRenderContext->GetDynamicMesh();
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0,0,0 );
		meshBuilder.Position3fv( (vOrigin + (vRight * -flSize) + (vUp * flSize)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0,1,0 );
		meshBuilder.Position3fv( (vOrigin + (vRight * flSize) + (vUp * flSize)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0,1,1 );
		meshBuilder.Position3fv( (vOrigin + (vRight * flSize) + (vUp * -flSize)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0,0,1 );
		meshBuilder.Position3fv( (vOrigin + (vRight * -flSize) + (vUp * -flSize)).Base() );
		meshBuilder.AdvanceVertex();
		meshBuilder.End();
		pMesh->Draw();
	}
}

extern ConVar cl_spec_mode;
bool CGEHudCrosshair::ShouldDrawCrosshair()
{
	C_GEPlayer *pPlayer = ToGEPlayer( C_BasePlayer::GetLocalPlayer() );
	if( !pPlayer )
		return false;

	if ( pPlayer->IsObserver() )
	{
		C_GEPlayer *obsTarget = ToGEPlayer( pPlayer->GetObserverTarget() );
		if ( obsTarget && obsTarget->IsPlayer() )
		{
			// Only show crosshair if we are "In Eye"
			if ( obsTarget->IsInAimMode() && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
				return true;
		}

		return false;
	}

	// Only draw the crosshair when we are actually established in aim mode
	if( pPlayer->IsInAimMode() )
		return true;

	return false;
}
