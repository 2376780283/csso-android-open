#include "cbase.h"
#include "BlurUnified.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "vgui/IPanel.h"
#include "tier0/memdbgon.h"

CUnifiedBlurHelper::CUnifiedBlurHelper() 
    : m_bInitialized(false), m_bCanUseHighQuality(false)
{
}

CUnifiedBlurHelper::~CUnifiedBlurHelper()
{
    Cleanup();
}

void CUnifiedBlurHelper::InitResources()
{
    if (m_bInitialized) 
        return;

    CMatRenderContextPtr pRenderContext(materials);

    // Try to use high-quality blur resources (in-game mode)
    // Check if we can access game render targets
    ITexture *pFullFrameFB = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
    if (pFullFrameFB)
    {
        m_xblur_mat.Init("dev/blurfilterx_nohdr", TEXTURE_GROUP_OTHER);
        m_yblur_mat.Init("dev/blurfiltery_nohdr", TEXTURE_GROUP_OTHER);
        
        m_dest_rt0.Init(materials->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET));
        m_dest_rt1.Init(materials->FindTexture("_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET));
        
        m_bCanUseHighQuality = (m_xblur_mat.IsValid() && m_yblur_mat.IsValid() && 
                                m_dest_rt0.IsValid() && m_dest_rt1.IsValid());
    }

    // Setup fallback blur RT for menu mode
    if (!m_BlurRT.IsValid())
    {
        materials->BeginRenderTargetAllocation();
        m_BlurRT.Init(materials->CreateNamedRenderTargetTextureEx2(
            "_rt_UnifiedBlur", 256, 256, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888,
            MATERIAL_RT_DEPTH_NONE, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 0
        ));
        materials->EndRenderTargetAllocation();
    }

    m_bInitialized = true;
}

void CUnifiedBlurHelper::ApplyBlur(int x, int y, int w, int h, float fStrength, bool bForceMenuMode)
{
    if (w <= 0 || h <= 0) 
        return;

    InitResources();

    // Determine which blur mode to use
    if (!bForceMenuMode && m_bCanUseHighQuality)
    {
        ApplyHighQualityBlur(x, y, w, h, fStrength);
    }
    else
    {
        ApplyFallbackBlur(x, y, w, h, fStrength);
    }
}

void CUnifiedBlurHelper::ApplyHighQualityBlur(int x, int y, int w, int h, float fStrength)
{
    if (!m_xblur_mat.IsValid() || !m_yblur_mat.IsValid() || 
        !m_dest_rt0.IsValid() || !m_dest_rt1.IsValid())
    {
        return;
    }

    CMatRenderContextPtr pRenderContext(materials);
    ITexture *pSrc = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
    
    if (!pSrc) 
        return;

    int nSrcWidth = pSrc->GetActualWidth();
    int nSrcHeight = pSrc->GetActualHeight();

    pRenderContext->PushRenderTargetAndViewport();

    // Gaussian blur X pass
    pRenderContext->SetRenderTarget(m_dest_rt1);
    pRenderContext->Viewport(0, 0, nSrcWidth / 4, nSrcHeight / 4);
    
    pRenderContext->DrawScreenSpaceRectangle(
        m_xblur_mat, 0, 0, nSrcWidth / 4, nSrcHeight / 4,
        0, 0, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
        nSrcWidth / 4, nSrcHeight / 4);

    if (IsX360())
    {
        pRenderContext->CopyRenderTargetToTextureEx(m_dest_rt1, 0, NULL, NULL);
    }

    // Gaussian blur Y pass with bloom amount
    pRenderContext->SetRenderTarget(m_dest_rt0);
    pRenderContext->Viewport(0, 0, nSrcWidth / 4, nSrcHeight / 4);

    IMaterialVar *pBloomAmountVar = m_yblur_mat->FindVar("$bloomamount", NULL);
    if (pBloomAmountVar)
    {
        pBloomAmountVar->SetFloatValue(fStrength);
    }

    pRenderContext->DrawScreenSpaceRectangle(
        m_yblur_mat, 0, 0, nSrcWidth / 4, nSrcHeight / 4,
        0, 0, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
        nSrcWidth / 4, nSrcHeight / 4);

    if (IsX360())
    {
        pRenderContext->CopyRenderTargetToTextureEx(m_dest_rt0, 0, NULL, NULL);
    }

    pRenderContext->PopRenderTargetAndViewport();
}

void CUnifiedBlurHelper::ApplyFallbackBlur(int x, int y, int w, int h, float fStrength)
{
    if (!m_BlurRT.IsValid()) 
        return;

    CMatRenderContextPtr pRenderContext(materials);

    // Capture screen area behind the panel
    pRenderContext->Flush();
    Rect_t srcRect = {x, y, w, h};
    Rect_t dstRect = {0, 0, 256, 256};
    pRenderContext->CopyRenderTargetToTextureEx(m_BlurRT, 0, &srcRect, &dstRect);

    // Draw blurred result back (we keep this simple for menu mode)
    // For better results in menu, load blur material if available
    IMaterial *pBlurMat = materials->FindMaterial("vgui/blur_x", TEXTURE_GROUP_OTHER);
    if (pBlurMat)
    {
        IMaterialVar *pBaseTextureVar = pBlurMat->FindVar("$basetexture", NULL);
        if (pBaseTextureVar)
        {
            pBaseTextureVar->SetTextureValue(m_BlurRT);
        }

        pRenderContext->DrawScreenSpaceRectangle(
            pBlurMat, x, y, w, h,
            0, 0, 255, 255,
            256, 256);
    }

    // Apply darkness overlay using materials system
    IMaterial *pOverlay = materials->FindMaterial("vgui/white", TEXTURE_GROUP_OTHER, true);
    if (pOverlay)
    {
        pRenderContext->Bind(pOverlay);
        CMeshBuilder meshBuilder;
        IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
        meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

        int alpha = (int)(fStrength * 255);
        unsigned int color = (30 << 0) | (30 << 8) | (30 << 16) | (alpha << 24);

        meshBuilder.Position3f(x, y, 0.0f);
        meshBuilder.Color4ubv((unsigned char*)&color);
        meshBuilder.AdvanceVertex();

        meshBuilder.Position3f(x + w, y, 0.0f);
        meshBuilder.Color4ubv((unsigned char*)&color);
        meshBuilder.AdvanceVertex();

        meshBuilder.Position3f(x + w, y + h, 0.0f);
        meshBuilder.Color4ubv((unsigned char*)&color);
        meshBuilder.AdvanceVertex();

        meshBuilder.Position3f(x, y + h, 0.0f);
        meshBuilder.Color4ubv((unsigned char*)&color);
        meshBuilder.AdvanceVertex();

        meshBuilder.End();
        pMesh->Draw();
    }
}

void CUnifiedBlurHelper::Cleanup()
{
    if (m_bInitialized)
    {
        m_xblur_mat.Shutdown();
        m_yblur_mat.Shutdown();
        m_dest_rt0.Shutdown();
        m_dest_rt1.Shutdown();
        m_BlurRT.Shutdown();
        m_bInitialized = false;
        m_bCanUseHighQuality = false;
    }
}
