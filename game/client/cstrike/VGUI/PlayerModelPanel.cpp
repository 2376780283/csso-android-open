#include "cbase.h"
#include "PlayerModelPanel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/MenuItem.h"
#include "vgui_controls/PHandle.h"
#include "vgui_controls/MessageBox.h"
#include "vgui_controls/QueryBox.h"
#include "vgui_controls/ControllerMap.h"
#include "vgui_controls/KeyRepeat.h"
#include "matsys_controls/matsyscontrols.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "istudiorender.h"
#include "vgui/IInput.h"
#include "inputsystem/iinputsystem.h"
#include "renderparm.h"
#include "animation.h"
#include "cs_shareddefs.h"
#include "bone_setup.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CBasePlayerModelPanel::CBasePlayerModelPanel( Panel* parent, const char* panelName ): BaseClass( parent, panelName )
{
	m_DefaultEnvCubemap.Init( vgui::MaterialSystem()->FindTexture( "editor/cube_vertigo", NULL, true ) );
	m_Camera.m_origin = vec3_origin;
	m_Camera.m_angles = vec3_angle;
	m_Camera.m_flZNear = 3.0f;
	m_Camera.m_flZFar = 16384.0f * 1.73205080757f;
	m_Camera.m_flFOV = 30.0f;
	m_nNumLightDescs = 0;
	m_bMousePressed = false;
	m_flRotationAngleLeft = 0.0f;
	m_flRotationTimeLeft = 0.0f;
	m_angPlayerModel.Init();
	SetIdentityMatrix( m_MDLToWorld );

	memset( &m_pLightDesc[0], 0, sizeof( LightDesc_t ) );
	m_pLightDesc[0].m_Type = MATERIAL_LIGHT_DIRECTIONAL;
	m_pLightDesc[0].m_Color.Init( 1.0f, 1.0f, 1.0f );
	m_pLightDesc[0].m_Direction.Init( 0.0f, 0.0f, -1.0f );
	m_pLightDesc[0].m_Range = 0.0;
	m_pLightDesc[0].m_Attenuation0 = 1.0;
	m_pLightDesc[0].m_Attenuation1 = 0;
	m_pLightDesc[0].m_Attenuation2 = 0;
	m_pLightDesc[0].RecalculateDerivedValues();
	m_nNumLightDescs = 1;
	for ( int i = 0; i < 6; ++i )
	{
		m_vecAmbientCube[i].Init( 0.4f, 0.4f, 0.4f, 1.0f );
	}
}

CBasePlayerModelPanel::~CBasePlayerModelPanel()
{
	ClearMergeMDLs();
	m_DefaultEnvCubemap.Shutdown();
}

void CBasePlayerModelPanel::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	const char* pCameraOrigin = inResourceData->GetString( "camera_origin" );
	if ( pCameraOrigin[0] != 0 )
	{
		sscanf( pCameraOrigin, "%f %f %f", &m_Camera.m_origin.x, &m_Camera.m_origin.y, &m_Camera.m_origin.z );
	}
	const char* pCameraAngles = inResourceData->GetString( "camera_angles" );
	if ( pCameraAngles[0] != 0 )
	{
		sscanf( pCameraAngles, "%f %f %f", &m_Camera.m_angles.x, &m_Camera.m_angles.y, &m_Camera.m_angles.z );
	}

	m_Camera.m_flFOV = inResourceData->GetInt( "fov", 54 );

	KeyValues* pData = inResourceData->FindKey( "lights" );
	if ( pData )
	{
		ParseLightInfo( pData );
	}
}

void CBasePlayerModelPanel::OnMousePressed( vgui::MouseCode code )
{
	RequestFocus();

	// Save where they clicked
	input()->GetCursorPosition( m_nLastMouseX, m_nLastMouseY );

	m_bMousePressed = true;
}

void CBasePlayerModelPanel::OnMouseReleased( vgui::MouseCode code )
{
	m_bMousePressed = false;
}

#define ROTATION_TIME 1.0f // seconds
void CBasePlayerModelPanel::OnCursorMoved( int x, int y )
{
	if ( m_bMousePressed )
	{
		int xpos, ypos;
		input()->GetCursorPos( xpos, ypos );

		// Only want the x delta.
		float flDelta = xpos - m_nLastMouseX;

		m_flRotationAngleLeft += flDelta;
		if ( m_flRotationAngleLeft != 0 )
			m_flRotationTimeLeft = ROTATION_TIME;

		m_nLastMouseX = xpos;
		m_nLastMouseY = ypos;
	}
}

void CBasePlayerModelPanel::OnCursorExited()
{
	m_bMousePressed = false;
}

void CBasePlayerModelPanel::SetupRenderState( int nDisplayWidth, int nDisplayHeight )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );

	VMatrix view, projection;
	ComputeViewMatrix( &view, m_Camera );
	ComputeProjectionMatrix( &projection, m_Camera, nDisplayWidth, nDisplayHeight );

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->LoadIdentity();

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->LoadMatrix( view );

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->LoadMatrix( projection );
	
	for ( int i = 0; i < m_nNumLightDescs; ++i )
	{
		pRenderContext->SetLight( i, m_pLightDesc[i] );
	}

	LightDesc_t desc;
	desc.m_Type = MATERIAL_LIGHT_DISABLE;
	int nMaxLightCount = g_pMaterialSystemHardwareConfig->MaxNumLights();
	for ( int i = m_nNumLightDescs; i < nMaxLightCount; ++i )
	{
		pRenderContext->SetLight( i, desc );
	}

	pRenderContext->SetAmbientLightCube( m_vecAmbientCube );

	// FIXME: Remove this! This should automatically happen in DrawModel
	// in studiorender.
	if ( !g_pStudioRender )
		return;

	VMatrix worldToCamera;
	MatrixInverseTR( view, worldToCamera );
	Vector vecOrigin, vecRight, vecUp, vecForward;
	MatrixGetColumn( worldToCamera, 0, &vecRight );
	MatrixGetColumn( worldToCamera, 1, &vecUp );
	MatrixGetColumn( worldToCamera, 2, &vecForward );
	MatrixGetColumn( worldToCamera, 3, &vecOrigin );
	g_pStudioRender->SetViewState( vecOrigin, vecRight, vecUp, vecForward );

	g_pStudioRender->SetLocalLights( m_nNumLightDescs, m_pLightDesc );
	g_pStudioRender->SetAmbientLightColors( m_vecAmbientCube );
}

void CBasePlayerModelPanel::ParseLightInfo( KeyValues* inResourceData )
{
	const char* pAmbientColor = inResourceData->GetString( "ambient_light" );
	if ( pAmbientColor[0] != 0 )
	{
		Vector vecAmbientLight;
		sscanf( pAmbientColor, "%f %f %f", &vecAmbientLight.x, &vecAmbientLight.y, &vecAmbientLight.z );

		for ( int i = 0; i < 6; ++i )
		{
			m_vecAmbientCube[i].Init( vecAmbientLight, 1.0f );
		}
	}

	KeyValues* pLightKeys = inResourceData->GetFirstTrueSubKey();
	while ( pLightKeys )
	{
		if ( m_nNumLightDescs >= MATERIAL_MAX_LIGHT_COUNT )
		{
			DevMsg( "Too many lights defined in %s. Only using first %d. \n", GetName(), MATERIAL_MAX_LIGHT_COUNT );
			break;
		}

		const char* pLightType = pLightKeys->GetName();
		if ( pLightType[0] != 0 )
		{
			LightType_t lightType = MATERIAL_LIGHT_DISABLE;

			if ( V_strnicmp( pLightType, "point_light", 11 ) == 0 )
			{
				lightType = MATERIAL_LIGHT_POINT;
			}
			else if ( V_strnicmp( pLightType, "directional_light", 17 ) == 0 )
			{
				lightType = MATERIAL_LIGHT_DIRECTIONAL;
			}
			else if ( V_strnicmp( pLightType, "spot_light", 10 ) == 0 )
			{
				lightType = MATERIAL_LIGHT_SPOT;
			}
			else
			{
				DevMsg( "Error Parsing lights in %s! Unknown light type %s. \n", GetName(), pLightType );
			}

			if ( lightType != MATERIAL_LIGHT_DISABLE )
			{
				Vector lightPosOrDir( 0, 0, 0 );
				Vector lightColor( 0, 0, 0 );
				const char* pLightPosOrDir = pLightKeys->GetString( (lightType == MATERIAL_LIGHT_DIRECTIONAL) ? "direction" : "position" );
				if ( pLightPosOrDir[0] != 0 )
				{
					sscanf( pLightPosOrDir, "%f %f %f", &(lightPosOrDir.x), &(lightPosOrDir.y), &(lightPosOrDir.z) );
				}
				const char* pLightColor = pLightKeys->GetString( "color" );
				if ( pLightColor[0] != 0 )
				{
					sscanf( pLightColor, "%f %f %f", &(lightColor.x), &(lightColor.y), &(lightColor.z) );
				}

				Vector lightLookAt( 0, 0, 0 );
				float lightInnerCone = 1.0f;
				float lightOuterCone = 10.0f;
				if ( lightType == MATERIAL_LIGHT_SPOT )
				{
					const char* pLightLookAt = pLightKeys->GetString( "lookat" );
					if ( pLightLookAt[0] != 0 )
					{
						sscanf( pLightLookAt, "%f %f %f", &(lightLookAt.x), &(lightLookAt.y), &(lightLookAt.z) );
					}
					lightInnerCone = pLightKeys->GetFloat( "inner_cone", 1.0f );
					lightOuterCone = pLightKeys->GetFloat( "outer_cone", 8.0f );
				}

				switch ( lightType )
				{
					case MATERIAL_LIGHT_DIRECTIONAL:
						m_pLightDesc[m_nNumLightDescs].InitDirectional( lightPosOrDir, lightColor );
						break;
					case MATERIAL_LIGHT_POINT:
						m_pLightDesc[m_nNumLightDescs].InitPoint( lightPosOrDir, lightColor );
						break;
					case MATERIAL_LIGHT_SPOT:
						m_pLightDesc[m_nNumLightDescs].InitSpot( lightPosOrDir, lightColor, lightLookAt, lightInnerCone, lightOuterCone );
						break;
				}
				m_nNumLightDescs++;
			}
		}

		pLightKeys = pLightKeys->GetNextTrueSubKey();
	}
}

void CBasePlayerModelPanel::Paint()
{
	int iWidth, iHeight;
	GetSize( iWidth, iHeight );

	int screenw, screenh;
	vgui::surface()->GetScreenSize( screenw, screenh );

	int windowposx = 0, windowposy = 0;
	GetPos( windowposx, windowposy );

	int windowposright = windowposx + iWidth;
	int windowposbottom = windowposy + iHeight;

	if ( windowposright >= screenw )
	{
		iWidth -= (windowposright - screenw);
	}
	if ( windowposbottom >= screenh )
	{
		iHeight -= (windowposbottom - screenh);
	}

	int startx = 0, starty = 0;
	if ( windowposx < 0 )
	{
		startx = -windowposx;
		iWidth -= startx;
	}
	if ( windowposy < 0 )
	{
		starty = -windowposy;
		iHeight -= starty;
	}

	int w, h;
	GetSize( w, h );
	vgui::MatSystemSurface()->Begin3DPaint( 0, 0, w, h, false );

	// Set up the render state for the camera + light
	SetupRenderState( iWidth, iHeight );

	CMatRenderContextPtr pRenderContext( vgui::MaterialSystem() );

	Color clrBg = GetBgColor();
	pRenderContext->ClearColor4ub( clrBg.r(), clrBg.g(), clrBg.b(), clrBg.a() );
	pRenderContext->ClearBuffers( false, true );

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );
	pRenderContext->SetIntRenderingParameter( INT_RENDERPARM_WRITE_DEPTH_TO_DESTALPHA, false );

	OnPaint3D();

	pRenderContext->CullMode( MATERIAL_CULLMODE_CW );

	vgui::MatSystemSurface()->End3DPaint();
}

extern float GetAutoPlayTime( void );
void CBasePlayerModelPanel::OnThink()
{
	if ( m_MDL.GetMDL() != MDLHANDLE_INVALID )
	{
		m_MDL.m_flTime += gpGlobals->frametime;
	}

	if ( m_flRotationTimeLeft > 0.0f )
	{
		float flPercentage = m_flRotationTimeLeft / ROTATION_TIME;
		float flDelta = m_flRotationAngleLeft * flPercentage * gpGlobals->frametime;
		m_angPlayerModel.y += flDelta;
		m_flRotationAngleLeft -= flDelta;

		if ( m_angPlayerModel.y > 360.0f )
			m_angPlayerModel.y -= 360.0f;
		else if ( m_angPlayerModel.y < -360.0f )
			m_angPlayerModel.y += 360.0f;

		matrix3x4_t matRotate, matTranslate;
        
        AngleMatrix( m_angPlayerModel, matRotate );

        SetIdentityMatrix( matTranslate );
        MatrixSetColumn( -m_vecModelCenter, 3, matTranslate );

        ConcatTransforms( matRotate, matTranslate, m_MDLToWorld );

        m_flRotationTimeLeft -= gpGlobals->frametime;
	}
	else
	{
		m_flRotationAngleLeft = 0.0f;
	}
}

void CBasePlayerModelPanel::OnPaint3D()
{
    if ( m_MDL.GetMDL() == MDLHANDLE_INVALID )
        return;

    MDLCACHE_CRITICAL_SECTION();

    CMatRenderContextPtr pRenderContext( vgui::MaterialSystem() );
    pRenderContext->BindLocalCubemap( m_DefaultEnvCubemap );

    // Получаем studioHdr
    CStudioHdr studioHdr( g_pMDLCache->GetStudioHdr( m_MDL.GetMDL() ), g_pMDLCache );

    // Локальные матрицы костей
    matrix3x4_t* pBoneToWorld = g_pStudioRender->LockBoneMatrices( studioHdr.numbones() );

    // --- Pivot: вращение вокруг центра модели ---
    matrix3x4_t matToCenter, matFromCenter, matFinal;

    // Сдвигаем модель так, чтобы центр оказался в (0,0,0)
    SetIdentityMatrix( matToCenter );
    MatrixSetColumn( -m_vecModelCenter, 3, matToCenter );

    // Возвращаем обратно + пользовательский offset
    SetIdentityMatrix( matFromCenter );
    MatrixSetColumn( m_vecModelCenter, 3, matFromCenter );

    // Комбинируем с текущей трансформацией модели (например, auto-rotation)
    ConcatTransforms( matToCenter, m_MDLToWorld, matFinal );
    ConcatTransforms( matFromCenter, matFinal, m_MDLToWorld );

    // --- Setup bones и отрисовка ---
    m_MDL.SetUpBones( m_MDLToWorld, studioHdr.numbones(), pBoneToWorld );
    g_pStudioRender->UnlockBoneMatrices();

    /*
    IMaterial* pMaterialOverride = GetMaterialOverrideForCurrentWeapon();
    if ( pMaterialOverride )
        modelrender->ForcedMaterialOverride( pMaterialOverride );
*/
    // Рисуем основную модель
    m_MDL.Draw( m_MDLToWorld, pBoneToWorld );

    // Если был override — сбрасываем
   /* if ( pMaterialOverride )
        modelrender->ForcedMaterialOverride( nullptr );*/

    // --- Draw merge MDLs (если есть) ---
    matrix3x4_t matMergeBoneToWorld[MAXSTUDIOBONES];
    int nMergeCount = m_aMergeMDLs.Count();
    for ( int iMerge = 0; iMerge < nMergeCount; ++iMerge )
    {
        studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr( m_aMergeMDLs[iMerge].GetMDL() );
        if ( !pStudioHdr )
            continue;

        matrix3x4_t MDLToWorld;
        SetIdentityMatrix( MDLToWorld );

        CStudioHdr mergeHdr( pStudioHdr, g_pMDLCache );
        m_aMergeMDLs[iMerge].SetupBonesWithBoneMerge( &mergeHdr, matMergeBoneToWorld, &studioHdr, pBoneToWorld, MDLToWorld );

        m_aMergeMDLs[iMerge].Draw( MDLToWorld, matMergeBoneToWorld );
    }

    pRenderContext->Flush();
}

void CBasePlayerModelPanel::SetMDL( const char* pMDLName )
{
	MDLHandle_t hMDLFindResult = vgui::MDLCache()->FindMDL( pMDLName );
	MDLHandle_t hMDL = pMDLName ? hMDLFindResult : MDLHANDLE_INVALID;
	if ( vgui::MDLCache()->IsErrorModel( hMDL ) )
	{
		hMDL = MDLHANDLE_INVALID;
	}

	m_MDL.SetMDL( hMDL );
	m_MDL.m_pProxyData = NULL;

	Vector vecMins, vecMaxs;
    GetMDLBoundingBox( &vecMins, &vecMaxs, hMDL, m_MDL.m_nSequence );

    m_vecModelCenter = ( vecMins + vecMaxs ) * 0.5f;

	m_MDL.m_bWorldSpaceViewTarget = false;
	m_MDL.m_vecViewTarget.Init( 100.0f, 0.0f, vecMaxs.z );

	// FindMDL takes a reference and the the CMDL will also hold a reference for as long as it sticks around. Release the FindMDL reference.
	int nRef = vgui::MDLCache()->Release( hMDLFindResult );
	(void) nRef; // Avoid unreferenced variable warning
	AssertMsg( hMDL == MDLHANDLE_INVALID || nRef > 0, "CMDLPanel::SetMDL referenced a model that has a zero ref count." );
}
CMDL *CBasePlayerModelPanel::SetMergeMDL( const char* pMDLName )
{
	// Verify that we have a root model to merge to.
	if ( m_MDL.GetMDL() == MDLHANDLE_INVALID )
		return NULL;

	MDLHandle_t hMDLFindResult = vgui::MDLCache()->FindMDL( pMDLName );
	MDLHandle_t hMDL = pMDLName ? hMDLFindResult : MDLHANDLE_INVALID;
	if ( vgui::MDLCache()->IsErrorModel( hMDL ) )
	{
		hMDL = MDLHANDLE_INVALID;
	}

	int iIndex = m_aMergeMDLs.AddToTail();
	if ( !m_aMergeMDLs.IsValidIndex( iIndex ) )
		return NULL;

	m_aMergeMDLs[iIndex].SetMDL( hMDL );

	// FindMDL takes a reference and the the CMDL will also hold a reference for as long as it sticks around. Release the FindMDL reference.
	int nRef = vgui::MDLCache()->Release( hMDLFindResult );
	(void) nRef; // Avoid unreferenced variable warning
	AssertMsg( hMDL == MDLHANDLE_INVALID || nRef > 0, "CMDLPanel::SetMergeMDL referenced a model that has a zero ref count." );

	return &m_aMergeMDLs[iIndex];
}

void CBasePlayerModelPanel::ClearMergeMDLs()
{
	m_aMergeMDLs.Purge();
}

bool CBasePlayerModelPanel::SetBodygroup( const char* pBodygroupName, int nValue )
{
	CStudioHdr studioHDR( m_MDL.GetStudioHdr(), g_pMDLCache );
	int nBodygroup = ::FindBodygroupByName( &studioHDR, pBodygroupName );
	if ( nBodygroup == -1 )
		return false;

	::SetBodygroup( &studioHDR, m_MDL.m_nBody, nBodygroup, nValue );
	return true;
}

void CBasePlayerModelPanel::PlaySequence( const char* pszSequenceName )
{
	CStudioHdr studioHDR( m_MDL.GetStudioHdr(), g_pMDLCache );
	int iSeq = ::LookupSequence( &studioHDR, pszSequenceName );
	if ( iSeq != ACT_INVALID )
	{
		m_MDL.m_nSequence = iSeq;
		m_MDL.m_flTime = 0.0f;
	}
}