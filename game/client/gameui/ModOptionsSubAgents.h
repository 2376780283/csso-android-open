//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBAGENTS_H
#define MODOPTIONSSUBAGENTS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>

class CLabeledCommandComboBox;
class CBitmapImagePanel;

class CModOptionsSubAgents;

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubAgents: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubAgents, vgui::PropertyPage );

public:
	CModOptionsSubAgents( vgui::Panel *parent );
	~CModOptionsSubAgents();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );

	// Called when layout needs to be performed
	virtual void PerformLayout() override;

protected:
	// Called when page is loaded.  Data should be reloaded from document into controls.
	virtual void OnResetData();
	// Called when the OK / Apply button is pressed.  Changed data should be written into document.
	virtual void OnApplyChanges();

private:
	void					RemapAgentsImage();

	// Labels
	vgui::Label*			m_pAgentCTComboBoxLabel;
	vgui::Label*			m_pAgentTComboBoxLabel;
	vgui::Label*			m_pMainMenuWeaponCTLabel;
	vgui::Label*			m_pMainMenuWeaponTLabel;

	// Image panels
	CBitmapImagePanel		*m_pAgentImageCT;
	CBitmapImagePanel		*m_pAgentImageT;

	// ComboBoxes
	CLabeledCommandComboBox *m_pLoadoutAgentCTComboBox;
	CLabeledCommandComboBox *m_pLoadoutAgentTComboBox;
	CLabeledCommandComboBox *m_pLoadoutMainMenuWeaponCTComboBox;
	CLabeledCommandComboBox * m_pLoadoutMainMenuWeaponTComboBox;
};

#endif // MODOPTIONSSUBAGENTS_H
