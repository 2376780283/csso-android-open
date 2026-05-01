#include "cbase.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/PHandle.h"
#include "vgui_controls/MenuItem.h"
#include "vgui_controls/MessageDialog.h"
#include "KeyValues.h"
#include "utlvector.h"
#include "tier1/CommandBuffer.h"
#include "tier2/camerautils.h"
#include "tier3/mdlutils.h"
#include "materialsystem/MaterialSystemUtil.h"

class IMaterial;
class IInput;

#define MATERIAL_MAX_LIGHT_COUNT 4
class CBasePlayerModelPanel: public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CBasePlayerModelPanel, vgui::Panel );

public:
	CBasePlayerModelPanel( Panel* parent, const char* panelName );
	virtual ~CBasePlayerModelPanel();

public:
	virtual void Paint();
	virtual void OnThink();
	virtual void ApplySettings( KeyValues* inResourceData );
	virtual void OnMousePressed( vgui::MouseCode code );
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void OnCursorMoved( int x, int y );
	virtual void OnCursorExited();

	void SetupRenderState( int nDisplayWidth, int nDisplayHeight );
	void ParseLightInfo( KeyValues* inResourceData );
	void OnPaint3D();
	void SetMDL( const char* pMDLName );
	CMDL *SetMergeMDL( const char* pMDLName );
	void ClearMergeMDLs();
	bool SetBodygroup( const char* pBodygroupName, int nValue );
	void PlaySequence( const char* pszSequenceName );

public:
	Camera_t m_Camera;
	Vector4D m_vecAmbientCube[6];
    Vector m_vecModelCenter;
	LightDesc_t m_pLightDesc[MATERIAL_MAX_LIGHT_COUNT];
	CMDL m_MDL;
	CUtlVector<CMDL> m_aMergeMDLs;
	matrix3x4_t	m_MDLToWorld;
	CTextureReference m_DefaultEnvCubemap;
	QAngle m_angPlayerModel;
	int m_nNumLightDescs;
	int m_nLastMouseX;
	int m_nLastMouseY;
	float m_flRotationAngleLeft;
	float m_flRotationTimeLeft;
	bool m_bMousePressed;
};
