#include "cbase.h"
#include "vgui_weapon_inspect_dialog.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>

using namespace vgui;

CWeaponInspectDialog* g_pWeaponInspect = nullptr;

CWeaponInspectDialog::CWeaponInspectDialog(Panel *parent) 
    : BaseClass(parent, "WeaponInspectDialog")
{
    g_pWeaponInspect = this;
    SetTitle("", true);
    SetSizeable(false);
    SetDeleteSelfOnClose(false);
    SetMoveable(false);
    
    int screenWidth, screenHeight;
    vgui::surface()->GetScreenSize(screenWidth, screenHeight);

    int w = static_cast<int>(screenWidth * 0.9f); 
    int h = static_cast<int>(screenHeight * 1.0f); 

    w = MAX(w, 800); 
    h = MAX(h, 600); 

    w = MIN(w, 2674); 
    h = MIN(h, 1220); 

    SetSize(w, h);
    SetMinimumSize(800, 600);
    
    m_CurrentWeaponID = WEAPON_NONE;
    m_iCurrentPaintKit = 0;
    m_pWeaponInfo = NULL;
    m_pSkinDef = NULL;
    
    // Создаем панель с 3D моделью оружия
    m_pWeaponModelPanel = new CWeaponModelPanel(this, "WeaponModel");
    
    // Лейбл для "Weapon Name | Skin Name" сверху по центру
    m_pTitleLabel = new Label(this, "TitleLabel", "");
    
    m_pRarityBar = new Panel(this, "RaityBar");
    
    // Кнопка Close справа снизу
    m_pCloseButton = new Button(this, "CloseButton", "Close", this, "close");
    
    InvalidateLayout(true, true);
    MoveToCenterOfScreen();
}

CWeaponInspectDialog::~CWeaponInspectDialog()
{
}

void CWeaponInspectDialog::ShowWeaponWithSkin(CSWeaponID weaponID, int iPaintKit, const char *pszWeaponName, const char *pszSkinName)
{
    m_CurrentWeaponID = weaponID;
    m_iCurrentPaintKit = iPaintKit;
    
    // Формируем заголовок "Weapon Name | Skin Name"
    char szTitle[256];
    Q_snprintf(szTitle, sizeof(szTitle), "%s | %s", pszWeaponName, pszSkinName);
    m_pTitleLabel->SetText(szTitle);
    
    // Настраиваем 3D панель с оружием
    CUtlVector<const char*> weaponList;
    
    const char *pszModel = GetModelPatchFormWeaponID(weaponID);
    const char *pszSeq;
    
    weaponList.AddToTail(pszModel);
    m_pWeaponModelPanel->SetWeaponList(weaponList);
    m_pWeaponModelPanel->SetCurrentWeapon(0);
    m_pWeaponModelPanel->m_vecWeaponOffset.Init( 0, 0, 0 );
    m_pWeaponModelPanel->m_Camera.m_origin.Init( 75.0f, 0.0f, 0.0f );
    m_pWeaponModelPanel->PlaySequence(pszSeq);
    
    if (iPaintKit > 0)
    {
        IMaterial* pMaterial = g_SkinDatabase.GetSkinMaterial( iPaintKit );
        
        m_pWeaponModelPanel->SetMaterialOverride( pMaterial );
    }
    
    if (m_pRarityBar)
    {
        m_pRarityBar->SetBgColor(GetRarityColor());
    }
    
    // Настраиваем камеру для лучшего вида
    m_pWeaponModelPanel->SetAutoRotate(true);
    m_pWeaponModelPanel->SetRotationSpeed(30.0f);
    
    Activate();
    SetVisible(true);
    SetSizeable(false);
    MoveToCenterOfScreen();
    MoveToFront();
}

void CWeaponInspectDialog::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    
    SetBgColor(pScheme->GetColor("Frame.BgColor", Color(40, 40, 40, 255)));
    
    if (m_pTitleLabel)
    {
        m_pTitleLabel->SetFont(pScheme->GetFont("DefaultLarge", true));
        m_pTitleLabel->SetFgColor(Color(255, 255, 255, 255));
    }
    
    if (m_pRarityBar)
    {
        m_pRarityBar->SetBgColor(GetRarityColor());
    }
}

Color CWeaponInspectDialog::GetRarityColor() const
{
    if (!m_pSkinDef)
        return Color(100, 100, 100, 255);
    
    switch (m_pSkinDef->rarity)
    {
        case SKIN_RARITY_COMMON:      return Color(176, 195, 217, 255);
        case SKIN_RARITY_UNCOMMON:    return Color(94, 152, 217, 255);
        case SKIN_RARITY_RARE:        return Color(75, 105, 255, 255);
        case SKIN_RARITY_MYTHICAL:    return Color(136, 71, 255, 255);
        case SKIN_RARITY_LEGENDARY:   return Color(211, 44, 230, 255);
        case SKIN_RARITY_ANCIENT:     return Color(235, 75, 75, 255);
        case SKIN_RARITY_CONTRABAND:  return Color(228, 174, 57, 255);
        default:                      return Color(100, 100, 100, 255);
    }
}

void CWeaponInspectDialog::PerformLayout()
{
    BaseClass::PerformLayout();
    
    int wide, tall;
    GetSize(wide, tall);
    
    int padding = 15;
    int buttonWidth = 100;
    int buttonHeight = 35;
    
    // Заголовок сверху по центру
    m_pTitleLabel->SetBounds(padding, padding, wide - padding * 2, 30);
    m_pTitleLabel->SetContentAlignment(Label::a_center);

    int barWidth = wide / 2;
    int barHeight = 8;
    int barX = (wide - barWidth) / 2;
    int barY = padding + 35;
    
    if (m_pRarityBar)
    {
        m_pRarityBar->SetBgColor(GetRarityColor());
        m_pRarityBar->SetBounds(barX, barY, barWidth, barHeight);
    }
    
    // 3D модель в середине диалога
    int modelTop = padding + 40;
    int modelHeight = tall - modelTop - buttonHeight - padding * 3;
    m_pWeaponModelPanel->SetBounds(padding, modelTop, wide - padding * 2, modelHeight);
    
    int buttonY = tall - buttonHeight - padding;
    int buttonX = wide - buttonWidth - padding;
    
    m_pCloseButton->SetBounds(buttonX, buttonY, buttonWidth, buttonHeight);
}

void CWeaponInspectDialog::OnCommand(const char *command)
{
    if (Q_stricmp(command, "close") == 0)
    {
        OnClose();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void CWeaponInspectDialog::OnClose()
{
    SetVisible(false);
    BaseClass::OnClose();
}

const char* CWeaponInspectDialog::GetModelPatchFormWeaponID(CSWeaponID weaponID)
{
    switch (weaponID)
    {
        case WEAPON_AK47:       return "models/weapons/w_rif_ak47.mdl";
        case WEAPON_M4A4:       return "models/weapons/w_rif_m4a4.mdl";
        case WEAPON_M4A1:       return "models/weapons/w_rif_m4a1_silencer.mdl";
        case WEAPON_AWP:        return "models/weapons/w_snip_awp.mdl";
        case WEAPON_DEAGLE:     return "models/weapons/w_pist_deagle.mdl";
        case WEAPON_REVOLVER:   return "models/weapons/w_pist_revolver.mdl";
        case WEAPON_GLOCK:      return "models/weapons/w_pist_glock18.mdl";
        case WEAPON_USP:        return "models/weapons/w_pist_usp.mdl";
        case WEAPON_HKP2000:    return "models/weapons/w_pist_hkp2000.mdl";
        case WEAPON_P250:       return "models/weapons/w_pist_p250.mdl";
        case WEAPON_FIVESEVEN:  return "models/weapons/w_pist_fiveseven.mdl";
        case WEAPON_TEC9:       return "models/weapons/w_psit_tec9.mdl";
        case WEAPON_CZ75A:      return "models/weapons/w_pist_cz75a.mdl";
        case WEAPON_ELITE:      return "models/weapons/w_pist_elite.mdl";
        case WEAPON_MAG7:       return "models/weapons/w_shot_mag7.mdl";
        case WEAPON_NOVA:       return "models/weapons/w_shot_nove.mdl";
        case WEAPON_SAWEDOFF:   return "models/weapons/w_shot_sawedoff.mdl";
        case WEAPON_XM1014:     return "models/weapons/w_shot_xm1014.mdl";
        case WEAPON_M249:       return "models/weapons/w_mach_m249.mdl";
        case WEAPON_NEGEV:      return "models/weapons/w_mach_negev.mdl";
        case WEAPON_MAC10:      return "models/weapons/w_smg_mac10.mdl";
        case WEAPON_MP9:        return "models/weapons/w_smg_mp9.mdl";
        case WEAPON_MP7:        return "models/weapons/w_smg_mp7.mdl";
        case WEAPON_MP5SD:      return "models/weapons/w_smg_mp5sd.mdl";
        case WEAPON_UMP45:      return "models/weapons/w_smg_ump45.mdl";
        case WEAPON_P90:        return "models/weapons/w_smg_p90.mdl";
        case WEAPON_BIZON:      return "models/weapons/w_smg_bizon.mdl";
        case WEAPON_GALILAR:    return "models/weapons/w_rif_galilar.mdl";
        case WEAPON_FAMAS:      return "models/weapons/w_rif_famas.mdl";
        case WEAPON_AUG:        return "models/weapons/w_rif_aug.mdl";
        case WEAPON_SG556:      return "models/weapons/w_rif_sg556.mdl";
        case WEAPON_SSG08:      return "models/weapons/w_snip_ssg08.mdl";
        case WEAPON_SCAR20:     return "models/weapons/w_snip_scar20.mdl";
        case WEAPON_G3SG1:      return "models/weapons/w_snip_g3sg1.mdl";
        default:                return nullptr;
    }
}

CON_COMMAND(open_weaponInspect, "Open skin inventory")
{
    if (!g_pWeaponInspect)
    {
        g_pWeaponInspect = new CWeaponInspectDialog(nullptr);
    }
    
    g_pWeaponInspect->Activate();
    g_pWeaponInspect->SetVisible(true);
    
    // Панель инвентаря сама загрузит скины через OnThink когда получит размеры
    DevMsg("[SkinEditor] Inventory window opened\n");
}

CON_COMMAND(close_weaponInspect, "Close skin inventory")
{
    if (g_pWeaponInspect)
    {
        g_pWeaponInspect->OnClose();
    }
}