///////////// Copyright © 2017 GE:S Team. All rights reserved. /////////////
//
//   Project     : Client
//   File        : ge_popup.h
//   Description :
//      Generic popup with title and text.
//
////////////////////////////////////////////////////////////////////////////

#ifndef GE_POPUP_H
#define GE_POPUP_H
#pragma once

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include "ge_panelhelper.h"

class CGEPopupBox : public vgui::Frame
{
private:
	DECLARE_CLASS_SIMPLE(CGEPopupBox, vgui::Frame);

public:
	CGEPopupBox( vgui::VPANEL parent );
	~CGEPopupBox();

	virtual const char *GetName( void ) { return "GESPopupBox"; }

	virtual bool NeedsUpdate() { return false; };
	virtual bool HasInputElements( void ) { return true; }

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual void SetParent( vgui::Panel* parent ) { BaseClass::SetParent( parent ); }

	void displayPopup( const char* title, const char* message );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

extern GameUI<CGEPopupBox>* GetGEPopupBox();
#endif //GE_POPUP_H
