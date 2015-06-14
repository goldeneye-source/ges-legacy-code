///////////// Copyright © 2008, Anthony Iacono. All rights reserved. /////////////
// 
// File: ge_hudcrosshair.h
// Description:
//      I can has crosshairs?
//
// Created On: 03/05/2008
// Created By: Anthony Iacono
/////////////////////////////////////////////////////////////////////////////

#include "hudelement.h"
#include "vgui_controls/panel.h"

using namespace vgui;

class CGEHudCrosshair : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CGEHudCrosshair, vgui::Panel );

public:
	CGEHudCrosshair( const char *pElementName );
	~CGEHudCrosshair();

	void VidInit( void );
	void DrawCrosshair();
	bool ShouldDrawCrosshair();

protected:
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );

private:
	CHudTexture		*m_pCrosshair;
	IMaterial		*m_p3DCrosshair;
};
