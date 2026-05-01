#ifndef VGUI_ITEM_CONTEXT_MENU_H
#define VGUI_ITEM_CONTEXT_MENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Menu.h>
#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include "weapon_csbase.h"

enum ItemType_t
{
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_KNIFE,
    ITEM_TYPE_GLOVES,
    ITEM_TYPE_AGENT_CT,
    ITEM_TYPE_AGENT_T,
    ITEM_TYPE_MUSIC
};

class CItemContextMenu : public vgui::Menu
{
    DECLARE_CLASS_SIMPLE(CItemContextMenu, vgui::Menu);

public:
    CItemContextMenu(vgui::Panel *parent, const char *panelName);
    virtual ~CItemContextMenu();
    
    void ShowForItem(int iPaintKit, CSWeaponID weaponID, ItemType_t itemType, int iSpecialIndex, 
                    const char *pszWeaponName, const char *pszSkinName, int x, int y);
    virtual void PerformLayout() OVERRIDE;
    
    int GetCurrentPaintKit() const { return m_iCurrentPaintKit; }
    CSWeaponID GetCurrentWeaponID() const { return m_CurrentWeaponID; }
    ItemType_t GetItemType() const { return m_ItemType; }
    int GetSpecialIndex() const { return m_iSpecialIndex; }
    const char* GetCurrentWeaponName() const { return m_szWeaponName; }
    const char* GetCurrentSkinName() const { return m_szSkinName; }

private:
    void BuildMenuForItem();
    
    int m_iCurrentPaintKit;
    CSWeaponID m_CurrentWeaponID;
    ItemType_t m_ItemType;
    int m_iSpecialIndex;
    char m_szWeaponName[64];
    char m_szSkinName[128];
};

#endif // VGUI_ITEM_CONTEXT_MENU_H
