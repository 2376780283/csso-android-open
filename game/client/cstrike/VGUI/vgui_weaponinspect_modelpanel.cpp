#include "cbase.h"
#include "vgui_weaponinspect_modelpanel.h"
#include "vgui_controls/AnimationController.h"
#include "matsys_controls/matsyscontrols.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "istudiorender.h"
#include "bone_setup.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "SkinProcessor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CWeaponModelPanel::CWeaponModelPanel( Panel* parent, const char* panelName ) 
	: BaseClass( parent, panelName )
{
	m_nCurrentWeapon = -1;
	m_bUseSkinProcessor = true;  // По умолчанию используем SkinProcessor
	m_bAutoRotate = false;
	m_flAutoRotateSpeed = 45.0f;
	m_vecWeaponOffset.Init( 0, 0, 0 );
    m_vecOffset = Vector(-25, 0, 0); 
	m_flWeaponScale = 1.0f;
	SetIdentityMatrix( m_WeaponTransform );

	// Настройки камеры по умолчанию для оружия
	m_Camera.m_origin.Init( 50.0f, 0.0f, 0.0f );
	m_Camera.m_angles.Init( 0.0f, 180.0f, 0.0f );
	m_Camera.m_flFOV = 54.0f;
	
	// Убеждаемся что SkinProcessor инициализирован
	g_SkinProcessor.Initialize();
}

CWeaponModelPanel::~CWeaponModelPanel()
{
	ClearAllMaterialOverrides();
	m_WeaponList.Purge();
}

void CWeaponModelPanel::SetOffset(const Vector& vec) { m_vecOffset = vec; }

void CWeaponModelPanel::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	// Использование SkinProcessor
	m_bUseSkinProcessor = inResourceData->GetBool( "use_skin_processor", true );

	// Загрузка списка оружия из ресурсов
	KeyValues* pWeaponList = inResourceData->FindKey( "weapon_list" );
	if ( pWeaponList )
	{
		CUtlVector<const char*> tempList;
		for ( KeyValues* pWeapon = pWeaponList->GetFirstSubKey(); pWeapon; pWeapon = pWeapon->GetNextKey() )
		{
			const char* pPath = pWeapon->GetString();
			if ( pPath && pPath[0] )
			{
				tempList.AddToTail( pPath );
			}
		}
		
		if ( tempList.Count() > 0 )
		{
			SetWeaponList( tempList );
		}
	}

	// Регистрация оружия с ConVar'ами для SkinProcessor
	KeyValues* pWeaponClasses = inResourceData->FindKey( "weapon_classes" );
	if ( pWeaponClasses && m_bUseSkinProcessor )
	{
		for ( KeyValues* pClass = pWeaponClasses->GetFirstSubKey(); pClass; pClass = pClass->GetNextKey() )
		{
			const char* pWeaponName = pClass->GetName();
			const char* pWeaponClass = pClass->GetString();
			
			if ( pWeaponName && pWeaponName[0] && pWeaponClass && pWeaponClass[0] )
			{
				RegisterWeaponWithConVar( pWeaponName, pWeaponClass );
			}
		}
	}

	// Ручные переопределения материалов (если не используем SkinProcessor)
	KeyValues* pMaterialOverrides = inResourceData->FindKey( "material_overrides" );
	if ( pMaterialOverrides && !m_bUseSkinProcessor )
	{
		for ( KeyValues* pOverride = pMaterialOverrides->GetFirstSubKey(); pOverride; pOverride = pOverride->GetNextKey() )
		{
			const char* pWeaponName = pOverride->GetName();
			const char* pMaterialName = pOverride->GetString();
			
			if ( pWeaponName && pWeaponName[0] && pMaterialName && pMaterialName[0] )
			{
				SetWeaponMaterialOverride( pWeaponName, pMaterialName );
			}
		}
	}

	// Глобальный material override
	const char* pGlobalMaterial = inResourceData->GetString( "global_material_override", "" );
	if ( pGlobalMaterial[0] != 0 )
	{
		SetGlobalMaterialOverride( pGlobalMaterial );
	}

	// Параметры отображения
	m_bAutoRotate = inResourceData->GetBool( "auto_rotate", false );
	m_flAutoRotateSpeed = inResourceData->GetFloat( "rotation_speed", 45.0f );
	m_flWeaponScale = inResourceData->GetFloat( "weapon_scale", 1.0f );

	const char* pOffset = inResourceData->GetString( "weapon_offset", "" );
	if ( pOffset[0] != 0 )
	{
		sscanf( pOffset, "%f %f %f", &m_vecWeaponOffset.x, &m_vecWeaponOffset.y, &m_vecWeaponOffset.z );
	}

	// Загрузка первого оружия по умолчанию
	int nStartIndex = inResourceData->GetInt( "start_weapon", 0 );
	if ( m_WeaponList.Count() > 0 )
	{
		SetCurrentWeapon( nStartIndex );
	}
}

void CWeaponModelPanel::SetWeaponList( const CUtlVector<const char*>& weaponPaths )
{
	m_WeaponList.Purge();
	m_nCurrentWeapon = -1;

	for ( int i = 0; i < weaponPaths.Count(); ++i )
	{
		WeaponInfo_t info;
		V_strncpy( info.szPath, weaponPaths[i], sizeof( info.szPath ) );
		ExtractWeaponName( weaponPaths[i], info.szName, sizeof( info.szName ) );
		info.szWeaponClass[0] = '\0';
		info.pMaterialOverride.Init( (IMaterial*)NULL );
		info.bRegisteredWithSkinProcessor = false;
		m_WeaponList.AddToTail( info );
	}
}

void CWeaponModelPanel::SetCurrentWeapon( int nIndex )
{
	if ( nIndex < 0 || nIndex >= m_WeaponList.Count() )
	{
		Warning( "CWeaponModelPanel::SetCurrentWeapon - Invalid weapon index %d\n", nIndex );
		return;
	}

	m_nCurrentWeapon = nIndex;
	SetMDL( m_WeaponList[nIndex].szPath );
	
	// Сброс анимации и времени
	m_MDL.m_flTime = 0.0f;
	m_MDL.m_nSequence = 0;

	// Обновление трансформации
	UpdateWeaponTransform();
	
	DevMsg( "CWeaponModelPanel: Switched to weapon %s (%s)\n", 
			m_WeaponList[nIndex].szName, 
			m_WeaponList[nIndex].szWeaponClass );
}

void CWeaponModelPanel::SetCurrentWeapon( const char* pWeaponName )
{
	for ( int i = 0; i < m_WeaponList.Count(); ++i )
	{
		if ( V_stricmp( m_WeaponList[i].szName, pWeaponName ) == 0 )
		{
			SetCurrentWeapon( i );
			return;
		}
	}

	Warning( "CWeaponModelPanel::SetCurrentWeapon - Weapon '%s' not found in list\n", pWeaponName );
}

void CWeaponModelPanel::RegisterWeaponWithConVar( int nIndex, const char* pWeaponClass )
{
	if ( nIndex < 0 || nIndex >= m_WeaponList.Count() )
	{
		Warning( "CWeaponModelPanel::RegisterWeaponWithConVar - Invalid weapon index %d\n", nIndex );
		return;
	}

	if ( !pWeaponClass || !pWeaponClass[0] )
	{
		Warning( "CWeaponModelPanel::RegisterWeaponWithConVar - Invalid weapon class\n" );
		return;
	}

	V_strncpy( m_WeaponList[nIndex].szWeaponClass, pWeaponClass, sizeof( m_WeaponList[nIndex].szWeaponClass ) );
	m_WeaponList[nIndex].bRegisteredWithSkinProcessor = true;
	
	DevMsg( "CWeaponModelPanel: Registered weapon %s with class %s\n", 
			m_WeaponList[nIndex].szName, pWeaponClass );
}

void CWeaponModelPanel::RegisterWeaponWithConVar( const char* pWeaponName, const char* pWeaponClass )
{
	for ( int i = 0; i < m_WeaponList.Count(); ++i )
	{
		if ( V_stricmp( m_WeaponList[i].szName, pWeaponName ) == 0 )
		{
			RegisterWeaponWithConVar( i, pWeaponClass );
			return;
		}
	}

	Warning( "CWeaponModelPanel::RegisterWeaponWithConVar - Weapon '%s' not found in list\n", pWeaponName );
}

void CWeaponModelPanel::RefreshCurrentWeaponSkin()
{
	// Принудительно обновить скин текущего оружия из ConVar
	if ( m_nCurrentWeapon >= 0 && m_nCurrentWeapon < m_WeaponList.Count() )
	{
		WeaponInfo_t& weapon = m_WeaponList[m_nCurrentWeapon];
		
		if ( weapon.bRegisteredWithSkinProcessor && weapon.szWeaponClass[0] )
		{
			DevMsg( "CWeaponModelPanel: Refreshing skin for %s\n", weapon.szWeaponClass );
		}
	}
}

void CWeaponModelPanel::SetWeaponMaterialOverride( int nWeaponIndex, const char* pMaterialName )
{
	if ( nWeaponIndex < 0 || nWeaponIndex >= m_WeaponList.Count() )
	{
		Warning( "CWeaponModelPanel::SetWeaponMaterialOverride - Invalid weapon index %d\n", nWeaponIndex );
		return;
	}

	if ( !pMaterialName || pMaterialName[0] == 0 )
	{
		ClearWeaponMaterialOverride( nWeaponIndex );
		return;
	}

	IMaterial* pMaterial = vgui::MaterialSystem()->FindMaterial( pMaterialName, TEXTURE_GROUP_OTHER, true );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
	{
		Warning( "CWeaponModelPanel::SetWeaponMaterialOverride - Material '%s' not found\n", pMaterialName );
		return;
	}

	m_WeaponList[nWeaponIndex].pMaterialOverride.Init( pMaterial );
}

void CWeaponModelPanel::SetWeaponMaterialOverride( const char* pWeaponName, const char* pMaterialName )
{
	for ( int i = 0; i < m_WeaponList.Count(); ++i )
	{
		if ( V_stricmp( m_WeaponList[i].szName, pWeaponName ) == 0 )
		{
			SetWeaponMaterialOverride( i, pMaterialName );
			return;
		}
	}

	Warning( "CWeaponModelPanel::SetWeaponMaterialOverride - Weapon '%s' not found in list\n", pWeaponName );
}

void CWeaponModelPanel::ClearWeaponMaterialOverride( int nWeaponIndex )
{
	if ( nWeaponIndex < 0 || nWeaponIndex >= m_WeaponList.Count() )
		return;

	m_WeaponList[nWeaponIndex].pMaterialOverride.Init( (IMaterial*)NULL ); 
}

void CWeaponModelPanel::ClearWeaponMaterialOverride( const char* pWeaponName )
{
	for ( int i = 0; i < m_WeaponList.Count(); ++i )
	{
		if ( V_stricmp( m_WeaponList[i].szName, pWeaponName ) == 0 )
		{
			ClearWeaponMaterialOverride( i );
			return;
		}
	}
}

void CWeaponModelPanel::ClearAllMaterialOverrides()
{
	for ( int i = 0; i < m_WeaponList.Count(); ++i )
	{
		m_WeaponList[i].pMaterialOverride.Init( (IMaterial*)NULL );
	}
	
	ClearGlobalMaterialOverride();
}

void CWeaponModelPanel::SetGlobalMaterialOverride( const char* pMaterialName )
{
	if ( !pMaterialName || pMaterialName[0] == 0 )
	{
		ClearGlobalMaterialOverride();
		return;
	}

	IMaterial* pMaterial = vgui::MaterialSystem()->FindMaterial( pMaterialName, TEXTURE_GROUP_OTHER, true );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
	{
		Warning( "CWeaponModelPanel::SetGlobalMaterialOverride - Material '%s' not found\n", pMaterialName );
		return;
	}

	m_pGlobalMaterialOverride.Init( pMaterial );
}

void CWeaponModelPanel::ClearGlobalMaterialOverride()
{
	m_pGlobalMaterialOverride.Init( (IMaterial*)NULL );
}

IMaterial* CWeaponModelPanel::GetMaterialOverrideForCurrentWeapon()
{
	if ( m_nCurrentWeapon < 0 || m_nCurrentWeapon >= m_WeaponList.Count() )
		return NULL;

	WeaponInfo_t& weapon = m_WeaponList[m_nCurrentWeapon];

	// Приоритет 1: Ручной override для конкретного оружия
	if ( weapon.pMaterialOverride.IsValid() )
	{
		return weapon.pMaterialOverride;
	}

	// Приоритет 2: SkinProcessor (если включен и оружие зарегистрировано)
	if ( m_bUseSkinProcessor && weapon.bRegisteredWithSkinProcessor && weapon.szWeaponClass[0] )
	{
		IMaterial* pSkinMat = g_SkinProcessor.GetSkinMaterialForClass( weapon.szWeaponClass );
		if ( pSkinMat )
		{
			return pSkinMat;
		}
	}

	// Приоритет 3: Глобальный override
	if ( m_pGlobalMaterialOverride.IsValid() )
	{
		return m_pGlobalMaterialOverride;
	}

	return NULL;
}

const char* CWeaponModelPanel::GetWeaponPath( int nIndex ) const
{
	if ( nIndex < 0 || nIndex >= m_WeaponList.Count() )
		return NULL;

	return m_WeaponList[nIndex].szPath;
}

const char* CWeaponModelPanel::GetWeaponName( int nIndex ) const
{
	if ( nIndex < 0 || nIndex >= m_WeaponList.Count() )
		return NULL;

	return m_WeaponList[nIndex].szName;
}

const char* CWeaponModelPanel::GetWeaponClass( int nIndex ) const
{
	if ( nIndex < 0 || nIndex >= m_WeaponList.Count() )
		return NULL;

	return m_WeaponList[nIndex].szWeaponClass;
}

void CWeaponModelPanel::NextWeapon()
{
	if ( m_WeaponList.Count() == 0 )
		return;

	int nNext = m_nCurrentWeapon + 1;
	if ( nNext >= m_WeaponList.Count() )
		nNext = 0;

	SetCurrentWeapon( nNext );
}

void CWeaponModelPanel::PreviousWeapon()
{
	if ( m_WeaponList.Count() == 0 )
		return;

	int nPrev = m_nCurrentWeapon - 1;
	if ( nPrev < 0 )
		nPrev = m_WeaponList.Count() - 1;

	SetCurrentWeapon( nPrev );
}

void CWeaponModelPanel::OnThink()
{
	BaseClass::OnThink();

	// Автоматическое вращение
	if ( m_bAutoRotate && m_MDL.GetMDL() != MDLHANDLE_INVALID )
	{
		m_angPlayerModel.y += m_flAutoRotateSpeed * gpGlobals->frametime;
		
		if ( m_angPlayerModel.y > 360.0f )
			m_angPlayerModel.y -= 360.0f;

		UpdateWeaponTransform();
	}
}

void CWeaponModelPanel::UpdateWeaponTransform()
{
    if ( m_MDL.GetMDL() == MDLHANDLE_INVALID )
        return;

    matrix3x4_t matRotate;
    AngleMatrix( m_angPlayerModel, matRotate );

    Vector vecLocalOffset = m_vecWeaponOffset;

    matrix3x4_t matOffset;
    SetIdentityMatrix(matOffset);
    MatrixSetColumn(vecLocalOffset, 3, matOffset);

    matrix3x4_t matTemp;
    ConcatTransforms(matRotate, matOffset, matTemp);

    m_MDLToWorld = matTemp;
}

void CWeaponModelPanel::OnPaint3D()
{
    if ( m_MDL.GetMDL() == MDLHANDLE_INVALID || m_nCurrentWeapon < 0 )
        return;

    MDLCACHE_CRITICAL_SECTION();

    CMatRenderContextPtr pRenderContext( vgui::MaterialSystem() );
    pRenderContext->BindLocalCubemap( m_DefaultEnvCubemap );

    // --- material override ---
    IMaterial* pMaterialOverride = GetMaterialOverrideForCurrentWeapon();
    if ( pMaterialOverride )
        modelrender->ForcedMaterialOverride( pMaterialOverride );

    // --- подготовка студии ---
    CStudioHdr studioHdr( g_pMDLCache->GetStudioHdr( m_MDL.GetMDL() ), g_pMDLCache );
    matrix3x4_t* pBoneToWorld = g_pStudioRender->LockBoneMatrices( studioHdr.numbones() );

    // --- центрирование pivot ---
    matrix3x4_t matToCenter, matFromCenter, matTemp;

    // Сдвигаем модель к (0,0,0)
    SetIdentityMatrix( matToCenter );
    MatrixSetColumn( -m_vecModelCenter, 3, matToCenter );

    // Возвращаем обратно + offset
    SetIdentityMatrix( matFromCenter );
    MatrixSetColumn( m_vecModelCenter + m_vecWeaponOffset, 3, matFromCenter );

    // Комбинируем с текущей матрицей m_MDLToWorld (вращение/масштаб)
    ConcatTransforms( matToCenter, m_MDLToWorld, matTemp );
    ConcatTransforms( matFromCenter, matTemp, m_MDLToWorld );

    // --- Setup bones и отрисовка ---
    m_MDL.SetUpBones( m_MDLToWorld, studioHdr.numbones(), pBoneToWorld );
    g_pStudioRender->UnlockBoneMatrices();

    m_MDL.Draw( m_MDLToWorld, pBoneToWorld );

    // --- сброс override ---
    if ( pMaterialOverride )
        modelrender->ForcedMaterialOverride( nullptr );

    pRenderContext->Flush();
}

void CWeaponModelPanel::SetMaterialOverride( IMaterial *pMaterial )
{
    modelrender->ForcedMaterialOverride( pMaterial, OVERRIDE_SELECTIVE, 0 );
}

void CWeaponModelPanel::ExtractWeaponName( const char* pPath, char* pOutName, int nMaxLen )
{
	// Извлекаем имя файла из пути
	const char* pFileName = V_GetFileName( pPath );
	
	// Копируем без расширения
	V_StripExtension( pFileName, pOutName, nMaxLen );
}

void CWeaponModelPanel::PlaySequence( const char* pszSequenceName )
{
	CStudioHdr studioHDR( m_MDL.GetStudioHdr(), g_pMDLCache );
	int iSeq = ::LookupSequence( &studioHDR, pszSequenceName );
	if ( iSeq != ACT_INVALID )
	{
		m_MDL.m_nSequence = iSeq;
		m_MDL.m_flTime = 0.0f;
	}
}