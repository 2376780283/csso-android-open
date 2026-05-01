#include "cbase.h"
#include "vgui_skin_editor.h"
#include "cs_inventory.h"
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include "filesystem.h"
#include "cs_skin_database.h"

#include "tier0/icommandline.h"
#include "vgui/ISurface.h"


using namespace vgui;

CSkinEditorPanel* g_pSkinEditor = nullptr;

//=============================================================================
// CSkinEditorPanel - Конструктор
//=============================================================================
CSkinEditorPanel::CSkinEditorPanel(Panel* parent) : BaseClass(parent, "SkinEditorPanel")
{    
    g_pSkinEditor = this;        
    
    // Инициализируем указатели
    m_pInventoryPanel = nullptr;
    m_pShowAllButton = nullptr;
    m_pWeaponFilter = nullptr;
    m_pRarityFilter = nullptr;
    m_bControlsCreated = false;
    
    CreateControls();
    
    InvalidateLayout(true, true);
}

CSkinEditorPanel::~CSkinEditorPanel()
{
    // Элементы удаляются автоматически через VGUI
}

void CSkinEditorPanel::CreateControls()
{
    if (m_bControlsCreated)
        return;
        
    m_bControlsCreated = true;
    
    // Создаем панель инвентаря
    m_pInventoryPanel = new vgui::CInventoryPanel(this, "InventoryPanel");
    m_pInventoryPanel->AddActionSignalTarget(this);
    
    // Создаем кнопку "Показать все"
    m_pShowAllButton = new vgui::Button(this, "ShowAllButton", "Show All Skins", this, "ShowAll");
    
    // Создаем комбобокс выбора оружия
    m_pWeaponFilter = new vgui::ComboBox(this, "WeaponFilter", 15, false);
    m_pWeaponFilter->AddItem("All Weapons", new KeyValues("weapon", "id", WEAPON_NONE));
    m_pWeaponFilter->AddItem("AK-47", new KeyValues("weapon", "id", WEAPON_AK47));
    m_pWeaponFilter->AddItem("M4A4", new KeyValues("weapon", "id", WEAPON_M4A4));
    m_pWeaponFilter->AddItem("M4A1-S", new KeyValues("weapon", "id", WEAPON_M4A1));
    m_pWeaponFilter->AddItem("AWP", new KeyValues("weapon", "id", WEAPON_AWP));
    m_pWeaponFilter->AddItem("Desert Eagle", new KeyValues("weapon", "id", WEAPON_DEAGLE));
    m_pWeaponFilter->AddItem("Glock-18", new KeyValues("weapon", "id", WEAPON_GLOCK));
    m_pWeaponFilter->AddItem("USP-S", new KeyValues("weapon", "id", WEAPON_USP));
    m_pWeaponFilter->AddItem("P2000", new KeyValues("weapon", "id", WEAPON_HKP2000));
    m_pWeaponFilter->AddItem("P250", new KeyValues("weapon", "id", WEAPON_P250));
    m_pWeaponFilter->AddItem("Five-SeveN", new KeyValues("weapon", "id", WEAPON_FIVESEVEN));
    m_pWeaponFilter->AddItem("Tec-9", new KeyValues("weapon", "id", WEAPON_TEC9));
    m_pWeaponFilter->AddItem("CZ75-Auto", new KeyValues("weapon", "id", WEAPON_CZ75A));
    m_pWeaponFilter->AddItem("Dual Berettas", new KeyValues("weapon", "id", WEAPON_ELITE));
    m_pWeaponFilter->AddItem("Nova", new KeyValues("weapon", "id", WEAPON_NOVA));
    m_pWeaponFilter->AddItem("XM1014", new KeyValues("weapon", "id", WEAPON_XM1014));
    m_pWeaponFilter->AddItem("MAG-7", new KeyValues("weapon", "id", WEAPON_MAG7));
    m_pWeaponFilter->AddItem("Sawed-Off", new KeyValues("weapon", "id", WEAPON_SAWEDOFF));
    m_pWeaponFilter->AddItem("M249", new KeyValues("weapon", "id", WEAPON_M249));
    m_pWeaponFilter->AddItem("Negev", new KeyValues("weapon", "id", WEAPON_NEGEV));
    m_pWeaponFilter->AddItem("MAC-10", new KeyValues("weapon", "id", WEAPON_MAC10));
    m_pWeaponFilter->AddItem("MP9", new KeyValues("weapon", "id", WEAPON_MP9));
    m_pWeaponFilter->AddItem("MP7", new KeyValues("weapon", "id", WEAPON_MP7));
    m_pWeaponFilter->AddItem("MP5-SD", new KeyValues("weapon", "id", WEAPON_MP5SD));
    m_pWeaponFilter->AddItem("UMP-45", new KeyValues("weapon", "id", WEAPON_UMP45));
    m_pWeaponFilter->AddItem("P90", new KeyValues("weapon", "id", WEAPON_P90));
    m_pWeaponFilter->AddItem("PP-Bizon", new KeyValues("weapon", "id", WEAPON_BIZON));
    m_pWeaponFilter->AddItem("Galil AR", new KeyValues("weapon", "id", WEAPON_GALILAR));
    m_pWeaponFilter->AddItem("FAMAS", new KeyValues("weapon", "id", WEAPON_FAMAS));
    m_pWeaponFilter->AddItem("AUG", new KeyValues("weapon", "id", WEAPON_AUG));
    m_pWeaponFilter->AddItem("SG 553", new KeyValues("weapon", "id", WEAPON_SG556));
    m_pWeaponFilter->AddItem("SSG 08", new KeyValues("weapon", "id", WEAPON_SSG08));
    m_pWeaponFilter->AddItem("SCAR-20", new KeyValues("weapon", "id", WEAPON_SCAR20));
    m_pWeaponFilter->AddItem("G3SG1", new KeyValues("weapon", "id", WEAPON_G3SG1));
    m_pWeaponFilter->AddItem("R8 Revolver", new KeyValues("weapon", "id", WEAPON_REVOLVER));
    
    // Активируем первый элемент (All Weapons)
    m_pWeaponFilter->ActivateItemByRow(0);
    m_pWeaponFilter->AddActionSignalTarget(this);
    
    // Создаем комбобокс редкости
    m_pRarityFilter = new vgui::ComboBox(this, "RarityFilter", 10, false);
    m_pRarityFilter->AddItem("All Rarities", new KeyValues("rarity", "id", -1));
    m_pRarityFilter->AddItem("Common", new KeyValues("rarity", "id", SKIN_RARITY_COMMON));
    m_pRarityFilter->AddItem("Uncommon", new KeyValues("rarity", "id", SKIN_RARITY_UNCOMMON));
    m_pRarityFilter->AddItem("Rare", new KeyValues("rarity", "id", SKIN_RARITY_RARE));
    m_pRarityFilter->AddItem("Mythical", new KeyValues("rarity", "id", SKIN_RARITY_MYTHICAL));
    m_pRarityFilter->AddItem("Legendary", new KeyValues("rarity", "id", SKIN_RARITY_LEGENDARY));
    m_pRarityFilter->AddItem("Ancient", new KeyValues("rarity", "id", SKIN_RARITY_ANCIENT));
    m_pRarityFilter->AddItem("Contraband", new KeyValues("rarity", "id", SKIN_RARITY_CONTRABAND));
    
    // Активируем первый элемент (All Rarities)
    m_pRarityFilter->ActivateItemByRow(0);
    m_pRarityFilter->AddActionSignalTarget(this);
    
    DevMsg("[SkinEditor] Controls created successfully\n");
}

void CSkinEditorPanel::PerformLayout()
{
    BaseClass::PerformLayout();
    
    if (NeedProportional())
        SetProportional(true);
    
    int wide, tall;
    GetSize(wide, tall);
    
    // Получаем масштаб для адаптации под разрешение
    int screenWidth, screenHeight;
    vgui::surface()->GetScreenSize(screenWidth, screenHeight);
    float scale = 0;
    
    // Панель фильтров сверху
    int filterPanelHeight = 40;
    int padding = 10;
    int buttonWidth = 120;
    int comboWidth = 100;
    int comboHeight = 25;
    
    if (IsProportional())
    {
        filterPanelHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), filterPanelHeight);
        padding = scheme()->GetProportionalScaledValueEx(GetScheme(), padding);
        buttonWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), buttonWidth);
        comboWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), comboWidth);
        comboHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), comboHeight);
    }
    
    int xPos = padding;
    
    if (m_pShowAllButton)
    {
        m_pShowAllButton->SetBounds(xPos, padding, buttonWidth, comboHeight);
        xPos += buttonWidth + padding;
    }
    
    if (m_pWeaponFilter)
    {
        m_pWeaponFilter->SetBounds(xPos, padding, comboWidth, comboHeight);
        xPos += comboWidth + padding;
    }
    
    if (m_pRarityFilter)
    {
        m_pRarityFilter->SetBounds(xPos, padding, comboWidth, comboHeight);
    }
    
    // Инвентарь занимает всё оставшееся пространство
    if (m_pInventoryPanel)
    {
        m_pInventoryPanel->SetBounds(0, filterPanelHeight + padding, 
                                    wide, 
                                    tall - filterPanelHeight - padding);
    }
}

void CSkinEditorPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    // SetBgColor(pScheme->GetColor("Frame.BgColor", Color(50, 50, 50, 255)));
}

void CSkinEditorPanel::OnCommand(const char *command)
{
    if (Q_stricmp(command, "ShowAll") == 0)
    {
        if (m_pInventoryPanel)
        {
            m_pInventoryPanel->ClearFilters();
        }
        
        if (m_pWeaponFilter)
            m_pWeaponFilter->ActivateItemByRow(0);
            
        if (m_pRarityFilter)
            m_pRarityFilter->ActivateItemByRow(0);
    }
    else {
        BaseClass::OnCommand(command);
    }
}

void CSkinEditorPanel::OnTextChanged(vgui::Panel *panel)
{
    if (!m_pInventoryPanel)
        return;
    
    if (panel == m_pWeaponFilter)
    {
        KeyValues *pUserData = m_pWeaponFilter->GetActiveItemUserData();
        
        if (pUserData)
        {
            int weaponID = pUserData->GetInt("id", WEAPON_NONE);
            
            if (weaponID != WEAPON_NONE)
            {
                m_pInventoryPanel->SetWeaponFilter((CSWeaponID)weaponID);
                DevMsg("[SkinEditor] Filtering by weapon ID: %d\n", weaponID);
            }
            else
            {
                m_pInventoryPanel->ClearFilters();
                
                // Восстанавливаем фильтр редкости если он был активен
                if (m_pRarityFilter && m_pRarityFilter->GetActiveItem() > 0)
                {
                    KeyValues *pRarityData = m_pRarityFilter->GetActiveItemUserData();
                    if (pRarityData)
                    {
                        int rarityID = pRarityData->GetInt("id", -1);
                        if (rarityID >= 0)
                        {
                            m_pInventoryPanel->SetRarityFilter((ESkinRarity)rarityID);
                        }
                    }
                }
            }
        }
        else
        {
            m_pInventoryPanel->ClearFilters();
        }
    }
    else if (panel == m_pRarityFilter)
    {
        KeyValues *pUserData = m_pRarityFilter->GetActiveItemUserData();
        
        if (pUserData)
        {
            int rarityID = pUserData->GetInt("id", -1);
            
            if (rarityID >= 0)
            {
                m_pInventoryPanel->SetRarityFilter((ESkinRarity)rarityID);
                DevMsg("[SkinEditor] Filtering by rarity: %d\n", rarityID);
            }
            else
            {
                m_pInventoryPanel->ClearFilters();
                
                // Восстанавливаем фильтр оружия если он был активен
                if (m_pWeaponFilter && m_pWeaponFilter->GetActiveItem() > 0)
                {
                    KeyValues *pWeaponData = m_pWeaponFilter->GetActiveItemUserData();
                    if (pWeaponData)
                    {
                        int weaponID = pWeaponData->GetInt("id", WEAPON_NONE);
                        if (weaponID != WEAPON_NONE)
                        {
                            m_pInventoryPanel->SetWeaponFilter((CSWeaponID)weaponID);
                        }
                    }
                }
            }
        }
        else
        {
            m_pInventoryPanel->ClearFilters();
        }
    }
}

void CSkinEditorPanel::OnSkinSelected(KeyValues *data)
{
    int iPaintKit = data->GetInt("paintkit", 0);
    
    if (iPaintKit > 0)
    {
        const SkinDefinition_t *pSkinDef = g_SkinDatabase.FindSkinByPaintKit(iPaintKit);
        
        if (pSkinDef)
        {
            Msg("[Inventory] Equipped skin: %s (PaintKit: %d)\n", 
                pSkinDef->szName, 
                iPaintKit);
        }
    }
}

void CSkinEditorPanel::OnThink()
{
    BaseClass::OnThink();
}

//=============================================================================
// Console Commands
//=============================================================================
CON_COMMAND(open_inventory, "Open skin inventory")
{
    // Now part of ModOptionsDialog
    engine->ClientCmd_Unrestricted("gameui_activate; open_mod_options_dialog");
}

CON_COMMAND(close_inventory, "Close skin inventory")
{
    // Now part of ModOptionsDialog
    engine->ClientCmd_Unrestricted("gameui_hide");
}

CON_COMMAND(inventory_reload, "Reload skin inventory")
{
    if (g_pSkinEditor && g_pSkinEditor->m_pInventoryPanel)
    {
        g_pSkinEditor->m_pInventoryPanel->LoadAllSkins();
        Msg("Inventory reloaded\n");
    }
    else
    {
        Warning("Inventory not open\n");
    }
}
