//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubGloves.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"

#include "LabeledCommandComboBox.h"
#include "tier1/convar.h"
#include "BitmapImagePanel.h"
#include "GameUI_Interface.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct Gloves
{
	const char*		m_szUIName;
	const char*		m_szImage;
};

static Gloves gloveNames[] =
{
	{ "#GameUI_Loadout_Glove_Default",					NULL							},
	{ "#GameUI_Loadout_Glove_Bloodhound",				"glove_bloodhound"				},
	{ "#GameUI_Loadout_Glove_Bloodhound_PerfectWorld",	"glove_bloodhound_perfectworld"	},
	{ "#GameUI_Loadout_Glove_Bloodhound_BrokenFang",	"glove_bloodhound_brokenfang"	},
	{ "#GameUI_Loadout_Glove_Bloodhound_Hydra",			"glove_bloodhound_hydra"		},
	{ "#GameUI_Loadout_Glove_Fingerless",				"glove_fingerless"				},
	{ "#GameUI_Loadout_Glove_Fullfinger",				"glove_fullfinger"				},
	{ "#GameUI_Loadout_Glove_Handwrap_Leathery",		"glove_handwrap_leathery"		},
	{ "#GameUI_Loadout_Glove_Hardknuckle",				"glove_hardknuckle"				},
	{ "#GameUI_Loadout_Glove_Hardknuckle_black",		"glove_hardknuckle_black"		},
	{ "#GameUI_Loadout_Glove_Hardknuckle_blue",			"glove_hardknuckle_blue"		},
	{ "#GameUI_Loadout_Glove_Motorcycle",				"glove_motorcycle"				},
	{ "#GameUI_Loadout_Glove_Slick",					"glove_slick"					},
	{ "#GameUI_Loadout_Glove_Specialist",				"glove_specialist"				},
	{ "#GameUI_Loadout_Glove_Sporty",					"glove_sporty"					},
	{ "#GameUI_Loadout_Glove_SAS_Old",					"glove_sas_old"					},
	{ "#GameUI_Loadout_Glove_FBI_Old",					"glove_fbi_old"					},
	{ "#GameUI_Loadout_Glove_Phoenix_Old",				"glove_phoenix_old"				},
	{ "#GameUI_Loadout_Glove_Leet_Old",					"glove_leet_old"				},
	{ "#GameUI_Loadout_Glove_Bare",						"glove_bare"					},
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CModOptionsSubGloves::CModOptionsSubGloves(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubGloves")
{
	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create labels
	m_pGloveCTComboBoxLabel = new vgui::Label( this, "GloveCTComboBoxLabel", "#GameUI_Loadout_Glove_CT" );
	m_pGloveTComboBoxLabel = new vgui::Label( this, "GloveTComboBoxLabel", "#GameUI_Loadout_Glove_T" );

	// Create ComboBoxes
	m_pLoadoutGloveCTComboBox = new CLabeledCommandComboBox( this, "GloveCTComboBox" );
	m_pLoadoutGloveTComboBox = new CLabeledCommandComboBox( this, "GloveTComboBox" );

	int i;
	char command[64];
	for ( i = 0; i < ARRAYSIZE( gloveNames ); i++ )
	{
		Q_snprintf( command, sizeof( command ), "loadout_slot_gloves_ct %d", i );
		m_pLoadoutGloveCTComboBox->AddItem( gloveNames[i].m_szUIName, command );
		Q_snprintf( command, sizeof( command ), "loadout_slot_gloves_t %d", i );
		m_pLoadoutGloveTComboBox->AddItem( gloveNames[i].m_szUIName, command );
	}

	// Create image panels
	m_pGloveImageCT = new CBitmapImagePanel( this, "GloveImageCT", NULL );
	m_pGloveImageCT->AddActionSignalTarget( this );
	m_pGloveImageT = new CBitmapImagePanel( this, "GloveImageT", NULL );
	m_pGloveImageT->AddActionSignalTarget( this );

	m_pLoadoutGloveCTComboBox->AddActionSignalTarget( this );
	m_pLoadoutGloveTComboBox->AddActionSignalTarget( this );
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

#ifndef PROPVAL
	#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

	int margin = PROPVAL(16);
	int halfWidth = PROPVAL(224);
	int controlHeight = PROPVAL(24);
	int labelHeight = PROPVAL(24);
	int previewHeight = PROPVAL(168);

	// Calculate total width and center offset
	int totalWidth = margin + halfWidth + PROPVAL(12) + halfWidth;
	int centerOffset = (pw - totalWidth) / 2;

	// CT side (left column)
	int leftX = margin + centerOffset;
	// Glove CT ComboBox Label at y=8
	m_pGloveCTComboBoxLabel->SetPos(leftX, margin + PROPVAL(8));
	m_pGloveCTComboBoxLabel->SetSize(halfWidth, labelHeight);
	// Glove Image CT at y=32
	m_pGloveImageCT->SetPos(leftX, margin + PROPVAL(32));
	m_pGloveImageCT->SetSize(halfWidth, previewHeight);
	// Glove CT ComboBox at y=216
	m_pLoadoutGloveCTComboBox->SetPos(leftX, margin + PROPVAL(216));
	m_pLoadoutGloveCTComboBox->SetSize(halfWidth, controlHeight);

	// T side (right column)
	int rightX = margin + halfWidth + PROPVAL(12) + centerOffset;
	// Glove T ComboBox Label at y=8
	m_pGloveTComboBoxLabel->SetPos(rightX, margin + PROPVAL(8));
	m_pGloveTComboBoxLabel->SetSize(halfWidth, labelHeight);
	// Glove Image T at y=32
	m_pGloveImageT->SetPos(rightX, margin + PROPVAL(32));
	m_pGloveImageT->SetSize(halfWidth, previewHeight);
	// Glove T ComboBox at y=216
	m_pLoadoutGloveTComboBox->SetPos(rightX, margin + PROPVAL(216));
	m_pLoadoutGloveTComboBox->SetSize(halfWidth, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubGloves::~CModOptionsSubGloves()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::RemapGlovesImage()
{
	const char *pImageNameCT = gloveNames[m_pLoadoutGloveCTComboBox->GetActiveItem()].m_szImage;
	const char *pImageNameT = gloveNames[m_pLoadoutGloveTComboBox->GetActiveItem()].m_szImage;

	char texture[256];
	if ( pImageNameCT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/gloves/%s", pImageNameCT );
		m_pGloveImageCT->setTexture( texture );
	}
	else
	{
		m_pGloveImageCT->setTexture( "vgui/gloves/ct_none" );
	}

	if ( pImageNameT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/gloves/%s", pImageNameT );
		m_pGloveImageT->setTexture( texture );
	}
	else
	{
		m_pGloveImageT->setTexture( "vgui/gloves/t_none" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnTextChanged( vgui::Panel *panel )
{
	RemapGlovesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnResetData()
{
	ConVarRef loadout_slot_gloves_ct( "loadout_slot_gloves_ct" );
	if ( loadout_slot_gloves_ct.IsValid() )
		m_pLoadoutGloveCTComboBox->SetInitialItem( loadout_slot_gloves_ct.GetInt() );

	ConVarRef loadout_slot_gloves_t( "loadout_slot_gloves_t" );
	if ( loadout_slot_gloves_t.IsValid() )
		m_pLoadoutGloveTComboBox->SetInitialItem( loadout_slot_gloves_t.GetInt() );

	RemapGlovesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGloves::OnApplyChanges()
{
	m_pLoadoutGloveCTComboBox->ApplyChanges();
	m_pLoadoutGloveTComboBox->ApplyChanges();

	// update gloves on main menu agents
	GameUI().UpdateAgentModel();
}
