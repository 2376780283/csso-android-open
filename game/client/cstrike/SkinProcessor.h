#ifndef SKIN_PROCESSOR_H
#define SKIN_PROCESSOR_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "cs_skin_shareddefs.h"

class IMaterial;
class ITexture;
class C_BaseCombatWeapon;
class ConVar;

// Forward declarations - используем существующее определение из cs_weapon_parse.h
#include "cs_weapon_parse.h"

// =============================================================================
// WEAPON SKIN CACHE STRUCTURE
// =============================================================================

struct WeaponSkinCache_t
{
    const char*     pszWeaponClass;     // Weapon class name
    ConVar*         pSkinConVar;        // ConVar with paint kit ID
    IMaterial*      pMaterial;          // Cached material
    int             iLastPaintKit;      // Last loaded paint kit

    WeaponSkinCache_t()
        : pszWeaponClass(nullptr)
        , pSkinConVar(nullptr)
        , pMaterial(nullptr)
        , iLastPaintKit(0)
    {
    }

    WeaponSkinCache_t(const char* szClass, ConVar* pConVar)
        : pszWeaponClass(szClass)
        , pSkinConVar(pConVar)
        , pMaterial(nullptr)
        , iLastPaintKit(0)
    {
    }
};

// =============================================================================
// SKIN PROCESSOR CLASS
// =============================================================================

class CSkinProcessor
{
public:
    CSkinProcessor();
    ~CSkinProcessor();

    // Инициализация
    bool Initialize();
    void Shutdown();

    // Получение материала по оружию (PaintKit система)
    IMaterial* GetSkinMaterial(C_BaseCombatWeapon* pWeapon);  // Основной метод
    IMaterial* GetSkinMaterialForWeapon(C_BaseCombatWeapon* pWeapon);
    IMaterial* GetSkinMaterialForClass(const char* pszClass);
    IMaterial* GetWeaponMaterial(CSWeaponID weaponID);
    
    // Получение иконки скина
    ITexture* GetSkinIconForWeapon(C_BaseCombatWeapon* pWeapon);
    ITexture* GetSkinIcon(CSWeaponID weaponID);
    
    // Получение по SkinInfo
    IMaterial* GetSkinMaterialBySkinInfo(CSWeaponID weaponID, const SkinInfo_t& skinInfo);

    // Glove skins (пока остаются через ConVar)
    const char* GetGloveSkin(int gloveID);
    int GetGloveCount() const;

    // Buy menu support
    void SetActiveBuyMenuWeaponMaterial(IMaterial* pMat);
    void ResetActiveBuyMenuWeaponMaterial();
    IMaterial* GetActiveBuyMenuWeaponMaterial() const;

    // Utility
    void Clear();
    void ReloadAllMaterials();
    WeaponSkinCache_t* FindSkinEntry(const char* pszClass);

private:
    // Initialization
    void InitializeWeaponSkins();
    void InitializeGloveSkins();
    void AddWeaponSkin(const char* pszClass, ConVar* pConVar);

    // Material management
    IMaterial* LoadMaterialByPaintKit(WeaponSkinCache_t& skin, int iPaintKit);
    void ReleaseMaterial(WeaponSkinCache_t* pSkin);

    // Data members
    CUtlVector<WeaponSkinCache_t>   m_WeaponSkins;
    CUtlVector<ConVar*>             m_GloveSkinConVars;
    IMaterial*                      m_pActiveBuyMenuWeaponMaterial;
    
    bool                            m_bInitialized;
};

// Global instance
extern CSkinProcessor g_SkinProcessor;

#endif // SKIN_PROCESSOR_H