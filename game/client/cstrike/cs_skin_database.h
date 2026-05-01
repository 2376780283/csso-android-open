#ifndef CS_SKIN_DATABASE_H
#define CS_SKIN_DATABASE_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "utlmap.h"
#include "utlstring.h"
#include "cs_skin_shareddefs.h"
#include "cs_weapon_parse.h"  // Для CSWeaponID

class IMaterial;
class ITexture;
class KeyValues;

#define MAX_DESCRIPTION_NAME 512
#define MAX_SKIN_MATERIALS 8  // Максимальное количество материалов в одном PaintKit

// =============================================================================
// ITEM TYPE ENUM (NEW)
// =============================================================================

enum ESkinItemType
{
    SKIN_ITEM_WEAPON = 0,   
    SKIN_ITEM_GLOVES = 1,
    SKIN_ITEM_KNIFE = 2,
    
    SKIN_ITEM_COUNT
};

// =============================================================================
// SKIN DEFINITION (Client-side data)
// =============================================================================

struct SkinDefinition_t
{
    int             iPaintKit;
    CSWeaponID      weaponID;
    ESkinItemType   itemType;
    char            szItemClass[64];
    char            szName[MAX_SKIN_NAME];
    char            szDescription[MAX_DESCRIPTION_NAME];
    char            szIconPath[256];
    ESkinRarity     rarity;
    
    struct MaterialData_t
    {
        int             iMaterialIndex;         
        char            szBaseMaterial[256];    
        
        struct VMTParams_t
        {
            // ========================================
            // CORE PARAMETERS
            // ========================================
            char szShader[64];              // Shader name
            
            // ========================================
            // TEXTURE PARAMETERS
            // ========================================
            char szBaseTexture[256];        // $basetexture
            char szBaseTexture2[256];       // $basetexture2
            char szBumpMap[256];            // $bumpmap (normal map)
            char szBumpMap2[256];           // $bumpmap2
            char szDetailTexture[256];      // $detail
            char szEnvMap[256];             // $envmap
            char szPhongExponentTexture[256]; // $phongexponenttexture
            char szPhongWarpTexture[256];   // $phongwarptexture
            
            // ========================================
            // PHONG LIGHTING
            // ========================================
            int   bPhong;                   // $phong (0/1/-1 = not set)
            float flPhongBoost;             // $phongboost
            float flPhongFresnelRanges[3];  // $phongfresnelranges "[min mid max]"
            float flPhongAlbedoTint;        // $phongalbedotint
            int   bPhongAlbedoBoost;        // $phongalbedoboost
            float flPhongExponent;          // $phongexponent
            
            // ========================================
            // ENVIRONMENT MAPPING
            // ========================================
            float flEnvMapTint[3];          // $envmaptint "[r g b]"
            float flEnvMapSaturation;       // $envmapsaturation
            float flEnvMapContrast;         // $envmapcontrast
            float flEnvMapFresnel;          // $envmapfresnel
            float flFresnelReflection;      // $fresnelreflection
            
            // ========================================
            // ALPHA & TRANSPARENCY
            // ========================================
            int   bAlphaTest;               // $alphatest
            float flAlphaTestReference;     // $alphatestreference
            int   bTranslucent;             // $translucent
            int   bAdditive;                // $additive
            
            // ========================================
            // TEXTURE MODIFIERS
            // ========================================
            float flBaseTextureTransform[16]; // $basetexturetransform (matrix)
            float flBumpTransform[16];        // $bumptransform
            int   bBaseTextureNoEnvMap;     // $basemapalphaphongmask
            int   bNormalMapAlphaPhongMask; // $normalmapalphaenvmapmask
            int   bBaseAlphaEnvMapMask;     // $basealphaenvmapmask
            
            // ========================================
            // DETAIL TEXTURE
            // ========================================
            float flDetailScale;            // $detailscale
            int   iDetailBlendMode;         // $detailblendmode
            float flDetailBlendFactor;      // $detailblendfactor
            
            // ========================================
            // RIM LIGHTING
            // ========================================
            int   bRimLight;                // $rimlight
            float flRimLightExponent;       // $rimlightexponent
            float flRimLightBoost;          // $rimlightboost
            
            // ========================================
            // SELF ILLUMINATION
            // ========================================
            int   bSelfIllum;               // $selfillum
            float flSelfIllumTint[3];       // $selfillumtint "[r g b]"
            float flSelfIllumFresnelMinMaxExp[3]; // $selfillum_envmapmask_alpha
            
            // ========================================
            // COLOR MODULATION
            // ========================================
            float flColor[3];               // $color "[r g b]"
            float flColor2[3];              // $color2
            
            // ========================================
            // MISC RENDERING
            // ========================================
            int   bNoCull;                  // $nocull
            int   bNoDecal;                 // $nodecal
            int   bHalfLambert;             // $halflambert
            
            // ========================================
            // CUSTOM CS:GO SKIN PARAMETERS
            // ========================================
            float flWear;                   // $wear (custom)
            int   iSeed;                    // $seed (custom)
            float flPatternScale;           // $patternscale (custom)
            float flPatternRotate;          // $patternrotate (custom)
            
            VMTParams_t()
            {
                // Core
                Q_strncpy(szShader, "VertexLitGeneric", sizeof(szShader));
                
                // Textures 
                szBaseTexture[0] = '\0';
                szBaseTexture2[0] = '\0';
                szBumpMap[0] = '\0';
                szBumpMap2[0] = '\0';
                szDetailTexture[0] = '\0';
                szEnvMap[0] = '\0';
                szPhongExponentTexture[0] = '\0';
                szPhongWarpTexture[0] = '\0';
                
                // Phong 
                bPhong = -1;
                flPhongBoost = -1.0f;
                flPhongFresnelRanges[0] = -1.0f;
                flPhongFresnelRanges[1] = -1.0f;
                flPhongFresnelRanges[2] = -1.0f;
                flPhongAlbedoTint = -1.0f;
                bPhongAlbedoBoost = -1;
                flPhongExponent = -1.0f;
                
                // EnvMap
                flEnvMapTint[0] = -1.0f;
                flEnvMapTint[1] = -1.0f;
                flEnvMapTint[2] = -1.0f;
                flEnvMapSaturation = -1.0f;
                flEnvMapContrast = -1.0f;
                flEnvMapFresnel = -1.0f;
                flFresnelReflection = -1.0f;
                
                // Alpha
                bAlphaTest = -1;
                flAlphaTestReference = -1.0f;
                bTranslucent = -1;
                bAdditive = -1;
                
                // Texture modifiers
                for (int i = 0; i < 16; i++)
                {
                    flBaseTextureTransform[i] = 0.0f;
                    flBumpTransform[i] = 0.0f;
                }
                bBaseTextureNoEnvMap = -1;
                bNormalMapAlphaPhongMask = -1;
                bBaseAlphaEnvMapMask = -1;
                
                // Detail
                flDetailScale = -1.0f;
                iDetailBlendMode = -1;
                flDetailBlendFactor = -1.0f;
                
                // Rim
                bRimLight = -1;
                flRimLightExponent = -1.0f;
                flRimLightBoost = -1.0f;
                
                // Self illum
                bSelfIllum = -1;
                flSelfIllumTint[0] = -1.0f;
                flSelfIllumTint[1] = -1.0f;
                flSelfIllumTint[2] = -1.0f;
                flSelfIllumFresnelMinMaxExp[0] = -1.0f;
                flSelfIllumFresnelMinMaxExp[1] = -1.0f;
                flSelfIllumFresnelMinMaxExp[2] = -1.0f;
                
                // Color
                flColor[0] = -1.0f;
                flColor[1] = -1.0f;
                flColor[2] = -1.0f;
                flColor2[0] = -1.0f;
                flColor2[1] = -1.0f;
                flColor2[2] = -1.0f;
                
                // Misc
                bNoCull = -1;
                bNoDecal = -1;
                bHalfLambert = -1;
                
                // Custom
                flWear = -1.0f;
                iSeed = -1;
                flPatternScale = -1.0f;
                flPatternRotate = -1.0f;
            }
        };
        
        VMTParams_t vmtParams;
        
        MaterialData_t()
        {
            iMaterialIndex = 0;
            szBaseMaterial[0] = '\0';
        }
    };
    
    CUtlVector<MaterialData_t> materials;
    
    SkinDefinition_t()
    {
        iPaintKit = 0;
        weaponID = WEAPON_NONE;
        itemType = SKIN_ITEM_WEAPON;
        szItemClass[0] = '\0';
        szName[0] = '\0';
        szDescription[0] = '\0';
        szIconPath[0] = '\0';
        rarity = SKIN_RARITY_COMMON;
    }
    
    SkinDefinition_t(const SkinDefinition_t& other)
    {
        iPaintKit = other.iPaintKit;
        weaponID = other.weaponID;
        itemType = other.itemType;
        Q_strncpy(szItemClass, other.szItemClass, sizeof(szItemClass));
        Q_strncpy(szName, other.szName, sizeof(szName));
        Q_strncpy(szDescription, other.szDescription, sizeof(szDescription));
        Q_strncpy(szIconPath, other.szIconPath, sizeof(szIconPath));
        rarity = other.rarity;
        
        materials.CopyArray(other.materials.Base(), other.materials.Count());
    }
    
    SkinDefinition_t& operator=(const SkinDefinition_t& other)
    {
        if (this != &other)
        {
            iPaintKit = other.iPaintKit;
            weaponID = other.weaponID;
            itemType = other.itemType;
            Q_strncpy(szItemClass, other.szItemClass, sizeof(szItemClass));
            Q_strncpy(szName, other.szName, sizeof(szName));
            Q_strncpy(szDescription, other.szDescription, sizeof(szDescription));
            Q_strncpy(szIconPath, other.szIconPath, sizeof(szIconPath));
            rarity = other.rarity;
            materials.RemoveAll();
            materials.CopyArray(other.materials.Base(), other.materials.Count());
        }
        return *this;
    }
    
    const char* GetDisplayName() const
    {
        return szName;
    }
    
    bool IsWeapon() const { return itemType == SKIN_ITEM_WEAPON; }
    bool IsGloves() const { return itemType == SKIN_ITEM_GLOVES; }
    bool IsKnife() const { return itemType == SKIN_ITEM_KNIFE; }
    
    int GetMaterialCount() const 
    { 
        return materials.Count(); 
    }
    
    const MaterialData_t* GetMaterialData(int index) const
    {
        if (index < 0 || index >= materials.Count())
            return nullptr;
        return &materials[index];
    }
    
    MaterialData_t* GetMaterialData(int index)
    {
        if (index < 0 || index >= materials.Count())
            return nullptr;
        return &materials[index];
    }
    
    const MaterialData_t* FindMaterialByIndex(int iMaterialIndex) const
    {
        FOR_EACH_VEC(materials, i)
        {
            if (materials[i].iMaterialIndex == iMaterialIndex)
                return &materials[i];
        }
        return nullptr;
    }
    
    MaterialData_t* FindMaterialByIndex(int iMaterialIndex)
    {
        FOR_EACH_VEC(materials, i)
        {
            if (materials[i].iMaterialIndex == iMaterialIndex)
                return &materials[i];
        }
        return nullptr;
    }
};

// =============================================================================
// SKIN DATABASE CLASS
// =============================================================================

class CCSkinDatabase
{
public:
    CCSkinDatabase();
    ~CCSkinDatabase();
    
    bool Initialize();
    bool LoadFromFile(const char* pszFilePath);
    void Shutdown();
    
    const SkinDefinition_t* FindSkinByPaintKit(int iPaintKit) const;
    const SkinDefinition_t* FindSkinByName(const char* pszName, CSWeaponID weaponID = WEAPON_NONE) const;
    
    const SkinDefinition_t* FindGlovesByClass(const char* pszGloveClass) const;
    
    void GetSkinsForWeapon(CSWeaponID weaponID, CUtlVector<const SkinDefinition_t*>& skins) const;
    void GetAllSkins(CUtlVector<const SkinDefinition_t*>& skins) const;
    
    void GetAllGloves(CUtlVector<const SkinDefinition_t*>& gloves) const;
    
    int GetSkinCount() const { return m_SkinDefinitions.Count(); }
    
    IMaterial* GetSkinMaterial(int iPaintKit, int iMaterialIndex = 0);
    void GetAllSkinMaterials(int iPaintKit, CUtlVector<IMaterial*>& materials);
    ITexture* GetSkinIcon(int iPaintKit);
    
    void ClearMaterialCache();
    void ClearIconCache();
    
    // Debug
    void PrintAllSkins() const;
    void PrintSkinsForWeapon(CSWeaponID weaponID) const;
    void PrintAllGloves() const;  // НОВОЕ
    
    bool SaveSkinToFile(const SkinDefinition_t& skin, const char* pszFilePath = "scripts/skins.txt");
    
    int GenerateNewPaintKitID() const;
    
    KeyValues* CloneBaseMaterialKeyValues(IMaterial* pBaseMat);
    void ApplyVMTPatch(KeyValues* pKV, const SkinDefinition_t::MaterialData_t::VMTParams_t& params);
    
private:
    bool ParseSkinDefinition(KeyValues* pKV, SkinDefinition_t& def);
    bool ParseMaterialData(KeyValues* pKV, SkinDefinition_t::MaterialData_t& matData);
    
    IMaterial* LoadMaterial(const SkinDefinition_t::MaterialData_t* pMatData);
    ITexture* LoadIcon(const SkinDefinition_t* pDef);
    void ReleaseMaterial(int iPaintKit, int iMaterialIndex);
    void ReleaseAllMaterials(int iPaintKit);
    void ReleaseIcon(int iPaintKit);
    void ParseVMTParams(KeyValues* pKV, SkinDefinition_t::MaterialData_t::VMTParams_t& params);
    
    struct MaterialCacheKey_t
    {
        int iPaintKit;
        int iMaterialIndex;
        
        bool operator<(const MaterialCacheKey_t& other) const
        {
            if (iPaintKit != other.iPaintKit)
                return iPaintKit < other.iPaintKit;
            return iMaterialIndex < other.iMaterialIndex;
        }
        
        bool operator==(const MaterialCacheKey_t& other) const
        {
            return iPaintKit == other.iPaintKit && iMaterialIndex == other.iMaterialIndex;
        }
    };
    
    CUtlMap<int, SkinDefinition_t>              m_SkinDefinitions;  // PaintKit -> Definition
    CUtlMap<MaterialCacheKey_t, IMaterial*>     m_MaterialCache;    // (PaintKit, MaterialIndex) -> Material
    CUtlMap<int, ITexture*>                     m_IconCache;        // PaintKit -> Icon
    
    struct WeaponSkinList_t
    {
        int weaponID;
        CUtlVector<int> paintKits;  
        
        WeaponSkinList_t()
        {
            weaponID = 0;
        }
        
        WeaponSkinList_t(const WeaponSkinList_t& other)
        {
            weaponID = other.weaponID;
            paintKits.CopyArray(other.paintKits.Base(), other.paintKits.Count());
        }
        
        WeaponSkinList_t& operator=(const WeaponSkinList_t& other)
        {
            weaponID = other.weaponID;
            paintKits.CopyArray(other.paintKits.Base(), other.paintKits.Count());
            return *this;
        }
        
        void AddPaintKit(int iPaintKit)
        {
            if (paintKits.Find(iPaintKit) == paintKits.InvalidIndex())
            {
                paintKits.AddToTail(iPaintKit);
            }
        }
        
        int GetCount() const { return paintKits.Count(); }
        int GetPaintKit(int index) const { return paintKits[index]; }
    };
    CUtlVector<WeaponSkinList_t> m_WeaponSkins;
    
    bool m_bInitialized;
};

// Global instance
extern CCSkinDatabase g_SkinDatabase;

#endif // CS_SKIN_DATABASE_H
