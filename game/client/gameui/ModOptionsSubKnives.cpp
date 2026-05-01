//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubKnives.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"

#include "LabeledCommandComboBox.h"
#include "tier1/convar.h"
#include "BitmapImagePanel.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

using namespace vgui;

struct Knives
{
	const char*		m_szUIName;
	const char*		m_szImage;
};

static Knives knifeNames[] =
{
	{ "#GameUI_Loadout_Knife_Default",		NULL							},
	{ "#GameUI_Loadout_Knife_CSS",			"weapon_knife_css"				},
	{ "#GameUI_Loadout_Knife_Karambit",		"weapon_knife_karambit"			},
	{ "#GameUI_Loadout_Knife_Flip",			"weapon_knife_flip"				},
	{ "#GameUI_Loadout_Knife_Bayonet",		"weapon_knife_bayonet"			},
	{ "#GameUI_Loadout_Knife_M9_Bayonet",	"weapon_knife_m9_bayonet"		},
	{ "#GameUI_Loadout_Knife_Butterfly",	"weapon_knife_butterfly"		},
	{ "#GameUI_Loadout_Knife_Gut",			"weapon_knife_gut"				},
	{ "#GameUI_Loadout_Knife_Tactical",		"weapon_knife_tactical"			},
	{ "#GameUI_Loadout_Knife_Falchion",		"weapon_knife_falchion"			},
	{ "#GameUI_Loadout_Knife_Bowie",		"weapon_knife_survival_bowie"	},
	{ "#GameUI_Loadout_Knife_Canis",		"weapon_knife_canis"			},
	{ "#GameUI_Loadout_Knife_Cord",			"weapon_knife_cord"				},
	{ "#GameUI_Loadout_Knife_Gypsy",		"weapon_knife_gypsy_jackknife"	},
	{ "#GameUI_Loadout_Knife_Outdoor",		"weapon_knife_outdoor"			},
	{ "#GameUI_Loadout_Knife_Skeleton",		"weapon_knife_skeleton"			},
	{ "#GameUI_Loadout_Knife_Stiletto",		"weapon_knife_stiletto"			},
	{ "#GameUI_Loadout_Knife_Ursus",		"weapon_knife_ursus"			},
	{ "#GameUI_Loadout_Knife_Widowmaker",	"weapon_knife_widowmaker"		},
	{ "#GameUI_Loadout_Knife_Push",			"weapon_knife_push"				},
};

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout
//-----------------------------------------------------------------------------
CModOptionsSubKnives::CModOptionsSubKnives(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubKnives")
{
	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create ComboBox Labels (matching RES file naming)
	m_pKnifeCTComboBoxLabel = new vgui::Label(this, "KnifeCTComboBoxLabel", "#GameUI_Loadout_Knife_CT");
	m_pKnifeTComboBoxLabel = new vgui::Label(this, "KnifeTComboBoxLabel", "#GameUI_Loadout_Knife_T");

	// Create controls
	m_pKnifeImageCT = new CBitmapImagePanel(this, "KnifeImageCT", NULL);
	m_pKnifeImageCT->AddActionSignalTarget(this);

	m_pLoadoutKnifeCTComboBox = new CLabeledCommandComboBox(this, "KnifeCTComboBox");
	char command[64];
	for (int i = 0; i < ARRAYSIZE(knifeNames); i++)
	{
		Q_snprintf(command, sizeof(command), "loadout_slot_knife_weapon_ct %d", i);
		m_pLoadoutKnifeCTComboBox->AddItem(knifeNames[i].m_szUIName, command);
	}
	m_pLoadoutKnifeCTComboBox->AddActionSignalTarget(this);

	m_pKnifeImageT = new CBitmapImagePanel(this, "KnifeImageT", NULL);
	m_pKnifeImageT->AddActionSignalTarget(this);

	m_pLoadoutKnifeTComboBox = new CLabeledCommandComboBox(this, "KnifeTComboBox");
	for (int i = 0; i < ARRAYSIZE(knifeNames); i++)
	{
		Q_snprintf(command, sizeof(command), "loadout_slot_knife_weapon_t %d", i);
		m_pLoadoutKnifeTComboBox->AddItem(knifeNames[i].m_szUIName, command);
	}
	m_pLoadoutKnifeTComboBox->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// Use RES file coordinate values
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
	// Knife CT ComboBox Label at y=8
	m_pKnifeCTComboBoxLabel->SetPos(leftX, margin + PROPVAL(8));
	m_pKnifeCTComboBoxLabel->SetSize(halfWidth, labelHeight);
	// Knife Image CT at y=32
	m_pKnifeImageCT->SetPos(leftX, margin + PROPVAL(32));
	m_pKnifeImageCT->SetSize(halfWidth, previewHeight);
	// Knife CT ComboBox at y=216
	m_pLoadoutKnifeCTComboBox->SetPos(leftX, margin + PROPVAL(216));
	m_pLoadoutKnifeCTComboBox->SetSize(halfWidth, controlHeight);

	// T side (right column)
	int rightX = margin + halfWidth + PROPVAL(12) + centerOffset;
	// Knife T ComboBox Label at y=8
	m_pKnifeTComboBoxLabel->SetPos(rightX, margin + PROPVAL(8));
	m_pKnifeTComboBoxLabel->SetSize(halfWidth, labelHeight);
	// Knife Image T at y=32
	m_pKnifeImageT->SetPos(rightX, margin + PROPVAL(32));
	m_pKnifeImageT->SetSize(halfWidth, previewHeight);
	// Knife T ComboBox at y=216
	m_pLoadoutKnifeTComboBox->SetPos(rightX, margin + PROPVAL(216));
	m_pLoadoutKnifeTComboBox->SetSize(halfWidth, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubKnives::~CModOptionsSubKnives()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::RemapKnivesImage()
{
	const char *pImageNameCT = knifeNames[m_pLoadoutKnifeCTComboBox->GetActiveItem()].m_szImage;
	const char *pImageNameT = knifeNames[m_pLoadoutKnifeTComboBox->GetActiveItem()].m_szImage;

	char texture[256];
	if ( pImageNameCT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/knives/%s", pImageNameCT );
		m_pKnifeImageCT->setTexture( texture );
	}
	else
	{
		m_pKnifeImageCT->setTexture( "vgui/knives/weapon_knife" );
	}

	if ( pImageNameT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/knives/%s", pImageNameT );
		m_pKnifeImageT->setTexture( texture );
	}
	else
	{
		m_pKnifeImageT->setTexture( "vgui/knives/weapon_knife_t" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnTextChanged( vgui::Panel *panel )
{
	RemapKnivesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnResetData()
{
	ConVarRef loadout_slot_knife_weapon_ct( "loadout_slot_knife_weapon_ct" );
	if ( loadout_slot_knife_weapon_ct.IsValid() )
		m_pLoadoutKnifeCTComboBox->SetInitialItem( loadout_slot_knife_weapon_ct.GetInt() );

	ConVarRef loadout_slot_knife_weapon_t( "loadout_slot_knife_weapon_t" );
	if ( loadout_slot_knife_weapon_t.IsValid() )
		m_pLoadoutKnifeTComboBox->SetInitialItem( loadout_slot_knife_weapon_t.GetInt() );

	RemapKnivesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnApplyChanges()
{
	m_pLoadoutKnifeCTComboBox->ApplyChanges();
	m_pLoadoutKnifeTComboBox->ApplyChanges();
}
