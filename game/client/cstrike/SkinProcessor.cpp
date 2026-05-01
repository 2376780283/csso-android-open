#include "cbase.h"
#include "SkinProcessor.h"
#include "cs_skin_database.h"
#include "cs_weapon_parse.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"

CSkinProcessor g_SkinProcessor;

// =============================================================================
// CONVARS (Paint Kit IDs only)
// =============================================================================

// Rifles
ConVar loadout_ak47_skin("loadout_ak47_skin", "0", FCVAR_ARCHIVE, "AK-47 paint kit ID");
ConVar loadout_aug_skin("loadout_aug_skin", "0", FCVAR_ARCHIVE, "AUG paint kit ID");
ConVar loadout_awp_skin("loadout_awp_skin", "0", FCVAR_ARCHIVE, "AWP paint kit ID");
ConVar loadout_famas_skin("loadout_famas_skin", "0", FCVAR_ARCHIVE, "FAMAS paint kit ID");
ConVar loadout_galilar_skin("loadout_galilar_skin", "0", FCVAR_ARCHIVE, "Galil AR paint kit ID");
ConVar loadout_m4a1_silencer_skin("loadout_m4a1_silencer_skin", "0", FCVAR_ARCHIVE, "M4A1-S paint kit ID");
ConVar loadout_m4a4_skin("loadout_m4a4_skin", "0", FCVAR_ARCHIVE, "M4A4 paint kit ID");
ConVar loadout_sg556_skin("loadout_sg556_skin", "0", FCVAR_ARCHIVE, "SG556 paint kit ID");
ConVar loadout_scar20_skin("loadout_scar20_skin", "0", FCVAR_ARCHIVE, "SCAR-20 paint kit ID");
ConVar loadout_g3sg1_skin("loadout_g3sg1_skin", "0", FCVAR_ARCHIVE, "G3SG1 paint kit ID");

// SMGs
ConVar loadout_bizon_skin("loadout_bizon_skin", "0", FCVAR_ARCHIVE, "PP-Bizon paint kit ID");
ConVar loadout_mac10_skin("loadout_mac10_skin", "0", FCVAR_ARCHIVE, "MAC-10 paint kit ID");
ConVar loadout_mp5sd_skin("loadout_mp5sd_skin", "0", FCVAR_ARCHIVE, "MP5-SD paint kit ID");
ConVar loadout_mp7_skin("loadout_mp7_skin", "0", FCVAR_ARCHIVE, "MP7 paint kit ID");
ConVar loadout_mp9_skin("loadout_mp9_skin", "0", FCVAR_ARCHIVE, "MP9 paint kit ID");
ConVar loadout_p90_skin("loadout_p90_skin", "0", FCVAR_ARCHIVE, "P90 paint kit ID");
ConVar loadout_ump45_skin("loadout_ump45_skin", "0", FCVAR_ARCHIVE, "UMP-45 paint kit ID");

// Heavy
ConVar loadout_m249_skin("loadout_m249_skin", "0", FCVAR_ARCHIVE, "M249 paint kit ID");
ConVar loadout_negev_skin("loadout_negev_skin", "0", FCVAR_ARCHIVE, "Negev paint kit ID");
ConVar loadout_mag7_skin("loadout_mag7_skin", "0", FCVAR_ARCHIVE, "MAG-7 paint kit ID");
ConVar loadout_nova_skin("loadout_nova_skin", "0", FCVAR_ARCHIVE, "Nova paint kit ID");
ConVar loadout_sawedoff_skin("loadout_sawedoff_skin", "0", FCVAR_ARCHIVE, "Sawed-Off paint kit ID");
ConVar loadout_xm1014_skin("loadout_xm1014_skin", "0", FCVAR_ARCHIVE, "XM1014 paint kit ID");

// Pistols
ConVar loadout_cz75a_skin("loadout_cz75a_skin", "0", FCVAR_ARCHIVE, "CZ75-Auto paint kit ID");
ConVar loadout_deagle_skin("loadout_deagle_skin", "0", FCVAR_ARCHIVE, "Desert Eagle paint kit ID");
ConVar loadout_elite_skin("loadout_elite_skin", "0", FCVAR_ARCHIVE, "Dual Berettas paint kit ID");
ConVar loadout_fiveseven_skin("loadout_fiveseven_skin", "0", FCVAR_ARCHIVE, "Five-SeveN paint kit ID");
ConVar loadout_glock_skin("loadout_glock_skin", "0", FCVAR_ARCHIVE, "Glock-18 paint kit ID");
ConVar loadout_hkp2000_skin("loadout_hkp2000_skin", "0", FCVAR_ARCHIVE, "P2000 paint kit ID");
ConVar loadout_p250_skin("loadout_p250_skin", "0", FCVAR_ARCHIVE, "P250 paint kit ID");
ConVar loadout_revolver_skin("loadout_revolver_skin", "0", FCVAR_ARCHIVE, "R8 Revolver paint kit ID");
ConVar loadout_tec9_skin("loadout_tec9_skin", "0", FCVAR_ARCHIVE, "Tec-9 paint kit ID");
ConVar loadout_usp_silencer_skin("loadout_usp_silencer_skin", "0", FCVAR_ARCHIVE, "USP-S paint kit ID");

// Snipers
ConVar loadout_ssg08_skin("loadout_ssg08_skin", "0", FCVAR_ARCHIVE, "SSG 08 paint kit ID");

// Knives
ConVar loadout_knife_skin("loadout_knife_skin", "0", FCVAR_ARCHIVE, "Default Knife paint kit ID");
ConVar loadout_knife_bayonet_skin("loadout_knife_bayonet_skin", "0", FCVAR_ARCHIVE, "Bayonet paint kit ID");
ConVar loadout_knife_butterfly_skin("loadout_knife_butterfly_skin", "0", FCVAR_ARCHIVE, "Butterfly Knife paint kit ID");
ConVar loadout_knife_flip_skin("loadout_knife_flip_skin", "0", FCVAR_ARCHIVE, "Flip Knife paint kit ID");
ConVar loadout_knife_gut_skin("loadout_knife_gut_skin", "0", FCVAR_ARCHIVE, "Gut Knife paint kit ID");
ConVar loadout_knife_karambit_skin("loadout_knife_karambit_skin", "0", FCVAR_ARCHIVE, "Karambit paint kit ID");
ConVar loadout_knife_m9_bayonet_skin("loadout_knife_m9_bayonet_skin", "0", FCVAR_ARCHIVE, "M9 Bayonet paint kit ID");

// Gloves (остаются как есть - через material path)
ConVar loadout_glove_bloodhound_skin("loadout_glove_bloodhound_skin", "", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Gloves material");
ConVar loadout_glove_sporty_skin("loadout_glove_sporty_skin", "", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sport Gloves material");
ConVar loadout_glove_slick_skin("loadout_glove_slick_skin", "", FCVAR_ARCHIVE | FCVAR_USERINFO, "Driver Gloves material");
ConVar loadout_glove_handwrap_leathery_skin("loadout_glove_handwrap_leathery_skin", "", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hand Wraps material");
ConVar loadout_glove_motorcycle_skin("loadout_glove_motorcycle_skin", "", FCVAR_ARCHIVE | FCVAR_USERINFO, "Moto Gloves material");
ConVar loadout_glove_specialist_skin("loadout_glove_specialist_skin", "", FCVAR_ARCHIVE | FCVAR_USERINFO, "Specialist Gloves material");

// =============================================================================
// CONSTRUCTOR / DESTRUCTOR
// =============================================================================

CSkinProcessor::CSkinProcessor() 
    : m_pActiveBuyMenuWeaponMaterial(nullptr)
    , m_bInitialized(false)
{
}

CSkinProcessor::~CSkinProcessor()
{
    Shutdown();
}

// =============================================================================
// INITIALIZATION
// =============================================================================

bool CSkinProcessor::Initialize()
{
    if (m_bInitialized)
        return true;
    
    DevMsg("[SkinProcessor] Initializing...\n");
    
    // Инициализируем weapon skins
    InitializeWeaponSkins();
    
    // Инициализируем glove skins
    InitializeGloveSkins();
    
    // Инициализируем базу данных скинов
    g_SkinDatabase.Initialize();
    
    m_bInitialized = true;
    DevMsg("[SkinProcessor] Initialized successfully\n");
    
    return true;
}

void CSkinProcessor::Shutdown()
{
    if (!m_bInitialized)
        return;
    
    DevMsg("[SkinProcessor] Shutting down...\n");
    
    Clear();
    m_bInitialized = false;
}

void CSkinProcessor::InitializeWeaponSkins()
{
    m_WeaponSkins.EnsureCapacity(50);
    
    // Rifles
    AddWeaponSkin("weapon_ak47", &loadout_ak47_skin);
    AddWeaponSkin("weapon_aug", &loadout_aug_skin);
    AddWeaponSkin("weapon_awp", &loadout_awp_skin);
    AddWeaponSkin("weapon_famas", &loadout_famas_skin);
    AddWeaponSkin("weapon_galilar", &loadout_galilar_skin);
    AddWeaponSkin("weapon_m4a1_silencer", &loadout_m4a1_silencer_skin);
    AddWeaponSkin("weapon_m4a4", &loadout_m4a4_skin);
    AddWeaponSkin("weapon_sg556", &loadout_sg556_skin);
    AddWeaponSkin("weapon_scar20", &loadout_scar20_skin);
    AddWeaponSkin("weapon_g3sg1", &loadout_g3sg1_skin);
    
    // SMGs
    AddWeaponSkin("weapon_bizon", &loadout_bizon_skin);
    AddWeaponSkin("weapon_mac10", &loadout_mac10_skin);
    AddWeaponSkin("weapon_mp5sd", &loadout_mp5sd_skin);
    AddWeaponSkin("weapon_mp7", &loadout_mp7_skin);
    AddWeaponSkin("weapon_mp9", &loadout_mp9_skin);
    AddWeaponSkin("weapon_p90", &loadout_p90_skin);
    AddWeaponSkin("weapon_ump45", &loadout_ump45_skin);
    
    // Heavy
    AddWeaponSkin("weapon_m249", &loadout_m249_skin);
    AddWeaponSkin("weapon_negev", &loadout_negev_skin);
    AddWeaponSkin("weapon_mag7", &loadout_mag7_skin);
    AddWeaponSkin("weapon_nova", &loadout_nova_skin);
    AddWeaponSkin("weapon_sawedoff", &loadout_sawedoff_skin);
    AddWeaponSkin("weapon_xm1014", &loadout_xm1014_skin);
    
    // Pistols
    AddWeaponSkin("weapon_cz75a", &loadout_cz75a_skin);
    AddWeaponSkin("weapon_deagle", &loadout_deagle_skin);
    AddWeaponSkin("weapon_elite", &loadout_elite_skin);
    AddWeaponSkin("weapon_fiveseven", &loadout_fiveseven_skin);
    AddWeaponSkin("weapon_glock", &loadout_glock_skin);
    AddWeaponSkin("weapon_hkp2000", &loadout_hkp2000_skin);
    AddWeaponSkin("weapon_p250", &loadout_p250_skin);
    AddWeaponSkin("weapon_revolver", &loadout_revolver_skin);
    AddWeaponSkin("weapon_tec9", &loadout_tec9_skin);
    AddWeaponSkin("weapon_usp_silencer", &loadout_usp_silencer_skin);
    
    // Snipers
    AddWeaponSkin("weapon_ssg08", &loadout_ssg08_skin);
    
    // Knives
    AddWeaponSkin("weapon_knife", &loadout_knife_skin);
    AddWeaponSkin("weapon_knife_bayonet", &loadout_knife_bayonet_skin);
    AddWeaponSkin("weapon_knife_butterfly", &loadout_knife_butterfly_skin);
    AddWeaponSkin("weapon_knife_flip", &loadout_knife_flip_skin);
    AddWeaponSkin("weapon_knife_gut", &loadout_knife_gut_skin);
    AddWeaponSkin("weapon_knife_karambit", &loadout_knife_karambit_skin);
    AddWeaponSkin("weapon_knife_m9_bayonet", &loadout_knife_m9_bayonet_skin);
}

void CSkinProcessor::InitializeGloveSkins()
{
    m_GloveSkinConVars.EnsureCapacity(10);
    
    m_GloveSkinConVars.AddToTail(&loadout_glove_bloodhound_skin);
    m_GloveSkinConVars.AddToTail(&loadout_glove_sporty_skin);
    m_GloveSkinConVars.AddToTail(&loadout_glove_slick_skin);
    m_GloveSkinConVars.AddToTail(&loadout_glove_handwrap_leathery_skin);
    m_GloveSkinConVars.AddToTail(&loadout_glove_motorcycle_skin);
    m_GloveSkinConVars.AddToTail(&loadout_glove_specialist_skin);
}

void CSkinProcessor::AddWeaponSkin(const char* pszClass, ConVar* pConVar)
{
    if (!pszClass || !pConVar)
        return;
        
    WeaponSkinCache_t skin;
    skin.pszWeaponClass = pszClass;
    skin.pSkinConVar = pConVar;
    skin.pMaterial = nullptr;
    
    m_WeaponSkins.AddToTail(skin);
}

// =============================================================================
// MAIN MATERIAL RETRIEVAL
// =============================================================================

IMaterial* CSkinProcessor::GetSkinMaterial(C_BaseCombatWeapon* pWeapon)
{
    return GetSkinMaterialForWeapon(pWeapon);
}

IMaterial* CSkinProcessor::GetSkinMaterialForWeapon(C_BaseCombatWeapon* pWeapon)
{
    if (!pWeapon)
        return nullptr;

    const char* pszClass = pWeapon->GetClassname();
    if (!pszClass || !pszClass[0])
        return nullptr;

    return GetSkinMaterialForClass(pszClass);
}

IMaterial* CSkinProcessor::GetSkinMaterialBySkinInfo(CSWeaponID weaponID, const SkinInfo_t& skinInfo)
{
    if (!m_bInitialized || !skinInfo.IsValid())
        return nullptr;
    
    // Используем базу данных для получения материала
    return g_SkinDatabase.GetSkinMaterial(skinInfo.iPaintKit);
}

IMaterial* CSkinProcessor::GetSkinMaterialForClass(const char* pszClass)
{
    if (!pszClass || !pszClass[0])
        return nullptr;

    WeaponSkinCache_t* pSkin = FindSkinEntry(pszClass);
    if (!pSkin || !pSkin->pSkinConVar)
        return nullptr;

    // Получаем paint kit ID из ConVar
    int iPaintKit = pSkin->pSkinConVar->GetInt();
    
    // Если paint kit = 0, используем стандартный скин
    if (iPaintKit <= 0)
        return nullptr;

    // Проверяем нужно ли перезагрузить материал
    if (iPaintKit != pSkin->iLastPaintKit || !pSkin->pMaterial)
    {
        ReleaseMaterial(pSkin);
        pSkin->pMaterial = LoadMaterialByPaintKit(*pSkin, iPaintKit);
        pSkin->iLastPaintKit = iPaintKit;
    }

    return pSkin->pMaterial;
}

IMaterial* CSkinProcessor::GetWeaponMaterial(CSWeaponID weaponID)
{
    // Этот метод нуждается в функции WeaponIDToAlias
    // Если у вас её нет, закомментируйте этот метод
    return nullptr;
}

ITexture* CSkinProcessor::GetSkinIconForWeapon(C_BaseCombatWeapon* pWeapon)
{
    if (!pWeapon)
        return nullptr;

    const char* pszClass = pWeapon->GetClassname();
    if (!pszClass || !pszClass[0])
        return nullptr;

    WeaponSkinCache_t* pSkin = FindSkinEntry(pszClass);
    if (!pSkin || !pSkin->pSkinConVar)
        return nullptr;

    int iPaintKit = pSkin->pSkinConVar->GetInt();
    if (iPaintKit <= 0)
        return nullptr;

    return g_SkinDatabase.GetSkinIcon(iPaintKit);
}

ITexture* CSkinProcessor::GetSkinIcon(CSWeaponID weaponID)
{
    return nullptr;
}

// =============================================================================
// MATERIAL LOADING
// =============================================================================

IMaterial* CSkinProcessor::LoadMaterialByPaintKit(WeaponSkinCache_t& skin, int iPaintKit)
{
    if (iPaintKit <= 0)
        return nullptr;

    IMaterial* pMat = g_SkinDatabase.GetSkinMaterial(iPaintKit);
    
    if (pMat)
    {
        DevMsg("[SkinProcessor] Loaded paint kit %d for %s\n", iPaintKit, skin.pszWeaponClass);
    }
    else
    {
        Warning("[SkinProcessor] Failed to load paint kit %d for %s\n", iPaintKit, skin.pszWeaponClass);
    }
    
    return pMat;
}

void CSkinProcessor::ReleaseMaterial(WeaponSkinCache_t* pSkin)
{
    if (!pSkin || !pSkin->pMaterial)
        return;

    // Материалы из базы данных управляются самой БД
    // Мы только очищаем указатель
    pSkin->pMaterial = nullptr;
}

// =============================================================================
// LOOKUP FUNCTIONS
// =============================================================================

WeaponSkinCache_t* CSkinProcessor::FindSkinEntry(const char* pszClass)
{
    if (!pszClass || !pszClass[0])
        return nullptr;

    FOR_EACH_VEC(m_WeaponSkins, i)
    {
        if (FStrEq(m_WeaponSkins[i].pszWeaponClass, pszClass))
            return &m_WeaponSkins[i];
    }

    return nullptr;
}

// =============================================================================
// GLOVE SKINS
// =============================================================================

const char* CSkinProcessor::GetGloveSkin(int gloveID)
{
    if (gloveID < 0 || gloveID >= m_GloveSkinConVars.Count())
        return nullptr;

    ConVar* pConVar = m_GloveSkinConVars[gloveID];
    if (!pConVar)
        return nullptr;

    const char* pszSkin = pConVar->GetString();
    if (!pszSkin || !pszSkin[0])
        return nullptr;

    return pszSkin;
}

int CSkinProcessor::GetGloveCount() const
{
    return m_GloveSkinConVars.Count();
}

// =============================================================================
// BUY MENU SUPPORT
// =============================================================================

void CSkinProcessor::SetActiveBuyMenuWeaponMaterial(IMaterial* pMat)
{
    m_pActiveBuyMenuWeaponMaterial = pMat;
}

void CSkinProcessor::ResetActiveBuyMenuWeaponMaterial()
{
    m_pActiveBuyMenuWeaponMaterial = nullptr;
}

IMaterial* CSkinProcessor::GetActiveBuyMenuWeaponMaterial() const
{
    return m_pActiveBuyMenuWeaponMaterial;
}

// =============================================================================
// CLEANUP
// =============================================================================

void CSkinProcessor::Clear()
{
    FOR_EACH_VEC(m_WeaponSkins, i)
    {
        ReleaseMaterial(&m_WeaponSkins[i]);
    }

    m_pActiveBuyMenuWeaponMaterial = nullptr;
}

void CSkinProcessor::ReloadAllMaterials()
{
    DevMsg("[SkinProcessor] Reloading all materials...\n");
    
    // Очищаем кэш базы данных
    g_SkinDatabase.ClearMaterialCache();
    
    // Перезагружаем материалы
    FOR_EACH_VEC(m_WeaponSkins, i)
    {
        WeaponSkinCache_t& skin = m_WeaponSkins[i];
        
        if (!skin.pSkinConVar)
            continue;
            
        int iPaintKit = skin.pSkinConVar->GetInt();
        if (iPaintKit <= 0)
            continue;
        
        ReleaseMaterial(&skin);
        skin.pMaterial = LoadMaterialByPaintKit(skin, iPaintKit);
    }
    
    DevMsg("[SkinProcessor] Material reload complete.\n");
}