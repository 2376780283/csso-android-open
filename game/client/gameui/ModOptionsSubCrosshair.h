//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBCROSSHAIR_H
#define MODOPTIONSSUBCROSSHAIR_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ScrollBar.h>
#include "imageutils.h"

class CLabeledCommandComboBox;

class CCvarToggleCheckButton;
class CCvarTextEntry;
class CCvarSlider;

class CModOptionsSubCrosshair;
 
class CrosshairImagePanelBase : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE( CrosshairImagePanelBase, vgui::ImagePanel );
public:
	CrosshairImagePanelBase( Panel *parent, const char *name ) : BaseClass(parent, name) {}
	virtual void ResetData() {}
	virtual void ApplyChanges() {}
	virtual void UpdateVisibility() {}
};

class CrosshairImagePanelCS;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubCrosshair: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubCrosshair, vgui::PropertyPage );

public:
	CModOptionsSubCrosshair( vgui::Panel *parent );
	~CModOptionsSubCrosshair();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );

	// Called when layout needs to be performed
	virtual void PerformLayout() override;

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	CrosshairImagePanelCS *m_pCrosshairImage;

	// Scroll components
	vgui::Panel *m_pScrollContainer;
	vgui::ScrollBar *m_pVScrollBar;

	// Message handlers for scroll
	MESSAGE_FUNC_PARAMS( OnScrollBarSliderMoved, "ScrollBarSliderMoved", data );

public:
	// Getter for scroll container (used by child panels to parent controls)
	vgui::Panel* GetScrollContainer() { return m_pScrollContainer; }
};

#endif // MODOPTIONSSUBCROSSHAIR_H