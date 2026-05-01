//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: player loadout
//
//=============================================================================//

#include "cbase.h"
#include "cs_loadout.h"
#include "cs_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#else
#include "cs_player.h"
#endif

#ifdef CLIENT_DLL
ConVar loadout_slot_m4_weapon( "loadout_slot_m4_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in M4 slot.\n 0 - M4A4\n 1 - M4A1-S", true, 0, true, 1 );
ConVar loadout_slot_hkp2000_weapon( "loadout_slot_hkp2000_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in HKP2000 slot.\n 0 - HKP2000\n 1 - USP-S", true, 0, true, 1 );
ConVar loadout_slot_knife_weapon_ct( "loadout_slot_knife_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in knife slot for CTs.\n 0 - Default CT knife\n 1 - CS:S knife\n 2 - Karambit\n 3 - Flip\n 4 - Bayonet\n 5 - M9 Bayonet\n 6 - Butterfly\n 7 - Gut\n 8 - Huntsman\n 9 - Falchion\n 10 - Bowie\n 11 - Survival\n 12 - Paracord\n 13 - Navaja\n 14 - Nomad\n 15 - Skeleton\n 16 - Stiletto\n 17 - Ursus\n 18 - Talon", true, 0, true, MAX_KNIVES );
ConVar loadout_slot_knife_weapon_t( "loadout_slot_knife_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in knife slot for Ts.\n 0 - Default T knife\n 1 - CS:S knife\n 2 - Karambit\n 3 - Flip\n 4 - Bayonet\n 5 - M9 Bayonet\n 6 - Butterfly\n 7 - Gut\n 8 - Huntsman\n 9 - Falchion\n 10 - Bowie\n 11 - Survival\n 12 - Paracord\n 13 - Navaja\n 14 - Nomad\n 15 - Skeleton\n 16 - Stiletto\n 17 - Ursus\n 18 - Talon", true, 0, true, MAX_KNIVES );
ConVar loadout_slot_knife_weapon_skin_ct( "loadout_slot_knife_weapon_skin_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which skin to use in knife slot for CTs.");
ConVar loadout_slot_knife_weapon_skin_t( "loadout_slot_knife_weapon_skin_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which skin to use in knife slot for Ts.");
ConVar loadout_slot_fiveseven_weapon( "loadout_slot_fiveseven_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Five-SeveN slot.\n 0 - Five-SeveN\n 1 - CZ-75", true, 0, true, 1 );
ConVar loadout_slot_tec9_weapon( "loadout_slot_tec9_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Tec-9 slot.\n 0 - Tec-9\n 1 - CZ-75", true, 0, true, 1 );
ConVar loadout_slot_mp7_weapon_ct( "loadout_slot_mp7_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in MP7 slot for CTs.\n 0 - MP7\n 1 - MP5SD", true, 0, true, 1 );
ConVar loadout_slot_mp7_weapon_t( "loadout_slot_mp7_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in MP7 slot for Ts.\n 0 - MP7\n 1 - MP5SD", true, 0, true, 1 );
ConVar loadout_slot_deagle_weapon_ct( "loadout_slot_deagle_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Deagle slot for CTs.\n 0 - Deagle\n 1 - R8 Revolver", true, 0, true, 1 );
ConVar loadout_slot_deagle_weapon_t( "loadout_slot_deagle_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Deagle slot for Ts.\n 0 - Deagle\n 1 - R8 Revolver", true, 0, true, 1 );
ConVar loadout_slot_agent_ct( "loadout_slot_agent_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which agent to use for CTs.", true, 0, true, MAX_AGENTS_CT );
ConVar loadout_slot_agent_t( "loadout_slot_agent_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which agent to use for Ts.", true, 0, true, MAX_AGENTS_T );
ConVar loadout_slot_gloves_ct( "loadout_slot_gloves_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which gloves to use for CTs.", true, 0, true, MAX_GLOVES );
ConVar loadout_slot_gloves_t( "loadout_slot_gloves_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which gloves to use for Ts.", true, 0, true, MAX_GLOVES );
ConVar loadout_stattrak( "loadout_stattrak", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Enable or disable StatTrak on weapons.", true, 0, true, 1 );
ConVar loadout_music( "loadout_music", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which music kit to use.", true, 0, true, MAX_MUSIC - 1 );
ConVar loadout_mainmenu_agent( "loadout_mainmenu_agent", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "What agent to use for main menu.\n 0 - None\n 1 - T\n 2 - CT", true, 0, true, 2 );
ConVar loadout_mainmenu_weapon_t( "loadout_mainmenu_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "What weapon to use for T main menu agent.", true, 0, true, MAX_MAINMENU_WEAPONS_T - 1 );
ConVar loadout_mainmenu_weapon_ct( "loadout_mainmenu_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "What weapon to use for CT main menu agent.", true, 0, true, MAX_MAINMENU_WEAPONS_CT - 1 );

// Weapon skins
ConVar loadout_skin_ak47( "loadout_skin_ak47", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for AK-47" );
ConVar loadout_skin_m4a4( "loadout_skin_m4a4", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for M4A4" );
ConVar loadout_skin_m4a1s( "loadout_skin_m4a1s", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for M4A1-S" );
ConVar loadout_skin_awp( "loadout_skin_awp", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for AWP" );
ConVar loadout_skin_deagle( "loadout_skin_deagle", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Desert Eagle" );
ConVar loadout_skin_revolver( "loadout_skin_revolver", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for R8 Revolver" );
ConVar loadout_skin_glock( "loadout_skin_glock", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Glock-18" );
ConVar loadout_skin_usp( "loadout_skin_usp", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for USP-S" );
ConVar loadout_skin_hkp2000( "loadout_skin_hkp2000", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for HKP2000" );
ConVar loadout_skin_p250( "loadout_skin_p250", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for P250" );
ConVar loadout_skin_fiveseven( "loadout_skin_fiveseven", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Five-SeveN" );
ConVar loadout_skin_tec9( "loadout_skin_tec9", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Tec-9" );
ConVar loadout_skin_cz75a( "loadout_skin_cz75a", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for CZ75-Auto" );
ConVar loadout_skin_dualberettas( "loadout_skin_dualberettas", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Dual Berettas" );
ConVar loadout_skin_p2000( "loadout_skin_p2000", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for P2000" );
ConVar loadout_skin_mag7( "loadout_skin_mag7", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for MAG-7" );
ConVar loadout_skin_nova( "loadout_skin_nova", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Nova" );
ConVar loadout_skin_sawedoff( "loadout_skin_sawedoff", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Sawed-Off" );
ConVar loadout_skin_xm1014( "loadout_skin_xm1014", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for XM1014" );
ConVar loadout_skin_m249( "loadout_skin_m249", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for M249" );
ConVar loadout_skin_negev( "loadout_skin_negev", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Negev" );
ConVar loadout_skin_mac10( "loadout_skin_mac10", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for MAC-10" );
ConVar loadout_skin_mp9( "loadout_skin_mp9", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for MP9" );
ConVar loadout_skin_mp7( "loadout_skin_mp7", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for MP7" );
ConVar loadout_skin_mp5sd( "loadout_skin_mp5sd", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for MP5-SD" );
ConVar loadout_skin_ump45( "loadout_skin_ump45", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for UMP-45" );
ConVar loadout_skin_p90( "loadout_skin_p90", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for P90" );
ConVar loadout_skin_bizon( "loadout_skin_bizon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for PP-Bizon" );
ConVar loadout_skin_galilar( "loadout_skin_galilar", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Galil AR" );
ConVar loadout_skin_famas( "loadout_skin_famas", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for FAMAS" );
ConVar loadout_skin_aug( "loadout_skin_aug", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for AUG" );
ConVar loadout_skin_sg556( "loadout_skin_sg556", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for SG 553" );
ConVar loadout_skin_ssg08( "loadout_skin_ssg08", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for SSG 08" );
ConVar loadout_skin_scar20( "loadout_skin_scar20", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for SCAR-20" );
ConVar loadout_skin_g3sg1( "loadout_skin_g3sg1", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for G3SG1" );

//Knifes
ConVar loadout_skin_knife_css( "loadout_skin_knife_css", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for CSS Knife" );
ConVar loadout_skin_knife_karambit( "loadout_skin_knife_karambit", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Karambit" );
ConVar loadout_skin_knife_flip( "loadout_skin_knife_flip", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Flip Knife" );
ConVar loadout_skin_knife_bayonet( "loadout_skin_knife_bayonet", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Bayonet" );
ConVar loadout_skin_knife_m9_bayonet( "loadout_skin_knife_m9_bayonet", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for M9 Bayonet" );
ConVar loadout_skin_knife_butterfly( "loadout_skin_knife_butterfly", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Butterfly Knife" );
ConVar loadout_skin_knife_gut( "loadout_skin_knife_gut", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Gut Knife" );
ConVar loadout_skin_knife_tactical( "loadout_skin_knife_tactical", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Huntsman Knife" );
ConVar loadout_skin_knife_falchion( "loadout_skin_knife_falchion", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Falchion Knife" );
ConVar loadout_skin_knife_survival_bowie( "loadout_skin_knife_survival_bowie", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Bowie Knife" );
ConVar loadout_skin_knife_canis( "loadout_skin_knife_canis", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Survival Knife" );
ConVar loadout_skin_knife_cord( "loadout_skin_knife_cord", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Paracord Knife" );
ConVar loadout_skin_knife_gypsy_jackknife( "loadout_skin_knife_gypsy_jackknife", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Nomad Knife" );
ConVar loadout_skin_knife_outdoor( "loadout_skin_knife_outdoor", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Skeleton Knife" );
ConVar loadout_skin_knife_skeleton( "loadout_skin_knife_skeleton", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Skeleton Knife" );
ConVar loadout_skin_knife_stiletto( "loadout_skin_knife_stiletto", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Stiletto Knife" );
ConVar loadout_skin_knife_ursus( "loadout_skin_knife_ursus", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Ursus Knife" );
ConVar loadout_skin_knife_widowmaker( "loadout_skin_knife_widowmaker", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Talon Knife" );
ConVar loadout_skin_knife_push( "loadout_skin_knife_push", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Skin ID for Classic Knife" );

//Gloves
ConVar loadout_gloves_ct_skin0("loadout_gloves_ct_skin0", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "No gloves");
ConVar loadout_gloves_ct_skin1("loadout_gloves_ct_skin1", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound");
ConVar loadout_gloves_ct_skin2("loadout_gloves_ct_skin2", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Perfect World");
ConVar loadout_gloves_ct_skin3("loadout_gloves_ct_skin3", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Broken Fang");
ConVar loadout_gloves_ct_skin4("loadout_gloves_ct_skin4", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Hydra");
ConVar loadout_gloves_ct_skin5("loadout_gloves_ct_skin5", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Fingerless");
ConVar loadout_gloves_ct_skin6("loadout_gloves_ct_skin6", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Fullfinger");
ConVar loadout_gloves_ct_skin7("loadout_gloves_ct_skin7", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Handwrap Leathery");
ConVar loadout_gloves_ct_skin8("loadout_gloves_ct_skin8", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hardknuckle");
ConVar loadout_gloves_ct_skin9("loadout_gloves_ct_skin9", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hardknuckle Black");
ConVar loadout_gloves_ct_skin10("loadout_gloves_ct_skin10", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hardknuckle Blue");
ConVar loadout_gloves_ct_skin11("loadout_gloves_ct_skin11", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Motorcycle");
ConVar loadout_gloves_ct_skin12("loadout_gloves_ct_skin12", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Slick");
ConVar loadout_gloves_ct_skin13("loadout_gloves_ct_skin13", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Specialist");
ConVar loadout_gloves_ct_skin14("loadout_gloves_ct_skin14", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sporty");
ConVar loadout_gloves_ct_skin15("loadout_gloves_ct_skin15", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "SAS Old");
ConVar loadout_gloves_ct_skin16("loadout_gloves_ct_skin16", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "FBI Old");
ConVar loadout_gloves_ct_skin17("loadout_gloves_ct_skin17", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Phoenix Old");
ConVar loadout_gloves_ct_skin18("loadout_gloves_ct_skin18", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Leet Old");
ConVar loadout_gloves_ct_skin19("loadout_gloves_ct_skin19", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bare Hands");

// T
ConVar loadout_gloves_t_skin0("loadout_gloves_t_skin0", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "No gloves");
ConVar loadout_gloves_t_skin1("loadout_gloves_t_skin1", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound");
ConVar loadout_gloves_t_skin2("loadout_gloves_t_skin2", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Perfect World");
ConVar loadout_gloves_t_skin3("loadout_gloves_t_skin3", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Broken Fang");
ConVar loadout_gloves_t_skin4("loadout_gloves_t_skin4", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bloodhound Hydra");
ConVar loadout_gloves_t_skin5("loadout_gloves_t_skin5", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Fingerless");
ConVar loadout_gloves_t_skin6("loadout_gloves_t_skin6", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Fullfinger");
ConVar loadout_gloves_t_skin7("loadout_gloves_t_skin7", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Handwrap Leathery");
ConVar loadout_gloves_t_skin8("loadout_gloves_t_skin8", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hardknuckle");
ConVar loadout_gloves_t_skin9("loadout_gloves_t_skin9", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hardknuckle Black");
ConVar loadout_gloves_t_skin10("loadout_gloves_t_skin10", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Hardknuckle Blue");
ConVar loadout_gloves_t_skin11("loadout_gloves_t_skin11", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Motorcycle");
ConVar loadout_gloves_t_skin12("loadout_gloves_t_skin12", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Slick");
ConVar loadout_gloves_t_skin13("loadout_gloves_t_skin13", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Specialist");
ConVar loadout_gloves_t_skin14("loadout_gloves_t_skin14", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sporty");
ConVar loadout_gloves_t_skin15("loadout_gloves_t_skin15", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "SAS Old");
ConVar loadout_gloves_t_skin16("loadout_gloves_t_skin16", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "FBI Old");
ConVar loadout_gloves_t_skin17("loadout_gloves_t_skin17", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Phoenix Old");
ConVar loadout_gloves_t_skin18("loadout_gloves_t_skin18", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Leet Old");
ConVar loadout_gloves_t_skin19("loadout_gloves_t_skin19", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Bare Hands");

#endif
ConVar bot_loadout_random_knives( "bot_loadout_random_knives", "0", FCVAR_REPLICATED, "Whether or not the bots will have random knives." );
ConVar bot_loadout_random_gloves( "bot_loadout_random_gloves", "0", FCVAR_REPLICATED, "Whether or not the bots will have random gloves." );
ConVar bot_loadout_random_agents( "bot_loadout_random_agents", "0", FCVAR_REPLICATED, "Whether or not the bots will have random agents." );

CCSLoadout*	g_pCSLoadout = NULL;
CCSLoadout::CCSLoadout()
{
	Assert( !g_pCSLoadout );
	g_pCSLoadout = this;
}
CCSLoadout::~CCSLoadout()
{
	Assert( g_pCSLoadout == this );
	g_pCSLoadout = NULL;
}

CLoadout WeaponLoadout[]
{
	{	"loadout_slot_m4_weapon",			"m4a4",			"m4a1_silencer",	WEAPON_M4A4,		WEAPON_M4A1		},
	{	"loadout_slot_hkp2000_weapon",		"hkp2000",		"usp_silencer",		WEAPON_HKP2000,		WEAPON_USP		},
	{	"loadout_slot_fiveseven_weapon",	"fiveseven",	"cz75a",			WEAPON_FIVESEVEN,	WEAPON_CZ75A	},
	{	"loadout_slot_tec9_weapon",			"tec9",			"cz75a",			WEAPON_TEC9,		WEAPON_CZ75A	},
	{	"loadout_slot_mp7_weapon_ct",		"mp7",			"mp5sd",			WEAPON_MP7,			WEAPON_MP5SD	},
	{	"loadout_slot_mp7_weapon_t",		"mp7",			"mp5sd",			WEAPON_MP7,			WEAPON_MP5SD	},
	{	"loadout_slot_deagle_weapon_ct",	"deagle",		"revolver",			WEAPON_DEAGLE,		WEAPON_REVOLVER	},
	{	"loadout_slot_deagle_weapon_t",		"deagle",		"revolver",			WEAPON_DEAGLE,		WEAPON_REVOLVER	},
};

struct CWeaponSkinMapping
{
	CSWeaponID		m_iWeaponID;
	const char*		m_szWeaponName;
	const char*		m_szSkinConVar;
};

struct CGloveSkinCTMapping
{
    const char* m_szSkinConVar;
};

struct CGloveSkinTMapping
{
    const char* m_szSkinConVar;
};

CGloveSkinCTMapping GloveSkinCT[MAX_GLOVES +1] =
{
	{"loadout_gloves_ct_skin0"},
	{"loadout_gloves_ct_skin1"},
	{"loadout_gloves_ct_skin2"},
	{"loadout_gloves_ct_skin3"},
	{"loadout_gloves_ct_skin4"},
	{"loadout_gloves_ct_skin5"},
	{"loadout_gloves_ct_skin6"},
	{"loadout_gloves_ct_skin7"},
	{"loadout_gloves_ct_skin8"},
	{"loadout_gloves_ct_skin9"},
	{"loadout_gloves_ct_skin10"},
	{"loadout_gloves_ct_skin11"},
	{"loadout_gloves_ct_skin12"},
	{"loadout_gloves_ct_skin13"},
	{"loadout_gloves_ct_skin14"},
	{"loadout_gloves_ct_skin15"},
	{"loadout_gloves_ct_skin16"},
	{"loadout_gloves_ct_skin17"},
	{"loadout_gloves_ct_skin18"},
	{"loadout_gloves_ct_skin19"},
};

CGloveSkinTMapping GloveSkinT[MAX_GLOVES +1] =
{
	{"loadout_gloves_t_skin0"},
	{"loadout_gloves_t_skin1"},
	{"loadout_gloves_t_skin2"},
	{"loadout_gloves_t_skin3"},
	{"loadout_gloves_t_skin4"},
	{"loadout_gloves_t_skin5"},
	{"loadout_gloves_t_skin6"},
	{"loadout_gloves_t_skin7"},
	{"loadout_gloves_t_skin8"},
	{"loadout_gloves_t_skin9"},
	{"loadout_gloves_t_skin10"},
	{"loadout_gloves_t_skin11"},
	{"loadout_gloves_t_skin12"},
	{"loadout_gloves_t_skin13"},
	{"loadout_gloves_t_skin14"},
	{"loadout_gloves_t_skin15"},
	{"loadout_gloves_t_skin16"},
	{"loadout_gloves_t_skin17"},
	{"loadout_gloves_t_skin18"},
	{"loadout_gloves_t_skin19"},
};

CWeaponSkinMapping WeaponSkinMappings[] =
{
	{ WEAPON_AK47,			"ak47",				"loadout_skin_ak47" },
	{ WEAPON_M4A4,			"m4a4",				"loadout_skin_m4a4" },
	{ WEAPON_M4A1,			"m4a1_silencer",	"loadout_skin_m4a1s" },
	{ WEAPON_AWP,			"awp",				"loadout_skin_awp" },
	{ WEAPON_DEAGLE,		"deagle",			"loadout_skin_deagle" },
	{ WEAPON_REVOLVER,		"revolver",			"loadout_skin_revolver" },
	{ WEAPON_GLOCK,			"glock",			"loadout_skin_glock" },
	{ WEAPON_USP,			"usp_silencer",		"loadout_skin_usp" },
	{ WEAPON_HKP2000,		"hkp2000",			"loadout_skin_hkp2000" },
	{ WEAPON_P250,			"p250",				"loadout_skin_p250" },
	{ WEAPON_FIVESEVEN,		"fiveseven",		"loadout_skin_fiveseven" },
	{ WEAPON_TEC9,			"tec9",				"loadout_skin_tec9" },
	{ WEAPON_CZ75A,			"cz75a",			"loadout_skin_cz75a" },
	{ WEAPON_ELITE,			"elite",			"loadout_skin_dualberettas" },
	{ WEAPON_MAG7,			"mag7",				"loadout_skin_mag7" },
	{ WEAPON_NOVA,			"nova",				"loadout_skin_nova" },
	{ WEAPON_SAWEDOFF,		"sawedoff",			"loadout_skin_sawedoff" },
	{ WEAPON_XM1014,		"xm1014",			"loadout_skin_xm1014" },
	{ WEAPON_M249,			"m249",				"loadout_skin_m249" },
	{ WEAPON_NEGEV,			"negev",			"loadout_skin_negev" },
	{ WEAPON_MAC10,			"mac10",			"loadout_skin_mac10" },
	{ WEAPON_MP9,			"mp9",				"loadout_skin_mp9" },
	{ WEAPON_MP7,			"mp7",				"loadout_skin_mp7" },
	{ WEAPON_MP5SD,			"mp5sd",			"loadout_skin_mp5sd" },
	{ WEAPON_UMP45,			"ump45",			"loadout_skin_ump45" },
	{ WEAPON_P90,			"p90",				"loadout_skin_p90" },
	{ WEAPON_BIZON,			"bizon",			"loadout_skin_bizon" },
	{ WEAPON_GALILAR,		"galilar",			"loadout_skin_galilar" },
	{ WEAPON_FAMAS,			"famas",			"loadout_skin_famas" },
	{ WEAPON_AUG,			"aug",				"loadout_skin_aug" },
	{ WEAPON_SG556,			"sg556",			"loadout_skin_sg556" },
	{ WEAPON_SSG08,			"ssg08",			"loadout_skin_ssg08" },
	{ WEAPON_SCAR20,		"scar20",			"loadout_skin_scar20" },
	{ WEAPON_G3SG1,			"g3sg1",			"loadout_skin_g3sg1" },
	{ WEAPON_KNIFE_CSS,			"knife_css", "loadout_skin_knife_css" },
	{ WEAPON_KNIFE_KARAMBIT,	"knife_karambit", "loadout_skin_knife_karambit" },
	{ WEAPON_KNIFE_FLIP,		"knife_flip", "loadout_skin_knife_flip" },
	{ WEAPON_KNIFE_BAYONET,		"knife_bayonet", "loadout_skin_knife_bayonet" },
	{ WEAPON_KNIFE_M9_BAYONET,	"knife_m9_bayonet", "loadout_skin_knife_m9_bayonet" },
	{ WEAPON_KNIFE_BUTTERFLY,	"knife_butterfly", "loadout_skin_knife_butterfly" },
	{ WEAPON_KNIFE_GUT,			"knife_gut", "loadout_skin_knife_gut" },
	{ WEAPON_KNIFE_TACTICAL,	"knife_tactical", "loadout_skin_knife_tactical" },
	{ WEAPON_KNIFE_FALCHION,	"knife_falchion", "loadout_skin_knife_falchion" },
	{ WEAPON_KNIFE_SURVIVAL_BOWIE,"knife_survival_bowie", "loadout_skin_knife_survival_bowie" },
	{ WEAPON_KNIFE_CANIS,		"knife_canis", "loadout_skin_knife_canis" },
	{ WEAPON_KNIFE_CORD,		"knife_cord", "loadout_skin_knife_cord" },
	{ WEAPON_KNIFE_GYPSY,		"knife_gypsy_jackknife", "loadout_skin_knife_gypsy_jackknife" },
	{ WEAPON_KNIFE_OUTDOOR,		"knife_outdoor", "loadout_skin_knife_outdoor" },
	{ WEAPON_KNIFE_SKELETON,	"knife_skeleton", "loadout_skin_knife_skeleton" },
	{ WEAPON_KNIFE_STILETTO,	"knife_stiletto", "loadout_skin_knife_stiletto" },
	{ WEAPON_KNIFE_URSUS,		"knife_ursus", "loadout_skin_knife_ursus" },
	{ WEAPON_KNIFE_WIDOWMAKER,	"knife_widowmaker", "loadout_skin_knife_widowmaker" },
	{ WEAPON_KNIFE_PUSH,		"knife_push", "loadout_skin_knife_push" },
};

LoadoutSlot_t CCSLoadout::GetSlotFromWeapon( int team, const char* weaponName )
{
	LoadoutSlot_t slot = SLOT_NONE;

	for ( int i = 0; i < ARRAYSIZE( WeaponLoadout ); i++ )
	{
		if ( Q_strcmp( WeaponLoadout[i].m_szFirstWeapon, weaponName ) == 0 ||
			 Q_strcmp( WeaponLoadout[i].m_szSecondWeapon, weaponName ) == 0 )
			slot = (LoadoutSlot_t)i;

		if ( slot == SLOT_MP7_CT || slot == SLOT_MP7_T )
		{
			slot = (team == TEAM_CT) ? SLOT_MP7_CT : SLOT_MP7_T;
		}
		if ( slot == SLOT_DEAGLE_CT || slot == SLOT_DEAGLE_T )
		{
			slot = (team == TEAM_CT) ? SLOT_DEAGLE_CT : SLOT_DEAGLE_T;
		}

		if ( slot != SLOT_NONE )
			break;
	}

	return slot;
}

const char* CCSLoadout::GetWeaponFromSlot( CCSPlayer* pPlayer, LoadoutSlot_t slot )
{
	if ( slot >= 0 && slot < SLOT_MAX )
	{
		if ( pPlayer->IsBotOrControllingBot() )
		{
			return WeaponLoadout[slot].m_szFirstWeapon;
		}

		int value = 0;
#ifdef CLIENT_DLL
		ConVarRef convar( WeaponLoadout[slot].m_szCommand );
		if (convar.IsValid())
			value = convar.GetInt();
#else
		value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[slot].m_szCommand ) );
#endif
		return (value > 0) ? WeaponLoadout[slot].m_szSecondWeapon : WeaponLoadout[slot].m_szFirstWeapon;
	}

	return NULL;
}

CSWeaponID CCSLoadout::GetLoadoutWeaponID( CCSPlayer* pPlayer, int iTeamNumber, CSWeaponID iWeaponID )
{
	if ( pPlayer->IsBotOrControllingBot() )
		return iWeaponID;

	LoadoutSlot_t iSlot = SLOT_NONE;

	for ( int i = 0; i < ARRAYSIZE( WeaponLoadout ); i++ )
	{
		if ( WeaponLoadout[i].m_iFirstWeaponID == iWeaponID ||
			 WeaponLoadout[i].m_iSecondWeaponID == iWeaponID )
			iSlot = (LoadoutSlot_t)i;

		if ( iSlot == SLOT_MP7_CT || iSlot == SLOT_MP7_T )
		{
			iSlot = (iTeamNumber == TEAM_CT) ? SLOT_MP7_CT : SLOT_MP7_T;
		}
		if ( iSlot == SLOT_DEAGLE_CT || iSlot == SLOT_DEAGLE_T )
		{
			iSlot = (iTeamNumber == TEAM_CT) ? SLOT_DEAGLE_CT : SLOT_DEAGLE_T;
		}

		if ( iSlot != SLOT_NONE )
			break;
	}

	if ( iSlot != SLOT_NONE )
	{
		int value = 0;
#ifdef CLIENT_DLL
		ConVarRef convar( WeaponLoadout[iSlot].m_szCommand );
		if ( convar.IsValid() )
			value = convar.GetInt();
#else
		value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[iSlot].m_szCommand ) );
#endif
		return (value > 0) ? WeaponLoadout[iSlot].m_iSecondWeaponID : WeaponLoadout[iSlot].m_iFirstWeaponID;
	}

	return iWeaponID;
}

const char* CCSLoadout::GetLoadoutWeapon( CCSPlayer* pPlayer, const char* pszWeaponName )
{
	if ( pPlayer->IsBotOrControllingBot() )
		return pszWeaponName;

	LoadoutSlot_t iSlot = GetSlotFromWeapon( pPlayer->GetTeamNumber(), pszWeaponName );

	if ( iSlot != SLOT_NONE )
	{
		int value = 0;
#ifdef CLIENT_DLL
		ConVarRef convar( WeaponLoadout[iSlot].m_szCommand );
		if ( convar.IsValid() )
			value = convar.GetInt();
#else
		value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[iSlot].m_szCommand ) );
#endif
		return (value > 0) ? WeaponLoadout[iSlot].m_szSecondWeapon : WeaponLoadout[iSlot].m_szFirstWeapon;
	}

	return pszWeaponName;
}

bool CCSLoadout::HasGlovesSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_gloves.GetBool() )
	{
		return false;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return false;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetGlovesForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_gloves.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return value;
}

bool CCSLoadout::HasKnifeSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_knives.GetBool() )
	{
		return false;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return false;
	}

	int value = 0;
    int skin = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetKnifeForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_knives.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT;
			break;
		default:
			break;
	}

	return value - 1; // arrays are started with index 0 not 1
}

int CCSLoadout::ApplyKnifeSkin( CCSPlayer* pPlayer, int team )
{
    if ( !pPlayer )
		return 0;

/*	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_skin_knives.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif*/

	int skin = 0;

	switch ( team )
	{
		case TEAM_CT:
			skin = pPlayer->m_iLoadoutSlotKnifeWeaponSkinCT;
			break;

		case TEAM_TERRORIST:
			skin = pPlayer->m_iLoadoutSlotKnifeWeaponSkinT;
			break;
	}

	return skin - 1; // arrays are started with index 0 not 1
}

bool CCSLoadout::HasAgentSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_agents.GetBool() )
	{
		return false;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return false;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotAgentCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotAgentT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetAgentForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_agents.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotAgentCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotAgentT;
			break;
		default:
			break;
	}

	return value;
}

int CCSLoadout::GetWeaponSkinForPlayerWeaponid( CCSPlayer* pPlayer, CSWeaponID weaponID )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() )
		return 0;

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	// Find the mapping for this weapon
	for ( int i = 0; i < ARRAYSIZE( WeaponSkinMappings ); i++ )
	{
		if ( WeaponSkinMappings[i].m_iWeaponID == weaponID )
		{
			int skinID = 0;
#ifdef CLIENT_DLL
			ConVarRef convar( WeaponSkinMappings[i].m_szSkinConVar );
			if ( convar.IsValid() )
				skinID = convar.GetInt();
#else
			skinID = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponSkinMappings[i].m_szSkinConVar ) );
#endif
			return skinID;
		}
	}

	return -1;    // arrays are started with index 0 not 1
}

int CCSLoadout::GetWeaponSkinForPlayer( CCSPlayer* pPlayer, const char* pszWeaponName )
{
	if ( !pPlayer || !pszWeaponName )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() )
		return 0;

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	// Remove "weapon_" prefix if present
	const char* weaponName = pszWeaponName;
	if ( !V_strncmp( pszWeaponName, "weapon_", 7 ) )
		weaponName = pszWeaponName + 7;

	// Find the mapping for this weapon
	for ( int i = 0; i < ARRAYSIZE( WeaponSkinMappings ); i++ )
	{
		if ( Q_strcmp( WeaponSkinMappings[i].m_szWeaponName, weaponName ) == 0 )
		{
			int skinID = 0;
#ifdef CLIENT_DLL
			ConVarRef convar( WeaponSkinMappings[i].m_szSkinConVar );
			if ( convar.IsValid() )
				skinID = convar.GetInt();
#else
			
#endif
			return skinID;
		}
	}

	return -1;    // arrays are started with index 0 not 1
}

int CCSLoadout::GetGlovesSkinForPlayer(CCSPlayer* pPlayer, int team)
{
	if (!pPlayer)
		return 0;

	if (pPlayer->IsBotOrControllingBot() && !bot_loadout_random_gloves.GetBool())
		return 0;

	if (pPlayer->IsControllingBot())
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer(UTIL_PlayerByIndex(pPlayer->GetControlledBotIndex()));
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if (!pPlayer)
			return 0;
	}

	int gloveIndex = GetGlovesForPlayer(pPlayer, team);
	
	if (gloveIndex < 0 || gloveIndex >= ARRAYSIZE(GloveSkinCT))
        return 0;

    const char* szConVarName = (team == TEAM_CT) ? GloveSkinCT[gloveIndex].m_szSkinConVar : GloveSkinT[gloveIndex].m_szSkinConVar;

    int skinID = 0;

#ifdef CLIENT_DLL
    ConVarRef convar(szConVarName);
    if (convar.IsValid())
        skinID = convar.GetInt();
#else
    skinID = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), szConVarName ) );
#endif

    return skinID;
}
