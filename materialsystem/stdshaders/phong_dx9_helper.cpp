//===== Copyright � 1996-2008, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "BaseVSShader.h"
#include "phong_dx9_helper.h"
#include "convar.h"
#include "cpp_shader_constant_register_map.h"
#include "commandbuilder.h"
#include "tier0/vprof.h"

#include "phong_vs20.inc"
#include "phong_ps20b.inc"
#include "phong_vs30.inc"
#include "phong_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_displacementmap( "mat_displacementmap", "1", FCVAR_CHEAT );
static ConVar r_lightwarpidentity( "r_lightwarpidentity", "0", FCVAR_CHEAT );
static ConVar r_rimlight( "r_rimlight", "1", FCVAR_CHEAT );

static ConVar cl_teamid_min( "cl_teamid_min", "200" );
static ConVar cl_teamid_max( "cl_teamid_max", "1000" );

// Textures may be bound to the following samplers:
//	SHADER_SAMPLER0	 Base (Albedo) / Gloss in alpha
//	SHADER_SAMPLER1	 Specular warp (including iridescence)
//	SHADER_SAMPLER2	 Diffuse Lighting warp texture
//	SHADER_SAMPLER3	 Normal Map
//	SHADER_SAMPLER4	 Flashlight Shadow Depth Map
//	SHADER_SAMPLER5	 Normalization cube map
//	SHADER_SAMPLER6	 Flashlight Cookie
//	SHADER_SAMPLER7	 Specular exponent
//	SHADER_SAMPLER8	 Cubic environment map
//  SHADER_SAMPLER9  Compressed wrinklemap
//  SHADER_SAMPLER10 Stretched wrinklemap
//  SHADER_SAMPLER11 Compressed wrinkle normal map
//  SHADER_SAMPLER12 Stretched wrinkle normal map
//  SHADER_SAMPLER13 Detail texture
//  SHADER_SAMPLER14 Separate self illumination mask
//  SHADER_SAMPLER15 Screen space ambient occlusion texture (on PC only)


//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsPhong_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, VertexLitGeneric_DX9_Vars_t &info )
{	
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert( info.m_nFlashlightTexture >= 0 );

	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}

	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if( info.m_nAlbedo != -1 && g_pConfig->UseBumpmapping() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined() && params[info.m_nAlbedo]->IsDefined() &&
		params[info.m_nBaseTexture]->IsDefined() )
	{
		params[info.m_nBaseTexture]->SetStringValue( params[info.m_nAlbedo]->GetStringValue() );
	}

	// This shader can be used with hw skinning
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	// No texture means no env mask in base alpha
	if ( !params[info.m_nBaseTexture]->IsDefined() )
	{
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	// We always specify we're using user data, therefore we always need tangent spaces
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	bool bBump = (info.m_nBumpmap != -1) && g_pConfig->UseBumpmapping() && params[info.m_nBumpmap]->IsDefined();
	bool bEnvMap = (info.m_nEnvmap != -1) && params[info.m_nEnvmap]->IsDefined();
	bool bDiffuseWarp = (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined();
	bool bPhong = (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined();
	if( !bBump && !bEnvMap && !bDiffuseWarp && !bPhong )
	{
		CLEAR_FLAGS( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	}

	if ( ( info.m_nSelfIllumFresnelMinMaxExp != -1 ) && ( !params[info.m_nSelfIllumFresnelMinMaxExp]->IsDefined() ) )
	{
		params[info.m_nSelfIllumFresnelMinMaxExp]->SetVecValue( 0.0f, 1.0f, 1.0f );
	}

	InitFloatParam( info.m_nEnvmapFresnel, params, 0.0f );

	InitIntParam( info.m_nSelfIllumFresnel, params, 0 );
	InitIntParam( info.m_nBaseMapAlphaPhongMask, params, 0 );
	InitIntParam( info.m_nBaseMapLuminancePhongMask, params, 0 );

	InitIntParam( info.m_nPhongDisableHalfLambert, params, 0 );

	InitIntParam( info.m_nSelfIllumFresnelEnabledThisFrame, params, 0 );
}

//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------
void InitPhong_DX9( CBaseVSShader *pShader, IMaterialVar** params, VertexLitGeneric_DX9_Vars_t &info )
{
	Assert( info.m_nFlashlightTexture >= 0 );
	pShader->LoadTexture( info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB );


	bool bIsBaseTextureTranslucent = false;
	if ( params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );

		if ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() )
		{
			bIsBaseTextureTranslucent = true;
		}

		if ( ( info.m_nWrinkle != -1 ) && ( info.m_nStretch != -1 ) &&
			params[info.m_nWrinkle]->IsDefined() && params[info.m_nStretch]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nWrinkle, TEXTUREFLAGS_SRGB );
			pShader->LoadTexture( info.m_nStretch, TEXTUREFLAGS_SRGB );
		}
	}

	bool bHasSelfIllumMask = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && (info.m_nSelfIllumMask != -1) && params[info.m_nSelfIllumMask]->IsDefined();

	// No alpha channel in any of the textures? No self illum or envmapmask
	if ( !bIsBaseTextureTranslucent )
	{
		bool bHasSelfIllumFresnel = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 );

		// Can still be self illum with no base alpha if using one of these alternate modes
		if ( !bHasSelfIllumFresnel && !bHasSelfIllumMask )
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
		}

		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	if ( (info.m_nPhongExponentTexture != -1) && params[info.m_nPhongExponentTexture]->IsDefined() &&
		 (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nPhongExponentTexture );
	}

	if ( (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined() &&
		 (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDiffuseWarpTexture );
	}

	if ( (info.m_nPhongWarpTexture != -1) && params[info.m_nPhongWarpTexture]->IsDefined() &&
		 (info.m_nPhong != -1) && params[info.m_nPhong]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nPhongWarpTexture );
	}

	if ( info.m_nDetail != -1 && params[info.m_nDetail]->IsDefined() )
	{
		int nDetailBlendMode = ( info.m_nDetailTextureCombineMode == -1 ) ? 0 : params[info.m_nDetailTextureCombineMode]->GetIntValue();
		pShader->LoadTexture( info.m_nDetail, IsSRGBDetailTexture( nDetailBlendMode ) ? TEXTUREFLAGS_SRGB : 0 ); // SRGB if non-Mod2X
	}

	if ( g_pConfig->UseBumpmapping() )
	{
		if ( (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsDefined() )
		{
			pShader->LoadBumpMap( info.m_nBumpmap );
			SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );

			if ( ( info.m_nNormalWrinkle != -1 ) && ( info.m_nNormalStretch != -1 ) &&
				params[info.m_nNormalWrinkle]->IsDefined() && params[info.m_nNormalStretch]->IsDefined() )
			{
				pShader->LoadTexture( info.m_nNormalWrinkle );
				pShader->LoadTexture( info.m_nNormalStretch );
			}
		}
	}
	
	if ( params[info.m_nEnvmap]->IsDefined() )
	{
		pShader->LoadCubeMap( info.m_nEnvmap, ( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0 ) );
	}

	if ( bHasSelfIllumMask )
	{
		pShader->LoadTexture( info.m_nSelfIllumMask );
	}

	// TODO
	/*if ( (info.m_nDecalTexture != -1) && params[info.m_nDecalTexture]->IsDefined() )
	{
		int nDecalBlendMode = (info.m_nDecalTextureCombineMode == -1) ? 0 : params[info.m_nDecalTextureCombineMode]->GetIntValue();
		pShader->LoadTexture( info.m_nDecalTexture, IsSRGBDecalTexture( nDecalBlendMode ) ? TEXTUREFLAGS_SRGB : 0 ); // SRGB if non-Mod2X
	}

	if ( (info.m_nTintMaskTexture != -1) && params[info.m_nTintMaskTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nTintMaskTexture, TEXTUREFLAGS_SRGB );
	}*/
}

//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
class CPhong_DX9_Context : public CBasePerMaterialContextData
{
public:
	CCommandBufferBuilder< CFixedCommandStorageBuffer< 800 > > m_SemiStaticCmdsOut;
};

struct PhongShaderInfo_t
{
	bool m_bHasBaseTexture;
	bool m_bHasBaseTextureWrinkle;
	bool m_bHasBumpWrinkle;
	bool m_bHasDiffuseWarp;
	bool m_bHasPhong;
	bool m_bHasPhongWarp;
	bool m_bHasDetailTexture;
	bool m_bHasSelfIllum;
	bool m_bHasSelfIllumFresnel;
	bool m_bHasEnvmap;
	bool m_bHasRimLight;
	bool m_bHasDecalTexture;
	bool m_bHasTintMaskTexture;
};

static void ComputePhongShaderInfo( CBaseVSShader *pShader, IMaterialVar** params,
	VertexLitGeneric_DX9_Vars_t &info, bool bHasFlashlightOnly, PhongShaderInfo_t *pInfo )
{
	pInfo->m_bHasBaseTexture = (info.m_nBaseTexture != -1) && params[info.m_nBaseTexture]->IsTexture();
	pInfo->m_bHasBaseTextureWrinkle = pInfo->m_bHasBaseTexture &&
									 (info.m_nWrinkle != -1) && params[info.m_nWrinkle]->IsTexture() &&
									 (info.m_nStretch != -1) && params[info.m_nStretch]->IsTexture();
	pInfo->m_bHasBumpWrinkle = (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsTexture() && 
							   (info.m_nNormalWrinkle != -1) && params[info.m_nNormalWrinkle]->IsTexture() &&
							   (info.m_nNormalStretch != -1) && params[info.m_nNormalStretch]->IsTexture();
	/*pInfo->m_bHasDecalTexture = (!pInfo->m_bHasBumpWrinkle) &&
								((info.m_nDecalTexture != -1) && params[info.m_nDecalTexture]->IsTexture());*/
	pInfo->m_bHasDecalTexture = false; // TODO
	/*pInfo->m_bHasTintMaskTexture = (!pInfo->m_bHasBumpWrinkle) &&
								   ((info.m_nTintMaskTexture != -1) && params[info.m_nTintMaskTexture]->IsTexture());*/
	pInfo->m_bHasTintMaskTexture = false; // TODO
	pInfo->m_bHasEnvmap = !bHasFlashlightOnly && params[info.m_nEnvmap]->IsTexture();
	pInfo->m_bHasSelfIllum = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) != 0;
	pInfo->m_bHasSelfIllumFresnel = ( pInfo->m_bHasSelfIllum ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 );
	pInfo->m_bHasPhong = (info.m_nPhong != -1) && ( params[info.m_nPhong]->GetIntValue() != 0 );
	pInfo->m_bHasPhongWarp = (info.m_nPhongWarpTexture != -1) && params[info.m_nPhongWarpTexture]->IsTexture();
	pInfo->m_bHasDiffuseWarp = (info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsTexture();
	pInfo->m_bHasDetailTexture = ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture();

	// Rimlight must be set to non-zero to trigger rim light combo (also requires Phong)
	pInfo->m_bHasRimLight = r_rimlight.GetBool() && pInfo->m_bHasPhong && (info.m_nRimLight != -1) && ( params[info.m_nRimLight]->GetIntValue() != 0 );
}

void DrawPhong_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
	                         VertexLitGeneric_DX9_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )
{
	CPhong_DX9_Context *pContextData = reinterpret_cast< CPhong_DX9_Context *> ( *pContextDataPtr );

	bool bSupportsSM3 = g_pHardwareConfig->SupportsShaderModel_3_0() && !g_pHardwareConfig->PreferReducedFillrate();
	bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

	bool bHasFlashlight = pShader->UsingFlashlight( params );
	bool bHasFlashlightOnly = bHasFlashlight && !IsGameConsole();
	bool bIsDecal = IS_FLAG_SET( MATERIAL_VAR_DECAL );
	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
	BlendType_t nBlendType = pShader->EvaluateBlendRequirements( info.m_nBaseTexture, true );
	bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested && !bHasFlashlightOnly; //dest alpha is free for special use

	bool bHDR = g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;

	if( pShader->IsSnapshotting() )
	{
		PhongShaderInfo_t phongInfo;
		ComputePhongShaderInfo( pShader, params, info, bHasFlashlightOnly, &phongInfo );

		int nDetailBlendMode = ( info.m_nDetailTextureCombineMode == -1 ) ? 0 : params[info.m_nDetailTextureCombineMode]->GetIntValue();
		int nDecalBlendMode = (info.m_nDecalTextureCombineMode == -1) ? 0 : params[info.m_nDecalTextureCombineMode]->GetIntValue();
		bool bHasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
		bool bHasVertexAlpha = IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );

		// look at color and alphamod stuff.
		// Unlit generic never uses the flashlight
		bool bHasEnvmap = !bHasFlashlightOnly && params[info.m_nEnvmap]->IsTexture();

		// Alpha test: FIXME: shouldn't this be handled in CBaseVSShader::SetInitialShadowState
		pShaderShadow->EnableAlphaTest( bIsAlphaTested );

		if( info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
		{
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue() );
		}

		// Based upon vendor and device dependent formats
		int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;
		if( bHasFlashlightOnly )
		{
			if (params[info.m_nBaseTexture]->IsTexture())
			{
				pShader->SetAdditiveBlendingShadowState( info.m_nBaseTexture, true );
			}

			if( bIsAlphaTested )
			{
				// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
				// be the same on both the regular pass and the flashlight pass.
				pShaderShadow->EnableAlphaTest( false );
				pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
			}
			pShaderShadow->EnableBlending( true );
			pShaderShadow->EnableDepthWrites( false );

			// Be sure not to write to dest alpha
			pShaderShadow->EnableAlphaWrites( false );
		}

		if ( !bHasFlashlightOnly ) // not flashlight pass
		{
			if (params[info.m_nBaseTexture]->IsTexture())
			{
				pShader->SetDefaultBlendingShadowState( info.m_nBaseTexture, true );
			}
		}
		
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		int userDataSize = 0;

		// Always enable...will bind white if nothing specified...
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );		// Base (albedo) map
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

		if ( phongInfo.m_bHasDiffuseWarp && phongInfo.m_bHasPhong )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, r_lightwarpidentity.GetBool() );
		}

		if ( phongInfo.m_bHasPhongWarp )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		}

		// Specular exponent map or dummy
		pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );	// Specular exponent map

		if( bHasFlashlight )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );	// Shadow depth map
			//pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER4 );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Flashlight cookie
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER6, true );
		}

		// Always enable, since flat normal will be bound
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );		// Normal map
		userDataSize = 4; // tangent S
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );		// Normalizing cube map

		if ( phongInfo.m_bHasBaseTextureWrinkle || phongInfo.m_bHasBumpWrinkle )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );	// Normal compression map
			pShaderShadow->EnableTexture( SHADER_SAMPLER12, true );	// Normal expansion map
		}

		if ( phongInfo.m_bHasDetailTexture )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER13, true );
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER13, IsSRGBDetailTexture(nDetailBlendMode));
		}

		if ( phongInfo.m_bHasSelfIllum )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER14, true );
		}

		if( bHasVertexColor || bHasVertexAlpha )
		{
			flags |= VERTEX_COLOR;
		}

		if ( phongInfo.m_bHasDecalTexture )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER10, true ); // decal overlay
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER10, IsSRGBDecalTexture( nDecalBlendMode ) );
		}

		if ( phongInfo.m_bHasTintMaskTexture )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER9, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER9, true );
		}

		if ( !bHasFlashlightOnly && phongInfo.m_bHasEnvmap )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, !bHDR );
		}

		pShaderShadow->EnableSRGBWrite( true );
		
		// texcoord0 : base texcoord, texcoord2 : decal hw morph delta
		int pTexCoordDim[3] = { 2, 0, 3 };
		int nTexCoordCount = 1;

		// This shader supports compressed vertices, so OR in that flag:
		flags |= VERTEX_FORMAT_COMPRESSED;

		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, pTexCoordDim, userDataSize );

		bool bWorldNormal = false;

		// This is to allow phong materials to disable half lambert. Half lambert has always been forced on in phong,
		// so the only safe way to allow artists to disable half lambert is to create this param that disables the
		// default behavior of forcing half lambert on.
		//bool bPhongHalfLambert = false; IS_PARAM_DEFINED( info.m_nPhongDisableHalfLambert ) ? (params[info.m_nPhongDisableHalfLambert]->GetIntValue() == 0) : true;

		// Disabling half-lambert for CSGO (not 'compatible' with CSM's - fixes bad shadow aliasing on viewmodels in particular).
		bool bPhongHalfLambert = false;

		if ( !bSupportsSM3 )
		{
			DECLARE_STATIC_VERTEX_SHADER_CSGO( phong_vs20 );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( SFM, false );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( WORLD_NORMAL, 0 );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( FLATTEN_STATIC_CONTROL_FLOW, !bUseStaticControlFlow );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( CASCADED_SHADOW_MAPPING, false );
			SET_STATIC_VERTEX_SHADER_CSGO( phong_vs20 );

			// Assume we're only going to get in here if we support 2b
			DECLARE_STATIC_PIXEL_SHADER_CSGO( phong_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SFM, false );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( FLASHLIGHT, bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SELFILLUM,  phongInfo.m_bHasSelfIllum && !bHasFlashlightOnly );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SELFILLUMFRESNEL,  phongInfo.m_bHasSelfIllumFresnel && !bHasFlashlightOnly );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( LIGHTWARPTEXTURE, phongInfo.m_bHasDiffuseWarp && phongInfo.m_bHasPhong );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( PHONGWARPTEXTURE, phongInfo.m_bHasPhongWarp && phongInfo.m_bHasPhong );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( WRINKLEMAP, phongInfo.m_bHasBaseTextureWrinkle );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( DETAILTEXTURE, phongInfo.m_bHasDetailTexture );
			ClampDetailBlendModeAndWarn( nDetailBlendMode, 0, 7 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( DETAIL_BLEND_MODE, nDetailBlendMode );
			ClampDecalBlendModeAndWarn( nDecalBlendMode, 0, 1 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( DECAL_BLEND_MODE, phongInfo.m_bHasDecalTexture ? nDecalBlendMode : 2 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( TINTMASKTEXTURE, 0 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( RIMLIGHT, phongInfo.m_bHasRimLight );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( CUBEMAP, bHasEnvmap );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SHADER_SRGB_READ, false );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( WORLD_NORMAL, 0 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( PHONG_HALFLAMBERT, bPhongHalfLambert );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( CASCADED_SHADOW_MAPPING, false );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( CSM_MODE, 0 );
			SET_STATIC_PIXEL_SHADER_CSGO( phong_ps20b );
		}
		else
		{
			DECLARE_STATIC_VERTEX_SHADER_CSGO( phong_vs30 );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( SFM, false );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( WORLD_NORMAL, bWorldNormal );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( DECAL, bIsDecal );
			SET_STATIC_VERTEX_SHADER_COMBO_CSGO( CASCADED_SHADOW_MAPPING, false );
			SET_STATIC_VERTEX_SHADER_CSGO( phong_vs30 );

			DECLARE_STATIC_PIXEL_SHADER_CSGO( phong_ps30 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SFM, false );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( FLASHLIGHT, bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SELFILLUM,  phongInfo.m_bHasSelfIllum && !bHasFlashlightOnly );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SELFILLUMFRESNEL,  phongInfo.m_bHasSelfIllumFresnel && !bHasFlashlightOnly );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( LIGHTWARPTEXTURE, phongInfo.m_bHasDiffuseWarp && phongInfo.m_bHasPhong );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( PHONGWARPTEXTURE, phongInfo.m_bHasPhongWarp && phongInfo.m_bHasPhong );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( WRINKLEMAP, phongInfo.m_bHasBaseTextureWrinkle );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( DETAILTEXTURE, phongInfo.m_bHasDetailTexture );
			ClampDetailBlendModeAndWarn( nDetailBlendMode, 0, 7 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( DETAIL_BLEND_MODE, nDetailBlendMode );
			ClampDecalBlendModeAndWarn( nDecalBlendMode, 0, 1 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( DECAL_BLEND_MODE, phongInfo.m_bHasDecalTexture ? nDecalBlendMode : 2 );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( TINTMASKTEXTURE, phongInfo.m_bHasTintMaskTexture );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( RIMLIGHT, phongInfo.m_bHasRimLight );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( CUBEMAP, bHasEnvmap );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( SHADER_SRGB_READ, false );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( WORLD_NORMAL, bWorldNormal );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( PHONG_HALFLAMBERT, bPhongHalfLambert );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( CASCADED_SHADOW_MAPPING, false );
			SET_STATIC_PIXEL_SHADER_COMBO_CSGO( CSM_MODE, 0 );
			SET_STATIC_PIXEL_SHADER_CSGO( phong_ps30 );
		}

		if( bHasFlashlightOnly )
		{
			pShader->FogToBlack();
		}
		else
		{
			pShader->DefaultFog();
		}

		// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
		pShaderShadow->EnableAlphaWrites( bFullyOpaque );
	}
	else // not snapshotting -- begin dynamic state
	{
		// Deal with semisatic
		if ( ( !pContextData ) || ( pContextData->m_bMaterialVarsChanged ) )
		{
			if ( !pContextData )								// make sure allocated
			{
				pContextData = new CPhong_DX9_Context;
				*pContextDataPtr = pContextData;
			}

			pContextData->m_SemiStaticCmdsOut.Reset();
			pContextData->m_bMaterialVarsChanged = false;

			PhongShaderInfo_t phongInfo;
			ComputePhongShaderInfo( pShader, params, info, bHasFlashlightOnly, &phongInfo );

			//int nDecalBlendMode = (info.m_nDecalTextureCombineMode == -1) ? 0 : params[info.m_nDecalTextureCombineMode]->GetIntValue();
			bool bHasBump = (info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsTexture();
			bool bLightingOnly = g_pConfig->nFullbright == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
			bool bHasSelfIllumMask = ( phongInfo.m_bHasSelfIllum ) && (info.m_nSelfIllumMask != -1) && params[info.m_nSelfIllumMask]->IsTexture();
			float flBlendFactorOrPhongAlbedoBoost;
			if ( phongInfo.m_bHasDetailTexture )
			{
				flBlendFactorOrPhongAlbedoBoost = ( info.m_nDetailTextureBlendFactor == -1 )? 1 : params[info.m_nDetailTextureBlendFactor]->GetFloatValue();
			}
			else
			{
				flBlendFactorOrPhongAlbedoBoost = ( info.m_nPhongAlbedoBoost == -1 ) ? 1.0f : params[info.m_nPhongAlbedoBoost]->GetFloatValue();
			}
			bool bHasSpecularExponentTexture = (info.m_nPhongExponentTexture != -1) && params[info.m_nPhongExponentTexture]->IsTexture();
			bool bHasPhongTintMap = bHasSpecularExponentTexture && (info.m_nPhongAlbedoTint != -1) && ( params[info.m_nPhongAlbedoTint]->GetIntValue() != 0 );
			bool bHasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
			bool bHasRimMaskMap = bHasSpecularExponentTexture && phongInfo.m_bHasRimLight && (info.m_nRimMask != -1) && ( params[info.m_nRimMask]->GetIntValue() != 0 );

			if( phongInfo.m_bHasBaseTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
			}
			else
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_WHITE );
			}

			if ( phongInfo.m_bHasBaseTextureWrinkle )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER9, info.m_nWrinkle, info.m_nBaseTextureFrame );
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER10, info.m_nStretch, info.m_nBaseTextureFrame );
			}

			if( phongInfo.m_bHasDiffuseWarp && phongInfo.m_bHasPhong )
			{
				if ( r_lightwarpidentity.GetBool() )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER2, TEXTURE_IDENTITY_LIGHTWARP ); // [mariod] - TODO why does this conflict with other shader uses of this texture (i.e. not srgb - cloak_dx9, eye_refract_dx9, vertexlitgeneric_dx9)
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER2, info.m_nDiffuseWarpTexture, -1 );
				}
			}

			if( phongInfo.m_bHasPhongWarp )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER1, info.m_nPhongWarpTexture, -1 );
			}

			// Specular exponent map or dummy
			if( bHasSpecularExponentTexture && phongInfo.m_bHasPhong )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER7, info.m_nPhongExponentTexture, -1 );
			}
			else
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER7, TEXTURE_WHITE );
			}

			if( !g_pConfig->m_bFastNoBump )
			{
				if( bHasBump )
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER3, info.m_nBumpmap, info.m_nBumpFrame );
				else
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT );

				if ( phongInfo.m_bHasBumpWrinkle )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, info.m_nNormalWrinkle, info.m_nBumpFrame );
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, info.m_nNormalStretch, info.m_nBumpFrame );
				}
				else if ( phongInfo.m_bHasBaseTextureWrinkle )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, info.m_nBumpmap, info.m_nBumpFrame );
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, info.m_nBumpmap, info.m_nBumpFrame );
				}
			}
			else // Just flat bump maps
			{
				if ( bHasBump )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT );
				}

				if ( phongInfo.m_bHasBaseTextureWrinkle || phongInfo.m_bHasBumpWrinkle )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, TEXTURE_NORMALMAP_FLAT, -1 );
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, TEXTURE_NORMALMAP_FLAT, -1 );
				}
			}

			if ( phongInfo.m_bHasDetailTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER13, info.m_nDetail, info.m_nDetailFrame );
			}

			// TODO
			/*if ( phongInfo.m_bHasDecalTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER10, info.m_nDecalTexture, -1 );
			}

			if ( phongInfo.m_bHasTintMaskTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER9, info.m_nTintMaskTexture );
			}*/

			if ( phongInfo.m_bHasSelfIllum )
			{
				if ( bHasSelfIllumMask )												// Separate texture for self illum?
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER14, info.m_nSelfIllumMask, -1 );	// Bind it
				}
				else																	// else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER14, TEXTURE_BLACK );	// Bind dummy
				}
			}

			if( !bHasFlashlightOnly )
			{
				if ( phongInfo.m_bHasEnvmap )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER8, info.m_nEnvmap, info.m_nEnvmapFrame );
				}
			}
			
			pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );

			if( bHasBump )
			{
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, info.m_nBumpTransform );
			}

			if ( phongInfo.m_bHasDetailTexture )
			{
				if ( IS_PARAM_DEFINED( info.m_nDetailTextureTransform ) )
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4,
						info.m_nDetailTextureTransform, 
						info.m_nDetailScale );
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4,
						info.m_nBaseTextureTransform, 
						info.m_nDetailScale );
				}
			}

			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant_W( PSREG_SELFILLUMTINT, info.m_nSelfIllumTint, flBlendFactorOrPhongAlbedoBoost );
			bool bInvertPhongMask = ( info.m_nInvertPhongMask != -1 ) && ( params[info.m_nInvertPhongMask]->GetIntValue() != 0 );
			float fInvertPhongMask = bInvertPhongMask ? 1 : 0;

			bool bHasBaseAlphaPhongMask = (info.m_nBaseMapAlphaPhongMask != -1) && ( params[info.m_nBaseMapAlphaPhongMask]->GetIntValue() != 0 );
			float fHasBaseAlphaPhongMask = bHasBaseAlphaPhongMask ? 1 : 0;

			bool bBlendTintByBaseAlpha = (info.m_nBlendTintByBaseAlpha != -1) && ( params[info.m_nBlendTintByBaseAlpha]->GetIntValue() != 0 );
			float fBlendTintByBaseAlpha = bBlendTintByBaseAlpha ? 1 : 0;
			bool bNoTint = (info.m_nNoTint != -1) && ( params[info.m_nNoTint]->GetIntValue() != 0 );
			
			// Controls for lerp-style paths through shader code
			float vShaderControls[4] = { fHasBaseAlphaPhongMask, 0.0f, bNoTint ? -1.0f : ( 1.0f - fBlendTintByBaseAlpha ), fInvertPhongMask };
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_CONSTANT_27, vShaderControls, 1 );

			if ( phongInfo.m_bHasSelfIllumFresnel && !bHasFlashlightOnly )
			{
				float vConstScaleBiasExp[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
				float flMin = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelMinMaxExp ) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[0] : 0.0f;
				float flMax = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelMinMaxExp ) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[1] : 1.0f;
				float flExp = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelMinMaxExp ) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[2] : 1.0f;

				vConstScaleBiasExp[1] = ( flMax != 0.0f ) ? ( flMin / flMax ) : 0.0f; // Bias
				vConstScaleBiasExp[0] = 1.0f - vConstScaleBiasExp[1]; // Scale
				vConstScaleBiasExp[2] = flExp; // Exp
				vConstScaleBiasExp[3] = flMax; // Brightness

				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_SELFILLUM_SCALE_BIAS_EXP, vConstScaleBiasExp, 1 );
			}

			if( !bHasFlashlightOnly )
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED );

				if( phongInfo.m_bHasEnvmap )
				{
					float vEnvMapTint_MaskControl[4] = {1.0f, 1.0f, 1.0f, 0.0f};

					// If we have a tint, grab it
					if ( (info.m_nEnvmapTint != -1) && params[info.m_nEnvmapTint]->IsDefined() )
						params[info.m_nEnvmapTint]->GetVecValue(vEnvMapTint_MaskControl, 3);

					// Set control for source of env map mask (normal alpha or base alpha)
					vEnvMapTint_MaskControl[3] = bHasNormalMapAlphaEnvmapMask ? 1.0f : 0.0f;

					// Handle mat_fullbright 2 (diffuse lighting only with 50% gamma space basetexture)
					if( bLightingOnly )
					{
						vEnvMapTint_MaskControl[0] = vEnvMapTint_MaskControl[1] = vEnvMapTint_MaskControl[2] = 0.0f;
					}

					pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, vEnvMapTint_MaskControl, 1 );
				}
			}

			// Pack Phong exponent in with the eye position
			float vSpecularTint[4] = {1, 1, 1, 4};
			float vFresnelRanges_SpecBoost[4] = {0, 0.5, 1, 1}, vRimBoost[4] = {1, 1, 1, 1};

			// Get the tint parameter
			if ( (info.m_nPhongTint != -1) && params[info.m_nPhongTint]->IsDefined() )
			{
				params[info.m_nPhongTint]->GetVecValue(vSpecularTint, 3);
			}

			// Get the rim light power (goes in w of Phong tint)
			if ( phongInfo.m_bHasRimLight && (info.m_nRimLightPower != -1) && params[info.m_nRimLightPower]->IsDefined() )
			{
				vSpecularTint[3] = params[info.m_nRimLightPower]->GetFloatValue();
				vSpecularTint[3] = MAX(vSpecularTint[3], 1.0f);	// Make sure this is at least 1
			}

			// Get the rim boost (goes in w of flashlight position)
			if ( phongInfo.m_bHasRimLight && (info.m_nRimLightBoost != -1) && params[info.m_nRimLightBoost]->IsDefined() )
			{
				vRimBoost[3] = params[info.m_nRimLightBoost]->GetFloatValue();
			}

			if ( phongInfo.m_bHasRimLight )
			{
				if ( !bHasFlashlight )
				{
					float vRimMaskControl[4] = {0, 0, 0, 0}; // Only x is relevant in shader code
					vRimMaskControl[0] = bHasRimMaskMap ? params[info.m_nRimMask]->GetFloatValue() : 0.0f;

					// Rim mask...if this is true, use alpha channel of spec exponent texture to mask the rim term
					pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, vRimMaskControl, 1 );
				}
			}

			float vRimParams[4] = { 0, 0, 0, 0 };
			vRimParams[3] = params[info.m_nPearlescent]->GetFloatValue();

			// Rim mask...if this is true, use alpha channel of spec exponent texture to mask the rim term
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_RIMPARAMS, vRimParams, 1 );

			// If it's all zeros, there was no constant tint in the vmt
			if ( (vSpecularTint[0] == 0.0f) && (vSpecularTint[1] == 0.0f) && (vSpecularTint[2] == 0.0f) )
			{
				if ( bHasPhongTintMap )				// If we have a map to use, tell the shader
				{
					vSpecularTint[0] = -1;
				}
				else								// Otherwise, just tint with white
				{
					vSpecularTint[0] = 1.0f;
					vSpecularTint[1] = 1.0f;
					vSpecularTint[2] = 1.0f;
				}
			}

			// handle mat_fullbright 2 (diffuse lighting only)
			if( bLightingOnly )
			{
				// BASETEXTURE
				if( phongInfo.m_bHasSelfIllum && !bHasFlashlightOnly )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY_ALPHA_ZERO );

					if ( phongInfo.m_bHasBaseTextureWrinkle || phongInfo.m_bHasBumpWrinkle )
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER9, TEXTURE_GREY_ALPHA_ZERO );	// Compressed wrinklemap
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER10, TEXTURE_GREY_ALPHA_ZERO );	// Stretched wrinklemap
					}
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY );

					if ( phongInfo.m_bHasBaseTextureWrinkle || phongInfo.m_bHasBumpWrinkle )
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER9, TEXTURE_GREY );		// Compressed wrinklemap
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER10, TEXTURE_GREY );	// Stretched wrinklemap
					}
				}

				// DETAILTEXTURE
				if ( phongInfo.m_bHasDetailTexture )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER13, TEXTURE_GREY );
				}

				// turn off specularity
				vSpecularTint[0] = vSpecularTint[1] = vSpecularTint[2] = 0.0f;
			}

			if ( (info.m_nPhongFresnelRanges != -1) && params[info.m_nPhongFresnelRanges]->IsDefined() )
			{
				params[info.m_nPhongFresnelRanges]->GetVecValue( vFresnelRanges_SpecBoost, 3 );	// Grab optional Fresnel range parameters
			}

			if ( (info.m_nPhongBoost != -1 ) && params[info.m_nPhongBoost]->IsDefined())		// Grab optional Phong boost param
			{
				vFresnelRanges_SpecBoost[3] = params[info.m_nPhongBoost]->GetFloatValue();
			}
			else
			{
				vFresnelRanges_SpecBoost[3] = 1.0f;
			}

			bool bHasBaseLuminancePhongMask = (info.m_nBaseMapLuminancePhongMask != -1) && ( params[info.m_nBaseMapLuminancePhongMask]->GetIntValue() != 0 );
			float fHasBaseLuminancePhongMask = bHasBaseLuminancePhongMask ? 1 : 0;
			float vShaderControls2[4] = {0.0f, fHasBaseLuminancePhongMask, 0.0f, 0.0f};
			if ( !bHasFlashlightOnly )
			{
				if ( phongInfo.m_bHasEnvmap ) 
				{
					if ( (info.m_nEnvmapFresnel != -1) && params[info.m_nEnvmapFresnel]->IsDefined() )
					{
						vShaderControls2[0] = params[info.m_nEnvmapFresnel]->GetFloatValue();
					}
				}
			}
			if ( (info.m_nPhongExponent != -1) && params[info.m_nPhongExponent]->IsDefined() )
			{
				vShaderControls2[2] = params[info.m_nPhongExponent]->GetFloatValue();		// This overrides the channel in the map
			}
			else
			{
				vShaderControls2[2] = 0;													// Use the alpha channel of the normal map for the exponent
			}

			// Driven by the material proxy for team ID in csgo
			int nSelfIllumFresnelEnabledThisFrame = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelEnabledThisFrame ) ? params[info.m_nSelfIllumFresnelEnabledThisFrame]->GetIntValue() : 1;
			vShaderControls2[3] = bHasSelfIllumMask ? 1.0f : 0.0f;
			if ( phongInfo.m_bHasSelfIllumFresnel && !nSelfIllumFresnelEnabledThisFrame )
			{
				vShaderControls2[3] = 0.0f;
			}

			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_FRESNEL_SPEC_PARAMS, vFresnelRanges_SpecBoost, 1 );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_FLASHLIGHT_POSITION_RIM_BOOST, vRimBoost, 1 );	// Rim boost in w on non-flashlight pass
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_SPEC_RIM_PARAMS, vSpecularTint, 1 );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_CONSTANT_10, vShaderControls2, 1 );

			pContextData->m_SemiStaticCmdsOut.SetPixelShaderFogParams_CSGO( PSREG_FOG_PARAMS );

			if ( bHasFlashlight )
			{
				CBCmdSetPixelShaderFlashlightState_t state;
				state.m_LightSampler = SHADER_SAMPLER6;
				state.m_DepthSampler = SHADER_SAMPLER4;
				state.m_ShadowNoiseSampler = SHADER_SAMPLER5;
				state.m_nColorConstant = PSREG_FLASHLIGHT_COLOR;
				state.m_nAttenConstant = PSREG_FLASHLIGHT_ATTENUATION;
				state.m_nOriginConstant = PSREG_FLASHLIGHT_POSITION_RIM_BOOST;
				state.m_nDepthTweakConstant = PSREG_ENVMAP_TINT__SHADOW_TWEAKS;
				state.m_nScreenScaleConstant = PSREG_FLASHLIGHT_SCREEN_SCALE;
				state.m_nWorldToTextureConstant = PSREG_FLASHLIGHT_TO_WORLD_TEXTURE;
				state.m_bFlashlightNoLambert = false;
				state.m_bSinglePassFlashlight = false;
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderFlashlightState( state );
			}
			pContextData->m_SemiStaticCmdsOut.End();
		}

		if ( pShaderAPI && pShader )
		{
			pShaderAPI->SetPixelShaderStateAmbientLightCube( PSREG_AMBIENT_CUBE );
			pShaderAPI->CommitPixelShaderLighting( PSREG_LIGHT_INFO_ARRAY );
			pShaderAPI->SetVertexShaderStateAmbientLightCube();
			bool bAllowDiffuseModulation = true;

			if ( bAllowDiffuseModulation )
			{
				pShader->SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );
			}
			else
			{
				pShader->SetModulationPixelShaderDynamicState( 1 );
			}
		}

		CCommandBufferBuilder< CFixedCommandStorageBuffer< 1000 > > DynamicCmdsOut;
		DynamicCmdsOut.Call( pContextData->m_SemiStaticCmdsOut.Base() );

		bool bFlashlightShadows = false;
		if ( bHasFlashlight )
		{
			VMatrix worldToTexture;
			ITexture* pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );
			bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);
		}

		float vEyePos_AmbientOcclusion[4];
		pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_AmbientOcclusion );
		vEyePos_AmbientOcclusion[3] = 0.0f;
		DynamicCmdsOut.SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_AmbientOcclusion, 1 );

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int numBones = pShaderAPI->GetCurrentNumBones();

		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;

		if( bFullyOpaque ) 
		{
			// Disable for CS:GO
			//bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg( !(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time." );
		}

		LightState_t lightState = { 0, false, false };
		if( !bHasFlashlightOnly )
		{
			pShaderAPI->GetDX9LightState( &lightState );
		}

		float vTeamIdMinMax[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		vTeamIdMinMax[0] = cl_teamid_min.GetFloat();
		vTeamIdMinMax[1] = cl_teamid_max.GetFloat();
		if ( vTeamIdMinMax[1] <= vTeamIdMinMax[0] )
		{
			vTeamIdMinMax[1] = vTeamIdMinMax[0] + 0.01f; // Avoid a divide by zero in the shader
		}
		pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, vTeamIdMinMax );

		if ( !bSupportsSM3 )
		{
			DECLARE_DYNAMIC_VERTEX_SHADER_CSGO( phong_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( SKINNING, numBones > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( TESSELLATION, 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( NUM_LIGHTS, bUseStaticControlFlow ? 0 : MIN(2, lightState.m_nNumLights) );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( CSM_VIEWMODELQUALITY, 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( STATICLIGHT3, false );
			SET_DYNAMIC_VERTEX_SHADER( phong_vs20 );

			DECLARE_DYNAMIC_PIXEL_SHADER_CSGO( phong_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( NUM_LIGHTS, MIN(2, lightState.m_nNumLights) );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( FLASHLIGHTSHADOWS, bFlashlightShadows );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( CASCADE_SIZE, 0 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( CSM_VIEWMODELQUALITY, 0 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( STATICLIGHT3, false );
			SET_DYNAMIC_PIXEL_SHADER_CSGO( phong_ps20b );
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER_CSGO( phong_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( SKINNING, numBones > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( COMPRESSED_VERTS, (int)vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( TESSELLATION, 0 );
			//SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( MORPHING, bMorphing );
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( CSM_VIEWMODELQUALITY, 0 );  // TODO: not used in sm3.0, combo should be removed
			SET_DYNAMIC_VERTEX_SHADER_COMBO_CSGO( STATICLIGHT3, false );
			SET_DYNAMIC_VERTEX_SHADER( phong_vs30 );

			DECLARE_DYNAMIC_PIXEL_SHADER_CSGO( phong_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( NUM_LIGHTS, lightState.m_nNumLights );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( FLASHLIGHTSHADOWS, bFlashlightShadows );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( UBERLIGHT, false );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( CASCADE_SIZE, 0 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( CSM_VIEWMODELQUALITY, 0 );  // TODO: not used in sm3.0, combo should be removed
			SET_DYNAMIC_PIXEL_SHADER_COMBO_CSGO( STATICLIGHT3, false );
			SET_DYNAMIC_PIXEL_SHADER_CSGO( phong_ps30 );

			bool bUnusedTexCoords[3] = { false, false, true };
			pShaderAPI->MarkUnusedVertexFields( 0, 3, bUnusedTexCoords );

			// Set constant to enable translation of VPOS to render target coordinates in ps_3_0
			pShaderAPI->SetScreenSizeForVPOS();
		}

		DynamicCmdsOut.End();
		pShaderAPI->ExecuteCommandBuffer( DynamicCmdsOut.Base() );
	}
	pShader->Draw();
}
