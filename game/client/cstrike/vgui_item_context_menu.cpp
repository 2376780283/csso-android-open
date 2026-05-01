#include "cbase.h"
#include "vgui_item_context_menu.h"
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <KeyValues.h>
#include "tier0/icommandline.h"

using namespace vgui;

CItemContextMenu::CItemContextMenu(Panel *parent, const char *panelName)
    : BaseClass(parent, panelName)
{
    m_iCurrentPaintKit = 0;
    m_CurrentWeaponID = WEAPON_NONE;
    m_ItemType = ITEM_TYPE_WEAPON;
    m_iSpecialIndex = 0;
    m_szWeaponName[0] = '\0';
    m_szSkinName[0] = '\0';
}

CItemContextMenu::~CItemContextMenu()
{
}

void CItemContextMenu::PerformLayout()
{
    BaseClass::PerformLayout();
    
    if (NeedProportional())
        SetProportional(true);
    
    int itemCount = GetItemCount();
    int menuWidth = 150;
    int itemHeight = 20;
    int menuHeight = itemCount * itemHeight + 4;
    
    if (IsProportional())
    {
        menuWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), menuWidth);
        itemHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), itemHeight);
        menuHeight = itemCount * itemHeight + scheme()->GetProportionalScaledValueEx(GetScheme(), 4);
    }
    
    SetSize(menuWidth, menuHeight);
}

void CItemContextMenu::BuildMenuForItem()
{
    DeleteAllItems();
    
    switch (m_ItemType)
    {
        case ITEM_TYPE_KNIFE:
        case ITEM_TYPE_GLOVES:
            AddMenuItem("Equip for all", new KeyValues("MenuCommand", "command", "equip_all"), GetParent());
            AddMenuItem("Equip for CT", new KeyValues("MenuCommand", "command", "equip_ct"), GetParent());
            AddMenuItem("Equip for T", new KeyValues("MenuCommand", "command", "equip_t"), GetParent());
            AddMenuItem("Unequip", new KeyValues("MenuCommand", "command", "unequip"), GetParent());
            break;
            
        case ITEM_TYPE_AGENT_CT:
        case ITEM_TYPE_AGENT_T:
        case ITEM_TYPE_MUSIC:
        case ITEM_TYPE_WEAPON:
        default:
            AddMenuItem("Equip", new KeyValues("MenuCommand", "command", "equip"), GetParent());
            AddMenuItem("Unequip", new KeyValues("MenuCommand", "command", "unequip"), GetParent());
            break;
    }
}

void CItemContextMenu::ShowForItem(int iPaintKit, CSWeaponID weaponID, ItemType_t itemType, int iSpecialIndex,
                                   const char *pszWeaponName, const char *pszSkinName, int x, int y)
{
    m_iCurrentPaintKit = iPaintKit;
    m_CurrentWeaponID = weaponID;
    m_ItemType = itemType;
    m_iSpecialIndex = iSpecialIndex;
    
    int screenWidth, screenHeight;
    vgui::surface()->GetScreenSize(screenWidth, screenHeight);
    
    Q_strncpy(m_szWeaponName, pszWeaponName, sizeof(m_szWeaponName));
    Q_strncpy(m_szSkinName, pszSkinName, sizeof(m_szSkinName));
    
    BuildMenuForItem();
    
    int menuWidth, menuHeight;
    GetSize(menuWidth, menuHeight);
    
    if (x + menuWidth > screenWidth)
        x = screenWidth - menuWidth;
    
    if (y + menuHeight > screenHeight)
        y = screenHeight - menuHeight;
    
    SetVisible(true);
    SetPos(x, y);
    MoveToFront();
    RequestFocus();
}
