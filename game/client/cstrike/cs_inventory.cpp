#include "cbase.h"
#include "cs_inventory.h"
#include "vgui_controls/ScrollableEditablePanel.h"
#include "cs_skin_database.h"
#include "cs_shareddefs.h"
#include <vgui_controls/Image.h>
#include <vgui/ISurface.h>
#include "convar.h"

using namespace vgui;

#define EQUIP_TEAM_NONE  0
#define EQUIP_TEAM_CT    (1 << 0)
#define EQUIP_TEAM_T     (1 << 1)
#define EQUIP_TEAM_BOTH  (EQUIP_TEAM_CT | EQUIP_TEAM_T)

static const char *g_AgentIconsCT[] = {
    "ct_none", "ctm_fbi_variantf", "ctm_fbi_variantf_legacy", "ctm_fbi_variantg", "ctm_fbi_varianth",
    "ctm_fbi_variantb", "ctm_sas_variantf", "ctm_st6_variantk", "ctm_st6_variantk_legacy", "ctm_st6_variante",
    "ctm_st6_variante_legacy", "ctm_st6_variantg", "ctm_st6_variantm", "ctm_st6_variantm_legacy", "ctm_st6_varianti",
    "ctm_st6_varianti_legacy", "ctm_swat_variantj", "ctm_swat_varianth", "ctm_st6_variantj", "ctm_swat_variantg",
    "ctm_swat_varianti", "ctm_swat_variantf", "ctm_st6_variantl", "ctm_swat_variante", "ctm_diver_varianta",
    "ctm_diver_variantb", "ctm_diver_variantc", "ctm_gendarmerie_varianta", "ctm_gendarmerie_variantb", "ctm_gendarmerie_variantc",
    "ctm_gendarmerie_variantd", "ctm_gendarmerie_variante", "ctm_sas_variantg", "ctm_st6_variantn", "ctm_swat_variantk",
    "ctm_sas_old", "ctm_fbi_old", "ctm_jumpsuit_varianta", "ctm_jumpsuit_variantb", "ctm_jumpsuit_variantc"
};

static const char *g_AgentIconsT[] = {
    "t_none", "tm_leet_variantg", "tm_leet_variantg_legacy", "tm_leet_varianth", "tm_leet_varianti",
    "tm_leet_varianti_legacy", "tm_leet_variantf", "tm_phoenix_varianth", "tm_phoenix_variantf", "tm_phoenix_variantf_legacy",
    "tm_phoenix_variantg", "tm_balkan_variantf", "tm_balkan_varianti", "tm_balkan_variantg", "tm_balkan_variantj",
    "tm_balkan_varianth", "tm_balkan_variantl", "tm_phoenix_varianti", "tm_professional_varj", "tm_professional_varh",
    "tm_balkan_variantk", "tm_professional_varg", "tm_professional_vari", "tm_professional_varf", "tm_professional_varf1",
    "tm_professional_varf2", "tm_professional_varf3", "tm_professional_varf4", "tm_jungle_raider_varianta", "tm_jungle_raider_variantb",
    "tm_jungle_raider_variantb2", "tm_jungle_raider_variantc", "tm_jungle_raider_variantd", "tm_jungle_raider_variante", "tm_jungle_raider_variantf",
    "tm_jungle_raider_variantf2", "tm_leet_variantj", "tm_professional_varf5", "tm_phoenix_old", "tm_leet_old",
    "tm_jumpsuit_varianta", "tm_jumpsuit_variantb", "tm_jumpsuit_variantc"
};

static const char *g_MusicKitIcons[] = {
    "valve_csgo_01", // the default one should be on top
	"valve_csgo_02", // the default one should be on top
    "valve_cs2_01",
    "adambeyer_01",
	"amontobin_01",
    "austinwintory_04",
	"austinwintory_01",
	"austinwintory_02",
	"austinwintory_03",
	"awolnation_01",
	"bbnos_01",
	"beartooth_01",
	"beartooth_02",
    "benbromfield_01",
	"blitzkids_01",
	"chipzel_01",
	"damjanmravunac_01",
	"danielsadowski_01",
	"danielsadowski_02",
	"danielsadowski_03",
	"danielsadowski_04",
    "danielsadowski_05",
	"darude_01",
	"dren_01",
	"dren_02",
    "dren_03",
    "dryden_01",
	"feedme_01",
	"freakydna_01",
	"hades_01",
	"halflife_alyx_01",
	"halo_01",
	"hotlinemiami_01",
	"hundredth_01",
	"ianhultquist_01",
    "isoxo_01",
    "jamesandthecoldgun_01",
	"jesseharlin_01",
    "jonathanyoung_01",
    "juelz_02",
	"kellybailey_01",
	"kitheory_01",
    "killermike_01",
    "killscript_01",
    "knock2_02",
	"laurashigihara_01",
	"lenniemoore_01",
	"mateomessina_01",
	"mattlange_01",
	"mattlevine_01",
    "mattlevine_02",
	"michaelbross_01",
	"midnightriders_01",
	"mordfustang_01",
	"neckdeep_01",
	"neckdeep_02",
	"newbeatfund_01",
	"noisia_01",
    "perfectworld_01",
    "perfectworld_02",
	"proxy_01",
    "pvris_01",
    "radcat_01",
	"roam_01",
	"robertallaire_01",
	"sammarshall_01",
	"sarahschachner_01",
	"sasha_01",
	"scarlxrd_01",
	"scarlxrd_02",
    "sammarshall_02",
	"seanmurray_01",
    "selectiveresponse_01",
	"skog_01",
	"skog_02",
	"skog_03",
	"theverkkars_01",
	"theverkkars_02",
	"timhuling_01",
	"treeadams_benbromfield_01",
	"troelsfolmann_01",
    "tigercub_01",
    "twerl_01",
	"twinatlantic_01"
};

static const char *g_MusicKitNames[MAX_MUSIC] = {
    "valve_csgo_01", // the default one should be on top
	"valve_csgo_02", // the default one should be on top
    "valve_cs2_01",
    "adambeyer_01",
	"amontobin_01",
    "austinwintory_04",
	"austinwintory_01",
	"austinwintory_02",
	"austinwintory_03",
	"awolnation_01",
	"bbnos_01",
	"beartooth_01",
	"beartooth_02",
    "benbromfield_01",
	"blitzkids_01",
	"chipzel_01",
	"damjanmravunac_01",
	"danielsadowski_01",
	"danielsadowski_02",
	"danielsadowski_03",
	"danielsadowski_04",
    "danielsadowski_05",
	"darude_01",
	"dren_01",
	"dren_02",
    "dren_03",
    "dryden_01",
	"feedme_01",
	"freakydna_01",
	"hades_01",
	"halflife_alyx_01",
	"halo_01",
	"hotlinemiami_01",
	"hundredth_01",
	"ianhultquist_01",
    "isoxo_01",
    "jamesandthecoldgun_01",
	"jesseharlin_01",
    "jonathanyoung_01",
    "juelz_02",
	"kellybailey_01",
	"kitheory_01",
    "killermike_01",
    "killscript_01",
    "knock2_02",
	"laurashigihara_01",
	"lenniemoore_01",
	"mateomessina_01",
	"mattlange_01",
	"mattlevine_01",
    "mattlevine_02",
	"michaelbross_01",
	"midnightriders_01",
	"mordfustang_01",
	"neckdeep_01",
	"neckdeep_02",
	"newbeatfund_01",
	"noisia_01",
    "perfectworld_01",
    "perfectworld_02",
	"proxy_01",
    "pvris_01",
    "radcat_01",
	"roam_01",
	"robertallaire_01",
	"sammarshall_01",
	"sarahschachner_01",
	"sasha_01",
	"scarlxrd_01",
	"scarlxrd_02",
    "sammarshall_02",
	"seanmurray_01",
    "selectiveresponse_01",
	"skog_01",
	"skog_02",
	"skog_03",
	"theverkkars_01",
	"theverkkars_02",
	"timhuling_01",
	"treeadams_benbromfield_01",
	"troelsfolmann_01",
    "tigercub_01",
    "twerl_01",
	"twinatlantic_01"
};

static const char *g_AgentNamesCT[MAX_AGENTS_CT + 1] = 
{
    "#GameUI_Loadout_Agent_None",
	 "#GameUI_Loadout_Agent_ctm_fbi_variantf",
	 "#GameUI_Loadout_Agent_ctm_fbi_variantf_legacy",
	 "#GameUI_Loadout_Agent_ctm_fbi_variantg",
	 "#GameUI_Loadout_Agent_ctm_fbi_varianth",
	 "#GameUI_Loadout_Agent_ctm_fbi_variantb",
	 "#GameUI_Loadout_Agent_ctm_sas_variantf",
	 "#GameUI_Loadout_Agent_ctm_st6_variantk",
	 "#GameUI_Loadout_Agent_ctm_st6_variantk_legacy",
	 "#GameUI_Loadout_Agent_ctm_st6_variante",
	 "#GameUI_Loadout_Agent_ctm_st6_variante_legacy",
	 "#GameUI_Loadout_Agent_ctm_st6_variantg",
	 "#GameUI_Loadout_Agent_ctm_st6_variantm",
	 "#GameUI_Loadout_Agent_ctm_st6_variantm_legacy",
	 "#GameUI_Loadout_Agent_ctm_st6_varianti",
	 "#GameUI_Loadout_Agent_ctm_st6_varianti_legacy",
	 "#GameUI_Loadout_Agent_ctm_swat_variantj",
	 "#GameUI_Loadout_Agent_ctm_swat_varianth",
	 "#GameUI_Loadout_Agent_ctm_st6_variantj",
	 "#GameUI_Loadout_Agent_ctm_swat_variantg",
	 "#GameUI_Loadout_Agent_ctm_swat_varianti",
	 "#GameUI_Loadout_Agent_ctm_swat_variantf",
	 "#GameUI_Loadout_Agent_ctm_st6_variantl",
	 "#GameUI_Loadout_Agent_ctm_swat_variante",
	 "#GameUI_Loadout_Agent_ctm_diver_varianta",
	 "#GameUI_Loadout_Agent_ctm_diver_variantb",
	 "#GameUI_Loadout_Agent_ctm_diver_variantc",
	 "#GameUI_Loadout_Agent_ctm_gendarmerie_varianta",
	 "#GameUI_Loadout_Agent_ctm_gendarmerie_variantb",
	 "#GameUI_Loadout_Agent_ctm_gendarmerie_variantc",
	 "#GameUI_Loadout_Agent_ctm_gendarmerie_variantd",
	 "#GameUI_Loadout_Agent_ctm_gendarmerie_variante",
	 "#GameUI_Loadout_Agent_ctm_sas_variantg",
	 "#GameUI_Loadout_Agent_ctm_st6_variantn",
	 "#GameUI_Loadout_Agent_ctm_swat_variantk",
	 "#GameUI_Loadout_Agent_ctm_sas_old",
	 "#GameUI_Loadout_Agent_ctm_fbi_old",
	 "#GameUI_Loadout_Agent_ctm_jumpsuit_varianta",
	 "#GameUI_Loadout_Agent_ctm_jumpsuit_variantb",
	 "#GameUI_Loadout_Agent_ctm_jumpsuit_variantc",
};

static const char *g_AgentNamesT[MAX_AGENTS_T + 1] = 
{
    "#GameUI_Loadout_Agent_None",
    "#GameUI_Loadout_Agent_tm_leet_variantg",
    "#GameUI_Loadout_Agent_tm_leet_variantg_legacy",
    "#GameUI_Loadout_Agent_tm_leet_varianth",
    "#GameUI_Loadout_Agent_tm_leet_varianti",
    "#GameUI_Loadout_Agent_tm_leet_varianti_legacy",
    "#GameUI_Loadout_Agent_tm_leet_variantf",
    "#GameUI_Loadout_Agent_tm_phoenix_varianth",
    "#GameUI_Loadout_Agent_tm_phoenix_variantf",
    "#GameUI_Loadout_Agent_tm_phoenix_variantf_legacy",
    "#GameUI_Loadout_Agent_tm_phoenix_variantg",
    "#GameUI_Loadout_Agent_tm_balkan_variantf",
    "#GameUI_Loadout_Agent_tm_balkan_varianti",
    "#GameUI_Loadout_Agent_tm_balkan_variantg",
    "#GameUI_Loadout_Agent_tm_balkan_variantj",
    "#GameUI_Loadout_Agent_tm_balkan_varianth",
    "#GameUI_Loadout_Agent_tm_balkan_variantl",
    "#GameUI_Loadout_Agent_tm_phoenix_varianti",
    "#GameUI_Loadout_Agent_tm_professional_varj",
    "#GameUI_Loadout_Agent_tm_professional_varh",
    "#GameUI_Loadout_Agent_tm_balkan_variantk",
    "#GameUI_Loadout_Agent_tm_professional_varg",
    "#GameUI_Loadout_Agent_tm_professional_vari",
    "#GameUI_Loadout_Agent_tm_professional_varf",
    "#GameUI_Loadout_Agent_tm_professional_varf1",
    "#GameUI_Loadout_Agent_tm_professional_varf2",
    "#GameUI_Loadout_Agent_tm_professional_varf3",
    "#GameUI_Loadout_Agent_tm_professional_varf4",
    "#GameUI_Loadout_Agent_tm_jungle_raider_varianta",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variantb",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variantb2",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variantc",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variantd",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variante",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variantf",
    "#GameUI_Loadout_Agent_tm_jungle_raider_variantf2",
    "#GameUI_Loadout_Agent_tm_leet_variantj",
    "#GameUI_Loadout_Agent_tm_professional_varf5",
    "#GameUI_Loadout_Agent_tm_phoenix_old",
    "#GameUI_Loadout_Agent_tm_leet_old",
    "#GameUI_Loadout_Agent_tm_jumpsuit_varianta",
    "#GameUI_Loadout_Agent_tm_jumpsuit_variantb",
    "#GameUI_Loadout_Agent_tm_jumpsuit_variantc",
};

class CInventoryItemContainer : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CInventoryItemContainer, vgui::EditablePanel);
    public:
    CInventoryItemContainer(Panel *parent, const char *name) : BaseClass(parent, name) {}
    virtual void PerformLayout() override {}
    virtual void InvalidateLayout() {}
};

CInventoryItemPanel::CInventoryItemPanel(Panel *parent, const char *panelName, const SkinDefinition_t *pSkinDef, 
                                        InventoryItemType_t itemType, int iSpecialIndex, const char *pszIconPath, const char *pszCustomName)
    : BaseClass(parent, panelName)
{
    m_pSkinDef = pSkinDef;
    m_ItemType = itemType;
    m_iSpecialIndex = iSpecialIndex;
    m_bMouseOver = false;
    m_iEquipFlags = EQUIP_TEAM_NONE;
    
    if (pszIconPath)
        Q_strncpy(m_szCustomIconPath, pszIconPath, sizeof(m_szCustomIconPath));
    else
        m_szCustomIconPath[0] = '\0';
    
    if (pszCustomName)
        Q_strncpy(m_szCustomName, pszCustomName, sizeof(m_szCustomName));
    else
        m_szCustomName[0] = '\0';
            
    m_pExternalNameLabel = nullptr;
    m_pExternalModelLabel = nullptr;
    m_pExternalSkinLabel = nullptr;
    
    m_pWeaponLabel = new vgui::Label(this, "WeaponLabel", "");
    m_pWeaponLabel->SetMouseInputEnabled(false);
    
    m_pIconImage = new vgui::ImagePanel(this, "IconImage");
    m_pIconImage->SetMouseInputEnabled(false);
    m_pIconImage->SetShouldScaleImage(true);
    
    m_pRarityBar = new vgui::Panel(this, "RarityBar");
    m_pRarityBar->SetMouseInputEnabled(false);
    
    SetMouseInputEnabled(true);
    
    UpdateDisplay();
    UpdateEquipIndicators();
}

void CInventoryItemPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetBorder(pScheme->GetBorder("ButtonBorder"));
    SetBgColor(Color(0, 0, 0, 0));
    m_pRarityBar->SetBgColor(GetRarityColor());
    m_pWeaponLabel->SetFont(pScheme->GetFont("Default", true));
    m_pWeaponLabel->SetFgColor(Color(200, 200, 200, 255));
}

void CInventoryItemPanel::PerformLayout()
{
    BaseClass::PerformLayout();
    
    int wide, tall;
    GetSize(wide, tall);
    m_pRarityBar->SetBounds(0, 0, 8, tall);
           
    IImage *pImg = m_pIconImage->GetImage();
    if (pImg)
    {
        int texW, texH;
        pImg->GetSize(texW, texH);

        if (texW > 0 && texH > 0)
        {
            float scale = 1.0f;

            float scaleX = (float)wide / texW;
            float scaleY = (float)tall / texH;
            
            if (scaleX < 1.0f || scaleY < 1.0f)
            {
                scale = std::min(scaleX, scaleY);
            }

            int drawW = (int)(texW * scale + 0.5f);
            int drawH = (int)(texH * scale + 0.5f);

            int x = (wide - drawW) / 2;
            int y = (tall - drawH) / 2;

            m_pIconImage->SetBounds(x, y, drawW, drawH);
        }
    }
        
    int textStartY = tall - 30;
    m_pWeaponLabel->SetBounds(12, textStartY, wide - 24, 26);
    m_pWeaponLabel->SetContentAlignment(vgui::Label::a_center);
}

void CInventoryItemPanel::Paint()
{
    BaseClass::Paint();
    
    if (m_iEquipFlags != EQUIP_TEAM_NONE)
    {
        int wide, tall;
        GetSize(wide, tall);
        
        int indicatorSize = 12;
        int indicatorSpacing = 4;
        int indicatorY = 8;
        int indicatorStartX = wide - indicatorSize - 8;
        
        if (m_iEquipFlags & EQUIP_TEAM_CT)
        {
            vgui::surface()->DrawSetColor(Color(90, 140, 200, 255));
            vgui::surface()->DrawFilledRect(indicatorStartX, indicatorY, indicatorStartX + indicatorSize, indicatorY + indicatorSize);
            vgui::surface()->DrawSetColor(Color(255, 255, 255, 200));
            vgui::surface()->DrawOutlinedRect(indicatorStartX, indicatorY, indicatorStartX + indicatorSize, indicatorY + indicatorSize);
            indicatorStartX -= (indicatorSize + indicatorSpacing);
        }
        
        if (m_iEquipFlags & EQUIP_TEAM_T)
        {
            vgui::surface()->DrawSetColor(Color(200, 150, 100, 255));
            vgui::surface()->DrawFilledRect(indicatorStartX, indicatorY, indicatorStartX + indicatorSize, indicatorY + indicatorSize);
            vgui::surface()->DrawSetColor(Color(255, 255, 255, 200));
            vgui::surface()->DrawOutlinedRect(indicatorStartX, indicatorY, indicatorStartX + indicatorSize, indicatorY + indicatorSize);
        }
    }
}

void CInventoryItemPanel::OnMousePressed(vgui::MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        int cursorX, cursorY;
        vgui::input()->GetCursorPos(cursorX, cursorY);
        
        KeyValues *pKV = new KeyValues("ItemSelected");
        pKV->SetInt("paintkit", m_pSkinDef ? m_pSkinDef->iPaintKit : 0);
        pKV->SetInt("weaponid", m_pSkinDef ? m_pSkinDef->weaponID : WEAPON_NONE);
        pKV->SetInt("itemtype", m_ItemType);
        pKV->SetInt("specialindex", m_iSpecialIndex);
        pKV->SetInt("cursorx", cursorX);
        pKV->SetInt("cursory", cursorY);
        PostActionSignal(pKV);
    }
    BaseClass::OnMousePressed(code);
}

void CInventoryItemPanel::OnCursorEntered()
{
    m_bMouseOver = true;
    SetBgColor(Color(255, 255, 255, 60));
    BaseClass::OnCursorEntered();
}

void CInventoryItemPanel::OnCursorExited()
{
    m_bMouseOver = false;
    SetBgColor(Color(0, 0, 0, 0));
    BaseClass::OnCursorExited();
}

void CInventoryItemPanel::UpdateDisplay()
{
    if (m_ItemType == INVENTORY_ITEM_AGENT_CT || m_ItemType == INVENTORY_ITEM_AGENT_T)
    {
        m_pWeaponLabel->SetText("Agent");
        if (m_pExternalNameLabel && m_szCustomName[0])
            m_pExternalNameLabel->SetText(m_szCustomName);
        if (m_szCustomIconPath[0])
        {
            m_pIconImage->SetImage(m_szCustomIconPath);
            m_pIconImage->SetShouldScaleImage(true);
            m_pIconImage->SetVisible(true);
        }
        return;
    }
    
    if (m_ItemType == INVENTORY_ITEM_MUSIC)
    {
        m_pWeaponLabel->SetText("Music Kit");
        if (m_pExternalNameLabel && m_szCustomName[0])
            m_pExternalNameLabel->SetText(m_szCustomName);
        if (m_szCustomIconPath[0])
        {
            m_pIconImage->SetImage(m_szCustomIconPath);
            m_pIconImage->SetShouldScaleImage(true);
            m_pIconImage->SetVisible(true);
        }
        return;
    }
    
    if (m_ItemType == INVENTORY_ITEM_GLOVES)
    {
        m_pWeaponLabel->SetText("Gloves");
        if (m_pExternalNameLabel && m_szCustomName[0])
            m_pExternalNameLabel->SetText(m_szCustomName);
        if (m_szCustomIconPath[0])
        {
            m_pIconImage->SetImage(m_szCustomIconPath);
            m_pIconImage->SetShouldScaleImage(true);
            m_pIconImage->SetVisible(true);
        }
        return;
    }
    
    if (!m_pSkinDef)
        return;
    
    const char *pszWeaponName = WeaponIDToAlias(m_pSkinDef->weaponID);
    if (pszWeaponName && Q_strnicmp(pszWeaponName, "weapon_", 7) == 0)
        pszWeaponName += 7;
    
    char szWeaponDisplay[64];
    Q_snprintf(szWeaponDisplay, sizeof(szWeaponDisplay), "%s", pszWeaponName);
    
    for (int i = 0; szWeaponDisplay[i]; i++)
    {
        if (szWeaponDisplay[i] == '_')
            szWeaponDisplay[i] = ' ';
        if (i == 0 && szWeaponDisplay[i] >= 'a' && szWeaponDisplay[i] <= 'z')
            szWeaponDisplay[i] -= 32;
    }
    
    m_pWeaponLabel->SetText(szWeaponDisplay);
    
    if (m_pSkinDef->IsGloves())
    {
        if (m_pExternalNameLabel)
        {
            char szGloveModel[64];
            const char *szClass = m_pSkinDef->szItemClass;
            
            if (Q_stristr(szClass, "bloodhound")) Q_strncpy(szGloveModel, "Bloodhound", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "motorcycle")) Q_strncpy(szGloveModel, "Motorcycle", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "specialist")) Q_strncpy(szGloveModel, "Specialist", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "sporty")) Q_strncpy(szGloveModel, "Sporty", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "handwrap")) Q_strncpy(szGloveModel, "Hand Wraps", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "hydra")) Q_strncpy(szGloveModel, "Hydra", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "brokenfang")) Q_strncpy(szGloveModel, "Broken Fang", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "fingerless")) Q_strncpy(szGloveModel, "Fingerless", sizeof(szGloveModel));
            else if (Q_stristr(szClass, "slick")) Q_strncpy(szGloveModel, "Slick", sizeof(szGloveModel));
            else Q_strncpy(szGloveModel, "Gloves", sizeof(szGloveModel));
            
            m_pExternalNameLabel->SetText(szGloveModel);
        }
        if (m_pExternalSkinLabel)
            m_pExternalSkinLabel->SetText(m_pSkinDef->szName);
    }
    else if (m_pSkinDef->weaponID >= WEAPON_KNIFE && m_pSkinDef->weaponID <= WEAPON_KNIFE_LAST)
    {
        if (m_pExternalNameLabel)
        {
            const char *pszKnifeName = "";
            switch (m_pSkinDef->weaponID)
            {
                case WEAPON_KNIFE_KARAMBIT: pszKnifeName = "Karambit"; break;
                case WEAPON_KNIFE_FLIP: pszKnifeName = "Flip Knife"; break;
                case WEAPON_KNIFE_BAYONET: pszKnifeName = "Bayonet"; break;
                case WEAPON_KNIFE_M9_BAYONET: pszKnifeName = "M9 Bayonet"; break;
                case WEAPON_KNIFE_BUTTERFLY: pszKnifeName = "Butterfly Knife"; break;
                case WEAPON_KNIFE_GUT: pszKnifeName = "Gut Knife"; break;
                case WEAPON_KNIFE_TACTICAL: pszKnifeName = "Huntsman Knife"; break;
                case WEAPON_KNIFE_FALCHION: pszKnifeName = "Falchion Knife"; break;
                case WEAPON_KNIFE_SURVIVAL_BOWIE: pszKnifeName = "Bowie Knife"; break;
                case WEAPON_KNIFE_CANIS: pszKnifeName = "Survival Knife"; break;
                case WEAPON_KNIFE_SKELETON: pszKnifeName = "Skeleton Knife"; break;
                case WEAPON_KNIFE_STILETTO: pszKnifeName = "Stiletto Knife"; break;
                case WEAPON_KNIFE_URSUS: pszKnifeName = "Ursus Knife"; break;
                case WEAPON_KNIFE_WIDOWMAKER: pszKnifeName = "Talon Knife"; break;
                default: pszKnifeName = "Knife"; break;
            }
            m_pExternalNameLabel->SetText(pszKnifeName);
        }
        if (m_pExternalSkinLabel)
            m_pExternalSkinLabel->SetText(m_pSkinDef->szName);
    }
    else
    {
        if (m_pExternalNameLabel)
            m_pExternalNameLabel->SetText("");
        if (m_pExternalSkinLabel)
            m_pExternalSkinLabel->SetText(m_pSkinDef->szName);
    }
    
    if (m_pSkinDef->szIconPath[0])
    {
        m_pIconImage->SetImage(m_pSkinDef->szIconPath);
        m_pIconImage->SetShouldScaleImage(true);
        m_pIconImage->SetVisible(true);
    }
    else
    {
        m_pIconImage->SetVisible(false);
    }
}
int CInventoryItemPanel::GetEquipFlags() const
{
    int flags = EQUIP_TEAM_NONE;
    
    if (m_ItemType == INVENTORY_ITEM_AGENT_CT)
    {
        ConVar *pAgentSlot = cvar->FindVar("loadout_slot_agent_ct");
        if (pAgentSlot && pAgentSlot->GetInt() == m_iSpecialIndex)
            flags |= EQUIP_TEAM_CT;
    }
    else if (m_ItemType == INVENTORY_ITEM_AGENT_T)
    {
        ConVar *pAgentSlot = cvar->FindVar("loadout_slot_agent_t");
        if (pAgentSlot && pAgentSlot->GetInt() == m_iSpecialIndex)
            flags |= EQUIP_TEAM_T;
    }
    else if (m_ItemType == INVENTORY_ITEM_MUSIC)
    {
        ConVar *pMusicSlot = cvar->FindVar("loadout_music");
        if (pMusicSlot && pMusicSlot->GetInt() == m_iSpecialIndex)
            flags = EQUIP_TEAM_BOTH;
    }
    else if (m_pSkinDef && m_pSkinDef->weaponID >= WEAPON_KNIFE && m_pSkinDef->weaponID <= WEAPON_KNIFE_LAST)
    {
        const char *pszKnifeName = nullptr;
        switch (m_pSkinDef->weaponID)
        {
            case WEAPON_KNIFE_CSS: pszKnifeName = "knife_css"; break;
            case WEAPON_KNIFE_KARAMBIT: pszKnifeName = "knife_karambit"; break;
            case WEAPON_KNIFE_FLIP: pszKnifeName = "knife_flip"; break;
            case WEAPON_KNIFE_BAYONET: pszKnifeName = "knife_bayonet"; break;
            case WEAPON_KNIFE_M9_BAYONET: pszKnifeName = "knife_m9_bayonet"; break;
            case WEAPON_KNIFE_BUTTERFLY: pszKnifeName = "knife_butterfly"; break;
            case WEAPON_KNIFE_GUT: pszKnifeName = "knife_gut"; break;
            case WEAPON_KNIFE_TACTICAL: pszKnifeName = "knife_tactical"; break;
            case WEAPON_KNIFE_FALCHION: pszKnifeName = "knife_falchion"; break;
            case WEAPON_KNIFE_SURVIVAL_BOWIE: pszKnifeName = "knife_survival_bowie"; break;
            case WEAPON_KNIFE_CANIS: pszKnifeName = "knife_canis"; break;
            case WEAPON_KNIFE_CORD: pszKnifeName = "knife_cord"; break;
            case WEAPON_KNIFE_GYPSY: pszKnifeName = "knife_gypsy_jackknife"; break;
            case WEAPON_KNIFE_OUTDOOR: pszKnifeName = "knife_outdoor"; break;
            case WEAPON_KNIFE_SKELETON: pszKnifeName = "knife_skeleton"; break;
            case WEAPON_KNIFE_STILETTO: pszKnifeName = "knife_stiletto"; break;
            case WEAPON_KNIFE_URSUS: pszKnifeName = "knife_ursus"; break;
            case WEAPON_KNIFE_WIDOWMAKER: pszKnifeName = "knife_widowmaker"; break;
            case WEAPON_KNIFE_PUSH: pszKnifeName = "knife_push"; break;
        }
        
        if (pszKnifeName)
        {
            char szConVarName[64];
            Q_snprintf(szConVarName, sizeof(szConVarName), "loadout_skin_%s", pszKnifeName);
            ConVar *pSkinConVar = cvar->FindVar(szConVarName);
            ConVar *pKnifeSlotCT = cvar->FindVar("loadout_slot_knife_weapon_ct");
            ConVar *pKnifeSlotT = cvar->FindVar("loadout_slot_knife_weapon_t");
            
            if (pSkinConVar && pSkinConVar->GetInt() == m_pSkinDef->iPaintKit)
            {
                if (pKnifeSlotCT && pKnifeSlotCT->GetInt() > 0)
                    flags |= EQUIP_TEAM_CT;
                if (pKnifeSlotT && pKnifeSlotT->GetInt() > 0)
                    flags |= EQUIP_TEAM_T;
            }
        }
    }
    else if (m_ItemType == INVENTORY_ITEM_GLOVES && m_iSpecialIndex > 0)
    {
        ConVar *pSlotCT = cvar->FindVar("loadout_slot_gloves_ct");
        if (pSlotCT && pSlotCT->GetInt() == m_iSpecialIndex)
        {
            char szConVarName[64];
            Q_snprintf(szConVarName, sizeof(szConVarName), "loadout_gloves_ct_skin%d", m_iSpecialIndex);
            ConVar *pSkinCT = cvar->FindVar(szConVarName);
            if (pSkinCT && pSkinCT->GetInt() == (m_pSkinDef ? m_pSkinDef->iPaintKit : 0))
                flags |= EQUIP_TEAM_CT;
        }
        
        ConVar *pSlotT = cvar->FindVar("loadout_slot_gloves_t");
        if (pSlotT && pSlotT->GetInt() == m_iSpecialIndex)
        {
            char szConVarName[64];
            Q_snprintf(szConVarName, sizeof(szConVarName), "loadout_gloves_t_skin%d", m_iSpecialIndex);
            ConVar *pSkinT = cvar->FindVar(szConVarName);
            if (pSkinT && pSkinT->GetInt() == (m_pSkinDef ? m_pSkinDef->iPaintKit : 0))
                flags |= EQUIP_TEAM_T;
        }
    }
    else if (m_pSkinDef && m_pSkinDef->IsGloves())
    {
        int gloveIndex = -1;
        
        const char* szClass = m_pSkinDef->szItemClass;
        if (Q_stricmp(szClass, "default") == 0) gloveIndex = 0;
        else if (Q_stristr(szClass, "bloodhound") && !Q_stristr(szClass, "perfectworld") && !Q_stristr(szClass, "brokenfang") && !Q_stristr(szClass, "hydra")) gloveIndex = 1;
        else if (Q_stristr(szClass, "bloodhound") && Q_stristr(szClass, "perfectworld")) gloveIndex = 2;
        else if (Q_stristr(szClass, "brokenfang") || Q_stristr(szClass, "broken_fang")) gloveIndex = 3;
        else if (Q_stristr(szClass, "hydra")) gloveIndex = 4;
        else if (Q_stristr(szClass, "fingerless")) gloveIndex = 5;
        else if (Q_stristr(szClass, "fullfinger")) gloveIndex = 6;
        else if (Q_stristr(szClass, "handwrap") || Q_stristr(szClass, "leathery")) gloveIndex = 7;
        else if (Q_stristr(szClass, "hardknuckle") && Q_stristr(szClass, "black")) gloveIndex = 9;
        else if (Q_stristr(szClass, "hardknuckle") && Q_stristr(szClass, "blue")) gloveIndex = 10;
        else if (Q_stristr(szClass, "hardknuckle")) gloveIndex = 8;
        else if (Q_stristr(szClass, "motorcycle")) gloveIndex = 11;
        else if (Q_stristr(szClass, "slick")) gloveIndex = 12;
        else if (Q_stristr(szClass, "specialist")) gloveIndex = 13;
        else if (Q_stristr(szClass, "sporty")) gloveIndex = 14;
        else if (Q_stristr(szClass, "sas") && Q_stristr(szClass, "old")) gloveIndex = 15;
        else if (Q_stristr(szClass, "fbi") && Q_stristr(szClass, "old")) gloveIndex = 16;
        else if (Q_stristr(szClass, "phoenix") && Q_stristr(szClass, "old")) gloveIndex = 17;
        else if (Q_stristr(szClass, "leet") && Q_stristr(szClass, "old")) gloveIndex = 18;
        else if (Q_stristr(szClass, "bare")) gloveIndex = 19;
        
        if (gloveIndex >= 0 && gloveIndex < 20)
        {
            char szConVarNameCT[64];
            char szConVarNameT[64];
            Q_snprintf(szConVarNameCT, sizeof(szConVarNameCT), "loadout_gloves_ct_skin%d", gloveIndex);
            Q_snprintf(szConVarNameT, sizeof(szConVarNameT), "loadout_gloves_t_skin%d", gloveIndex);
            
            ConVar *pConVarCT = cvar->FindVar(szConVarNameCT);
            ConVar *pGloveSlotCT = cvar->FindVar("loadout_slot_gloves_ct");
            
            if (pConVarCT && pGloveSlotCT && 
                pConVarCT->GetInt() == m_pSkinDef->iPaintKit &&
                pGloveSlotCT->GetInt() == gloveIndex)
            {
                flags |= EQUIP_TEAM_CT;
            }
            
            ConVar *pConVarT = cvar->FindVar(szConVarNameT);
            ConVar *pGloveSlotT = cvar->FindVar("loadout_slot_gloves_t");
            
            if (pConVarT && pGloveSlotT && 
                pConVarT->GetInt() == m_pSkinDef->iPaintKit &&
                pGloveSlotT->GetInt() == gloveIndex)
            {
                flags |= EQUIP_TEAM_T;
            }
        }
    }
    else if (m_pSkinDef)
    {
        const char *pszConVarName = nullptr;
        switch (m_pSkinDef->weaponID)
        {
            case WEAPON_AK47: pszConVarName = "loadout_skin_ak47"; break;
            case WEAPON_M4A4: pszConVarName =  "loadout_skin_m4a4"; break;
            case WEAPON_M4A1: pszConVarName =  "loadout_skin_m4a1s"; break;
            case WEAPON_AWP:  pszConVarName =  "loadout_skin_awp"; break;
            case WEAPON_DEAGLE: pszConVarName =  "loadout_skin_deagle"; break;
            case WEAPON_REVOLVER: pszConVarName =  "loadout_skin_revolver"; break;
            case WEAPON_GLOCK: pszConVarName =  "loadout_skin_glock"; break;
            case WEAPON_USP: pszConVarName =  "loadout_skin_usp"; break;
            case WEAPON_HKP2000: pszConVarName =  "loadout_skin_hkp2000"; break;
            case WEAPON_P250: pszConVarName =  "loadout_skin_p250"; break;
            case WEAPON_FIVESEVEN: pszConVarName =  "loadout_skin_fiveseven"; break;
            case WEAPON_TEC9: pszConVarName =  "loadout_skin_tec9"; break;
            case WEAPON_CZ75A: pszConVarName = "loadout_skin_cz75a"; break;
            case WEAPON_ELITE: pszConVarName = "loadout_skin_dualberettas"; break;
            case WEAPON_MAG7: pszConVarName = "loadout_skin_mag7"; break;
            case WEAPON_NOVA: pszConVarName = "loadout_skin_nova"; break;
            case WEAPON_SAWEDOFF: pszConVarName = "loadout_skin_sawedoff"; break;
            case WEAPON_XM1014: pszConVarName = "loadout_skin_xm1014"; break;
            case WEAPON_M249: pszConVarName = "loadout_skin_m249"; break;
            case WEAPON_NEGEV: pszConVarName = "loadout_skin_negev"; break;
            case WEAPON_MAC10: pszConVarName = "loadout_skin_mac10"; break;
            case WEAPON_MP9: pszConVarName = "loadout_skin_mp9"; break;
            case WEAPON_MP7: pszConVarName = "loadout_skin_mp7"; break;
            case WEAPON_MP5SD: pszConVarName = "loadout_skin_mp5sd"; break;
            case WEAPON_UMP45: pszConVarName = "loadout_skin_ump45"; break;
            case WEAPON_P90: pszConVarName = "loadout_skin_p90"; break;
            case WEAPON_BIZON: pszConVarName = "loadout_skin_bizon"; break;
            case WEAPON_GALILAR: pszConVarName = "loadout_skin_galilar"; break;
            case WEAPON_FAMAS: pszConVarName = "loadout_skin_famas"; break;
            case WEAPON_AUG: pszConVarName = "loadout_skin_aug"; break;
            case WEAPON_SG556: pszConVarName = "loadout_skin_sg556"; break;
            case WEAPON_SSG08: pszConVarName = "loadout_skin_ssg08"; break;
            case WEAPON_SCAR20: pszConVarName = "loadout_skin_scar20"; break;
            case WEAPON_G3SG1: pszConVarName = "loadout_skin_g3sg1"; break;
            default: break;
        }
        
        if (pszConVarName)
        {
            ConVar *pConVar = cvar->FindVar(pszConVarName);
            if (pConVar && pConVar->GetInt() == m_pSkinDef->iPaintKit)
                flags = EQUIP_TEAM_BOTH;
        }
    }
    
    return flags;
}

void CInventoryItemPanel::UpdateEquipIndicators()
{
    m_iEquipFlags = GetEquipFlags();
}

Color CInventoryItemPanel::GetRarityColor() const
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

CInventoryPanel::CInventoryPanel(Panel *parent, const char *panelName)
    : BaseClass(parent, panelName)
{
    int screenWidth, screenHeight;
    vgui::surface()->GetScreenSize(screenWidth, screenHeight);
    
    float scaleX = screenWidth / 1920.0f;
    float scaleY = screenHeight / 1080.0f;
    float scale = (scaleX + scaleY) / 2.0f;
    
    int baseItemWidth = 240;
    int baseItemHeight = 120;
    
    m_iItemWidth = baseItemWidth * scale;
    m_iItemHeight = baseItemHeight * scale;
    m_iItemSpacing = 15 * scale;
    m_iItemsPerRow = 4;
    m_iLabelHeight = 25;
    
    m_iLeftMargin = 45 * scale;
    m_iRightMargin = 45 * scale;
    
    m_FilterWeaponID = WEAPON_NONE;
    m_FilterRarity = SKIN_RARITY_COMMON;
    m_FilterItemType = INVENTORY_ITEM_WEAPON_SKIN;
    m_bUseWeaponFilter = false;
    m_bUseRarityFilter = false;
    m_bUseItemTypeFilter = false;
    m_bFirstLayout = true;
    
    m_pItemContainer = new CInventoryItemContainer(NULL, "ItemContainer");
    m_pScrollablePanel = new vgui::ScrollableEditablePanel(this, m_pItemContainer, "ScrollablePanel");
    m_pScrollablePanel->GetScrollbar()->SetAutohideButtons(true);
    m_pScrollablePanel->SetMouseInputEnabled(true);
    
    m_pContextMenu = new CItemContextMenu(this, "ItemContextMenu");
    m_pContextMenu->SetVisible(false);
    m_pContextMenu->AddActionSignalTarget(this);
    
    ivgui()->AddTickSignal(GetVPanel());
    
    m_bNeedsRepositioning = false;
}

CInventoryPanel::~CInventoryPanel()
{
    ClearInventory();
    
    if (m_pContextMenu)
    {
        m_pContextMenu->MarkForDeletion();
        m_pContextMenu = nullptr;
    }
}

void CInventoryPanel::LoadAgents()
{
    for (int i = 0; i <= MAX_AGENTS_CT; i++)
    {
        if (i >= ARRAYSIZE(g_AgentIconsCT))
            break;
            
        char szIconPath[256];
        Q_snprintf(szIconPath, sizeof(szIconPath), "agents/%s", g_AgentIconsCT[i]);
        AddAgentToInventory(i, TEAM_CT, g_AgentNamesCT[i], szIconPath);
    }
    
    for (int i = 0; i <= MAX_AGENTS_T; i++)
    {
        if (i >= ARRAYSIZE(g_AgentIconsT))
            break;
            
        char szIconPath[256];
        Q_snprintf(szIconPath, sizeof(szIconPath), "agents/%s", g_AgentIconsT[i]);
        AddAgentToInventory(i, TEAM_TERRORIST, g_AgentNamesT[i], szIconPath);
    }
}

void CInventoryPanel::LoadMusic()
{
    for (int i = 0; i < MAX_MUSIC && i < ARRAYSIZE(g_MusicKitIcons); i++)
    {
        char szIconPath[256];
        Q_snprintf(szIconPath, sizeof(szIconPath), "music/%s", g_MusicKitIcons[i]);
        AddMusicToInventory(i, g_MusicKitNames[i], szIconPath);
    }
}

void CInventoryPanel::LoadGloves()
{
    const char *gloveNames[] = {
        "", "Bloodhound", "Bloodhound PW", "Bloodhound BF", "Bloodhound Hydra",
        "Fingerless", "Fullfinger", "Handwrap Leathery", "Hardknuckle", "Hardknuckle Black",
        "Hardknuckle Blue", "Motorcycle", "Slick", "Specialist", "Sporty",
        "SAS Old", "FBI Old", "Phoenix Old", "Leet Old", "Bare Hands"
    };
    
    const char *gloveIcons[] = {
        "", "glove_bloodhound", "glove_bloodhound_perfectworld", "glove_bloodhound_brokenfang", "glove_bloodhound_hydra",
        "glove_fingerless", "glove_fullfinger", "glove_handwrap_leathery", "glove_hardknuckle", "glove_hardknuckle_black",
        "glove_hardknuckle_blue", "glove_motorcycle", "glove_slick", "glove_specialist", "glove_sporty",
        "glove_sas_old", "glove_fbi_old", "glove_phoenix_old", "glove_leet_old", "glove_bare"
    };
    
    for (int i = 1; i < ARRAYSIZE(gloveNames); i++)
    {
        char szIconPath[256];
        Q_snprintf(szIconPath, sizeof(szIconPath), "gloves/%s", gloveIcons[i]);
        AddGlovesToInventory(i, gloveNames[i], szIconPath);
    }
}

void CInventoryPanel::AddAgentToInventory(int iAgentIndex, int team, const char *pszAgentName, const char *pszIconPath)
{
    char panelName[64];
    Q_snprintf(panelName, sizeof(panelName), "Agent_%s_%d", (team == TEAM_CT) ? "CT" : "T", iAgentIndex);
    
    InventoryItemType_t itemType = (team == TEAM_CT) ? INVENTORY_ITEM_AGENT_CT : INVENTORY_ITEM_AGENT_T;
    
    CInventoryItemPanel *pItem = new CInventoryItemPanel(m_pItemContainer, panelName, nullptr, itemType, iAgentIndex, pszIconPath, pszAgentName);
    pItem->AddActionSignalTarget(this);
    m_Items.AddToTail(pItem);
    
    int idx = m_Items.Count() - 1;
    const char *teamName = (team == TEAM_CT) ? "CT" : "T";
    
    vgui::Label *pNameLabel = new vgui::Label(m_pItemContainer, VarArgs("AgentLabel_%s_%d", teamName, iAgentIndex), pszAgentName);
    pNameLabel->SetMouseInputEnabled(false);
    pItem->SetExternalNameLabel(pNameLabel);
    m_ItemLabels.AddToTail(pNameLabel);
    
    vgui::Label *pModelLabel = new vgui::Label(m_pItemContainer, VarArgs("AgentModel_%s_%d", teamName, iAgentIndex), "");
    pModelLabel->SetMouseInputEnabled(false);
    pItem->SetExternalModelLabel(pModelLabel);
    m_ModelLabels.AddToTail(pModelLabel);
    
    vgui::Label *pSkinLabel = new vgui::Label(m_pItemContainer, VarArgs("AgentSkin_%s_%d", teamName, iAgentIndex), "");
    pSkinLabel->SetMouseInputEnabled(false);
    pItem->SetExternalSkinLabel(pSkinLabel);
    m_SkinLabels.AddToTail(pSkinLabel);
}

void CInventoryPanel::AddMusicToInventory(int iMusicIndex, const char *pszMusicName, const char *pszIconPath)
{
    char panelName[64];
    Q_snprintf(panelName, sizeof(panelName), "Music_%d", iMusicIndex);
    
    CInventoryItemPanel *pItem = new CInventoryItemPanel(m_pItemContainer, panelName, nullptr, INVENTORY_ITEM_MUSIC, iMusicIndex, pszIconPath, pszMusicName);
    pItem->AddActionSignalTarget(this);
    m_Items.AddToTail(pItem);
    
    int idx = m_Items.Count() - 1;
    
    vgui::Label *pNameLabel = new vgui::Label(m_pItemContainer, VarArgs("MusicLabel_%d", iMusicIndex), pszMusicName);
    pNameLabel->SetMouseInputEnabled(false);
    pItem->SetExternalNameLabel(pNameLabel);
    m_ItemLabels.AddToTail(pNameLabel);
    
    vgui::Label *pModelLabel = new vgui::Label(m_pItemContainer, VarArgs("MusicModel_%d", iMusicIndex), "");
    pModelLabel->SetMouseInputEnabled(false);
    pItem->SetExternalModelLabel(pModelLabel);
    m_ModelLabels.AddToTail(pModelLabel);
    
    vgui::Label *pSkinLabel = new vgui::Label(m_pItemContainer, VarArgs("MusicSkin_%d", iMusicIndex), "");
    pSkinLabel->SetMouseInputEnabled(false);
    pItem->SetExternalSkinLabel(pSkinLabel);
    m_SkinLabels.AddToTail(pSkinLabel);
}

void CInventoryPanel::AddGlovesToInventory(int iGloveSlot, const char *pszGloveName, const char *pszIconPath)
{
    char panelName[64];
    Q_snprintf(panelName, sizeof(panelName), "Gloves_%d", iGloveSlot);
    
    CInventoryItemPanel *pItem = new CInventoryItemPanel(m_pItemContainer, panelName, nullptr, INVENTORY_ITEM_GLOVES, iGloveSlot, pszIconPath, pszGloveName);
    pItem->AddActionSignalTarget(this);
    m_Items.AddToTail(pItem);
    
    int idx = m_Items.Count() - 1;
    
    vgui::Label *pNameLabel = new vgui::Label(m_pItemContainer, VarArgs("GloveLabel_%d", iGloveSlot), pszGloveName);
    pNameLabel->SetMouseInputEnabled(false);
    pItem->SetExternalNameLabel(pNameLabel);
    m_ItemLabels.AddToTail(pNameLabel);
    
    vgui::Label *pModelLabel = new vgui::Label(m_pItemContainer, VarArgs("GloveModel_%d", iGloveSlot), "");
    pModelLabel->SetMouseInputEnabled(false);
    pItem->SetExternalModelLabel(pModelLabel);
    m_ModelLabels.AddToTail(pModelLabel);
    
    vgui::Label *pSkinLabel = new vgui::Label(m_pItemContainer, VarArgs("GloveSkin_%d", iGloveSlot), "");
    pSkinLabel->SetMouseInputEnabled(false);
    pItem->SetExternalSkinLabel(pSkinLabel);
    m_SkinLabels.AddToTail(pSkinLabel);
}

void CInventoryPanel::OnCommand(const char *command)
{
    if (Q_stricmp(command, "DelayedLoad") == 0)
    {
        LoadAllSkins();
        LoadGloves();
        LoadAgents();
        LoadMusic();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}
void CInventoryPanel::SaveItemToLoadout(int iPaintKit, CSWeaponID weaponID, InventoryItemType_t itemType, int iSpecialIndex, int teamFlags)
{
    switch (itemType)
    {
        case INVENTORY_ITEM_AGENT_CT:
        {
            ConVar *pAgentSlot = cvar->FindVar("loadout_slot_agent_ct");
            if (pAgentSlot)
                pAgentSlot->SetValue(iSpecialIndex);
            break;
        }
        
        case INVENTORY_ITEM_AGENT_T:
        {
            ConVar *pAgentSlot = cvar->FindVar("loadout_slot_agent_t");
            if (pAgentSlot)
                pAgentSlot->SetValue(iSpecialIndex);
            break;
        }
        
        case INVENTORY_ITEM_MUSIC:
        {
            ConVar *pMusicSlot = cvar->FindVar("loadout_music");
            if (pMusicSlot)
                pMusicSlot->SetValue(iSpecialIndex);
            break;
        }
        
        case INVENTORY_ITEM_KNIFE:
        {
            const char *pszKnifeName = GetConVarNameForKnife(weaponID);
            if (!pszKnifeName)
                return;
            
            char szSkinConVarName[64];
            Q_snprintf(szSkinConVarName, sizeof(szSkinConVarName), "loadout_skin_%s", pszKnifeName);
            
            ConVar *pSkinConVar = cvar->FindVar(szSkinConVarName);
            if (pSkinConVar)
                pSkinConVar->SetValue(iPaintKit);
            
            int knifeIndex = 0;
            switch (weaponID)
            {
                case WEAPON_KNIFE_CSS: knifeIndex = 1; break;
                case WEAPON_KNIFE_KARAMBIT: knifeIndex = 2; break;
                case WEAPON_KNIFE_FLIP: knifeIndex = 3; break;
                case WEAPON_KNIFE_BAYONET: knifeIndex = 4; break;
                case WEAPON_KNIFE_M9_BAYONET: knifeIndex = 5; break;
                case WEAPON_KNIFE_BUTTERFLY: knifeIndex = 6; break;
                case WEAPON_KNIFE_GUT: knifeIndex = 7; break;
                case WEAPON_KNIFE_TACTICAL: knifeIndex = 8; break;
                case WEAPON_KNIFE_FALCHION: knifeIndex = 9; break;
                case WEAPON_KNIFE_SURVIVAL_BOWIE: knifeIndex = 10; break;
                case WEAPON_KNIFE_CANIS: knifeIndex = 11; break;
                case WEAPON_KNIFE_CORD: knifeIndex = 12; break;
                case WEAPON_KNIFE_GYPSY: knifeIndex = 13; break;
                case WEAPON_KNIFE_OUTDOOR: knifeIndex = 14; break;
                case WEAPON_KNIFE_SKELETON: knifeIndex = 15; break;
                case WEAPON_KNIFE_STILETTO: knifeIndex = 16; break;
                case WEAPON_KNIFE_URSUS: knifeIndex = 17; break;
                case WEAPON_KNIFE_WIDOWMAKER: knifeIndex = 18; break;
                case WEAPON_KNIFE_PUSH: knifeIndex = 19; break;
            }
            
            if (teamFlags & EQUIP_TEAM_CT)
            {
                ConVar *pKnifeSlotCT = cvar->FindVar("loadout_slot_knife_weapon_ct");
                if (pKnifeSlotCT)
                    pKnifeSlotCT->SetValue(knifeIndex);
            }
            
            if (teamFlags & EQUIP_TEAM_T)
            {
                ConVar *pKnifeSlotT = cvar->FindVar("loadout_slot_knife_weapon_t");
                if (pKnifeSlotT)
                    pKnifeSlotT->SetValue(knifeIndex);
            }
            break;
        }
        
        case INVENTORY_ITEM_GLOVES:
        {
            if (teamFlags & EQUIP_TEAM_CT)
            {
                ConVar *pGloveSlotCT = cvar->FindVar("loadout_slot_gloves_ct");
                if (pGloveSlotCT)
                    pGloveSlotCT->SetValue(iSpecialIndex);
                
                char szSkinConVarName[64];
                Q_snprintf(szSkinConVarName, sizeof(szSkinConVarName), "loadout_gloves_ct_skin%d", iSpecialIndex);
                ConVar *pSkinConVar = cvar->FindVar(szSkinConVarName);
                if (pSkinConVar)
                    pSkinConVar->SetValue(iPaintKit);
            }
            
            if (teamFlags & EQUIP_TEAM_T)
            {
                ConVar *pGloveSlotT = cvar->FindVar("loadout_slot_gloves_t");
                if (pGloveSlotT)
                    pGloveSlotT->SetValue(iSpecialIndex);
                
                char szSkinConVarName[64];
                Q_snprintf(szSkinConVarName, sizeof(szSkinConVarName), "loadout_gloves_t_skin%d", iSpecialIndex);
                ConVar *pSkinConVar = cvar->FindVar(szSkinConVarName);
                if (pSkinConVar)
                    pSkinConVar->SetValue(iPaintKit);
            }
            break;
        }
        
        case INVENTORY_ITEM_WEAPON_SKIN:
        default:
        {
            const char *pszConVarName = GetConVarNameForWeapon(weaponID);
            if (pszConVarName)
            {
                ConVar *pConVar = cvar->FindVar(pszConVarName);
                if (pConVar)
                    pConVar->SetValue(iPaintKit);
            }
            break;
        }
    }
}

void CInventoryPanel::UnequipItem(CSWeaponID weaponID, InventoryItemType_t itemType, int iSpecialIndex)
{
    switch (itemType)
    {
        case INVENTORY_ITEM_AGENT_CT:
        {
            ConVar *pAgentSlot = cvar->FindVar("loadout_slot_agent_ct");
            if (pAgentSlot)
                pAgentSlot->SetValue(0);
            break;
        }
        
        case INVENTORY_ITEM_AGENT_T:
        {
            ConVar *pAgentSlot = cvar->FindVar("loadout_slot_agent_t");
            if (pAgentSlot)
                pAgentSlot->SetValue(0);
            break;
        }
        
        case INVENTORY_ITEM_MUSIC:
        {
            ConVar *pMusicSlot = cvar->FindVar("loadout_music");
            if (pMusicSlot)
                pMusicSlot->SetValue(0);
            break;
        }
        
        case INVENTORY_ITEM_KNIFE:
        {
            const char *pszKnifeName = GetConVarNameForKnife(weaponID);
            if (pszKnifeName)
            {
                char szSkinConVarName[64];
                Q_snprintf(szSkinConVarName, sizeof(szSkinConVarName), "loadout_skin_%s", pszKnifeName);
                ConVar *pSkinConVar = cvar->FindVar(szSkinConVarName);
                if (pSkinConVar)
                    pSkinConVar->SetValue(0);
            }
            
            ConVar *pKnifeSlotCT = cvar->FindVar("loadout_slot_knife_weapon_ct");
            if (pKnifeSlotCT)
                pKnifeSlotCT->SetValue(0);
            
            ConVar *pKnifeSlotT = cvar->FindVar("loadout_slot_knife_weapon_t");
            if (pKnifeSlotT)
                pKnifeSlotT->SetValue(0);
            break;
        }
        
        case INVENTORY_ITEM_GLOVES:
        {
            ConVar *pGloveSlotCT = cvar->FindVar("loadout_slot_gloves_ct");
            if (pGloveSlotCT)
                pGloveSlotCT->SetValue(0);
            
            ConVar *pGloveSlotT = cvar->FindVar("loadout_slot_gloves_t");
            if (pGloveSlotT)
                pGloveSlotT->SetValue(0);
            
            char szSkinConVarName[64];
            Q_snprintf(szSkinConVarName, sizeof(szSkinConVarName), "loadout_gloves_ct_skin%d", iSpecialIndex);
            ConVar *pSkinConVar = cvar->FindVar(szSkinConVarName);
            if (pSkinConVar)
                pSkinConVar->SetValue(0);
            
            Q_snprintf(szSkinConVarName, sizeof(szSkinConVarName), "loadout_gloves_t_skin%d", iSpecialIndex);
            pSkinConVar = cvar->FindVar(szSkinConVarName);
            if (pSkinConVar)
                pSkinConVar->SetValue(0);
            break;
        }
        
        case INVENTORY_ITEM_WEAPON_SKIN:
        default:
        {
            const char *pszConVarName = GetConVarNameForWeapon(weaponID);
            if (pszConVarName)
            {
                ConVar *pConVar = cvar->FindVar(pszConVarName);
                if (pConVar)
                    pConVar->SetValue(0);
            }
            break;
        }
    }
}

void CInventoryPanel::OnMenuCommand(KeyValues *data)
{
    const char *command = data->GetString("command", "");
    
    if (!m_pContextMenu)
        return;
    
    int iPaintKit = m_pContextMenu->GetCurrentPaintKit();
    CSWeaponID weaponID = m_pContextMenu->GetCurrentWeaponID();
    InventoryItemType_t itemType = (InventoryItemType_t)m_pContextMenu->GetItemType();
    int iSpecialIndex = m_pContextMenu->GetSpecialIndex();
    
    int iGloveIndex = -1;
    if (itemType == INVENTORY_ITEM_GLOVES && iSpecialIndex == 0)
    {
        FOR_EACH_VEC(m_Items, i)
        {
            if (m_Items[i]->GetItemType() == INVENTORY_ITEM_GLOVES && 
                m_Items[i]->GetPaintKit() == iPaintKit &&
                m_Items[i]->GetSkinDefinition())
            {
                iGloveIndex = GetGloveIndexFromSkinDef(m_Items[i]->GetSkinDefinition());
                break;
            }
        }
    }
    
    if (iGloveIndex >= 0)
    {
        if (Q_stricmp(command, "equip_all") == 0)
            SaveGloveSkinToLoadout(iPaintKit, iGloveIndex, EQUIP_TEAM_BOTH);
        else if (Q_stricmp(command, "equip_ct") == 0)
            SaveGloveSkinToLoadout(iPaintKit, iGloveIndex, EQUIP_TEAM_CT);
        else if (Q_stricmp(command, "equip_t") == 0)
            SaveGloveSkinToLoadout(iPaintKit, iGloveIndex, EQUIP_TEAM_T);
        else if (Q_stricmp(command, "unequip") == 0)
            UnequipGloves(iGloveIndex);
    }
    else
    {
        if (Q_stricmp(command, "equip_all") == 0)
            SaveItemToLoadout(iPaintKit, weaponID, itemType, iSpecialIndex, EQUIP_TEAM_BOTH);
        else if (Q_stricmp(command, "equip_ct") == 0)
            SaveItemToLoadout(iPaintKit, weaponID, itemType, iSpecialIndex, EQUIP_TEAM_CT);
        else if (Q_stricmp(command, "equip_t") == 0)
            SaveItemToLoadout(iPaintKit, weaponID, itemType, iSpecialIndex, EQUIP_TEAM_T);
        else if (Q_stricmp(command, "equip") == 0)
            SaveItemToLoadout(iPaintKit, weaponID, itemType, iSpecialIndex, EQUIP_TEAM_BOTH);
        else if (Q_stricmp(command, "unequip") == 0)
            UnequipItem(weaponID, itemType, iSpecialIndex);
    }
    
    m_pContextMenu->SetVisible(false);
    
    FOR_EACH_VEC(m_Items, i)
    {
        m_Items[i]->UpdateEquipIndicators();
    }
}
void CInventoryPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(pScheme->GetColor("Frame.BgColor", Color(30, 30, 30, 255)));
}

void CInventoryPanel::OnThink()
{
    BaseClass::OnThink();
    
    if (m_bFirstLayout && GetWide() > 0 && GetTall() > 0)
    {
        m_bFirstLayout = false;
        PostMessage(this, new KeyValues("DelayedLoad"), 0.2f);
    }
}

void CInventoryPanel::PerformLayout()
{
    InvalidateLayout();
    BaseClass::PerformLayout();
    
    if (!m_pItemContainer)
    return;
    
    int wide, tall;
    GetSize(wide, tall);
    
    if (wide <= 0 || tall <= 0)
        return;
    
    m_pScrollablePanel->SetBounds(0, 0, wide, tall);
    
    int scrollBarWidth = 16;
    int availableWidth = wide - scrollBarWidth - m_iLeftMargin - m_iRightMargin;
    
    m_iItemsPerRow = availableWidth / (m_iItemWidth + m_iItemSpacing);
    if (m_iItemsPerRow < 1) m_iItemsPerRow = 1;
    
    int numItems = m_Items.Count();
    
    int itemYSpacing = (int)(m_iItemHeight * 1.6f);
    
    int textHeight = 53;
    
    int numRows = (numItems + m_iItemsPerRow - 1) / m_iItemsPerRow;
    
    for (int i = 0; i < numItems; i++)
    {
        int row = i / m_iItemsPerRow;
        int col = i % m_iItemsPerRow;
        
        int x = m_iLeftMargin + col * (m_iItemWidth + m_iItemSpacing);
        int y = m_iItemSpacing + row * itemYSpacing;
        
        m_Items[i]->SetBounds(x, y, m_iItemWidth, m_iItemHeight + textHeight);
        
        if (i < m_ItemLabels.Count())
        {
            int labelY = y + m_iItemHeight + textHeight;
            int labelHeight = 18;
            
            m_ItemLabels[i]->SetBounds(x, labelY, m_iItemWidth, labelHeight);
            m_ItemLabels[i]->SetContentAlignment(vgui::Label::a_center);
            
            if (i < m_ModelLabels.Count())
            {
                m_ModelLabels[i]->SetBounds(x, labelY + labelHeight, m_iItemWidth, labelHeight);
                m_ModelLabels[i]->SetContentAlignment(vgui::Label::a_center);
            }
            
            if (i < m_SkinLabels.Count())
            {
                m_SkinLabels[i]->SetBounds(x, labelY + labelHeight * 2, m_iItemWidth, labelHeight);
                m_SkinLabels[i]->SetContentAlignment(vgui::Label::a_center);
            }
        }
    }
    
    int containerHeight = max(tall, m_iItemSpacing + numRows * (itemYSpacing + m_iLabelHeight * 3) + m_iItemSpacing);
    m_pItemContainer->SetSize(wide - scrollBarWidth, containerHeight);
}

void CInventoryPanel::RepositionItems()
{
    InvalidateLayout();
}


void CInventoryPanel::LoadAllSkins()
{
    if (!g_SkinDatabase.Initialize())
        return;
    
    CUtlVector<const SkinDefinition_t*> allSkins;
    g_SkinDatabase.GetAllSkins(allSkins);
    
    FOR_EACH_VEC(allSkins, i)
    {
        const SkinDefinition_t *pSkinDef = allSkins[i];
        if (m_bUseWeaponFilter && pSkinDef->weaponID != m_FilterWeaponID)
            continue;
        if (m_bUseRarityFilter && pSkinDef->rarity != m_FilterRarity)
            continue;
        
        if (m_bUseItemTypeFilter)
        {
            InventoryItemType_t itemType = INVENTORY_ITEM_WEAPON_SKIN;
            if (pSkinDef->weaponID >= WEAPON_KNIFE && pSkinDef->weaponID <= WEAPON_KNIFE_LAST)
                itemType = INVENTORY_ITEM_KNIFE;
            else if (pSkinDef->IsGloves())
                itemType = INVENTORY_ITEM_GLOVES;
            
            if (itemType != m_FilterItemType)
                continue;
        }
        
        AddSkinToInventory(pSkinDef);
    }
    InvalidateLayout();
}

void CInventoryPanel::LoadSkinsForWeapon(CSWeaponID weaponID)
{
    ClearInventory();
    if (!g_SkinDatabase.Initialize())
        return;
    CUtlVector<const SkinDefinition_t*> weaponSkins;
    g_SkinDatabase.GetSkinsForWeapon(weaponID, weaponSkins);
    FOR_EACH_VEC(weaponSkins, i)
        AddSkinToInventory(weaponSkins[i]);
    InvalidateLayout();
}

void CInventoryPanel::AddSkinToInventory(const SkinDefinition_t *pSkinDef)
{
    if (!pSkinDef)
        return;
    
    char panelName[64];
    Q_snprintf(panelName, sizeof(panelName), "Item_%d", m_Items.Count());
    
    InventoryItemType_t itemType = INVENTORY_ITEM_WEAPON_SKIN;
    if (pSkinDef->weaponID >= WEAPON_KNIFE && pSkinDef->weaponID <= WEAPON_KNIFE_LAST)
        itemType = INVENTORY_ITEM_KNIFE;
    else if (pSkinDef->IsGloves())
        itemType = INVENTORY_ITEM_GLOVES;
    
    CInventoryItemPanel *pItem = new CInventoryItemPanel(m_pItemContainer, panelName, pSkinDef, itemType, 0, nullptr);
    pItem->AddActionSignalTarget(this);
    m_Items.AddToTail(pItem);
    
    int idx = m_Items.Count() - 1;
    
    vgui::Label *pNameLabel = new vgui::Label(m_pItemContainer, VarArgs("ItemLabel_%d", idx), "");
    pNameLabel->SetMouseInputEnabled(false);
    pItem->SetExternalNameLabel(pNameLabel);
    m_ItemLabels.AddToTail(pNameLabel);
    
    vgui::Label *pModelLabel = new vgui::Label(m_pItemContainer, VarArgs("ModelLabel_%d", idx), "");
    pModelLabel->SetMouseInputEnabled(false);
    pItem->SetExternalModelLabel(pModelLabel);
    m_ModelLabels.AddToTail(pModelLabel);
    
    vgui::Label *pSkinLabel = new vgui::Label(m_pItemContainer, VarArgs("SkinLabel_%d", idx), "");
    pSkinLabel->SetMouseInputEnabled(false);
    pItem->SetExternalSkinLabel(pSkinLabel);
    m_SkinLabels.AddToTail(pSkinLabel);
}

void CInventoryPanel::ClearInventory()
{
    FOR_EACH_VEC(m_Items, i)
        m_Items[i]->MarkForDeletion();
    m_Items.RemoveAll();
    
    FOR_EACH_VEC(m_ItemLabels, i)
        m_ItemLabels[i]->MarkForDeletion();
    m_ItemLabels.RemoveAll();
    
    FOR_EACH_VEC(m_ModelLabels, i)
        m_ModelLabels[i]->MarkForDeletion();
    m_ModelLabels.RemoveAll();
    
    FOR_EACH_VEC(m_SkinLabels, i)
        m_SkinLabels[i]->MarkForDeletion();
    m_SkinLabels.RemoveAll();
}

void CInventoryPanel::SetWeaponFilter(CSWeaponID weaponID)
{
    m_FilterWeaponID = weaponID;
    m_bUseWeaponFilter = true;
    RebuildInventory();
    InvalidateLayout();
}

void CInventoryPanel::SetRarityFilter(ESkinRarity rarity)
{
    m_FilterRarity = rarity;
    m_bUseRarityFilter = true;
    RebuildInventory();
    InvalidateLayout();
}

void CInventoryPanel::ClearFilters()
{
    m_bUseWeaponFilter = false;
    m_bUseRarityFilter = false;
    m_bUseItemTypeFilter = false;
    RebuildInventory();
}

void CInventoryPanel::SetItemTypeFilter(InventoryItemType_t itemType)
{
    m_FilterItemType = itemType;
    m_bUseItemTypeFilter = true;
    RebuildInventory();
    InvalidateLayout();
}

void CInventoryPanel::RebuildInventory()
{
    ClearInventory();
    
    if (m_bUseItemTypeFilter)
    {
        switch (m_FilterItemType)
        {
            case INVENTORY_ITEM_KNIFE:
            case INVENTORY_ITEM_WEAPON_SKIN:
            case INVENTORY_ITEM_GLOVES:
                LoadAllSkins();
                break;
            case INVENTORY_ITEM_AGENT_CT:
            case INVENTORY_ITEM_AGENT_T:
                LoadAgents();
                break;
            case INVENTORY_ITEM_MUSIC:
                LoadMusic();
                break;
        }
    }
    else
    {
        LoadAllSkins();
        LoadGloves();
        LoadAgents();
        LoadMusic();
    }
    
    InvalidateLayout();
}

void CInventoryPanel::OnItemSelected(KeyValues *data)
{
    int iPaintKit = data->GetInt("paintkit", 0);
    int iWeaponID = data->GetInt("weaponid", WEAPON_NONE);
    int iItemType = data->GetInt("itemtype", INVENTORY_ITEM_WEAPON_SKIN);
    int iSpecialIndex = data->GetInt("specialindex", 0);
    int cursorX = data->GetInt("cursorx", 0);
    int cursorY = data->GetInt("cursory", 0);
    
    if (m_pContextMenu)
    {
        const char *pszWeaponName = "Item";
        const char *pszSkinName = "Skin";
        
        FOR_EACH_VEC(m_Items, i)
        {
            if (m_Items[i]->GetItemType() == iItemType && m_Items[i]->GetSpecialIndex() == iSpecialIndex)
            {
                if (m_Items[i]->GetSkinDefinition())
                    pszSkinName = m_Items[i]->GetSkinDefinition()->szName;
                break;
            }
        }
        
        m_pContextMenu->ShowForItem(iPaintKit, (CSWeaponID)iWeaponID, (ItemType_t)iItemType, iSpecialIndex, pszWeaponName, pszSkinName, cursorX, cursorY);
    }
}

const char* CInventoryPanel::GetConVarNameForWeapon(CSWeaponID weaponID)
{
    switch (weaponID)
    {
        case WEAPON_AK47: return "loadout_skin_ak47";
        case WEAPON_M4A4: return "loadout_skin_m4a4";
        case WEAPON_M4A1: return "loadout_skin_m4a1s";
        case WEAPON_AWP: return "loadout_skin_awp";
        case WEAPON_DEAGLE: return "loadout_skin_deagle";
        case WEAPON_REVOLVER: return "loadout_skin_revolver";
        case WEAPON_GLOCK: return "loadout_skin_glock";
        case WEAPON_USP: return "loadout_skin_usp";
        case WEAPON_HKP2000: return "loadout_skin_hkp2000";
        case WEAPON_P250: return "loadout_skin_p250";
        case WEAPON_FIVESEVEN: return "loadout_skin_fiveseven";
        case WEAPON_TEC9: return "loadout_skin_tec9";
        case WEAPON_CZ75A: return "loadout_skin_cz75a";
        case WEAPON_ELITE: return "loadout_skin_dualberettas";
        case WEAPON_MAG7: return "loadout_skin_mag7";
        case WEAPON_NOVA: return "loadout_skin_nova";
        case WEAPON_SAWEDOFF: return "loadout_skin_sawedoff";
        case WEAPON_XM1014: return "loadout_skin_xm1014";
        case WEAPON_M249: return "loadout_skin_m249";
        case WEAPON_NEGEV: return "loadout_skin_negev";
        case WEAPON_MAC10: return "loadout_skin_mac10";
        case WEAPON_MP9: return "loadout_skin_mp9";
        case WEAPON_MP7: return "loadout_skin_mp7";
        case WEAPON_MP5SD: return "loadout_skin_mp5sd";
        case WEAPON_UMP45: return "loadout_skin_ump45";
        case WEAPON_P90: return "loadout_skin_p90";
        case WEAPON_BIZON: return "loadout_skin_bizon";
        case WEAPON_GALILAR: return "loadout_skin_galilar";
        case WEAPON_FAMAS: return "loadout_skin_famas";
        case WEAPON_AUG: return "loadout_skin_aug";
        case WEAPON_SG556: return "loadout_skin_sg556";
        case WEAPON_SSG08: return "loadout_skin_ssg08";
        case WEAPON_SCAR20: return "loadout_skin_scar20";
        case WEAPON_G3SG1: return "loadout_skin_g3sg1";
        default: return nullptr;
    }
}

const char* CInventoryPanel::GetConVarNameForKnife(CSWeaponID weaponID)
{
    switch (weaponID)
    {
        case WEAPON_KNIFE_CSS: return "knife_css";
        case WEAPON_KNIFE_KARAMBIT: return "knife_karambit";
        case WEAPON_KNIFE_FLIP: return "knife_flip";
        case WEAPON_KNIFE_BAYONET: return "knife_bayonet";
        case WEAPON_KNIFE_M9_BAYONET: return "knife_m9_bayonet";
        case WEAPON_KNIFE_BUTTERFLY: return "knife_butterfly";
        case WEAPON_KNIFE_GUT: return "knife_gut";
        case WEAPON_KNIFE_TACTICAL: return "knife_tactical";
        case WEAPON_KNIFE_FALCHION: return "knife_falchion";
        case WEAPON_KNIFE_SURVIVAL_BOWIE: return "knife_survival_bowie";
        case WEAPON_KNIFE_CANIS: return "knife_canis";
        case WEAPON_KNIFE_CORD: return "knife_cord";
        case WEAPON_KNIFE_GYPSY: return "knife_gypsy_jackknife";
        case WEAPON_KNIFE_OUTDOOR: return "knife_outdoor";
        case WEAPON_KNIFE_SKELETON: return "knife_skeleton";
        case WEAPON_KNIFE_STILETTO: return "knife_stiletto";
        case WEAPON_KNIFE_URSUS: return "knife_ursus";
        case WEAPON_KNIFE_WIDOWMAKER: return "knife_widowmaker";
        case WEAPON_KNIFE_PUSH: return "knife_push";
        default: return nullptr;
    }
}

int CInventoryPanel::GetGloveIndexFromSkinDef(const SkinDefinition_t *pSkinDef)
{
    if (!pSkinDef || !pSkinDef->IsGloves())
        return -1;
    
    const char* szClass = pSkinDef->szItemClass;
    
    if (Q_stricmp(szClass, "default") == 0) return 0;
    if (Q_stristr(szClass, "bloodhound") && !Q_stristr(szClass, "perfectworld") && !Q_stristr(szClass, "brokenfang") && !Q_stristr(szClass, "hydra")) return 1;
    if (Q_stristr(szClass, "bloodhound") && Q_stristr(szClass, "perfectworld")) return 2;
    if (Q_stristr(szClass, "brokenfang") || Q_stristr(szClass, "broken_fang")) return 3;
    if (Q_stristr(szClass, "hydra")) return 4;
    if (Q_stristr(szClass, "fingerless")) return 5;
    if (Q_stristr(szClass, "fullfinger")) return 6;
    if (Q_stristr(szClass, "handwrap") || Q_stristr(szClass, "leathery")) return 7;
    if (Q_stristr(szClass, "hardknuckle") && Q_stristr(szClass, "black")) return 9;
    if (Q_stristr(szClass, "hardknuckle") && Q_stristr(szClass, "blue")) return 10;
    if (Q_stristr(szClass, "hardknuckle")) return 8;
    if (Q_stristr(szClass, "motorcycle")) return 11;
    if (Q_stristr(szClass, "slick")) return 12;
    if (Q_stristr(szClass, "specialist")) return 13;
    if (Q_stristr(szClass, "sporty")) return 14;
    if (Q_stristr(szClass, "sas") && Q_stristr(szClass, "old")) return 15;
    if (Q_stristr(szClass, "fbi") && Q_stristr(szClass, "old")) return 16;
    if (Q_stristr(szClass, "phoenix") && Q_stristr(szClass, "old")) return 17;
    if (Q_stristr(szClass, "leet") && Q_stristr(szClass, "old")) return 18;
    if (Q_stristr(szClass, "bare")) return 19;
    
    return -1;
}

const char* CInventoryPanel::GetConVarNameForGloveSkin(int iGloveIndex, int team)
{
    static char szConVarName[64];
    
    if (iGloveIndex < 0 || iGloveIndex >= 20)
        return nullptr;
    
    const char* teamPrefix = (team == TEAM_CT) ? "loadout_gloves_ct_skin" : "loadout_gloves_t_skin";
    Q_snprintf(szConVarName, sizeof(szConVarName), "%s%d", teamPrefix, iGloveIndex);
    
    return szConVarName;
}

void CInventoryPanel::SaveGloveSkinToLoadout(int iPaintKit, int iGloveIndex, int teamFlags)
{
    if (iGloveIndex < 0 || iGloveIndex >= 20)
        return;
    
    if (teamFlags & EQUIP_TEAM_CT)
    {
        const char *pszConVarName = GetConVarNameForGloveSkin(iGloveIndex, TEAM_CT);
        if (pszConVarName)
        {
            ConVar *pSkinConVar = cvar->FindVar(pszConVarName);
            if (pSkinConVar)
                pSkinConVar->SetValue(iPaintKit);
        }
        
        ConVar *pGloveSlotCT = cvar->FindVar("loadout_slot_gloves_ct");
        if (pGloveSlotCT)
            pGloveSlotCT->SetValue(iGloveIndex);
    }
    
    if (teamFlags & EQUIP_TEAM_T)
    {
        const char *pszConVarName = GetConVarNameForGloveSkin(iGloveIndex, TEAM_TERRORIST);
        if (pszConVarName)
        {
            ConVar *pSkinConVar = cvar->FindVar(pszConVarName);
            if (pSkinConVar)
                pSkinConVar->SetValue(iPaintKit);
        }
        
        ConVar *pGloveSlotT = cvar->FindVar("loadout_slot_gloves_t");
        if (pGloveSlotT)
            pGloveSlotT->SetValue(iGloveIndex);
    }
}

void CInventoryPanel::UnequipGloves(int iGloveIndex)
{
    if (iGloveIndex < 0 || iGloveIndex >= 20)
        return;
    
    const char *pszConVarNameCT = GetConVarNameForGloveSkin(iGloveIndex, TEAM_CT);
    if (pszConVarNameCT)
    {
        ConVar *pSkinConVarCT = cvar->FindVar(pszConVarNameCT);
        if (pSkinConVarCT)
            pSkinConVarCT->SetValue(0);
    }
    
    const char *pszConVarNameT = GetConVarNameForGloveSkin(iGloveIndex, TEAM_TERRORIST);
    if (pszConVarNameT)
    {
        ConVar *pSkinConVarT = cvar->FindVar(pszConVarNameT);
        if (pSkinConVarT)
            pSkinConVarT->SetValue(0);
    }
    
    ConVar *pGloveSlotCT = cvar->FindVar("loadout_slot_gloves_ct");
    if (pGloveSlotCT)
        pGloveSlotCT->SetValue(0);
    
    ConVar *pGloveSlotT = cvar->FindVar("loadout_slot_gloves_t");
    if (pGloveSlotT)
        pGloveSlotT->SetValue(0);
}
