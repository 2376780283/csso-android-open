//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ModOptionsSubHUD_H
#define ModOptionsSubHUD_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/Panel.h>

class CLabeledCommandComboBox;
class CCvarSlider;
class CCvarToggleCheckButton;

class CModOptionsSubHUD;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubHUD: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubHUD, vgui::PropertyPage );

public:
	CModOptionsSubHUD( vgui::Panel *parent );
	~CModOptionsSubHUD();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );

	// Called when layout needs to be performed
	virtual void PerformLayout() override;

protected:
	MESSAGE_FUNC_PARAMS( OnScrollBarSliderMoved, "ScrollBarSliderMoved", data );
	MESSAGE_FUNC_PARAMS( OnSliderMoved, "SliderMoved", data );

	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	// Labels
	vgui::Label*				m_pPlayerCountPosLabel;
	vgui::Label*				m_pHealthAmmoStyleLabel;
	vgui::Label*				m_pHUDColorLabel;
	vgui::Label*				m_pHUDBackgroundAlphaLabel;
	vgui::Label*				m_pRadarScaleLabel;
	vgui::Label*				m_pRadarSquareLabel;
	vgui::Label*				m_pMenuBackgroundLabel;
	vgui::Label*				m_pMenuAgentLabel;

	// Controls
	CLabeledCommandComboBox*	m_pPlayerCountPos;
	CLabeledCommandComboBox*	m_pHealthAmmoStyle;
	CLabeledCommandComboBox*	m_pHUDColor;
	CCvarSlider*				m_pHUDBackgroundAlpha;
	CCvarSlider*				m_pRadarScale;
	CCvarToggleCheckButton*		m_pAlwaysShowInventory;
	CCvarToggleCheckButton*		m_pRadarRotate;
	CLabeledCommandComboBox*	m_pRadarSquare;
	CLabeledCommandComboBox*	m_pMenuBackground;
	CLabeledCommandComboBox*	m_pMenuAgent;

	// Scroll panel controls
	vgui::ScrollBar*			m_pVScrollBar;
	vgui::Panel*				m_pScrollContainer;
};

#endif // ModOptionsSubHUD_H
