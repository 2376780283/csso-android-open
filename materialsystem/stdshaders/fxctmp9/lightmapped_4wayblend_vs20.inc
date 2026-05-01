#include "shaderlib/cshader.h"
class lightmapped_4wayblend_vs20_Static_Index
{
private:
	int m_nENVMAP_MASK;
#ifdef _DEBUG
	bool m_bENVMAP_MASK;
#endif
public:
	void SetENVMAP_MASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAP_MASK = i;
#ifdef _DEBUG
		m_bENVMAP_MASK = true;
#endif
	}
	void SetENVMAP_MASK( bool i )
	{
		m_nENVMAP_MASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bENVMAP_MASK = true;
#endif
	}
private:
	int m_nTANGENTSPACE;
#ifdef _DEBUG
	bool m_bTANGENTSPACE;
#endif
public:
	void SetTANGENTSPACE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTANGENTSPACE = i;
#ifdef _DEBUG
		m_bTANGENTSPACE = true;
#endif
	}
	void SetTANGENTSPACE( bool i )
	{
		m_nTANGENTSPACE = i ? 1 : 0;
#ifdef _DEBUG
		m_bTANGENTSPACE = true;
#endif
	}
private:
	int m_nBUMPMAP;
#ifdef _DEBUG
	bool m_bBUMPMAP;
#endif
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
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
	int m_nDETAILTEXTURE;
#ifdef _DEBUG
	bool m_bDETAILTEXTURE;
#endif
public:
	void SetDETAILTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif
	}
	void SetDETAILTEXTURE( bool i )
	{
		m_nDETAILTEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif
	}
private:
	int m_nVERTEXCOLOR;
#ifdef _DEBUG
	bool m_bVERTEXCOLOR;
#endif
public:
	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif
	}
	void SetVERTEXCOLOR( bool i )
	{
		m_nVERTEXCOLOR = i ? 1 : 0;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
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
private:
	int m_nHARDWAREFOGBLEND;
#ifdef _DEBUG
	bool m_bHARDWAREFOGBLEND;
#endif
public:
	void SetHARDWAREFOGBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHARDWAREFOGBLEND = i;
#ifdef _DEBUG
		m_bHARDWAREFOGBLEND = true;
#endif
	}
	void SetHARDWAREFOGBLEND( bool i )
	{
		m_nHARDWAREFOGBLEND = i ? 1 : 0;
#ifdef _DEBUG
		m_bHARDWAREFOGBLEND = true;
#endif
	}
public:
	lightmapped_4wayblend_vs20_Static_Index( )
	{
#ifdef _DEBUG
		m_bENVMAP_MASK = false;
#endif // _DEBUG
		m_nENVMAP_MASK = 0;
#ifdef _DEBUG
		m_bTANGENTSPACE = false;
#endif // _DEBUG
		m_nTANGENTSPACE = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif // _DEBUG
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE = false;
#endif // _DEBUG
		m_nDETAILTEXTURE = 0;
#ifdef _DEBUG
		m_bVERTEXCOLOR = false;
#endif // _DEBUG
		m_nVERTEXCOLOR = 0;
#ifdef _DEBUG
		m_bSEAMLESS = false;
#endif // _DEBUG
		m_nSEAMLESS = 0;
#ifdef _DEBUG
		m_bSELFILLUM = false;
#endif // _DEBUG
		m_nSELFILLUM = 0;
#ifdef _DEBUG
		m_bDOPIXELFOG = false;
#endif // _DEBUG
		m_nDOPIXELFOG = 0;
#ifdef _DEBUG
		m_bHARDWAREFOGBLEND = false;
#endif // _DEBUG
		m_nHARDWAREFOGBLEND = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bENVMAP_MASK && m_bTANGENTSPACE && m_bBUMPMAP && m_bDETAILTEXTURE && m_bVERTEXCOLOR && m_bSEAMLESS && m_bSELFILLUM && m_bDOPIXELFOG && m_bHARDWAREFOGBLEND;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 4 * m_nENVMAP_MASK ) + ( 8 * m_nTANGENTSPACE ) + ( 16 * m_nBUMPMAP ) + ( 32 * m_nDETAILTEXTURE ) + ( 64 * m_nVERTEXCOLOR ) + ( 128 * m_nSEAMLESS ) + ( 256 * m_nSELFILLUM ) + ( 512 * m_nDOPIXELFOG ) + ( 1024 * m_nHARDWAREFOGBLEND ) + 0;
	}
};
#define shaderStaticTest_lightmapped_4wayblend_vs20 vsh_forgot_to_set_static_ENVMAP_MASK + vsh_forgot_to_set_static_TANGENTSPACE + vsh_forgot_to_set_static_BUMPMAP + vsh_forgot_to_set_static_DETAILTEXTURE + vsh_forgot_to_set_static_VERTEXCOLOR + vsh_forgot_to_set_static_SEAMLESS + vsh_forgot_to_set_static_SELFILLUM + vsh_forgot_to_set_static_DOPIXELFOG + vsh_forgot_to_set_static_HARDWAREFOGBLEND + 0
class lightmapped_4wayblend_vs20_Dynamic_Index
{
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
	int m_nDOWATERFOG;
#ifdef _DEBUG
	bool m_bDOWATERFOG;
#endif
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif
	}
	void SetDOWATERFOG( bool i )
	{
		m_nDOWATERFOG = i ? 1 : 0;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif
	}
public:
	lightmapped_4wayblend_vs20_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bFASTPATH = false;
#endif // _DEBUG
		m_nFASTPATH = 0;
#ifdef _DEBUG
		m_bDOWATERFOG = false;
#endif // _DEBUG
		m_nDOWATERFOG = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bFASTPATH && m_bDOWATERFOG;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nFASTPATH ) + ( 2 * m_nDOWATERFOG ) + 0;
	}
};
#define shaderDynamicTest_lightmapped_4wayblend_vs20 vsh_forgot_to_set_dynamic_FASTPATH + vsh_forgot_to_set_dynamic_DOWATERFOG + 0
