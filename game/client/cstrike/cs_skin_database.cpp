#include "cbase.h"
#include "cs_skin_database.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "cs_weapon_parse.h"

// Singleton instance
CCSkinDatabase g_SkinDatabase;

// =============================================================================
// CONSTRUCTOR / DESTRUCTOR
// =============================================================================

CCSkinDatabase::CCSkinDatabase() : m_bInitialized(false)
{
    SetDefLessFunc(m_SkinDefinitions);
    SetDefLessFunc(m_MaterialCache);
    SetDefLessFunc(m_IconCache);
}

CCSkinDatabase::~CCSkinDatabase()
{
    Shutdown();
}

// =============================================================================
// INITIALIZATION
// =============================================================================

bool CCSkinDatabase::Initialize()
{
    if (m_bInitialized)
    {
        DevMsg("[SkinDB] Already initialized\n");
        return true;
    }
    
    DevMsg("[SkinDB] Initializing skin database...\n");
    
    // load database like CS:GO's scripts/items_game.txt
    bool bSuccess = LoadFromFile("scripts/skins.txt");
    
    if (bSuccess && m_SkinDefinitions.Count() > 0)
    {
        m_bInitialized = true;
        DevMsg("[SkinDB] Successfully loaded %d skins\n", m_SkinDefinitions.Count());
    }
    else
    {
        DevMsg("[SkinDB] No skins loaded (file missing or empty)\n");
        m_bInitialized = true; // Все равно помечаем как инициализированную
    }
    
    return true;
}

bool CCSkinDatabase::LoadFromFile(const char* pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
    {
        DevMsg("[SkinDB] Invalid file path\n");
        return false;
    }
    
    // filesystem!!!
    if (!filesystem)
    {
        Warning("[SkinDB] Filesystem is NULL!\n");
        return false;
    }
    
    if (!filesystem->FileExists(pszFilePath, "MOD"))
    {
        DevMsg("[SkinDB] File not found: %s\n", pszFilePath);
        
        char szFullPath[MAX_PATH];
        filesystem->RelativePathToFullPath(pszFilePath, "MOD", szFullPath, sizeof(szFullPath));
        DevMsg("[SkinDB] Full path: %s\n", szFullPath);
        
        return false;
    }
    
    KeyValues* pKV = new KeyValues("Skins");
    if (!pKV)
    {
        Warning("[SkinDB] Failed to create KeyValues\n");
        return false;
    }
    
    if (!pKV->LoadFromFile(filesystem, pszFilePath, "MOD"))
    {
        DevMsg("[SkinDB] Failed to load KeyValues from: %s\n", pszFilePath);
        pKV->deleteThis();
        return false;
    }
    
    DevMsg("[SkinDB] Loading skins from: %s\n", pszFilePath);
    
    int iLoadedCount = 0;
    
    for (KeyValues* pSkin = pKV->GetFirstSubKey(); pSkin; pSkin = pSkin->GetNextKey())
    {
        SkinDefinition_t def;
        
        if (ParseSkinDefinition(pSkin, def))
        {
            m_SkinDefinitions.Insert(def.iPaintKit, def);
            
            bool bFound = false;
            FOR_EACH_VEC(m_WeaponSkins, i)
            {
                if (m_WeaponSkins[i].weaponID == def.weaponID)
                {
                    m_WeaponSkins[i].AddPaintKit(def.iPaintKit);
                    bFound = true;
                    break;
                }
            }
            
            if (!bFound)
            {
                WeaponSkinList_t newEntry;
                newEntry.weaponID = def.weaponID;
                newEntry.AddPaintKit(def.iPaintKit);
                m_WeaponSkins.AddToTail(newEntry);
            }
            
            iLoadedCount++;
        }
    }
    
    pKV->deleteThis();
    
    DevMsg("[SkinDB] Loaded %d skin definitions\n", iLoadedCount);
    return iLoadedCount > 0;
}

// =============================================================================
// Feb 06 2026, adding multiple load materials like "composite_texture" from CS:GO
// =============================================================================
bool CCSkinDatabase::ParseSkinDefinition(KeyValues* pKV, SkinDefinition_t& def)
{
    if (!pKV)
    {
        Warning("[SkinDB] Null KeyValues in ParseSkinDefinition\n");
        return false;
    }
    
    // Paint Kit ID
    const char* pszKeyName = pKV->GetName();
    if (!pszKeyName || !pszKeyName[0])
    {
        Warning("[SkinDB] KeyValues has no name\n");
        return false;
    }
    
    def.iPaintKit = atoi(pszKeyName);
    if (def.iPaintKit <= 0)
    {
        Warning("[SkinDB] Invalid paint kit ID: %s\n", pszKeyName);
        return false;
    }
    
    const char* pszName = pKV->GetString("name", nullptr);
    if (!pszName || !pszName[0])
    {
        Warning("[SkinDB] Skin %d has no name\n", def.iPaintKit);
        return false;
    }
    Q_strncpy(def.szName, pszName, sizeof(def.szName));
    
    const char* szDescription = pKV->GetString("description", nullptr);
    Q_strncpy(def.szDescription, szDescription, sizeof(def.szDescription));
    
    //NEWNEW: Item definition, it helps me to find specific items for inventory
    
    const char* pszItemType = pKV->GetString("item_type", nullptr);
    if (pszItemType && pszItemType[0])
    {
        if (Q_stricmp(pszItemType, "gloves") == 0 || Q_stricmp(pszItemType, "glove") == 0)
        {
            def.itemType = SKIN_ITEM_GLOVES;
        }
        else if (Q_stricmp(pszItemType, "knife") == 0)
        {
            def.itemType = SKIN_ITEM_KNIFE;
        }
        else if (Q_stricmp(pszItemType, "weapon") == 0)
        {
            def.itemType = SKIN_ITEM_WEAPON;
        }
        else
        {
            Warning("[SkinDB] Skin %d (%s) has unknown item_type: %s, defaulting to weapon\n", 
                    def.iPaintKit, def.szName, pszItemType);
            def.itemType = SKIN_ITEM_WEAPON;
        }
    }
    else
    {
        def.itemType = SKIN_ITEM_WEAPON;
    }
    
    if (def.itemType == SKIN_ITEM_GLOVES)
    {
        const char* pszGloveClass = pKV->GetString("glove_class", nullptr);
        if (!pszGloveClass || !pszGloveClass[0])
        {
            Warning("[SkinDB] Glove skin %d (%s) has no glove_class\n", def.iPaintKit, def.szName);
            return false;
        }
        Q_strncpy(def.szItemClass, pszGloveClass, sizeof(def.szItemClass));
        
        def.weaponID = WEAPON_NONE;
        
        DevMsg("[SkinDB] Loaded glove skin %d (%s) with class '%s'\n", 
               def.iPaintKit, def.szName, def.szItemClass);
    }
    else
    {
        const char* pszWeapon = pKV->GetString("weapon", nullptr);
        if (!pszWeapon || !pszWeapon[0])
        {
            Warning("[SkinDB] Skin %d (%s) has no weapon\n", def.iPaintKit, def.szName);
            return false;
        }
        def.weaponID = AliasToWeaponID(pszWeapon);
        if (def.weaponID == WEAPON_NONE)
        {
            Warning("[SkinDB] Skin %d (%s) has invalid weapon: %s\n", 
                    def.iPaintKit, def.szName, pszWeapon);
            return false;
        }
    }
    
    const char* pszIcon = pKV->GetString("icon", "");
    Q_strncpy(def.szIconPath, pszIcon, sizeof(def.szIconPath));
    
    def.rarity = (ESkinRarity)pKV->GetInt("rarity", SKIN_RARITY_COMMON);
    if (def.rarity < 0 || def.rarity >= SKIN_RARITY_COUNT)
    {
        Warning("[SkinDB] Skin %d (%s) has invalid rarity: %d, setting to Common\n", 
                def.iPaintKit, def.szName, (int)def.rarity);
        def.rarity = SKIN_RARITY_COMMON;
    }
    
    // ========================================
    // NEWNEW: parce material index from material[index]
    // ========================================
    
    const char* pszBaseMaterial = pKV->GetString("base", nullptr);
    if (pszBaseMaterial && pszBaseMaterial[0])
    {
        //Parce with 0 for "only_first_material" like CS:GO's "only_first_material" param
        SkinDefinition_t::MaterialData_t matData;
        matData.iMaterialIndex = 0;
        Q_strncpy(matData.szBaseMaterial, pszBaseMaterial, sizeof(matData.szBaseMaterial));
        
        ParseVMTParams(pKV, matData.vmtParams);
        
        def.materials.AddToTail(matData);
        
        DevMsg("[SkinDB] Skin %d (%s) loaded with single material (legacy format)\n", 
               def.iPaintKit, def.szName);
    }
    else
    {
        //NEWNEW: new index parce logic
        bool bFoundMaterials = false;
        
        for (int i = 0; i < MAX_SKIN_MATERIALS; i++)
        {
            char szMaterialKey[32];
            Q_snprintf(szMaterialKey, sizeof(szMaterialKey), "material%d", i);
            
            KeyValues* pMaterialKV = pKV->FindKey(szMaterialKey);
            if (!pMaterialKV)
                continue;
            
            SkinDefinition_t::MaterialData_t matData;
            
            if (ParseMaterialData(pMaterialKV, matData))
            {
                // Устанавливаем индекс материала
                matData.iMaterialIndex = i;
                def.materials.AddToTail(matData);
                bFoundMaterials = true;
                
                DevMsg("[SkinDB] Skin %d (%s) loaded material%d with base '%s'\n", 
                       def.iPaintKit, def.szName, i, matData.szBaseMaterial);
            }
        }
        
        if (!bFoundMaterials)
        {
            Warning("[SkinDB] Skin %d (%s) has no materials (neither 'base' nor 'material0', 'material1', etc.)\n", 
                    def.iPaintKit, def.szName);
            return false;
        }
    }
    
    return true;
}

// =============================================================================
// Purpose: ParseMaterialData
// =============================================================================
bool CCSkinDatabase::ParseMaterialData(KeyValues* pKV, SkinDefinition_t::MaterialData_t& matData)
{
    if (!pKV)
        return false;
    
    const char* pszBase = pKV->GetString("base", nullptr);
    if (!pszBase || !pszBase[0])
    {
        Warning("[SkinDB] Material has no 'base' parameter\n");
        return false;
    }
    
    Q_strncpy(matData.szBaseMaterial, pszBase, sizeof(matData.szBaseMaterial));
    
    ParseVMTParams(pKV, matData.vmtParams);
    
    return true;
}

void CCSkinDatabase::ParseVMTParams(KeyValues* pKV, SkinDefinition_t::MaterialData_t::VMTParams_t& params)
{
    // ========================================
    // TEXTURES
    // ========================================
    const char* pszValue;
    
    if ((pszValue = pKV->GetString("$basetexture", nullptr)) != nullptr)
        Q_strncpy(params.szBaseTexture, pszValue, sizeof(params.szBaseTexture));
    
    if ((pszValue = pKV->GetString("$basetexture2", nullptr)) != nullptr)
        Q_strncpy(params.szBaseTexture2, pszValue, sizeof(params.szBaseTexture2));
    
    if ((pszValue = pKV->GetString("$bumpmap", nullptr)) != nullptr)
        Q_strncpy(params.szBumpMap, pszValue, sizeof(params.szBumpMap));
    
    if ((pszValue = pKV->GetString("$bumpmap2", nullptr)) != nullptr)
        Q_strncpy(params.szBumpMap2, pszValue, sizeof(params.szBumpMap2));
    
    if ((pszValue = pKV->GetString("$detail", nullptr)) != nullptr)
        Q_strncpy(params.szDetailTexture, pszValue, sizeof(params.szDetailTexture));
    
    if ((pszValue = pKV->GetString("$envmap", nullptr)) != nullptr)
        Q_strncpy(params.szEnvMap, pszValue, sizeof(params.szEnvMap));
    
    if ((pszValue = pKV->GetString("$phongexponenttexture", nullptr)) != nullptr)
        Q_strncpy(params.szPhongExponentTexture, pszValue, sizeof(params.szPhongExponentTexture));
    
    if ((pszValue = pKV->GetString("$phongwarptexture", nullptr)) != nullptr)
        Q_strncpy(params.szPhongWarpTexture, pszValue, sizeof(params.szPhongWarpTexture));
    
    // ========================================
    // PHONG
    // ========================================
    if (pKV->FindKey("$phong"))
        params.bPhong = pKV->GetInt("$phong", 0);
    
    if (pKV->FindKey("$phongboost"))
        params.flPhongBoost = pKV->GetFloat("$phongboost", 1.0f);
    
    if (pKV->FindKey("$phongexponent"))
        params.flPhongExponent = pKV->GetFloat("$phongexponent", 5.0f);
    
    if (pKV->FindKey("$phongalbedotint"))
        params.flPhongAlbedoTint = pKV->GetFloat("$phongalbedotint", 1.0f);
    
    if (pKV->FindKey("$phongalbedoboost"))
        params.bPhongAlbedoBoost = pKV->GetInt("$phongalbedoboost", 0);
    
    // Phong fresnel ranges
    if ((pszValue = pKV->GetString("$phongfresnelranges", nullptr)) != nullptr)
    {
        sscanf(pszValue, "[%f %f %f]", 
               &params.flPhongFresnelRanges[0],
               &params.flPhongFresnelRanges[1],
               &params.flPhongFresnelRanges[2]);
    }
    
    // ========================================
    // ENVIRONMENT MAPPING
    // ========================================
    if ((pszValue = pKV->GetString("$envmaptint", nullptr)) != nullptr)
    {
        sscanf(pszValue, "[%f %f %f]", 
               &params.flEnvMapTint[0],
               &params.flEnvMapTint[1],
               &params.flEnvMapTint[2]);
    }
    
    if (pKV->FindKey("$envmapsaturation"))
        params.flEnvMapSaturation = pKV->GetFloat("$envmapsaturation", 1.0f);
    
    if (pKV->FindKey("$envmapcontrast"))
        params.flEnvMapContrast = pKV->GetFloat("$envmapcontrast", 0.0f);
    
    if (pKV->FindKey("$envmapfresnel"))
        params.flEnvMapFresnel = pKV->GetFloat("$envmapfresnel", 0.0f);
    
    if (pKV->FindKey("$fresnelreflection"))
        params.flFresnelReflection = pKV->GetFloat("$fresnelreflection", 1.0f);
    
    // ========================================
    // ALPHA & TRANSPARENCY
    // ========================================
    if (pKV->FindKey("$alphatest"))
        params.bAlphaTest = pKV->GetInt("$alphatest", 0);
    
    if (pKV->FindKey("$alphatestreference"))
        params.flAlphaTestReference = pKV->GetFloat("$alphatestreference", 0.5f);
    
    if (pKV->FindKey("$translucent"))
        params.bTranslucent = pKV->GetInt("$translucent", 0);
    
    if (pKV->FindKey("$additive"))
        params.bAdditive = pKV->GetInt("$additive", 0);
    
    // ========================================
    // TEXTURE MODIFIERS
    // ========================================
    if (pKV->FindKey("$basemapalphaphongmask"))
        params.bBaseTextureNoEnvMap = pKV->GetInt("$basemapalphaphongmask", 0);
    
    if (pKV->FindKey("$normalmapalphaenvmapmask"))
        params.bNormalMapAlphaPhongMask = pKV->GetInt("$normalmapalphaenvmapmask", 0);
    
    if (pKV->FindKey("$basealphaenvmapmask"))
        params.bBaseAlphaEnvMapMask = pKV->GetInt("$basealphaenvmapmask", 0);
    
    // ========================================
    // DETAIL TEXTURE
    // ========================================
    if (pKV->FindKey("$detailscale"))
        params.flDetailScale = pKV->GetFloat("$detailscale", 4.0f);
    
    if (pKV->FindKey("$detailblendmode"))
        params.iDetailBlendMode = pKV->GetInt("$detailblendmode", 0);
    
    if (pKV->FindKey("$detailblendfactor"))
        params.flDetailBlendFactor = pKV->GetFloat("$detailblendfactor", 1.0f);
    
    // ========================================
    // RIM LIGHTING
    // ========================================
    if (pKV->FindKey("$rimlight"))
        params.bRimLight = pKV->GetInt("$rimlight", 0);
    
    if (pKV->FindKey("$rimlightexponent"))
        params.flRimLightExponent = pKV->GetFloat("$rimlightexponent", 4.0f);
    
    if (pKV->FindKey("$rimlightboost"))
        params.flRimLightBoost = pKV->GetFloat("$rimlightboost", 1.0f);
    
    // ========================================
    // SELF ILLUMINATION
    // ========================================
    if (pKV->FindKey("$selfillum"))
        params.bSelfIllum = pKV->GetInt("$selfillum", 0);
    
    if ((pszValue = pKV->GetString("$selfillumtint", nullptr)) != nullptr)
    {
        sscanf(pszValue, "[%f %f %f]", 
               &params.flSelfIllumTint[0],
               &params.flSelfIllumTint[1],
               &params.flSelfIllumTint[2]);
    }
    
    // ========================================
    // COLOR MODULATION
    // ========================================
    if ((pszValue = pKV->GetString("$color", nullptr)) != nullptr)
    {
        sscanf(pszValue, "[%f %f %f]", 
               &params.flColor[0],
               &params.flColor[1],
               &params.flColor[2]);
    }
    
    if ((pszValue = pKV->GetString("$color2", nullptr)) != nullptr)
    {
        sscanf(pszValue, "[%f %f %f]", 
               &params.flColor2[0],
               &params.flColor2[1],
               &params.flColor2[2]);
    }
    
    // ========================================
    // MISC RENDERING
    // ========================================
    if (pKV->FindKey("$nocull"))
        params.bNoCull = pKV->GetInt("$nocull", 0);
    
    if (pKV->FindKey("$nodecal"))
        params.bNoDecal = pKV->GetInt("$nodecal", 0);
    
    if (pKV->FindKey("$halflambert"))
        params.bHalfLambert = pKV->GetInt("$halflambert", 0);
    
    // ========================================
    // CUSTOM CS:GO SKIN PARAMETERS (soon)
    // ========================================
    if (pKV->FindKey("$wear"))
        params.flWear = pKV->GetFloat("$wear", 0.0f);
    
    if (pKV->FindKey("$seed"))
        params.iSeed = pKV->GetInt("$seed", 0);
    
    if (pKV->FindKey("$patternscale"))
        params.flPatternScale = pKV->GetFloat("$patternscale", 1.0f);
    
    if (pKV->FindKey("$patternrotate"))
        params.flPatternRotate = pKV->GetFloat("$patternrotate", 0.0f);
}

// =============================================================================
// Purpose: Parse mayerials with [index]
// =============================================================================

IMaterial* CCSkinDatabase::GetSkinMaterial(int iPaintKit, int iMaterialIndex)
{
    if (!m_bInitialized || iPaintKit <= 0)
        return nullptr;
    
    MaterialCacheKey_t key;
    key.iPaintKit = iPaintKit;
    key.iMaterialIndex = iMaterialIndex;
    
    int cacheIdx = m_MaterialCache.Find(key);
    if (cacheIdx != m_MaterialCache.InvalidIndex())
    {
        return m_MaterialCache[cacheIdx];
    }
    
    const SkinDefinition_t* pDef = FindSkinByPaintKit(iPaintKit);
    if (!pDef)
    {
        Warning("[SkinDB] Paint kit %d not found\n", iPaintKit);
        return nullptr;
    }
    
    const SkinDefinition_t::MaterialData_t* pMatData = pDef->FindMaterialByIndex(iMaterialIndex);
    if (!pMatData)
    {
        Warning("[SkinDB] Paint kit %d has no material with index %d\n", iPaintKit, iMaterialIndex);
        return nullptr;
    }
    
    IMaterial* pMat = LoadMaterial(pMatData);
    if (pMat)
    {
        m_MaterialCache.Insert(key, pMat);
        DevMsg("[SkinDB] Cached material for paint kit %d, index %d\n", iPaintKit, iMaterialIndex);
    }
    
    return pMat;
}

void CCSkinDatabase::GetAllSkinMaterials(int iPaintKit, CUtlVector<IMaterial*>& materials)
{
    materials.RemoveAll();
    
    if (!m_bInitialized || iPaintKit <= 0)
        return;
    
    const SkinDefinition_t* pDef = FindSkinByPaintKit(iPaintKit);
    if (!pDef)
    {
        Warning("[SkinDB] Paint kit %d not found\n", iPaintKit);
        return;
    }
    
    FOR_EACH_VEC(pDef->materials, i)
    {
        const SkinDefinition_t::MaterialData_t& matData = pDef->materials[i];
        IMaterial* pMat = GetSkinMaterial(iPaintKit, matData.iMaterialIndex);
        
        if (pMat)
        {
            materials.AddToTail(pMat);
        }
    }
    
    DevMsg("[SkinDB] Loaded %d materials for paint kit %d\n", materials.Count(), iPaintKit);
}

ITexture* CCSkinDatabase::GetSkinIcon(int iPaintKit)
{
    if (!m_bInitialized || iPaintKit <= 0)
        return nullptr;
    
    int cacheIdx = m_IconCache.Find(iPaintKit);
    if (cacheIdx != m_IconCache.InvalidIndex())
    {
        return m_IconCache[cacheIdx];
    }
    
    const SkinDefinition_t* pDef = FindSkinByPaintKit(iPaintKit);
    if (!pDef)
        return nullptr;
    
    ITexture* pIcon = LoadIcon(pDef);
    if (pIcon)
    {
        m_IconCache.Insert(iPaintKit, pIcon);
    }
    
    return pIcon;
}

IMaterial* CCSkinDatabase::LoadMaterial(const SkinDefinition_t::MaterialData_t* pMatData)
{
    if (!pMatData)
        return nullptr;
    
    if (!materials)
    {
        Warning("[SkinDB] Materials system is NULL!\n");
        return nullptr;
    }
    
    IMaterial* pBaseMat = materials->FindMaterial(pMatData->szBaseMaterial, TEXTURE_GROUP_MODEL, true);
    
    if (!pBaseMat || IsErrorMaterial(pBaseMat))
    {
        Warning("[SkinDB] Failed to load base material: %s\n", pMatData->szBaseMaterial);
        return nullptr;
    }
    
    KeyValues* pVMT = CloneBaseMaterialKeyValues(pBaseMat);
    if (!pVMT)
    {
        Warning("[SkinDB] Failed to clone base material KV\n");
        return nullptr;
    }
    
    ApplyVMTPatch(pVMT, pMatData->vmtParams);
    
    char szBaseMaterialPath[256];
    Q_strncpy(szBaseMaterialPath, pMatData->szBaseMaterial, sizeof(szBaseMaterialPath));
    
    for (char* p = szBaseMaterialPath; *p; p++)
    {
        if (*p == '\\')
            *p = '/';
    }
    
    const char* pLastSlash = Q_strrchr(szBaseMaterialPath, '/');
    const char* pMaterialName = pLastSlash ? (pLastSlash + 1) : szBaseMaterialPath;
    
    char szUniqueMaterialName[256];
    Q_snprintf(szUniqueMaterialName, sizeof(szUniqueMaterialName), "%s_idx%d", 
               pMaterialName, pMatData->iMaterialIndex);
    
    DevMsg("[SkinDB] Creating material '%s' from base '%s'\n", 
           szUniqueMaterialName, pMatData->szBaseMaterial);
    
    IMaterial* pMat = materials->CreateMaterial(szUniqueMaterialName, pVMT);
    
    if (!pMat || IsErrorMaterial(pMat))
    {
        Warning("[SkinDB] Failed to create patched material '%s'\n", szUniqueMaterialName);
        pVMT->deleteThis();
        return nullptr;
    }
    
    // reference count
    pMat->IncrementReferenceCount();
    
    // Precache
    if (!pMat->IsPrecached())
    {
        MaterialLock_t hLock = materials->Lock();
        pMat->Refresh();
        materials->Unlock(hLock);
    }
    
    DevMsg("[SkinDB] Successfully created material '%s' (index %d)\n", 
           szUniqueMaterialName, pMatData->iMaterialIndex);
    
    return pMat;
}

ITexture* CCSkinDatabase::LoadIcon(const SkinDefinition_t* pDef)
{
    if (!pDef || !pDef->szIconPath[0])
        return nullptr;
    
    if (!materials)
    {
        Warning("[SkinDB] Materials system is NULL!\n");
        return nullptr;
    }
    
    ITexture* pIcon = materials->FindTexture(pDef->szIconPath, TEXTURE_GROUP_VGUI, true);
    
    if (!pIcon || pIcon->IsError())
    {
        DevMsg("[SkinDB] Failed to load icon: %s\n", pDef->szIconPath);
        return nullptr;
    }
    
    return pIcon;
}

// =============================================================================
// Purpose: Clear cache
// =============================================================================

void CCSkinDatabase::ClearMaterialCache()
{
    FOR_EACH_MAP_FAST(m_MaterialCache, i)
    {
        IMaterial* pMat = m_MaterialCache[i];
        if (pMat)
        {
            pMat->DecrementReferenceCount();
        }
    }
    
    m_MaterialCache.RemoveAll();
    DevMsg("[SkinDB] Material cache cleared\n");
}

void CCSkinDatabase::ClearIconCache()
{
    m_IconCache.RemoveAll();
    DevMsg("[SkinDB] Icon cache cleared\n");
}

void CCSkinDatabase::Shutdown()
{
    if (!m_bInitialized)
        return;
    
    DevMsg("[SkinDB] Shutting down...\n");
    
    ClearMaterialCache();
    ClearIconCache();
    
    m_SkinDefinitions.RemoveAll();
    m_WeaponSkins.RemoveAll();
    
    m_bInitialized = false;
}

void CCSkinDatabase::ReleaseMaterial(int iPaintKit, int iMaterialIndex)
{
    MaterialCacheKey_t key;
    key.iPaintKit = iPaintKit;
    key.iMaterialIndex = iMaterialIndex;
    
    int idx = m_MaterialCache.Find(key);
    if (idx != m_MaterialCache.InvalidIndex())
    {
        IMaterial* pMat = m_MaterialCache[idx];
        if (pMat)
        {
            pMat->DecrementReferenceCount();
        }
        m_MaterialCache.RemoveAt(idx);
    }
}

void CCSkinDatabase::ReleaseAllMaterials(int iPaintKit)
{
    const SkinDefinition_t* pDef = FindSkinByPaintKit(iPaintKit);
    if (!pDef)
        return;
    
    FOR_EACH_VEC(pDef->materials, i)
    {
        ReleaseMaterial(iPaintKit, pDef->materials[i].iMaterialIndex);
    }
}

void CCSkinDatabase::ReleaseIcon(int iPaintKit)
{
    int idx = m_IconCache.Find(iPaintKit);
    if (idx != m_IconCache.InvalidIndex())
    {
        m_IconCache.RemoveAt(idx);
    }
}

// =============================================================================
// Purpose: Find Skin (for inventory system, damn...)
// =============================================================================

const SkinDefinition_t* CCSkinDatabase::FindSkinByPaintKit(int iPaintKit) const
{
    if (!m_bInitialized || iPaintKit <= 0)
        return nullptr;
    
    int idx = m_SkinDefinitions.Find(iPaintKit);
    if (idx == m_SkinDefinitions.InvalidIndex())
        return nullptr;
    
    return &m_SkinDefinitions[idx];
}

const SkinDefinition_t* CCSkinDatabase::FindSkinByName(const char* pszName, CSWeaponID weaponID) const
{
    if (!m_bInitialized || !pszName || !pszName[0])
        return nullptr;
    
    FOR_EACH_MAP_FAST(m_SkinDefinitions, i)
    {
        const SkinDefinition_t& def = m_SkinDefinitions[i];
        
        if (weaponID != WEAPON_NONE && def.weaponID != weaponID)
            continue;
        
        if (Q_stricmp(def.szName, pszName) == 0)
            return &def;
    }
    
    return nullptr;
}

void CCSkinDatabase::GetSkinsForWeapon(CSWeaponID weaponID, CUtlVector<const SkinDefinition_t*>& skins) const
{
    skins.RemoveAll();
    
    if (!m_bInitialized || weaponID == WEAPON_NONE)
        return;
    
    FOR_EACH_VEC(m_WeaponSkins, i)
    {
        if (m_WeaponSkins[i].weaponID == weaponID)
        {
            for (int j = 0; j < m_WeaponSkins[i].GetCount(); j++)
            {
                int iPaintKit = m_WeaponSkins[i].GetPaintKit(j);
                const SkinDefinition_t* pDef = FindSkinByPaintKit(iPaintKit);
                if (pDef)
                {
                    skins.AddToTail(pDef);
                }
            }
            break;
        }
    }
}

void CCSkinDatabase::GetAllSkins(CUtlVector<const SkinDefinition_t*>& skins) const
{
    skins.RemoveAll();
    
    if (!m_bInitialized)
        return;
    
    FOR_EACH_MAP_FAST(m_SkinDefinitions, i)
    {
        skins.AddToTail(&m_SkinDefinitions[i]);
    }
}

// =============================================================================
// KeyValues & VMT patch (without changes from original)
// =============================================================================

KeyValues* CCSkinDatabase::CloneBaseMaterialKeyValues(IMaterial* pBaseMat)
{
    if (!pBaseMat)
        return nullptr;
    
    // g shader name
    const char* pszShaderName = pBaseMat->GetShaderName();
    if (!pszShaderName || !pszShaderName[0])
        pszShaderName = "VertexLitGeneric";
    
    KeyValues* pKV = new KeyValues(pszShaderName);
    if (!pKV)
        return nullptr;
    
    // copy material vars
    IMaterialVar** ppParams = pBaseMat->GetShaderParams();
    int numParams = pBaseMat->ShaderParamCount();
    
    for (int i = 0; i < numParams; i++)
    {
        IMaterialVar* pVar = ppParams[i];
        if (!pVar)
            continue;
        
        const char* pszName = pVar->GetName();
        if (!pszName || !pszName[0])
            continue;
        
        switch (pVar->GetType())
        {
            case MATERIAL_VAR_TYPE_FLOAT:
                pKV->SetFloat(pszName, pVar->GetFloatValue());
                break;
                
            case MATERIAL_VAR_TYPE_INT:
                pKV->SetInt(pszName, pVar->GetIntValue());
                break;
                
            case MATERIAL_VAR_TYPE_STRING:
                pKV->SetString(pszName, pVar->GetStringValue());
                break;
                
            case MATERIAL_VAR_TYPE_VECTOR:
            {
                float const* pVec = pVar->GetVecValue();
                int dim = pVar->VectorSize();
                
                if (dim == 3)
                {
                    char szValue[128];
                    Q_snprintf(szValue, sizeof(szValue), "[%f %f %f]", pVec[0], pVec[1], pVec[2]);
                    pKV->SetString(pszName, szValue);
                }
                else if (dim == 4)
                {
                    char szValue[128];
                    Q_snprintf(szValue, sizeof(szValue), "[%f %f %f %f]", pVec[0], pVec[1], pVec[2], pVec[3]);
                    pKV->SetString(pszName, szValue);
                }
                break;
            }
                
            case MATERIAL_VAR_TYPE_TEXTURE:
            {
                ITexture* pTex = pVar->GetTextureValue();
                if (pTex && !pTex->IsError())
                {
                    pKV->SetString(pszName, pTex->GetName());
                }
                break;
            }
                
            case MATERIAL_VAR_TYPE_MATERIAL:
            {
                IMaterial* pMat = pVar->GetMaterialValue();
                if (pMat && !IsErrorMaterial(pMat))
                {
                    pKV->SetString(pszName, pMat->GetName());
                }
                break;
            }
        }
    }
    
    return pKV;
}

void CCSkinDatabase::ApplyVMTPatch(KeyValues* pKV, const SkinDefinition_t::MaterialData_t::VMTParams_t& params)
{
    if (!pKV)
        return;
    
    // ========================================
    // TEXTURES
    // ========================================
    if (params.szBaseTexture[0])
        pKV->SetString("$basetexture", params.szBaseTexture);
    
    if (params.szBaseTexture2[0])
        pKV->SetString("$basetexture2", params.szBaseTexture2);
    
    if (params.szBumpMap[0])
        pKV->SetString("$bumpmap", params.szBumpMap);
    
    if (params.szBumpMap2[0])
        pKV->SetString("$bumpmap2", params.szBumpMap2);
    
    if (params.szDetailTexture[0])
        pKV->SetString("$detail", params.szDetailTexture);
    
    if (params.szEnvMap[0])
        pKV->SetString("$envmap", params.szEnvMap);
    
    if (params.szPhongExponentTexture[0])
        pKV->SetString("$phongexponenttexture", params.szPhongExponentTexture);
    
    if (params.szPhongWarpTexture[0])
        pKV->SetString("$phongwarptexture", params.szPhongWarpTexture);
    
    // ========================================
    // PHONG
    // ========================================
    if (params.bPhong >= 0)
        pKV->SetInt("$phong", params.bPhong);
    
    if (params.flPhongBoost >= 0.0f)
        pKV->SetFloat("$phongboost", params.flPhongBoost);
    
    if (params.flPhongExponent >= 0.0f)
        pKV->SetFloat("$phongexponent", params.flPhongExponent);
    
    if (params.flPhongAlbedoTint >= 0.0f)
        pKV->SetFloat("$phongalbedotint", params.flPhongAlbedoTint);
    
    if (params.bPhongAlbedoBoost >= 0)
        pKV->SetInt("$phongalbedoboost", params.bPhongAlbedoBoost);
    
    // Phong fresnel ranges
    if (params.flPhongFresnelRanges[0] >= 0.0f)
    {
        char szValue[128];
        Q_snprintf(szValue, sizeof(szValue), "[%f %f %f]",
                   params.flPhongFresnelRanges[0],
                   params.flPhongFresnelRanges[1],
                   params.flPhongFresnelRanges[2]);
        pKV->SetString("$phongfresnelranges", szValue);
    }
    
    // ========================================
    // ENVIRONMENT MAPPING
    // ========================================
    if (params.flEnvMapTint[0] >= 0.0f)
    {
        char szValue[128];
        Q_snprintf(szValue, sizeof(szValue), "[%f %f %f]",
                   params.flEnvMapTint[0],
                   params.flEnvMapTint[1],
                   params.flEnvMapTint[2]);
        pKV->SetString("$envmaptint", szValue);
    }
    
    if (params.flEnvMapSaturation >= 0.0f)
        pKV->SetFloat("$envmapsaturation", params.flEnvMapSaturation);
    
    if (params.flEnvMapContrast >= 0.0f)
        pKV->SetFloat("$envmapcontrast", params.flEnvMapContrast);
    
    if (params.flEnvMapFresnel >= 0.0f)
        pKV->SetFloat("$envmapfresnel", params.flEnvMapFresnel);
    
    if (params.flFresnelReflection >= 0.0f)
        pKV->SetFloat("$fresnelreflection", params.flFresnelReflection);
    
    // ========================================
    // ALPHA & TRANSPARENCY
    // ========================================
    if (params.bAlphaTest >= 0)
        pKV->SetInt("$alphatest", params.bAlphaTest);
    
    if (params.flAlphaTestReference >= 0.0f)
        pKV->SetFloat("$alphatestreference", params.flAlphaTestReference);
    
    if (params.bTranslucent >= 0)
        pKV->SetInt("$translucent", params.bTranslucent);
    
    if (params.bAdditive >= 0)
        pKV->SetInt("$additive", params.bAdditive);
    
    // ========================================
    // TEXTURE MODIFIERS
    // ========================================
    if (params.bBaseTextureNoEnvMap >= 0)
        pKV->SetInt("$basemapalphaphongmask", params.bBaseTextureNoEnvMap);
    
    if (params.bNormalMapAlphaPhongMask >= 0)
        pKV->SetInt("$normalmapalphaenvmapmask", params.bNormalMapAlphaPhongMask);
    
    if (params.bBaseAlphaEnvMapMask >= 0)
        pKV->SetInt("$basealphaenvmapmask", params.bBaseAlphaEnvMapMask);
    
    // ========================================
    // DETAIL TEXTURE
    // ========================================
    if (params.flDetailScale >= 0.0f)
        pKV->SetFloat("$detailscale", params.flDetailScale);
    
    if (params.iDetailBlendMode >= 0)
        pKV->SetInt("$detailblendmode", params.iDetailBlendMode);
    
    if (params.flDetailBlendFactor >= 0.0f)
        pKV->SetFloat("$detailblendfactor", params.flDetailBlendFactor);
    
    // ========================================
    // RIM LIGHTING
    // ========================================
    if (params.bRimLight >= 0)
        pKV->SetInt("$rimlight", params.bRimLight);
    
    if (params.flRimLightExponent >= 0.0f)
        pKV->SetFloat("$rimlightexponent", params.flRimLightExponent);
    
    if (params.flRimLightBoost >= 0.0f)
        pKV->SetFloat("$rimlightboost", params.flRimLightBoost);
    
    // ========================================
    // SELF ILLUMINATION
    // ========================================
    if (params.bSelfIllum >= 0)
        pKV->SetInt("$selfillum", params.bSelfIllum);
    
    if (params.flSelfIllumTint[0] >= 0.0f)
    {
        char szValue[128];
        Q_snprintf(szValue, sizeof(szValue), "[%f %f %f]",
                   params.flSelfIllumTint[0],
                   params.flSelfIllumTint[1],
                   params.flSelfIllumTint[2]);
        pKV->SetString("$selfillumtint", szValue);
    }
    
    // ========================================
    // COLOR MODULATION
    // ========================================
    if (params.flColor[0] >= 0.0f)
    {
        char szValue[128];
        Q_snprintf(szValue, sizeof(szValue), "[%f %f %f]",
                   params.flColor[0],
                   params.flColor[1],
                   params.flColor[2]);
        pKV->SetString("$color", szValue);
    }
    
    if (params.flColor2[0] >= 0.0f)
    {
        char szValue[128];
        Q_snprintf(szValue, sizeof(szValue), "[%f %f %f]",
                   params.flColor2[0],
                   params.flColor2[1],
                   params.flColor2[2]);
        pKV->SetString("$color2", szValue);
    }
    
    // ========================================
    // MISC RENDERING
    // ========================================
    if (params.bNoCull >= 0)
        pKV->SetInt("$nocull", params.bNoCull);
    
    if (params.bNoDecal >= 0)
        pKV->SetInt("$nodecal", params.bNoDecal);
    
    if (params.bHalfLambert >= 0)
        pKV->SetInt("$halflambert", params.bHalfLambert);
    
    // ========================================
    // CUSTOM CS:GO SKIN PARAMETERS
    // ========================================
    if (params.flWear >= 0.0f)
        pKV->SetFloat("$wear", params.flWear);
    
    if (params.iSeed >= 0)
        pKV->SetInt("$seed", params.iSeed);
    
    if (params.flPatternScale >= 0.0f)
        pKV->SetFloat("$patternscale", params.flPatternScale);
    
    if (params.flPatternRotate >= 0.0f)
        pKV->SetFloat("$patternrotate", params.flPatternRotate);
}

// =============================================================================
// Debug / Utility
// =============================================================================

void CCSkinDatabase::PrintAllSkins() const
{
    if (!m_bInitialized)
    {
        Msg("[SkinDB] Database not initialized\n");
        return;
    }
    
    Msg("[SkinDB] === All Skins (%d total) ===\n", m_SkinDefinitions.Count());
    Msg("%-6s %-30s %-15s %-10s %s\n", "ID", "Name", "Weapon", "Materials", "Rarity");
    Msg("--------------------------------------------------------------------------------\n");
    
    FOR_EACH_MAP_FAST(m_SkinDefinitions, i)
    {
        const SkinDefinition_t& def = m_SkinDefinitions[i];
        Msg("%-6d %-30s %-15s %-10d %s\n", 
            def.iPaintKit, 
            def.szName,
            WeaponIDToAlias(def.weaponID),
            def.GetMaterialCount(),
            GetRarityName(def.rarity));
    }
}

void CCSkinDatabase::PrintSkinsForWeapon(CSWeaponID weaponID) const
{
    if (!m_bInitialized)
    {
        Msg("[SkinDB] Database not initialized\n");
        return;
    }
    
    CUtlVector<const SkinDefinition_t*> skins;
    GetSkinsForWeapon(weaponID, skins);
    
    Msg("[SkinDB] Skins for %s: %d\n", WeaponIDToAlias(weaponID), skins.Count());
    Msg("%-6s %-30s %-10s %s\n", "ID", "Name", "Materials", "Rarity");
    Msg("--------------------------------------------------------------------------------\n");
    
    FOR_EACH_VEC(skins, i)
    {
        const SkinDefinition_t* pDef = skins[i];
        Msg("%-6d %-30s %-10d %s\n", 
            pDef->iPaintKit, 
            pDef->szName,
            pDef->GetMaterialCount(),
            GetRarityName(pDef->rarity));
    }
}

int CCSkinDatabase::GenerateNewPaintKitID() const
{
    int maxID = 0;
    
    FOR_EACH_MAP_FAST(m_SkinDefinitions, i)
    {
        if (m_SkinDefinitions[i].iPaintKit > maxID)
            maxID = m_SkinDefinitions[i].iPaintKit;
    }
    
    return maxID + 1;
}

const SkinDefinition_t* CCSkinDatabase::FindGlovesByClass(const char* pszGloveClass) const
{
    if (!pszGloveClass || !pszGloveClass[0])
        return nullptr;
    
    FOR_EACH_MAP_FAST(m_SkinDefinitions, i)
    {
        const SkinDefinition_t& def = m_SkinDefinitions[i];
        if (def.itemType == SKIN_ITEM_GLOVES && 
            Q_stricmp(def.szItemClass, pszGloveClass) == 0)
        {
            return &def;
        }
    }
    
    return nullptr;
}

void CCSkinDatabase::GetAllGloves(CUtlVector<const SkinDefinition_t*>& gloves) const
{
    gloves.RemoveAll();
    
    FOR_EACH_MAP_FAST(m_SkinDefinitions, i)
    {
        const SkinDefinition_t& def = m_SkinDefinitions[i];
        if (def.itemType == SKIN_ITEM_GLOVES)
        {
            gloves.AddToTail(&def);
        }
    }
}

void CCSkinDatabase::PrintAllGloves() const
{
    if (!m_bInitialized)
    {
        Msg("[SkinDB] Database not initialized\n");
        return;
    }
    
    CUtlVector<const SkinDefinition_t*> gloves;
    GetAllGloves(gloves);
    
    Msg("[SkinDB] === All Gloves (%d total) ===\n", gloves.Count());
    Msg("%-6s %-30s %-20s %-10s %s\n", "ID", "Name", "Class", "Materials", "Rarity");
    Msg("--------------------------------------------------------------------------------\n");
    
    FOR_EACH_VEC(gloves, i)
    {
        const SkinDefinition_t* pDef = gloves[i];
        Msg("%-6d %-30s %-20s %-10d %s\n", 
            pDef->iPaintKit, 
            pDef->szName,
            pDef->szItemClass,
            pDef->GetMaterialCount(),
            GetRarityName(pDef->rarity));
    }
}
