#ifndef WEAPONMODELPANEL_H
#define WEAPONMODELPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "PlayerModelPanel.h"
#include "SkinProcessor.h"
#include "renderparm.h"
#include "animation.h"

//-----------------------------------------------------------------------------
// Weapon model display panel with SkinProcessor integration
//-----------------------------------------------------------------------------
class CWeaponModelPanel : public CBasePlayerModelPanel
{
	DECLARE_CLASS_SIMPLE( CWeaponModelPanel, CBasePlayerModelPanel );

public:
	CWeaponModelPanel( vgui::Panel* parent, const char* panelName );
	virtual ~CWeaponModelPanel();

	// Weapon management
	void SetWeaponList( const CUtlVector<const char*>& weaponPaths );
	void SetCurrentWeapon( int nIndex );
	void SetCurrentWeapon( const char* pWeaponName );
	int GetCurrentWeaponIndex() const { return m_nCurrentWeapon; }
	int GetWeaponCount() const { return m_WeaponList.Count(); }
	const char* GetWeaponPath( int nIndex ) const;
	const char* GetWeaponName( int nIndex ) const;
	const char* GetWeaponClass( int nIndex ) const;
	
	// Skin processor integration
	void SetUseSkinProcessor( bool bUse ) { m_bUseSkinProcessor = bUse; }
	bool IsUsingSkinProcessor() const { return m_bUseSkinProcessor; }
	void RegisterWeaponWithConVar( int nIndex, const char* pWeaponClass );
	void RegisterWeaponWithConVar( const char* pWeaponName, const char* pWeaponClass );
	void RefreshCurrentWeaponSkin();
	
	// Manual material override (bypasses skin processor)
	void SetWeaponMaterialOverride( int nWeaponIndex, const char* pMaterialName );
	void SetWeaponMaterialOverride( const char* pWeaponName, const char* pMaterialName );
	void ClearWeaponMaterialOverride( int nWeaponIndex );
	void ClearWeaponMaterialOverride( const char* pWeaponName );
	void ClearAllMaterialOverrides();
    void SetMaterialOverride( IMaterial *pMaterial );
	
	// Global material override (applies to all weapons)
	void SetGlobalMaterialOverride( const char* pMaterialName );
	void ClearGlobalMaterialOverride();
	
	// Navigation
	void NextWeapon();
	void PreviousWeapon();

	// Display options
	void SetAutoRotate( bool bEnable ) { m_bAutoRotate = bEnable; }
	void SetRotationSpeed( float flSpeed ) { m_flAutoRotateSpeed = flSpeed; }
	void SetWeaponOffset( const Vector& vecOffset ) { m_vecWeaponOffset = vecOffset; }
	void SetWeaponScale( float flScale ) { m_flWeaponScale = flScale; }

	virtual void OnThink() OVERRIDE;
	virtual void ApplySettings( KeyValues* inResourceData ) OVERRIDE;
    virtual void PlaySequence( const char* pszSequenceName );
    void SetOffset(const Vector& vec);
    
    void UpdateWeaponTransform();
	void ExtractWeaponName( const char* pPath, char* pOutName, int nMaxLen );
	IMaterial* GetMaterialOverrideForCurrentWeapon();

	struct WeaponInfo_t
	{
		char szPath[MAX_PATH];
		char szName[64];
		char szWeaponClass[64];  // weapon_ak47, weapon_awp, etc.
		CMaterialReference pMaterialOverride; // Manual override
		bool bRegisteredWithSkinProcessor;
	};

	CUtlVector<WeaponInfo_t> m_WeaponList;
	int m_nCurrentWeapon;
	
	// Skin processor
	bool m_bUseSkinProcessor;
	
	// Global material override
	CMaterialReference m_pGlobalMaterialOverride;
	
	// Display parameters
	bool m_bAutoRotate;
	float m_flAutoRotateSpeed;
	Vector m_vecWeaponOffset;
    Vector m_vecOffset;
	float m_flWeaponScale;
	matrix3x4_t m_WeaponTransform;

protected:
	virtual void OnPaint3D();
};

#endif // WEAPONMODELPANEL_H 