//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUITEPTHMENU_H
#define VGUIDEPTHMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "viewpostprocess.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/SectionedListPanel.h"
#include "vgui_controls/Slider.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "game/client/iviewport.h"

using namespace vgui;

class CDepthMenu : public Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CDepthMenu, Frame );

public:
	CDepthMenu( IViewPort* pViewPort );

	// IViewPortPanel overrides
	virtual const char* GetName( void ) { return PANEL_DEPTH_MENU; }
	virtual void SetData( KeyValues* data ) {}
	virtual void Reset() {}
	virtual void Update() {}
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual void ShowPanel( bool bShow );
	
	virtual void PaintBackground();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );
	virtual void PerformLayout();
	virtual void OnSliderMoved();
	
private:
	IViewPort* m_pViewPort;
	
	Slider *m_pNearBlurDepthSlider;
	Slider *m_pNearFocusDepthSlider;
	Slider *m_pFarFocusDepthSlider;
	Slider *m_pFarBlurDepthSlider;
	Slider *m_pNearBlurRadiusSlider;
	Slider *m_pFarBlurRadiusSlider;
	
	Label *m_pNearBlurDepthLabel;
	Label *m_pNearFocusDepthLabel;
	Label *m_pFarFocusDepthLabel;
	Label *m_pFarBlurDepthLabel;
	Label *m_pNearBlurRadiusLabel;
	Label *m_pFarBlurRadiusLabel;
	
	Button *m_pResetButton;
	
	void CreateDOFControls();
	void UpdateDOFLabels();
	void ResetDOFToDefaults();
};

#endif
