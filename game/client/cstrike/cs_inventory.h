#ifndef CS_INVENTORY_H
#define CS_INVENTORY_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/ScrollableEditablePanel.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "cs_skin_database.h"
#include "vgui_item_context_menu.h"
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>

#pragma message("Using custom vgui::CInventoryPanel from cs_inventory.h")
namespace vgui
{

enum InventoryItemType_t
{
    INVENTORY_ITEM_WEAPON_SKIN = 0,
    INVENTORY_ITEM_KNIFE,
    INVENTORY_ITEM_GLOVES,
    INVENTORY_ITEM_AGENT_CT,
    INVENTORY_ITEM_AGENT_T,
    INVENTORY_ITEM_MUSIC
};

class CInventoryItemPanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CInventoryItemPanel, vgui::EditablePanel);

public:
    CInventoryItemPanel(Panel *parent, const char *panelName, const SkinDefinition_t *pSkinDef, 
                       InventoryItemType_t itemType = INVENTORY_ITEM_WEAPON_SKIN, int iSpecialIndex = 0, 
                       const char *pszIconPath = nullptr, const char *pszCustomName = nullptr);
    
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void PerformLayout();
    virtual void OnMousePressed(vgui::MouseCode code);
    virtual void OnCursorEntered();
    virtual void OnCursorExited();
    virtual void Paint();
    
    const SkinDefinition_t* GetSkinDefinition() const { return m_pSkinDef; }
    int GetPaintKit() const { return m_pSkinDef ? m_pSkinDef->iPaintKit : 0; }
    InventoryItemType_t GetItemType() const { return m_ItemType; }
    int GetSpecialIndex() const { return m_iSpecialIndex; }
    
    void UpdateEquipIndicators();
    void SetExternalNameLabel(vgui::Label *pLabel) { m_pExternalNameLabel = pLabel; }
    void SetExternalModelLabel(vgui::Label *pLabel) { m_pExternalModelLabel = pLabel; }
    void SetExternalSkinLabel(vgui::Label *pLabel) { m_pExternalSkinLabel = pLabel; }

private:
    void UpdateDisplay();
    Color GetRarityColor() const;
    int GetEquipFlags() const;
    
    const SkinDefinition_t *m_pSkinDef;
    InventoryItemType_t m_ItemType;
    int m_iSpecialIndex;
    char m_szCustomIconPath[256];
    char m_szCustomName[128];
    vgui::Label *m_pWeaponLabel;
    vgui::Label *m_pExternalNameLabel;
    vgui::Label *m_pExternalModelLabel;
    vgui::Label *m_pExternalSkinLabel;
    vgui::ImagePanel *m_pIconImage;
    vgui::Panel *m_pRarityBar;
    
    bool m_bMouseOver;
    int m_iEquipFlags;
};

class CInventoryPanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CInventoryPanel, vgui::EditablePanel);

public:
    CInventoryPanel(Panel *parent, const char *panelName);
    virtual ~CInventoryPanel();
    
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void PerformLayout();
    virtual void OnCommand(const char *command);
    virtual void OnThink();
    
    void LoadAllSkins();
    void LoadSkinsForWeapon(CSWeaponID weaponID);
    void LoadGloves();
    void LoadAgents();
    void LoadMusic();
    void ClearInventory();
    
    void SetWeaponFilter(CSWeaponID weaponID);
    void SetRarityFilter(ESkinRarity rarity);
    void SetItemTypeFilter(InventoryItemType_t itemType);
    void ClearFilters();
    void RepositionItems();
    
    MESSAGE_FUNC_PARAMS(OnItemSelected, "ItemSelected", data);
    MESSAGE_FUNC_PARAMS(OnMenuCommand, "MenuCommand", data);

protected:
    void RebuildInventory();
    void AddSkinToInventory(const SkinDefinition_t *pSkinDef);
    void AddGlovesToInventory(int iGloveSlot, const char *pszGloveName, const char *pszIconPath);
    void AddAgentToInventory(int iAgentIndex, int team, const char *pszAgentName, const char *pszIconPath);
    void AddMusicToInventory(int iMusicIndex, const char *pszMusicName, const char *pszIconPath);
    
    void SaveItemToLoadout(int iPaintKit, CSWeaponID weaponID, InventoryItemType_t itemType, int iSpecialIndex, int teamFlags);
    void UnequipItem(CSWeaponID weaponID, InventoryItemType_t itemType, int iSpecialIndex);
    
    void SaveGloveSkinToLoadout(int iPaintKit, int iGloveIndex, int teamFlags);
    void UnequipGloves(int iGloveIndex);
    
    const char* GetConVarNameForWeapon(CSWeaponID weaponID);
    const char* GetConVarNameForKnife(CSWeaponID weaponID);
    const char* GetConVarNameForGloveSkin(int iGloveIndex, int team);
    int GetGloveIndexFromSkinDef(const SkinDefinition_t *pSkinDef);
    
    vgui::ScrollableEditablePanel *m_pScrollablePanel;
    vgui::EditablePanel *m_pItemContainer;
    CUtlVector<CInventoryItemPanel*> m_Items;
    CUtlVector<vgui::Label*> m_ItemLabels;
    CUtlVector<vgui::Label*> m_ModelLabels;
    CUtlVector<vgui::Label*> m_SkinLabels;
    CItemContextMenu *m_pContextMenu;
    
    int m_iItemWidth;
    int m_iItemHeight;
    int m_iItemSpacing;
    int m_iItemsPerRow;
    int m_iLeftMargin;
    int m_iRightMargin;
    int m_iLabelHeight;
    
    CSWeaponID m_FilterWeaponID;
    ESkinRarity m_FilterRarity;
    InventoryItemType_t m_FilterItemType;
    bool m_bUseWeaponFilter;
    bool m_bUseRarityFilter;
    bool m_bUseItemTypeFilter;
    bool m_bFirstLayout;
    bool m_bNeedsRepositioning;
};

} // namespace vgui

#endif // CS_INVENTORY_H
