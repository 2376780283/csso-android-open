#ifndef VGUI_ITEM_CONTEXT_MENU_H
#define VGUI_ITEM_CONTEXT_MENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Menu.h>
#include "weapon_csbase"

class CItemContextMenu : public vgui::Menu
{
    DECLARE_CLASS_SIMPLE(CItemContextMenu, vgui::Menu);

public:
    CItemContextMenu(vgui::Panel *parent, const char *panelName);
    virtual ~CItemContextMenu();
    
    // Показать меню для конкретного айтема
    void ShowForItem(int iPaintKit, CSWeaponID weaponID, const char *pszWeaponName, const char *pszSkinName, int x, int y);
    
    int GetCurrentPaintKit() const { return m_iCurrentPaintKit; }
    CSWeaponID GetCurrentWeaponID() const { return m_CurrentWeaponID; }
    const char* GetCurrentWeaponName() const { return m_szWeaponName; }
    const char* GetCurrentSkinName() const { return m_szSkinName; }

private:
    int m_iCurrentPaintKit;
    CSWeaponID m_CurrentWeaponID;
    char m_szWeaponName[64];
    char m_szSkinName[128];
};

#endif // VGUI_ITEM_CONTEXT_MENU_H
