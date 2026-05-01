#ifndef CS_SKIN_SHAREDDEFS_H
#define CS_SKIN_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "Color.h"

// =============================================================================
// SKIN SYSTEM CONSTANTS
// =============================================================================

#define SKIN_ID_INVALID     -1
#define SKIN_ID_DEFAULT     0
#define MAX_SKIN_NAME       64

// =============================================================================
// SKIN RARITY
// =============================================================================

enum ESkinRarity
{
    SKIN_RARITY_COMMON = 0,         // Consumer Grade (White)
    SKIN_RARITY_UNCOMMON,           // Industrial Grade (Light Blue)
    SKIN_RARITY_RARE,               // Mil-Spec (Blue)
    SKIN_RARITY_MYTHICAL,           // Restricted (Purple)
    SKIN_RARITY_LEGENDARY,          // Classified (Pink)
    SKIN_RARITY_ANCIENT,            // Covert (Red)
    SKIN_RARITY_CONTRABAND,         // Contraband (Gold/Yellow)
    
    SKIN_RARITY_COUNT
};

// =============================================================================
// SKIN INFO STRUCTURE (Network Transmitted)
// =============================================================================

struct SkinInfo_t
{
    int iPaintKit;      // Paint kit ID (идентификатор скина в базе)
    
    SkinInfo_t()
    {
        iPaintKit = 0;
    }
    
    SkinInfo_t(int paintkit)
    {
        iPaintKit = paintkit;
    }
    
    bool IsValid() const
    {
        return iPaintKit > 0;
    }
};

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

inline const char* GetRarityName(ESkinRarity rarity)
{
    switch (rarity)
    {
        case SKIN_RARITY_COMMON:        return "Consumer Grade";
        case SKIN_RARITY_UNCOMMON:      return "Industrial Grade";
        case SKIN_RARITY_RARE:          return "Mil-Spec";
        case SKIN_RARITY_MYTHICAL:      return "Restricted";
        case SKIN_RARITY_LEGENDARY:     return "Classified";
        case SKIN_RARITY_ANCIENT:       return "Covert";
        case SKIN_RARITY_CONTRABAND:    return "Contraband";
        default:                        return "Unknown";
    }
}

inline Color GetRarityColor(ESkinRarity rarity)
{
    switch (rarity)
    {
        case SKIN_RARITY_COMMON:        return Color(176, 195, 217, 255);  // White
        case SKIN_RARITY_UNCOMMON:      return Color(94, 152, 217, 255);   // Light Blue
        case SKIN_RARITY_RARE:          return Color(75, 105, 255, 255);   // Blue
        case SKIN_RARITY_MYTHICAL:      return Color(136, 71, 255, 255);   // Purple
        case SKIN_RARITY_LEGENDARY:     return Color(211, 44, 230, 255);   // Pink
        case SKIN_RARITY_ANCIENT:       return Color(235, 75, 75, 255);    // Red
        case SKIN_RARITY_CONTRABAND:    return Color(228, 174, 57, 255);   // Gold
        default:                        return Color(255, 255, 255, 255);
    }
}

#endif // CS_SKIN_SHAREDDEFS_H