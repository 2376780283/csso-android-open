#ifndef VGUI_WEAPON_INSPECT_DIALOG_H
#define VGUI_WEAPON_INSPECT_DIALOG_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "vgui_weaponinspect_modelpanel.h"
#include "engine/ivmodelinfo.h"
#include "cs_skin_database.h"
#include "cs_weapon_parse.h"

class CModelPanelModelAnimation
{
public:
	CModelPanelModelAnimation()
	{
		m_pszName = NULL;
		m_pszSequence = NULL;
		m_pszActivity = NULL;
		m_pPoseParameters = NULL;
		m_bDefault = false;
	}

	~CModelPanelModelAnimation()
	{
		if ( m_pszName && m_pszName[0] )
		{
			delete [] m_pszName;
			m_pszName = NULL;
		}

		if ( m_pszSequence && m_pszSequence[0] )
		{
			delete [] m_pszSequence;
			m_pszSequence = NULL;
		}

		if ( m_pszActivity && m_pszActivity[0] )
		{
			delete [] m_pszActivity;
			m_pszActivity = NULL;
		}

		if ( m_pPoseParameters )
		{
			m_pPoseParameters->deleteThis();
			m_pPoseParameters = NULL;
		}
	}

public:
	const char	*m_pszName;
	const char	*m_pszSequence;
	const char	*m_pszActivity;
	KeyValues	*m_pPoseParameters;
	bool		m_bDefault;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanelAttachedModelInfo
{
public:
	CModelPanelAttachedModelInfo()
	{
		m_pszModelName = NULL;
		m_nSkin = 0;
	}

	~CModelPanelAttachedModelInfo()
	{
		if ( m_pszModelName && m_pszModelName[0] )
		{
			delete [] m_pszModelName;
			m_pszModelName = NULL;
		}
	}

public:
	const char	*m_pszModelName;
	int			m_nSkin;
};

class CModelPanelModelInfo
{
public:
	CModelPanelModelInfo()
	{
		m_pszModelName = NULL;
		m_pszModelName_HWM = NULL;
		m_nSkin = -1;
		m_vecAbsAngles.Init();
		m_vecOriginOffset.Init();
		m_vecFramedOriginOffset.Init();
		m_bUseSpotlight = false;
	}

	~CModelPanelModelInfo()
	{
		if ( m_pszModelName && m_pszModelName[0] )
		{
			delete [] m_pszModelName;
			m_pszModelName = NULL;
		}

		if ( m_pszModelName_HWM && m_pszModelName_HWM[0] )
		{
			delete [] m_pszModelName_HWM;
			m_pszModelName_HWM = NULL;
		}

		if ( m_pszVCD && m_pszVCD[0] )
		{
			delete [] m_pszVCD;
			m_pszVCD = NULL;
		}

		m_Animations.PurgeAndDeleteElements();
		m_AttachedModelsInfo.PurgeAndDeleteElements();
	}

public:
	const char	*m_pszModelName;
	const char	*m_pszModelName_HWM;
	int			m_nSkin;
	const char	*m_pszVCD;
	Vector		m_vecAbsAngles;
	Vector		m_vecOriginOffset;
	Vector2D	m_vecViewportOffset;
	Vector		m_vecFramedOriginOffset;
	bool		m_bUseSpotlight;

	CUtlVector<CModelPanelModelAnimation*>		m_Animations;
	CUtlVector<CModelPanelAttachedModelInfo*>	m_AttachedModelsInfo;
};

class CWeaponInspectDialog : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CWeaponInspectDialog, vgui::Frame);

public:
    CWeaponInspectDialog(vgui::Panel *parent);
    virtual ~CWeaponInspectDialog();
    
    void ShowWeaponWithSkin(CSWeaponID weaponID, int iPaintKit, const char *pszWeaponName, const char *pszSkinName);
    
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    virtual void PerformLayout() override;
    virtual void OnCommand(const char *command) override;
    virtual void OnClose() override;
    const char* GetModelPatchFormWeaponID(CSWeaponID weaponID);
    const SkinDefinition_t* GetSkinDefinition() const { return m_pSkinDef; }
    Color GetRarityColor() const;

private:
    CWeaponModelPanel *m_pWeaponModelPanel;
    Vector m_vecModelCenter;
    const SkinDefinition_t *m_pSkinDef;
    vgui::Label *m_pTitleLabel;  // "Weapon Name | Skin Name"
    vgui::Button *m_pCloseButton;
    vgui::Panel *m_pRarityBar;
    
    CSWeaponID m_CurrentWeaponID;
    int m_iCurrentPaintKit;
    CCSWeaponInfo* m_pWeaponInfo;
};

extern CWeaponInspectDialog* g_pWeaponInspect;

#endif // VGUI_WEAPON_INSPECT_DIALOG_H
