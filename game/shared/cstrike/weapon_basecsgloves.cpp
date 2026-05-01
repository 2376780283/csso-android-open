//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: Base class for all in-game gloves
//
//=============================================================================//

#include "cbase.h"
#include "weapon_basecsgloves.h"
#include "cs_loadout.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#include "cs_skin_database.h"
#else
#include "cs_player.h"
#endif

LINK_ENTITY_TO_CLASS( cs_base_glove, CBaseCSGloves );
IMPLEMENT_NETWORKCLASS_ALIASED( BaseCSGloves, DT_BaseCSGloves )

BEGIN_NETWORK_TABLE( CBaseCSGloves, DT_BaseCSGloves )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_nGloveID ) ),
    RecvPropInt( RECVINFO( iGlovePaintKit )),
#else
	SendPropInt( SENDINFO( m_nGloveID ), 8 ),
    SendPropInt( SENDINFO( iGlovePaintKit), 8 )
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC( CBaseCSGloves )
END_DATADESC()

CBaseCSGloves::CBaseCSGloves()
{
#ifndef CLIENT_DLL
	m_nGloveID = 0;
    iGlovePaintKit = 0;
#endif
}

#ifdef GAME_DLL
int CBaseCSGloves::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}
#endif

void CBaseCSGloves::Equip( CBaseAnimating* pOwner )
{
	if ( !pOwner )
		return;

	if ( !pOwner->IsAlive() )
		return;

	FollowEntity( pOwner, true );
	SetOwnerEntity( pOwner );
	AddEffects( EF_BONEMERGE_FASTCULL );
	SetSolid( SOLID_NONE );

	UpdateGlovesModel();

#ifdef CLIENT_DLL
	SetUseParentLightingOrigin( true );
    
    ClearMaterialOverride();
    
    int iPaintKit = GetGlovePaintKit();


    if ( iPaintKit > 0 )
	{
		const SkinDefinition_t* pSkinDef = g_SkinDatabase.FindSkinByPaintKit( iPaintKit );
		if ( pSkinDef )
		{
			FOR_EACH_VEC(pSkinDef->materials, i)
			{
				const SkinDefinition_t::MaterialData_t& matData = pSkinDef->materials[i];
				IMaterial* pMat = g_SkinDatabase.GetSkinMaterial( iPaintKit, matData.iMaterialIndex );
						
				if ( pMat )
				{
					this->SetMaterialOverride( pMat, matData.iMaterialIndex );
			    }
			}
    	}
	}
#endif

	// assuming that before equipping them, a DoesModelSupportGloves() check was made
	pOwner->SetBodygroup( pOwner->FindBodygroupByName( "gloves" ), 1 ); // hide default gloves
}
#ifdef CLIENT_DLL
void CBaseCSGloves::SetClientGlovePaintKit()
{
    ClearMaterialOverride();
    
    int iPaintKit = GetGlovePaintKit();


    if ( iPaintKit > 0 )
	{
		const SkinDefinition_t* pSkinDef = g_SkinDatabase.FindSkinByPaintKit( iPaintKit );
		if ( pSkinDef )
		{
			FOR_EACH_VEC(pSkinDef->materials, i)
			{
				const SkinDefinition_t::MaterialData_t& matData = pSkinDef->materials[i];
				IMaterial* pMat = g_SkinDatabase.GetSkinMaterial( iPaintKit, matData.iMaterialIndex );
						
				if ( pMat )
				{
					this->SetMaterialOverride( pMat, matData.iMaterialIndex );
			    }
			}
    	}
	}
}
#endif

void CBaseCSGloves::UnEquip()
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );

	if ( !pPlayerOwner )
	{
		return;
	}

	pPlayerOwner->SetBodygroup( pPlayerOwner->FindBodygroupByName( "gloves" ), 0 ); // restore default gloves

#ifdef CLIENT_DLL
	ClearMaterialOverride();
#endif

	SetOwnerEntity( NULL );
}

void CBaseCSGloves::UpdateGlovesModel()
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );
	if ( !pPlayerOwner )
		return;

#ifdef CLIENT_DLL
	MDLCACHE_CRITICAL_SECTION();
#endif
	const char *pszModel = GetGlovesInfo( m_nGloveID )->szWorldModel;
	SetModel( pszModel );

#ifdef CLIENT_DLL

    extern ConVar cl_showfirstperson_legs;
   SetClientGlovePaintKit(); 
   
   if (pPlayerOwner->InFirstPersonView() && cl_showfirstperson_legs.GetBool())
   {
	   AddEffects( EF_NODRAW );
   }
   else if (!pPlayerOwner->InFirstPersonView() && cl_showfirstperson_legs.GetBool())
   {
       RemoveEffects( EF_NODRAW );
   }
   else
   {
	   RemoveEffects( EF_NODRAW );
   }
   
   
	if ( pPlayerOwner->m_pViewmodelArmConfig != NULL )
		m_nSkin = pPlayerOwner->m_pViewmodelArmConfig->iSkintoneIndex;
	else
#endif
	{
		CStudioHdr *pHdr = pPlayerOwner->GetModelPtr();
		if ( pHdr )
			m_nSkin = GetPlayerViewmodelArmConfigForPlayerModel( pHdr->pszName() )->iSkintoneIndex;
	}
}

#ifdef CLIENT_DLL
void CBaseCSGloves::OnDataChanged( DataUpdateType_t type )
{
    if ( type == DATA_UPDATE_CREATED || type == DATA_UPDATE_DATATABLE_CHANGED )
    {
        ClearMaterialOverride();
        SetClientGlovePaintKit();
    }
}
#endif