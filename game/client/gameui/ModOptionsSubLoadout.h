//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODOPTIONSSUBLOADOUT_H
#define MODOPTIONSSUBLOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

class CCvarToggleCheckButton;
class CLabeledCommandComboBox;

class CModOptionsSubLoadout;

// PiMoN: change this to zero if you want to
// show a MessageBox to player that they need
// to restart the game or disconnect from a server
#define INSTANT_MUSIC_CHANGE 1

//-----------------------------------------------------------------------------
// Purpose: crosshair options property page
//-----------------------------------------------------------------------------
class CModOptionsSubLoadout: public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CModOptionsSubLoadout, vgui::PropertyPage );

public:
	CModOptionsSubLoadout( vgui::Panel *parent );
	~CModOptionsSubLoadout();

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
	// Labels for CT weapons
	vgui::Label* m_pM4ComboBoxLabel;
	vgui::Label* m_pHKP2000ComboBoxLabel;
	vgui::Label* m_pFiveSevenComboBoxLabel;
	vgui::Label* m_pMP7CTComboBoxLabel;
	vgui::Label* m_pDeagleCTComboBoxLabel;

	// Labels for T weapons
	vgui::Label* m_pTec9ComboBoxLabel;
	vgui::Label* m_pMP7TComboBoxLabel;
	vgui::Label* m_pDeagleTComboBoxLabel;

	// Other labels
	vgui::Label* m_pMusicSelectionLabel;

	// ComboBox controls
	CLabeledCommandComboBox *m_pLoadoutM4ComboBox;
	CLabeledCommandComboBox *m_pLoadoutHKP2000ComboBox;
	CLabeledCommandComboBox *m_pLoadoutFiveSevenComboBox;
	CLabeledCommandComboBox *m_pLoadoutTec9ComboBox;
	CLabeledCommandComboBox *m_pLoadoutMP7CTComboBox;
	CLabeledCommandComboBox *m_pLoadoutMP7TComboBox;
	CLabeledCommandComboBox *m_pLoadoutDeagleCTComboBox;
	CLabeledCommandComboBox *m_pLoadoutDeagleTComboBox;
	CCvarToggleCheckButton	*m_pStatTrak;
	CLabeledCommandComboBox *m_pMusicSelection;

#if !INSTANT_MUSIC_CHANGE
	bool						m_bNeedToWarnAboutMusic;
#endif

	// Scroll panel controls
	vgui::ScrollBar*			m_pVScrollBar;
	vgui::Panel*				m_pScrollContainer;
};

#endif // MODOPTIONSSUBLOADOUT_H