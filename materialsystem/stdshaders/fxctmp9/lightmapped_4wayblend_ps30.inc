#include "shaderlib/cshader.h"
class lightmapped_4wayblend_ps30_Static_Index
{
private:
	int m_nBUMPMAP;
#ifdef _DEBUG
	bool m_bBUMPMAP;
#endif
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif
	}
	void SetBUMPMAP( bool i )
	{
		m_nBUMPMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif
	}
private:
	int m_nBUMPMAP2;
#ifdef _DEBUG
	bool m_bBUMPMAP2;
#endif
public:
	void SetBUMPMAP2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP2 = i;
#ifdef _DEBUG
		m_bBUMPMAP2 = true;
#endif
	}
	void SetBUMPMAP2( bool i )
	{
		m_nBUMPMAP2 = i ? 1 : 0;
#ifdef _DEBUG
		m_bBUMPMAP2 = true;
#endif
	}
private:
	int m_nCUBEMAP;
#ifdef _DEBUG
	bool m_bCUBEMAP;
#endif
public:
	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
	void SetCUBEMAP( bool i )
	{
		m_nCUBEMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
private:
	int m_nENVMAPMASK;
#ifdef _DEBUG
	bool m_bENVMAPMASK;
#endif
public:
	void SetENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAPMASK = i;
#ifdef _DEBUG
		m_bENVMAPMASK = true;
#endif
	}
	void SetENVMAPMASK( bool i )
	{
		m_nENVMAPMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bENVMAPMASK = true;
#endif
	}
private:
	int m_nBASEALPHAENVMAPMASK;
#ifdef _DEBUG
	bool m_bBASEALPHAENVMAPMASK;
#endif
public:
	void SetBASEALPHAENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASEALPHAENVMAPMASK = i;
#ifdef _DEBUG
		m_bBASEALPHAENVMAPMASK = true;
#endif
	}
	void SetBASEALPHAENVMAPMASK( bool i )
	{
		m_nBASEALPHAENVMAPMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bBASEALPHAENVMAPMASK = true;
#endif
	}
private:
	int m_nSELFILLUM;
#ifdef _DEBUG
	bool m_bSELFILLUM;
#endif
public:
	void SetSELFILLUM( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSELFILLUM = i;
#ifdef _DEBUG
		m_bSELFILLUM = true;
#endif
	}
	void SetSELFILLUM( bool i )
	{
		m_nSELFILLUM = i ? 1 : 0;
#ifdef _DEBUG
		m_bSELFILLUM = true;
#endif
	}
private:
	int m_nSEAMLESS;
#ifdef _DEBUG
	bool m_bSEAMLESS;
#endif
public:
	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif
	}
	void SetSEAMLESS( bool i )
	{
		m_nSEAMLESS = i ? 1 : 0;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif
	}
private:
	int m_nENVMAPANISOTROPY;
#ifdef _DEBUG
	bool m_bENVMAPANISOTROPY;
#endif
public:
	void SetENVMAPANISOTROPY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAPANISOTROPY = i;
#ifdef _DEBUG
		m_bENVMAPANISOTROPY = true;
#endif
	}
	void SetENVMAPANISOTROPY( bool i )
	{
		m_nENVMAPANISOTROPY = i ? 1 : 0;
#ifdef _DEBUG
		m_bENVMAPANISOTROPY = true;
#endif
	}
private:
	int m_nTEXTURE3_BLENDMODE;
#ifdef _DEBUG
	bool m_bTEXTURE3_BLENDMODE;
#endif
public:
	void SetTEXTURE3_BLENDMODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTEXTURE3_BLENDMODE = i;
#ifdef _DEBUG
		m_bTEXTURE3_BLENDMODE = true;
#endif
	}
	void SetTEXTURE3_BLENDMODE( bool i )
	{
		m_nTEXTURE3_BLENDMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bTEXTURE3_BLENDMODE = true;
#endif
	}
private:
	int m_nTEXTURE4_BLENDMODE;
#ifdef _DEBUG
	bool m_bTEXTURE4_BLENDMODE;
#endif
public:
	void SetTEXTURE4_BLENDMODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTEXTURE4_BLENDMODE = i;
#ifdef _DEBUG
		m_bTEXTURE4_BLENDMODE = true;
#endif
	}
	void SetTEXTURE4_BLENDMODE( bool i )
	{
		m_nTEXTURE4_BLENDMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bTEXTURE4_BLENDMODE = true;
#endif
	}
private:
	int m_nDETAIL_BLEND_MODE;
#ifdef _DEBUG
	bool m_bDETAIL_BLEND_MODE;
#endif
public:
	void SetDETAIL_BLEND_MODE( int i )
	{
		Assert( i >= 0 && i <= 12 );
		m_nDETAIL_BLEND_MODE = i;
#ifdef _DEBUG
		m_bDETAIL_BLEND_MODE = true;
#endif
	}
	void SetDETAIL_BLEND_MODE( bool i )
	{
		m_nDETAIL_BLEND_MODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAIL_BLEND_MODE = true;
#endif
	}
private:
	int m_nDOPIXELFOG;
#ifdef _DEBUG
	bool m_bDOPIXELFOG;
#endif
public:
	void SetDOPIXELFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOPIXELFOG = i;
#ifdef _DEBUG
		m_bDOPIXELFOG = true;
#endif
	}
	void SetDOPIXELFOG( bool i )
	{
		m_nDOPIXELFOG = i ? 1 : 0;
#ifdef _DEBUG
		m_bDOPIXELFOG = true;
#endif
	}
public:
	lightmapped_4wayblend_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif // _DEBUG
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bBUMPMAP2 = false;
#endif // _DEBUG
		m_nBUMPMAP2 = 0;
#ifdef _DEBUG
		m_bCUBEMAP = false;
#endif // _DEBUG
		m_nCUBEMAP = 0;
#ifdef _DEBUG
		m_bENVMAPMASK = false;
#endif // _DEBUG
		m_nENVMAPMASK = 0;
#ifdef _DEBUG
		m_bBASEALPHAENVMAPMASK = false;
#endif // _DEBUG
		m_nBASEALPHAENVMAPMASK = 0;
#ifdef _DEBUG
		m_bSELFILLUM = false;
#endif // _DEBUG
		m_nSELFILLUM = 0;
#ifdef _DEBUG
		m_bSEAMLESS = false;
#endif // _DEBUG
		m_nSEAMLESS = 0;
#ifdef _DEBUG
		m_bENVMAPANISOTROPY = false;
#endif // _DEBUG
		m_nENVMAPANISOTROPY = 0;
#ifdef _DEBUG
		m_bTEXTURE3_BLENDMODE = false;
#endif // _DEBUG
		m_nTEXTURE3_BLENDMODE = 0;
#ifdef _DEBUG
		m_bTEXTURE4_BLENDMODE = false;
#endif // _DEBUG
		m_nTEXTURE4_BLENDMODE = 0;
#ifdef _DEBUG
		m_bDETAIL_BLEND_MODE = false;
#endif // _DEBUG
		m_nDETAIL_BLEND_MODE = 0;
#ifdef _DEBUG
		m_bDOPIXELFOG = false;
#endif // _DEBUG
		m_nDOPIXELFOG = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bBUMPMAP && m_bBUMPMAP2 && m_bCUBEMAP && m_bENVMAPMASK && m_bBASEALPHAENVMAPMASK && m_bSELFILLUM && m_bSEAMLESS && m_bENVMAPANISOTROPY && m_bTEXTURE3_BLENDMODE && m_bTEXTURE4_BLENDMODE && m_bDETAIL_BLEND_MODE && m_bDOPIXELFOG;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 32 * m_nBUMPMAP ) + ( 96 * m_nBUMPMAP2 ) + ( 192 * m_nCUBEMAP ) + ( 576 * m_nENVMAPMASK ) + ( 1152 * m_nBASEALPHAENVMAPMASK ) + ( 2304 * m_nSELFILLUM ) + ( 4608 * m_nSEAMLESS ) + ( 9216 * m_nENVMAPANISOTROPY ) + ( 18432 * m_nTEXTURE3_BLENDMODE ) + ( 36864 * m_nTEXTURE4_BLENDMODE ) + ( 73728 * m_nDETAIL_BLEND_MODE ) + ( 958464 * m_nDOPIXELFOG ) + 0;
	}
};
#define shaderStaticTest_lightmapped_4wayblend_ps30 psh_forgot_to_set_static_BUMPMAP + psh_forgot_to_set_static_BUMPMAP2 + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_ENVMAPMASK + psh_forgot_to_set_static_BASEALPHAENVMAPMASK + psh_forgot_to_set_static_SELFILLUM + psh_forgot_to_set_static_SEAMLESS + psh_forgot_to_set_static_ENVMAPANISOTROPY + psh_forgot_to_set_static_TEXTURE3_BLENDMODE + psh_forgot_to_set_static_TEXTURE4_BLENDMODE + psh_forgot_to_set_static_DETAIL_BLEND_MODE + psh_forgot_to_set_static_DOPIXELFOG + 0
class lightmapped_4wayblend_ps30_Dynamic_Index
{
private:
	int m_nFASTPATHENVMAPCONTRAST;
#ifdef _DEBUG
	bool m_bFASTPATHENVMAPCONTRAST;
#endif
public:
	void SetFASTPATHENVMAPCONTRAST( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFASTPATHENVMAPCONTRAST = i;
#ifdef _DEBUG
		m_bFASTPATHENVMAPCONTRAST = true;
#endif
	}
	void SetFASTPATHENVMAPCONTRAST( bool i )
	{
		m_nFASTPATHENVMAPCONTRAST = i ? 1 : 0;
#ifdef _DEBUG
		m_bFASTPATHENVMAPCONTRAST = true;
#endif
	}
private:
	int m_nFASTPATH;
#ifdef _DEBUG
	bool m_bFASTPATH;
#endif
public:
	void SetFASTPATH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFASTPATH = i;
#ifdef _DEBUG
		m_bFASTPATH = true;
#endif
	}
	void SetFASTPATH( bool i )
	{
		m_nFASTPATH = i ? 1 : 0;
#ifdef _DEBUG
		m_bFASTPATH = true;
#endif
	}
private:
	int m_nWRITEWATERFOGTODESTALPHA;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA;
#endif
public:
	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
	void SetWRITEWATERFOGTODESTALPHA( bool i )
	{
		m_nWRITEWATERFOGTODESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
private:
	int m_nWRITE_DEPTH_TO_DESTALPHA;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA;
#endif
public:
	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
	void SetWRITE_DEPTH_TO_DESTALPHA( bool i )
	{
		m_nWRITE_DEPTH_TO_DESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
private:
	int m_nPIXELFOGTYPE;
#ifdef _DEBUG
	bool m_bPIXELFOGTYPE;
#endif
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
	void SetPIXELFOGTYPE( bool i )
	{
		m_nPIXELFOGTYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
public:
	lightmapped_4wayblend_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bFASTPATHENVMAPCONTRAST = false;
#endif // _DEBUG
		m_nFASTPATHENVMAPCONTRAST = 0;
#ifdef _DEBUG
		m_bFASTPATH = false;
#endif // _DEBUG
		m_nFASTPATH = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
#endif // _DEBUG
		m_nWRITEWATERFOGTODESTALPHA = 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bFASTPATHENVMAPCONTRAST && m_bFASTPATH && m_bWRITEWATERFOGTODESTALPHA && m_bWRITE_DEPTH_TO_DESTALPHA && m_bPIXELFOGTYPE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nFASTPATHENVMAPCONTRAST ) + ( 2 * m_nFASTPATH ) + ( 4 * m_nWRITEWATERFOGTODESTALPHA ) + ( 8 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 16 * m_nPIXELFOGTYPE ) + 0;
	}
};
#define shaderDynamicTest_lightmapped_4wayblend_ps30 psh_forgot_to_set_dynamic_FASTPATHENVMAPCONTRAST + psh_forgot_to_set_dynamic_FASTPATH + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + 0
