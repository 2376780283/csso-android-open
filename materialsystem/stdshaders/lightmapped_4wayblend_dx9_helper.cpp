//========= Copyright (c) 1996-2014, Valve LLC, All rights reserved. ============
//
// Purpose: Lightmapped_4WayBlend shader
//
// $Header: $
// $NoKeywords: $
//=============================================================================

#include "lightmapped_4wayblend_dx9_helper.h"
#include "BaseVSShader.h"
#include "convar.h"
#include "lightmapped_4wayblend_vs20.inc"
#include "lightmapped_4wayblend_ps20b.inc"

#if !defined( _X360 ) && !defined( _PS3 )
	#include "lightmapped_4wayblend_vs30.inc"
	#include "lightmapped_4wayblend_ps30.inc"
#endif

#include "tier0/vprof.h"

#include "tier0/memdbgon.h"


static ConVar mat_force_vertexfog( "mat_force_vertexfog", "0", FCVAR_DEVELOPMENTONLY );

void InitParamsLightmapped_4WayBlend_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, Lightmapped_4WayBlend_DX9_Vars_t &info )
{
	// A little strange, but we do support "skinning" in that we can do
	// fast-path instance rendering with lightmapped generic, and this 
	// tells the system to insert the PI command buffer to set the per-instance matrix.
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );

	// Override vertex fog via the global setting if it isn't enabled/disabled in the material file.
	if ( !IS_FLAG_DEFINED( MATERIAL_VAR_VERTEXFOG ) && mat_force_vertexfog.GetBool() )
	{
		SET_FLAGS( MATERIAL_VAR_VERTEXFOG );
	}

	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}

	if( pShader->IsUsingGraphics() && params[info.m_nEnvmap]->IsDefined() && !pShader->CanUseEditorMaterials() )
	{
		if( stricmp( params[info.m_nEnvmap]->GetStringValue(), "env_cubemap" ) == 0 )
		{
			Warning( "env_cubemap used on world geometry without rebuilding map. . ignoring: %s\n", pMaterialName );
			params[info.m_nEnvmap]->SetUndefined();
		}
	}
	
	if( !params[info.m_nEnvmapTint]->IsDefined() )
		params[info.m_nEnvmapTint]->SetVecValue( 1.0f, 1.0f, 1.0f );

	if( !params[info.m_nNoDiffuseBumpLighting]->IsDefined() )
		params[info.m_nNoDiffuseBumpLighting]->SetIntValue( 0 );

	if( !params[info.m_nSelfIllumTint]->IsDefined() )
		params[info.m_nSelfIllumTint]->SetVecValue( 1.0f, 1.0f, 1.0f );

	if( !params[info.m_nDetailScale]->IsDefined() )
		params[info.m_nDetailScale]->SetFloatValue( 4.0f );

	if ( !params[info.m_nDetailTint]->IsDefined() )
		params[info.m_nDetailTint]->SetVecValue( 1.0f, 1.0f, 1.0f, 1.0f );

	InitFloatParam( info.m_nDetailTextureBlendFactor, params, 1.0 );
	InitFloatParam( info.m_nDetailTextureBlendFactor2, params, 1.0 );
	InitFloatParam( info.m_nDetailTextureBlendFactor3, params, 1.0 );
	InitFloatParam( info.m_nDetailTextureBlendFactor4, params, 1.0 );
	InitIntParam( info.m_nDetailTextureCombineMode, params, 0 );

	InitFloatParam( info.m_nTexture2uvScale, params, 1.0f );
	InitFloatParam( info.m_nTexture3uvScale, params, 1.0f );
	InitFloatParam( info.m_nTexture4uvScale, params, 1.0f );

	InitFloatParam( info.m_nTexture2BlendStart, params, 0.0f );
	InitFloatParam( info.m_nTexture3BlendStart, params, 0.0f );
	InitFloatParam( info.m_nTexture4BlendStart, params, 0.0f );

	InitFloatParam( info.m_nTexture2BlendEnd, params, 1.0f );
	InitFloatParam( info.m_nTexture3BlendEnd, params, 1.0f );
	InitFloatParam( info.m_nTexture4BlendEnd, params, 1.0f );

	InitFloatParam( info.m_nTexture1LumStart, params, 0.0f );
	InitFloatParam( info.m_nTexture2LumStart, params, 0.0f );
	InitFloatParam( info.m_nTexture3LumStart, params, 0.0f );
	InitFloatParam( info.m_nTexture4LumStart, params, 0.0f );

	InitFloatParam( info.m_nTexture1LumEnd, params, 1.0f );
	InitFloatParam( info.m_nTexture2LumEnd, params, 1.0f );
	InitFloatParam( info.m_nTexture3LumEnd, params, 1.0f );
	InitFloatParam( info.m_nTexture4LumEnd, params, 1.0f );

	InitFloatParam( info.m_nTexture2BumpBlendFactor, params, 1.0f );
	InitFloatParam( info.m_nTexture3BumpBlendFactor, params, 1.0f );
	InitFloatParam( info.m_nTexture4BumpBlendFactor, params, 1.0f );

	InitFloatParam( info.m_nLumBlendFactor2, params, 1.0f );
	InitFloatParam( info.m_nLumBlendFactor3, params, 1.0f );
	InitFloatParam( info.m_nLumBlendFactor4, params, 1.0f );

	if( !params[info.m_nFresnelReflection]->IsDefined() )
		params[info.m_nFresnelReflection]->SetFloatValue( 1.0f );

	if( !params[info.m_nEnvmapMaskFrame]->IsDefined() )
		params[info.m_nEnvmapMaskFrame]->SetIntValue( 0 );
	
	if( !params[info.m_nEnvmapFrame]->IsDefined() )
		params[info.m_nEnvmapFrame]->SetIntValue( 0 );

	if( !params[info.m_nBumpFrame]->IsDefined() )
		params[info.m_nBumpFrame]->SetIntValue( 0 );

	if( !params[info.m_nDetailFrame]->IsDefined() )
		params[info.m_nDetailFrame]->SetIntValue( 0 );

	if( !params[info.m_nEnvmapContrast]->IsDefined() )
		params[info.m_nEnvmapContrast]->SetFloatValue( 0.0f );
	
	if( !params[info.m_nEnvmapSaturation]->IsDefined() )
		params[info.m_nEnvmapSaturation]->SetFloatValue( 1.0f );

	if ( ( info.m_nEnvmapAnisotropyScale != -1 ) && !params[info.m_nEnvmapAnisotropyScale]->IsDefined() )
		params[info.m_nEnvmapAnisotropyScale]->SetFloatValue( 1.0f );

	if ( ( info.m_nEnvMapLightScaleMinMax != -1 ) && !params[info.m_nEnvMapLightScaleMinMax]->IsDefined() )
		params[info.m_nEnvMapLightScaleMinMax]->SetVecValue( 0.0, 1.0 );
	
	InitFloatParam( info.m_nAlphaTestReference, params, 0.0f );

	// No texture means no self-illum or env mask in base alpha
	if ( !params[info.m_nBaseTexture]->IsDefined() )
	{
		CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	if( params[info.m_nBumpmap]->IsDefined() )
	{
		params[info.m_nEnvmapMask]->SetUndefined();
	}
	
	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	if ( !params[info.m_nForceBumpEnable]->IsDefined() )
	{
		params[info.m_nForceBumpEnable]->SetIntValue( 0 );
	}

	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
	bool bShouldUseBump = g_pConfig->UseBumpmapping() || !!params[info.m_nForceBumpEnable]->GetIntValue();
	if( bShouldUseBump && params[info.m_nBumpmap]->IsDefined() && (params[info.m_nNoDiffuseBumpLighting]->GetIntValue() == 0) )
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );
	}

	// If mat_specular 0, then get rid of envmap
	if( !g_pConfig->UseSpecular() && params[info.m_nEnvmap]->IsDefined() && params[info.m_nBaseTexture]->IsDefined() )
	{
		params[info.m_nEnvmap]->SetUndefined();
	}

	if( ( info.m_nSelfShadowedBumpFlag != -1 ) &&
		( !params[info.m_nSelfShadowedBumpFlag]->IsDefined() )
		)
	{
		params[info.m_nSelfShadowedBumpFlag]->SetIntValue( 0 );
	}

	InitFloatParam( info.m_nEnvMapLightScale, params, 0.0f );
}

void InitLightmapped_4WayBlend_DX9( CBaseVSShader *pShader, IMaterialVar** params, Lightmapped_4WayBlend_DX9_Vars_t &info )
{
	bool bShouldUseBump = g_pConfig->UseBumpmapping() || !!params[info.m_nForceBumpEnable]->GetIntValue();

	if ( bShouldUseBump && params[info.m_nBumpmap]->IsDefined() )
	{
		pShader->LoadBumpMap( info.m_nBumpmap );
	}

	if ( bShouldUseBump && params[info.m_nBumpmap2]->IsDefined() )
	{
		pShader->LoadBumpMap( info.m_nBumpmap2 );
	}

	if (params[info.m_nBaseTexture]->IsDefined())
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );

		if (!params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent())
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
			CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
		}
	}

	if ( params[info.m_nBaseTexture2]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture2, TEXTUREFLAGS_SRGB );
	}

	if ( params[info.m_nBaseTexture3]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture3, TEXTUREFLAGS_SRGB );
	}

	if ( params[info.m_nBaseTexture4]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture4, TEXTUREFLAGS_SRGB );
	}

	if (params[info.m_nDetail]->IsDefined())
	{
		int nDetailBlendMode = ( info.m_nDetailTextureCombineMode == -1 ) ? 0 : params[info.m_nDetailTextureCombineMode]->GetIntValue();

		if ( nDetailBlendMode != DETAIL_BLEND_MODE_RGB_EQUALS_BASE_x_DETAILx2 && nDetailBlendMode != DETAIL_BLEND_MODE_MOD2X_SELECT_TWO_PATTERNS && nDetailBlendMode != DETAIL_BLEND_MODE_NONE )
		{
			Msg( "Invalid DetailBlendMode in %s, %d not supported!", pShader->GetName(), nDetailBlendMode );
		}

		if ( nDetailBlendMode != DETAIL_BLEND_MODE_NONE )
		{
			pShader->LoadTexture( info.m_nDetail, IsSRGBDetailTexture( nDetailBlendMode ) ? TEXTUREFLAGS_SRGB : 0 );
		}
	}

	pShader->LoadTexture( info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB );
	
	// Don't alpha test if the alpha channel is used for other purposes
	if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
	{
		CLEAR_FLAGS( MATERIAL_VAR_ALPHATEST );
	}
		
	if ( g_pConfig->UseSpecular() )
	{
		if ( params[info.m_nEnvmap]->IsDefined() )
		{
			pShader->LoadCubeMap( info.m_nEnvmap, ( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0 ) );

			if (params[info.m_nEnvmapMask]->IsDefined())
			{
				pShader->LoadTexture( info.m_nEnvmapMask );
			}
		}
		else
		{
			params[info.m_nEnvmapMask]->SetUndefined();
		}
	}
	else
	{
		params[info.m_nEnvmap]->SetUndefined();
		params[info.m_nEnvmapMask]->SetUndefined();
	}

	// We always need this because of the flashlight.
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
}

void DrawLightmapped_4WayBlend_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, 
									Lightmapped_4WayBlend_DX9_Vars_t &info, CBasePerMaterialContextData **pContextDataPtr )
{
	bool hasFlashlight = pShader->UsingFlashlight( params );
	CLightmapped_4WayBlend_DX9_Context *pContextData = reinterpret_cast< CLightmapped_4WayBlend_DX9_Context *> ( *pContextDataPtr );
	bool bHDR = g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;
	int nDetailBlendMode = GetIntParam( info.m_nDetailTextureCombineMode, params );

	if ( pShaderShadow || ( ! pContextData ) || pContextData->m_bMaterialVarsChanged || ( hasFlashlight && !( IsX360() || IsPS3() ) ) )
	{
		bool hasBaseTexture = params[info.m_nBaseTexture]->IsTexture();
		int nAlphaChannelTextureVar = hasBaseTexture ? (int)info.m_nBaseTexture : (int)info.m_nEnvmapMask;
		BlendType_t nBlendType = pShader->EvaluateBlendRequirements( nAlphaChannelTextureVar, hasBaseTexture );
		bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
		bool bFullyOpaqueWithoutAlphaTest = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && (!hasFlashlight || IsX360() || IsPS3()); //dest alpha is free for special use
		bool bFullyOpaque = bFullyOpaqueWithoutAlphaTest && !bIsAlphaTested;
		bool bNeedRegenStaticCmds = (! pContextData ) || pShaderShadow;

		if ( ! pContextData )								// make sure allocated
		{
			pContextData = new CLightmapped_4WayBlend_DX9_Context;
			*pContextDataPtr = pContextData;
		}

		bool shouldUseBump = g_pConfig->UseBumpmapping() || !!params[info.m_nForceBumpEnable]->GetIntValue();
		bool hasBump = ( params[info.m_nBumpmap]->IsTexture() ) && shouldUseBump;
		bool hasSSBump = hasBump && (info.m_nSelfShadowedBumpFlag != -1) &&	( params[info.m_nSelfShadowedBumpFlag]->GetIntValue() );
		bool hasBump2 = hasBump && params[info.m_nBumpmap2]->IsTexture();
		bool hasDetailTexture = params[info.m_nDetail]->IsTexture() && ( nDetailBlendMode != DETAIL_BLEND_MODE_NONE );
		bool hasSelfIllum = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM );

		if ( hasFlashlight && !( IsX360() || IsPS3() ) )				
		{
			// !!speed!! do this in the caller so we don't build struct every time
			CBaseVSShader::DrawFlashlight_dx90_Vars_t vars;
			vars.m_bBump = hasBump;
			vars.m_nBumpmapVar = info.m_nBumpmap;
			vars.m_nBumpmapFrame = info.m_nBumpFrame;
			vars.m_nBumpTransform = info.m_nBumpTransform;
			vars.m_nFlashlightTextureVar = info.m_nFlashlightTexture;
			vars.m_nFlashlightTextureFrameVar = info.m_nFlashlightTextureFrame;
			vars.m_bLightmappedGeneric = true;
			vars.m_bWorldVertexTransition = true;
			vars.m_nBaseTexture2Var = info.m_nBaseTexture2;
			vars.m_nBaseTexture2FrameVar = info.m_nBaseTexture2Frame;
			vars.m_nBumpmap2Var = info.m_nBumpmap2;
			vars.m_nBumpmap2Frame = info.m_nBumpFrame2;
			vars.m_nBump2Transform = info.m_nBumpTransform2;
			vars.m_nAlphaTestReference = info.m_nAlphaTestReference;
			vars.m_bSSBump = hasSSBump;
			vars.m_nDetailVar = info.m_nDetail;
			vars.m_nDetailScale = info.m_nDetailScale;
			vars.m_nDetailTextureCombineMode = info.m_nDetailTextureCombineMode;
			vars.m_nDetailTextureBlendFactor = info.m_nDetailTextureBlendFactor;
			vars.m_nDetailTint = info.m_nDetailTint;

			if ( ( info.m_nSeamlessMappingScale != -1 ) )
				vars.m_fSeamlessScale = params[info.m_nSeamlessMappingScale]->GetFloatValue();
			else
				vars.m_fSeamlessScale = 0.0;

			pShader->DrawFlashlight_dx90( params, pShaderAPI, pShaderShadow, vars );
			return;
		}

		pContextData->m_bFullyOpaque = bFullyOpaque;
		pContextData->m_bFullyOpaqueWithoutAlphaTest = bFullyOpaqueWithoutAlphaTest;

		bool hasEnvmapMask = params[info.m_nEnvmapMask]->IsTexture();
		bool bEnvmapAnisotropy = hasBump && !hasSSBump && ( info.m_nEnvmapAnisotropy != -1 ) && 
									  ( params[info.m_nEnvmapAnisotropy]->GetIntValue() == 1 );

		if ( pShaderShadow || bNeedRegenStaticCmds )
		{
			bool hasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
			
			bool hasEnvmap = params[info.m_nEnvmap]->IsTexture();
			bEnvmapAnisotropy = bEnvmapAnisotropy &&  hasEnvmap;
			int envmap_variant; //0 = no envmap, 1 = regular, 2 = darken in shadow mode
			if( hasEnvmap )
			{
				//only enabled darkened cubemap mode when the scale calls for it. And not supported in ps20 when also using a 2nd bumpmap
				envmap_variant = ((GetFloatParam( info.m_nEnvMapLightScale, params ) > 0.0f) && (g_pHardwareConfig->SupportsPixelShaders_2_b() || !hasBump2)) ? 2 : 1;
			}
			else
			{
				envmap_variant = 0; 
			}

			if ( hasDetailTexture )
			{
				ITexture *pDetailTexture = params[info.m_nDetail]->GetTextureValue();
				if ( pDetailTexture->GetFlags() & TEXTUREFLAGS_SSBUMP )
				{
					if ( hasBump )
						nDetailBlendMode = DETAIL_BLEND_MODE_SSBUMP_BUMP;
					else
						nDetailBlendMode = DETAIL_BLEND_MODE_SSBUMP_NOBUMP;
				}
			}

			bool bSeamlessMapping = ( ( info.m_nSeamlessMappingScale != -1 ) && 
									  ( params[info.m_nSeamlessMappingScale]->GetFloatValue() != 0.0 ) );
			
			if ( bNeedRegenStaticCmds )
			{
				pContextData->ResetStaticCmds();
				CCommandBufferBuilder< CFixedCommandStorageBuffer< 5000 > > staticCmdsBuf;

				if( !hasBaseTexture )
				{
					if( hasEnvmap )
					{
						// if we only have an envmap (no basetexture), then we want the albedo to be black.
						staticCmdsBuf.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_BLACK );
					}
					else
					{
						staticCmdsBuf.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_WHITE );
					}
				}
				staticCmdsBuf.BindStandardTexture( SHADER_SAMPLER1, TEXTURE_LIGHTMAP );

				if ( bSeamlessMapping )
				{
					staticCmdsBuf.SetVertexShaderConstant4(
						VERTEX_SHADER_SHADER_SPECIFIC_CONST_0,
						params[info.m_nSeamlessMappingScale]->GetFloatValue(),0,0,0 );
				}
				staticCmdsBuf.StoreEyePosInPixelShaderConstant( 10 );
#ifndef _PS3
				staticCmdsBuf.SetPixelShaderFogParams( 11 );
#endif
				staticCmdsBuf.End();
				// now, copy buf
				pContextData->m_pStaticCmds = new uint8[staticCmdsBuf.Size()];
				memcpy( pContextData->m_pStaticCmds, staticCmdsBuf.Base(), staticCmdsBuf.Size() );
			}
			if ( pShaderShadow )
			{
				// base texture
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, !bHDR );

				if ( hasBump )
					pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );

				if ( hasBump2 )
					pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );

				// base texture 2
				pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER7, true );

				// base texture 3
				pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, true );

				// base texture 4
				pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER11, true );

				// Alpha test: FIXME: shouldn't this be handled in Shader_t::SetInitialShadowState
				pShaderShadow->EnableAlphaTest( bIsAlphaTested );
				if ( info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
				{
					pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue() );
				}

				pShader->SetDefaultBlendingShadowState( nAlphaChannelTextureVar, hasBaseTexture );

				unsigned int flags = VERTEX_POSITION;

				if( hasEnvmap || ( ( IsX360() || IsPS3() ) && hasFlashlight ) )
				{
					if( hasEnvmap )
					{
						pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
						if( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
						{
							pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, true );
						}
					}
					flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL;
				}

				if ( hasDetailTexture )
				{
					ITexture *pDetailTexture = params[info.m_nDetail]->GetTextureValue();
					if ( pDetailTexture->GetFlags() & TEXTUREFLAGS_SSBUMP )
					{
						if ( hasBump )
							nDetailBlendMode = DETAIL_BLEND_MODE_SSBUMP_BUMP;
						else
							nDetailBlendMode = DETAIL_BLEND_MODE_SSBUMP_NOBUMP;
					}

					pShaderShadow->EnableTexture( SHADER_SAMPLER12, true );
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER12, IsSRGBDetailTexture( nDetailBlendMode ) );
				}

				if( hasVertexColor || hasBump2 )
				{
					flags |= VERTEX_COLOR;
				}
				flags |= VERTEX_SPECULAR;

				// texcoord0 : base texcoord
				// texcoord1 : lightmap texcoord
				// texcoord2 : lightmap texcoord offset
				int numTexCoords;
				
				// if ( pShaderAPI->InEditorMode() )
// 				if ( pShader->CanUseEditorMaterials() )
// 				{
// 					numTexCoords = 1;
// 				}
// 				else
				{
					numTexCoords = 2;
					if( hasBump )
					{
						numTexCoords = 3;
					}
				}

				int nTexture3BlendMode = GetIntParam( info.m_nTexture3BlendMode, params );
				int nTexture4BlendMode = GetIntParam( info.m_nTexture4BlendMode, params );

				pShaderShadow->VertexShaderVertexFormat( flags, numTexCoords, 0, 0 );

				// Pre-cache pixel shaders
				bool hasBaseAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK );

				int bumpmap_variant=(hasSSBump) ? 2 : hasBump;

				#if !defined( _X360 ) && !defined( _PS3 )
				if ( !g_pHardwareConfig->SupportsShaderModel_3_0() )
				#endif
				{
					DECLARE_STATIC_VERTEX_SHADER( lightmapped_4wayblend_vs20 );
					SET_STATIC_VERTEX_SHADER_COMBO( ENVMAP_MASK,  hasEnvmapMask );
					SET_STATIC_VERTEX_SHADER_COMBO( TANGENTSPACE,  params[info.m_nEnvmap]->IsTexture() );
					SET_STATIC_VERTEX_SHADER_COMBO( BUMPMAP,  hasBump );
					SET_STATIC_VERTEX_SHADER_COMBO( VERTEXCOLOR, IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) );
					SET_STATIC_VERTEX_SHADER_COMBO( SEAMLESS, bSeamlessMapping );
					SET_STATIC_VERTEX_SHADER_COMBO( DETAILTEXTURE, hasDetailTexture );
					SET_STATIC_VERTEX_SHADER_COMBO( SELFILLUM,  hasSelfIllum );
					SET_STATIC_VERTEX_SHADER_COMBO( DOPIXELFOG,  ( g_pHardwareConfig->SupportsPixelShaders_2_b() && !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ) ? 1 : 0 );
					SET_STATIC_VERTEX_SHADER_COMBO( HARDWAREFOGBLEND,  !g_pHardwareConfig->SupportsPixelShaders_2_b() );
					SET_STATIC_VERTEX_SHADER( lightmapped_4wayblend_vs20 );

					DECLARE_STATIC_PIXEL_SHADER( lightmapped_4wayblend_ps20b );
					SET_STATIC_PIXEL_SHADER_COMBO( BUMPMAP,  bumpmap_variant );
					SET_STATIC_PIXEL_SHADER_COMBO( BUMPMAP2, hasBump2 );
					SET_STATIC_PIXEL_SHADER_COMBO( TEXTURE3_BLENDMODE, nTexture3BlendMode );
					SET_STATIC_PIXEL_SHADER_COMBO( TEXTURE4_BLENDMODE, nTexture4BlendMode );
					SET_STATIC_PIXEL_SHADER_COMBO( CUBEMAP,  envmap_variant );
					SET_STATIC_PIXEL_SHADER_COMBO( ENVMAPMASK,  hasEnvmapMask );
					SET_STATIC_PIXEL_SHADER_COMBO( BASEALPHAENVMAPMASK,  hasBaseAlphaEnvmapMask );
					SET_STATIC_PIXEL_SHADER_COMBO( SELFILLUM,  hasSelfIllum );
					SET_STATIC_PIXEL_SHADER_COMBO( SEAMLESS, bSeamlessMapping );
					SET_STATIC_PIXEL_SHADER_COMBO( DETAIL_BLEND_MODE, nDetailBlendMode );
					SET_STATIC_PIXEL_SHADER_COMBO( ENVMAPANISOTROPY, bEnvmapAnisotropy );
					SET_STATIC_PIXEL_SHADER_COMBO( DOPIXELFOG, !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ? 1 : 0 );
					SET_STATIC_PIXEL_SHADER( lightmapped_4wayblend_ps20b );
				}
				#if !defined( _X360 ) && !defined( _PS3 )
				else // Shader model 3.0, PC only
				{
					DECLARE_STATIC_VERTEX_SHADER( lightmapped_4wayblend_vs30 );
					SET_STATIC_VERTEX_SHADER_COMBO( ENVMAP_MASK,  hasEnvmapMask );
					SET_STATIC_VERTEX_SHADER_COMBO( TANGENTSPACE,  params[info.m_nEnvmap]->IsTexture() );
					SET_STATIC_VERTEX_SHADER_COMBO( BUMPMAP,  hasBump );
					SET_STATIC_VERTEX_SHADER_COMBO( VERTEXCOLOR, IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) );
					SET_STATIC_VERTEX_SHADER_COMBO( SEAMLESS, bSeamlessMapping );
					SET_STATIC_VERTEX_SHADER_COMBO( DETAILTEXTURE, hasDetailTexture );
					SET_STATIC_VERTEX_SHADER_COMBO( SELFILLUM,  hasSelfIllum );
					SET_STATIC_VERTEX_SHADER_COMBO( DOPIXELFOG,  ( g_pHardwareConfig->SupportsPixelShaders_2_b() && !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ) ? 1 : 0 );
					SET_STATIC_VERTEX_SHADER_COMBO( HARDWAREFOGBLEND,  0 );
					SET_STATIC_VERTEX_SHADER( lightmapped_4wayblend_vs30 );

					DECLARE_STATIC_PIXEL_SHADER( lightmapped_4wayblend_ps30 );
					SET_STATIC_PIXEL_SHADER_COMBO( BUMPMAP,  bumpmap_variant );
					SET_STATIC_PIXEL_SHADER_COMBO( BUMPMAP2, hasBump2 );
					SET_STATIC_PIXEL_SHADER_COMBO( TEXTURE3_BLENDMODE, nTexture3BlendMode );
					SET_STATIC_PIXEL_SHADER_COMBO( TEXTURE4_BLENDMODE, nTexture4BlendMode );
					SET_STATIC_PIXEL_SHADER_COMBO( CUBEMAP,  envmap_variant );
					SET_STATIC_PIXEL_SHADER_COMBO( ENVMAPMASK,  hasEnvmapMask );
					SET_STATIC_PIXEL_SHADER_COMBO( BASEALPHAENVMAPMASK,  hasBaseAlphaEnvmapMask );
					SET_STATIC_PIXEL_SHADER_COMBO( SELFILLUM,  hasSelfIllum );
					SET_STATIC_PIXEL_SHADER_COMBO( SEAMLESS, bSeamlessMapping );
					SET_STATIC_PIXEL_SHADER_COMBO( DETAIL_BLEND_MODE, nDetailBlendMode );
					SET_STATIC_PIXEL_SHADER_COMBO( ENVMAPANISOTROPY, bEnvmapAnisotropy );
					SET_STATIC_PIXEL_SHADER_COMBO( DOPIXELFOG, !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ? 1 : 0 );
					SET_STATIC_PIXEL_SHADER( lightmapped_4wayblend_ps30 );
				}
				#endif

				// HACK HACK HACK - enable alpha writes all the time so that we have them for
				// underwater stuff and writing depth to dest alpha
				// But only do it if we're not using the alpha already for translucency
				pShaderShadow->EnableAlphaWrites( bFullyOpaque );

				pShaderShadow->EnableSRGBWrite( true );

				pShader->DefaultFog();
			} // end shadow state
		} // end shadow || regen display list

		if ( pShaderAPI && ( pContextData->m_bMaterialVarsChanged ) )
		{
			// need to regenerate the semistatic cmds
			pContextData->m_SemiStaticCmdsOut.Reset();
			pContextData->m_bMaterialVarsChanged = false;
			
			// If we don't have a texture transform, we don't have
			// to set vertex shader constants or run vertex shader instructions
			// for the texture transform.
			bool bHasTextureTransform = 
				!( params[info.m_nBaseTextureTransform]->MatrixIsIdentity() &&
				   params[info.m_nBumpTransform]->MatrixIsIdentity() &&
				   params[info.m_nBumpTransform2]->MatrixIsIdentity() &&
				   params[info.m_nEnvmapMaskTransform]->MatrixIsIdentity() );
						
			pContextData->m_bVertexShaderFastPath = !bHasTextureTransform;

			if( params[info.m_nDetail]->IsTexture() )
			{
				pContextData->m_bVertexShaderFastPath = false;
			}
			int nTransformToLoad = -1;
			if( ( hasBump || hasSSBump ) && hasDetailTexture && !hasSelfIllum )
			{
				nTransformToLoad = info.m_nBumpTransform;
			}
			pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( 
				VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, nTransformToLoad );

			if ( ! pContextData->m_bVertexShaderFastPath )
			{
				bool bSeamlessMapping = ( ( info.m_nSeamlessMappingScale != -1 ) && 
										  ( params[info.m_nSeamlessMappingScale]->GetFloatValue() != 0.0 ) );
				bool hasEnvmapMask = params[info.m_nEnvmapMask]->IsTexture();
				if (!bSeamlessMapping )
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );
				// If we have a detail texture, then the bump texcoords are the same as the base texcoords.
				if( hasBump && !hasDetailTexture )
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, info.m_nBumpTransform );
				}
				if( hasEnvmapMask )
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, info.m_nEnvmapMaskTransform );
				}
				else if ( hasBump2 )
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, info.m_nBumpTransform2 );
				}
			}
			pContextData->m_SemiStaticCmdsOut.SetEnvMapTintPixelShaderDynamicState( 0, info.m_nEnvmapTint );

			// set up shader modulation color
			float color[4] = { 1.0, 1.0, 1.0, 1.0 };
			pShader->ComputeModulationColor( color );
			float flLScale = pShaderAPI->GetLightMapScaleFactor();
			color[0] *= flLScale;
			color[1] *= flLScale;
			color[2] *= flLScale;
			pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant( VERTEX_SHADER_MODULATION_COLOR, color );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 12, color );
			
			float bumpblendfactors[ 3 ];
			bumpblendfactors[ 0 ] = clamp( GetFloatParam( info.m_nTexture2BumpBlendFactor, params, 1.0f ), 0.0f, 1.0f );
			bumpblendfactors[ 1 ] = clamp( GetFloatParam( info.m_nTexture3BumpBlendFactor, params, 1.0f ), 0.0f, 1.0f );
			bumpblendfactors[ 2 ] = clamp( GetFloatParam( info.m_nTexture4BumpBlendFactor, params, 1.0f ), 0.0f, 1.0f );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 1, bumpblendfactors );

			float ranges[ 4 ];
			ranges[ 0 ] = clamp( GetFloatParam( info.m_nTexture1LumStart, params, 0.0f ), 0.0f, 1.0f );
			ranges[ 1 ] = clamp( GetFloatParam( info.m_nTexture1LumEnd, params, 1.0f ), 0.0f, 1.0f );
			ranges[ 2 ] = clamp( GetFloatParam( info.m_nTexture2LumStart, params, 0.0f ), 0.0f, 1.0f );
			ranges[ 3 ] = clamp( GetFloatParam( info.m_nTexture2LumEnd, params, 1.0f ), 0.0f, 1.0f );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 5, ranges );

			ranges[ 0 ] = clamp( GetFloatParam( info.m_nTexture3LumStart, params, 0.0f ), 0.0f, 1.0f );
			ranges[ 1 ] = clamp( GetFloatParam( info.m_nTexture3LumEnd, params, 1.0f ), 0.0f, 1.0f );
			ranges[ 2 ] = clamp( GetFloatParam( info.m_nTexture4LumStart, params, 0.0f ), 0.0f, 1.0f );
			ranges[ 3 ] = clamp( GetFloatParam( info.m_nTexture4LumEnd, params, 1.0f ), 0.0f, 1.0f );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 28, ranges );

			if ( hasDetailTexture )
			{
				float detailTint[4] = {1, 1, 1, 1};
			
				if ( info.m_nDetailTint != -1 )
				{
					params[ info.m_nDetailTint ]->GetVecValue( detailTint, 3 );
				}
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 8, detailTint );

				float fDetailBlendFactors[ 4 ];
				fDetailBlendFactors[ 0 ] = clamp( GetFloatParam( info.m_nDetailTextureBlendFactor, params, 1.0 ), 0.0f, 1.0f );
				fDetailBlendFactors[ 1 ] = clamp( GetFloatParam( info.m_nDetailTextureBlendFactor2, params, 1.0 ), 0.0f, 1.0f );
				fDetailBlendFactors[ 2 ] = clamp( GetFloatParam( info.m_nDetailTextureBlendFactor3, params, 1.0 ), 0.0f, 1.0f );
				fDetailBlendFactors[ 3 ] = clamp( GetFloatParam( info.m_nDetailTextureBlendFactor4, params, 1.0 ), 0.0f, 1.0f );
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 9, fDetailBlendFactors );
			}

			ranges[ 0 ] = clamp( GetFloatParam( info.m_nTexture2BlendStart, params, 0.0f ), 0.0f, 1.0f );
			ranges[ 1 ] = clamp( GetFloatParam( info.m_nTexture2BlendEnd, params, 1.0f ), 0.0f, 1.0f );
			ranges[ 2 ] = clamp( GetFloatParam( info.m_nTexture3BlendStart, params, 0.0f ), 0.0f, 1.0f );
			ranges[ 3 ] = clamp( GetFloatParam( info.m_nTexture3BlendEnd, params, 1.0f ), 0.0f, 1.0f );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 15, ranges );

			float uvScales[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
			if ( info.m_nTexture2uvScale != -1 )
			{
				params[ info.m_nTexture2uvScale ]->GetVecValue( &uvScales[ 0 ], 2 );
			}
			if ( info.m_nTexture3uvScale != -1 )
			{
				params[ info.m_nTexture3uvScale ]->GetVecValue( &uvScales[ 2 ], 2 );
			}
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 16, uvScales );

			uvScales[ 0 ] = clamp( GetFloatParam( info.m_nTexture4BlendStart, params, 0.0f ), 0.0f, 1.0f );
			uvScales[ 1 ] = clamp( GetFloatParam( info.m_nTexture4BlendEnd, params, 1.0f ), 0.0f, 1.0f );
			if ( info.m_nTexture4uvScale != -1 )
			{
				params[ info.m_nTexture4uvScale ]->GetVecValue( &uvScales[ 2 ], 2 );
			}
			else
			{
				uvScales[ 2 ] = 1.0f;
				uvScales[ 3 ] = 1.0f;
			}
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 17, uvScales );

			float lumblends[ 3 ];
			lumblends[ 0 ] = clamp( GetFloatParam( info.m_nLumBlendFactor2, params, 1.0f ), 0.0f, 1.0f );
			lumblends[ 1 ] = clamp( GetFloatParam( info.m_nLumBlendFactor3, params, 1.0f ), 0.0f, 1.0f );
			lumblends[ 2 ] = clamp( GetFloatParam( info.m_nLumBlendFactor4, params, 1.0f ), 0.0f, 1.0f );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 18, lumblends );

			float envmapTintVal[4];
			float selfIllumTintVal[4];
			params[info.m_nEnvmapTint]->GetVecValue( envmapTintVal, 3 );
			params[info.m_nSelfIllumTint]->GetVecValue( selfIllumTintVal, 3 );
			float envmapContrast = params[info.m_nEnvmapContrast]->GetFloatValue();
			float envmapSaturation = params[info.m_nEnvmapSaturation]->GetFloatValue();
			float fresnelReflection = params[info.m_nFresnelReflection]->GetFloatValue();
			bool hasEnvmap = params[info.m_nEnvmap]->IsTexture();
				 bEnvmapAnisotropy = bEnvmapAnisotropy && hasEnvmap;
			int envmap_variant; //0 = no envmap, 1 = regular, 2 = darken in shadow mode
			if( hasEnvmap )
			{
				//only enabled darkened cubemap mode when the scale calls for it. And not supported in ps20 when also using a 2nd bumpmap
				envmap_variant = ((GetFloatParam( info.m_nEnvMapLightScale, params ) > 0.0f) && (g_pHardwareConfig->SupportsPixelShaders_2_b() || !hasBump2)) ? 2 : 1;
			}
			else
			{
				envmap_variant = 0; 
			}

			pContextData->m_bPixelShaderFastPath = true;
			bool bUsingContrastOrSaturation = hasEnvmap && ( ( (envmapContrast != 0.0f) && (envmapContrast != 1.0f) ) || (envmapSaturation != 1.0f) );
			bool bUsingFresnel = hasEnvmap && (fresnelReflection != 1.0f);
			bool bUsingSelfIllumTint = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && (selfIllumTintVal[0] != 1.0f || selfIllumTintVal[1] != 1.0f || selfIllumTintVal[2] != 1.0f); 
			if ( bUsingContrastOrSaturation || bUsingFresnel || bUsingSelfIllumTint || !g_pConfig->bShowSpecular )
			{
				pContextData->m_bPixelShaderFastPath = false;
			}
			if( !pContextData->m_bPixelShaderFastPath )
			{
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstants( 2, 3 );
				pContextData->m_SemiStaticCmdsOut.OutputConstantData( params[info.m_nEnvmapContrast]->GetVecValue() );
				pContextData->m_SemiStaticCmdsOut.OutputConstantData( params[info.m_nEnvmapSaturation]->GetVecValue() );
				float flFresnel = params[info.m_nFresnelReflection]->GetFloatValue();
				// [ 0, 0, 1-R(0), R(0) ]
				pContextData->m_SemiStaticCmdsOut.OutputConstantData4( 0., 0., 1.0 - flFresnel, flFresnel );
				
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 7, params[info.m_nSelfIllumTint]->GetVecValue() );
			}

			// cubemap light scale mapping parms (c20)
			if ( ( envmap_variant == 2 ) || bEnvmapAnisotropy )
			{
				float envMapParams[4] = { 0, 0, 0, 0 };
				if ( bEnvmapAnisotropy )
				{
					envMapParams[0] = GetFloatParam( info.m_nEnvmapAnisotropyScale, params );
				}
				if ( envmap_variant == 2 )
				{
					envMapParams[1] = GetFloatParam( info.m_nEnvMapLightScale, params );
					float lightScaleMinMax[2] = { 0.0, 0.0 };
					params[info.m_nEnvMapLightScaleMinMax]->GetVecValue( lightScaleMinMax, 2 );
					envMapParams[2] = lightScaleMinMax[0];
					envMapParams[3] = lightScaleMinMax[1] + lightScaleMinMax[0];
				}
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( 20, envMapParams );
			}

			// texture binds
			if( hasBaseTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
			}

			// always set the transform for detail textures since I'm assuming that you'll
			// always have a detailscale.
			if( hasDetailTexture )
			{
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, info.m_nBaseTextureTransform, info.m_nDetailScale );
			}

			pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER7, info.m_nBaseTexture2, info.m_nBaseTexture2Frame );
			pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER8, info.m_nBaseTexture3, info.m_nBaseTexture3Frame );
			pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, info.m_nBaseTexture4, info.m_nBaseTexture4Frame );

			if( hasDetailTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, info.m_nDetail, info.m_nDetailFrame );
			}

			if( hasBump )
			{
				if( !g_pConfig->m_bFastNoBump )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER4, info.m_nBumpmap, info.m_nBumpFrame );
				}
				else
				{
					if( hasSSBump )
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER4, TEXTURE_SSBUMP_FLAT );
					}
					else
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER4, TEXTURE_NORMALMAP_FLAT );
					}
				}
			}
			if( hasBump2 )
			{
				if( !g_pConfig->m_bFastNoBump )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER5, info.m_nBumpmap2, info.m_nBumpFrame2 );
				}
				else
				{
					if( hasSSBump )
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER5, TEXTURE_NORMALMAP_FLAT );
					}
					else
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER5, TEXTURE_SSBUMP_FLAT );
					}
				}
			}

			if( hasEnvmapMask )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER5, info.m_nEnvmapMask, info.m_nEnvmapMaskFrame );
			}

			// handle mat_fullbright 2
			bool bLightingOnly = g_pConfig->nFullbright == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
			if( bLightingOnly )
			{
				// BASE TEXTURE
				if( hasSelfIllum )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY_ALPHA_ZERO );
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY );
				}

				// BASE TEXTURE 2	
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER7, TEXTURE_GREY );
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER8, TEXTURE_GREY );
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER11, TEXTURE_GREY );

				// DETAIL TEXTURE
				if( hasDetailTexture )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER12, TEXTURE_GREY );
				}

				// disable color modulation
				float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant( VERTEX_SHADER_MODULATION_COLOR, color );

				// turn off environment mapping
				envmapTintVal[0] = 0.0f;
				envmapTintVal[1] = 0.0f;
				envmapTintVal[2] = 0.0f;
			}

			pContextData->m_SemiStaticCmdsOut.End();
		}
	}
	DYNAMIC_STATE
	{
		CCommandBufferBuilder< CFixedCommandStorageBuffer< 1000 > > DynamicCmdsOut;
		DynamicCmdsOut.Call( pContextData->m_pStaticCmds );
		DynamicCmdsOut.Call( pContextData->m_SemiStaticCmdsOut.Base() );

		bool hasEnvmap = params[info.m_nEnvmap]->IsTexture();

		if( hasEnvmap )
		{
			DynamicCmdsOut.BindTexture( pShader, SHADER_SAMPLER2, info.m_nEnvmap, info.m_nEnvmapFrame );
		}

		bool bVertexShaderFastPath = pContextData->m_bVertexShaderFastPath;

		int nFixedLightingMode = pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_ENABLE_FIXED_LIGHTING );
		if( nFixedLightingMode != ENABLE_FIXED_LIGHTING_NONE )
		{
			bVertexShaderFastPath = false;
		}

		bool bWorldNormal = ( nFixedLightingMode == ENABLE_FIXED_LIGHTING_OUTPUTNORMAL_AND_DEPTH );
		if ( bWorldNormal && IsPC() )
		{
			float vEyeDir[4];
			pShaderAPI->GetWorldSpaceCameraDirection( vEyeDir );

			float flFarZ = pShaderAPI->GetFarZ();
			vEyeDir[0] /= flFarZ;	// Divide by farZ for SSAO algorithm
			vEyeDir[1] /= flFarZ;
			vEyeDir[2] /= flFarZ;
			DynamicCmdsOut.SetVertexShaderConstant4( 12, vEyeDir[0], vEyeDir[1], vEyeDir[2], 1.0f );
		}

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();

#if !defined( _X360 ) && !defined( _PS3 )
		if ( g_pHardwareConfig->SupportsShaderModel_3_0() )
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( lightmapped_4wayblend_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( FASTPATH,  bVertexShaderFastPath );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG,  ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) );
			SET_DYNAMIC_VERTEX_SHADER_CMD( DynamicCmdsOut, lightmapped_4wayblend_vs30 );
		}
		else
#endif
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( lightmapped_4wayblend_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( FASTPATH,  bVertexShaderFastPath );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG,  ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) );
			SET_DYNAMIC_VERTEX_SHADER_CMD( DynamicCmdsOut, lightmapped_4wayblend_vs20 );
		}

		// This block of logic is up here and is so verbose because the compiler on the Mac was previously
		// optimizing much of this away and allowing out of range values into the logic which ultimately
		// computes the dynamic index.  Please leave this here and don't try to weave it into the dynamic
		// combo setting macros below.		
		bool bPixelShaderFastPath = false;
		if ( pContextData->m_bPixelShaderFastPath )
			bPixelShaderFastPath = true;
		bool bFastPath = false;
		if ( bPixelShaderFastPath )
			bFastPath = true;
		
		
		if ( nFixedLightingMode != ENABLE_FIXED_LIGHTING_NONE )
		{
			bPixelShaderFastPath = false;
		}
		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha;
		if(  pContextData->m_bFullyOpaque ) 
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg( !(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time." );
		}
		else
		{
			//can't write a special value to dest alpha if we're actually using as-intended alpha
			bWriteDepthToAlpha = false;
			bWriteWaterFogToAlpha = false;
		}

		/* Time - used for debugging
		float vTimeConst[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float flTime = pShaderAPI->CurrentTime();
		vTimeConst[0] = flTime;
		DynamicCmdsOut.SetPixelShaderConstant( 27, vTimeConst, 1 );
		//*/

		float envmapContrast = params[info.m_nEnvmapContrast]->GetFloatValue();
		
#if !defined( _X360 ) && !defined( _PS3 )
		if ( g_pHardwareConfig->SupportsShaderModel_3_0() )
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( lightmapped_4wayblend_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FASTPATH, bFastPath );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FASTPATHENVMAPCONTRAST, bPixelShaderFastPath && envmapContrast == 1.0f );

			// Don't write fog to alpha if we're using translucency
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) );
			SET_DYNAMIC_PIXEL_SHADER_CMD( DynamicCmdsOut, lightmapped_4wayblend_ps30 );
		}
		else
#endif
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( lightmapped_4wayblend_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FASTPATH, bFastPath );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FASTPATHENVMAPCONTRAST, bPixelShaderFastPath && envmapContrast == 1.0f );

			// Don't write fog to alpha if we're using translucency
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) );
			SET_DYNAMIC_PIXEL_SHADER_CMD( DynamicCmdsOut, lightmapped_4wayblend_ps20b );
		}

		DynamicCmdsOut.End();
#ifdef _PS3
		pShaderAPI->SetPixelShaderFogParams( 11 );
#endif 
		pShaderAPI->ExecuteCommandBuffer( DynamicCmdsOut.Base() );
	}
	pShader->Draw();

	if( IsPC() && (IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0) && pContextData->m_bFullyOpaqueWithoutAlphaTest )
	{
		//Alpha testing makes it so we can't write to dest alpha
		//Writing to depth makes it so later polygons can't write to dest alpha either
		//This leads to situations with garbage in dest alpha.

		//Fix it now by converting depth to dest alpha for any pixels that just wrote.
		pShader->DrawEqualDepthToDestAlpha();
	}
}
