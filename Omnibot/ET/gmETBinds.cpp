////////////////////////////////////////////////////////////////////////////////
//
// $LastChangedBy$
// $LastChangedDate$
// $LastChangedRevision$
//
////////////////////////////////////////////////////////////////////////////////

#include "ET_Game.h"
#include "ET_InterfaceFuncs.h"
#include "ET_Client.h"

#include "gmBotLibrary.h"
#include "gmConfig.h"
#include "gmThread.h"
#include "gmMachine.h"
#include "gmBot.h"

#define CHECK_THIS_BOT() \
	Client *native = gmBot::GetThisObject( a_thread ); \
	if(!native) \
		{ \
	GM_EXCEPTION_MSG("Script Function on NULL object"); \
	return GM_EXCEPTION; \
		}

// Title: ET Script Bindings

//////////////////////////////////////////////////////////////////////////

// function: ChangePrimaryWeapon
//		Sets the bots primary weapon to a new weapon to use upon respawn
//
// Parameters:
//
//		int - weapon id to choose for primary weapon
//
// Returns:
//		int - true if success, false if error
static int GM_CDECL gmfBotPickPrimaryWeapon( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( weaponId, 0 );

	bool bSucess = InterfaceFuncs::SelectPrimaryWeapon( native, (ET_Weapon)weaponId );
	a_thread->PushInt( bSucess ? 1 : 0 );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: ChangeSecondaryWeapon
//		Sets the bots secondary weapon to a new weapon to use upon respawn
//
// Parameters:
//
//		int - weapon id to choose for secondary weapon
//
// Returns:
//		int - true if success, false if error
static int GM_CDECL gmfBotPickSecondaryWeapon( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( weaponId, 0 );

	bool bSucess = InterfaceFuncs::SelectSecondaryWeapon( native, (ET_Weapon)weaponId );
	a_thread->PushInt( bSucess ? 1 : 0 );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetReinforceTime
//		Gets the current reinforcement time for this bots team
//
// Parameters:
//
//		none
//
// Returns:
//		int - reinforce timer
static int GM_CDECL gmfGetReinforceTime( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );

	a_thread->PushFloat( InterfaceFuncs::GetReinforceTime( native ) );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetCursorHint
//		Gets the current hint and hint value for the client
//
// Parameters:
//
//		table - table to store results. function sets 'type' and 'value'
//
// Returns:
//		none
static int GM_CDECL gmfGetCurrentCursorHint( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_TABLE_PARAM( hint, 0 );

	int iHintType = 0, iHintValue = 0;
	InterfaceFuncs::GetCurrentCursorHint( native, iHintType, iHintValue );

	hint->Set( a_thread->GetMachine(), "type", gmVariable( iHintType ) );
	hint->Set( a_thread->GetMachine(), "value", gmVariable( iHintValue ) );

	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: ChangeSpawnPoint
//		Changes the bots active spawn point
//
// Parameters:
//
//		int - Spawn point to change to
//
// Returns:
//		none
static int GM_CDECL gmfChangeSpawnPoint( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( spawnpoint, 0 );

	InterfaceFuncs::ChangeSpawnPoint( native, spawnpoint );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

static int GM_CDECL gmfFireteamCreate( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );
	InterfaceFuncs::FireTeamCreate( native );
	return GM_OK;
}

static int GM_CDECL gmfFireteamDisband( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );
	InterfaceFuncs::FireTeamDisband( native );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamLeave( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );
	InterfaceFuncs::FireTeamLeave( native );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamApply( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( fireteamnum, 0 );
	InterfaceFuncs::FireTeamApply( native, fireteamnum );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamInvite( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity ent;
	GM_CHECK_GAMEENTITY_FROM_PARAM( ent, 0 );
	InterfaceFuncs::FireTeamInvite( native, ent );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamWarn( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity ent;
	GM_CHECK_GAMEENTITY_FROM_PARAM( ent, 0 );
	InterfaceFuncs::FireTeamWarn( native, ent );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamKick( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity ent;
	GM_CHECK_GAMEENTITY_FROM_PARAM( ent, 0 );
	InterfaceFuncs::FireTeamKick( native, ent );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamPropose( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity ent;
	GM_CHECK_GAMEENTITY_FROM_PARAM( ent, 0 );
	InterfaceFuncs::FireTeamPropose( native, ent );
	return GM_OK;
}

static int GM_CDECL gmfFireTeamGetInfo( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );

	ET_FireTeamInfo ft;
	if ( InterfaceFuncs::FireTeamGetInfo( native, ft ) && ft.mInFireTeam )
	{
		DisableGCInScope gcEn( a_thread->GetMachine() );

		gmMachine *pM = a_thread->GetMachine();
		gmTableObject *tbl = pM->AllocTableObject();
		gmTableObject *mbrtbl = pM->AllocTableObject();
		tbl->Set( pM, "Members", gmVariable( mbrtbl ) );

		tbl->Set( pM, "FireTeamNum", gmVariable( ft.mFireTeamNum ) );
		tbl->Set( pM, "Leader", gmVariable::EntityVar( ft.mLeader.AsInt() ) );

		int m = 0;
		for ( int i = 0; i < ET_FireTeamInfo::MaxMembers; ++i )
		{
			if ( ft.mMembers[ i ].IsValid() )
			{
				mbrtbl->Set( pM, m++, gmVariable::EntityVar( ft.mMembers[ i ].AsInt() ) );
			}
		}
		a_thread->PushTable( tbl );
	}
	else
		a_thread->PushNull();

	return GM_OK;
}

static int GM_CDECL gmfIsInFireTeam( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );

	ET_FireTeamInfo ft;
	if ( InterfaceFuncs::FireTeamGetInfo( native, ft ) && ft.mInFireTeam )
		a_thread->PushInt( ft.mInFireTeam ? 1 : 0 );
	else
		a_thread->PushInt( 0 );

	return GM_OK;
}

static int GM_CDECL gmfVoteYes( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );
	native->GameCommand( "vote yes" );
	return GM_OK;
}

static int GM_CDECL gmfVoteNo( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 0 );
	native->GameCommand( "vote no" );
	return GM_OK;
}

int gmfSayFireTeam( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	if ( a_thread->GetNumParams() > 0 )
	{
		const int bufferSize = 512;
		char buffer[ bufferSize ];

		int iMsgPos = 0;
		const int chatMsgSize = 2048;
		char chatMsg[ chatMsgSize ] = { 0 };

		// build the string
		for ( int i = 0; i < a_thread->GetNumParams(); ++i )
		{
			const char *pAsString = a_thread->Param( i ).AsString( a_thread->GetMachine(), buffer, bufferSize );
			if ( pAsString )
			{
				int len = (int)strlen( pAsString );
				if ( chatMsgSize - iMsgPos > len )
				{
					Utils::StringCopy( &chatMsg[ iMsgPos ], pAsString, len );
					iMsgPos += len;
				}
			}
		}

		gEngineFuncs->BotCommand( native->GetGameID(), va( "say_buddy \"%s\"", chatMsg ) );
		return GM_OK;
	}

	GM_EXCEPTION_MSG( "Expected 1+ parameters" );
	return GM_EXCEPTION;
}

//////////////////////////////////////////////////////////////////////////

// function: BotPush
//		Set the bots push flag.
//
//
// Parameters:
//
//		int push - 0 or 1
//
// Returns:
//		none
static int GM_CDECL gmfDisableBotPush( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( botPush, 0 );

	InterfaceFuncs::DisableBotPush( native, botPush );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetExplosiveState
//		Return the state of the explosive; unarmed, armed, invalid.
//
//
// Parameters:
//
//		GameEntity
//
// Returns:
//		explosive state
static int GM_CDECL gmfGetExplosiveState( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	
	if ( gameEnt.IsValid() )
		a_thread->PushInt( InterfaceFuncs::GetExplosiveState( native, gameEnt ) );
	else
		a_thread->PushNull();
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetConstructableState
//		Return the state of the constructible; built, not built, invalid.
//
//
// Parameters:
//
//		GameEntity
//
// Returns:
//		constructable state
static int GM_CDECL gmfGetConstructableState( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	
	if ( gameEnt.IsValid() )
		a_thread->PushInt( InterfaceFuncs::GetConstructableState( native, gameEnt ) );
	else
		a_thread->PushNull();
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetDestroyableState
//		Return the state of the destroyable; exploded, not exploded, invalid.
//
//
// Parameters:
//
//		GameEntity
//
// Returns:
//		destroyable state
static int GM_CDECL gmfGetDestroyableState( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	
	if ( gameEnt.IsValid() )
		a_thread->PushInt( InterfaceFuncs::IsDestroyable( native, gameEnt ) );
	else
		a_thread->PushNull();
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: IsWaitingForMedic
//		Returns true if entity is waiting for a medic
//
//
// Parameters:
//
//		GameEntity
//
// Returns:
//		IsWaitingForMedic
static int GM_CDECL gmfIsWaitingForMedic( gmThread *a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	
	if ( InterfaceFuncs::IsWaitingForMedic( gameEnt ) )
		a_thread->PushInt( 1 );
	else
		a_thread->PushInt( 0 );

	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetMG42Info
//		Returns currently mounted mg42 info for the bot
//
//
// Parameters:
//
//		GameEntity
//		Table
//
// Returns:
//		MG42 Info
static int gmfGetMG42Info( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );

	GM_CHECK_TABLE_PARAM( tbl, 0 );

	DisableGCInScope gcEn( a_thread->GetMachine() );

	if ( !tbl )
		tbl = a_thread->GetMachine()->AllocTableObject();

	ET_MG42Info mg42Info;
	if ( tbl != NULL && InterfaceFuncs::GetMg42Properties( native, mg42Info ) )
	{
		tbl->Set( a_thread->GetMachine(), "CenterFacing", gmVariable( mg42Info.mCenterFacing ) );
		tbl->Set( a_thread->GetMachine(), "MinHorizontal", gmVariable( mg42Info.mMinHorizontalArc ) );
		tbl->Set( a_thread->GetMachine(), "MaxHorizontal", gmVariable( mg42Info.mMaxHorizontalArc ) );
		tbl->Set( a_thread->GetMachine(), "MinVertical", gmVariable( mg42Info.mMinVerticalArc ) );
		tbl->Set( a_thread->GetMachine(), "MaxVertical", gmVariable( mg42Info.mMaxVerticalArc ) );
		a_thread->PushInt( 1 );
	}
	else
	{
		a_thread->PushNull();
	}
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetMountedPlayerOnMG42
//		Returns entity currently mounted on the given mg42 entity
//
//
// Parameters:
//
//		GameEntity
//
// Returns:
//		Entity of the owner
static int gmfGetMountedPlayerOnMG42( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	
	GameEntity owner = InterfaceFuncs::GetMountedPlayerOnMG42( native, gameEnt );
	if ( owner.IsValid() )
	{
		gmVariable v;
		v.SetEntity( owner.AsInt() );
		a_thread->Push( v );
	}
	else
	{
		a_thread->PushNull();
	}
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: IsMG42Repairable
//		Returns whether or not the MG42 is repairable
//
//
// Parameters:
//
//		GameEntity
//
// Returns:
//		1 if the Mg42 is repairable
static int gmfIsMG42Repairable( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	GM_CHECK_NUM_PARAMS( 1 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	
	a_thread->PushInt( InterfaceFuncs::IsMountableGunRepairable( native, gameEnt ) ? 1 : 0 );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: TeamLandminesAvailable
//		Returns number of landmines available to be planted
//
//
// Parameters:
//
//		None
//
// Returns:
//		Number of landmines available to be planted
static int gmfTeamLandminesAvailable( gmThread *a_thread )
{
	CHECK_THIS_BOT();
	int currentMines, maxMines;
	InterfaceFuncs::NumTeamMines( native, currentMines, maxMines );
	a_thread->PushInt( maxMines - currentMines );
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

// function: GetCabinetData
//		Returns table of data for the cabinet
//
//
// Parameters:
//
//		GameEntity
//		Table
//
// Returns:
//		Table of Cabinet Data
static int gmfGetCabinetData( gmThread *a_thread )
{
	GM_CHECK_NUM_PARAMS( 2 );
	GameEntity gameEnt;
	GM_CHECK_GAMEENTITY_FROM_PARAM( gameEnt, 0 );
	GM_CHECK_TABLE_PARAM( tbl, 1 );

	DisableGCInScope gcEn( a_thread->GetMachine() );

	if ( !tbl )
		tbl = a_thread->GetMachine()->AllocTableObject();

	ET_CabinetData cabinetData;
	if ( tbl != NULL && InterfaceFuncs::GetCabinetData( gameEnt, cabinetData ) )
	{
		tbl->Set( a_thread->GetMachine(), "CurrentAmount", gmVariable( cabinetData.mCurrentAmount ) );
		tbl->Set( a_thread->GetMachine(), "MaxAmount", gmVariable( cabinetData.mMaxAmount ) );
		tbl->Set( a_thread->GetMachine(), "Rate", gmVariable( cabinetData.mRate ) );
		a_thread->PushInt( 1 );
	}
	else
	{
		a_thread->PushNull();
	}
	return GM_OK;
}

//////////////////////////////////////////////////////////////////////////

static gmFunctionEntry s_ExtendedBotTypeLib [] =
{
	{ "ChangePrimaryWeapon", gmfBotPickPrimaryWeapon, NULL },
	{ "ChangeSecondaryWeapon", gmfBotPickSecondaryWeapon, NULL },
	{ "GetReinforceTime", gmfGetReinforceTime, NULL },
	{ "GetCursorHint", gmfGetCurrentCursorHint, NULL },
	{ "ChangeSpawnPoint", gmfChangeSpawnPoint, NULL },

	{ "IsInFireTeam", gmfIsInFireTeam, NULL },
	{ "FireteamCreate", gmfFireteamCreate, NULL },
	{ "FireteamDisband", gmfFireteamDisband, NULL },
	{ "FireTeamLeave", gmfFireTeamLeave, NULL },
	{ "FireTeamInvite", gmfFireTeamInvite, NULL },
	{ "FireTeamApply", gmfFireTeamApply, NULL },
	{ "FireTeamWarn", gmfFireTeamWarn, NULL },
	{ "FireTeamKick", gmfFireTeamKick, NULL },
	{ "FireTeamPropose", gmfFireTeamPropose, NULL },

	{ "FireTeamGetInfo", gmfFireTeamGetInfo, NULL },

	{ "VoteYes", gmfVoteYes, NULL },
	{ "VoteNo", gmfVoteNo, NULL },

	{ "SayFireTeam", gmfSayFireTeam, NULL },

	{ "DisableBotPush", gmfDisableBotPush, NULL },

	{ "GetExplosiveState", gmfGetExplosiveState, NULL },
	{ "GetConstructableState", gmfGetConstructableState, NULL },
	{ "GetDestroyableState", gmfGetDestroyableState, NULL },

	// TODO: add owner to MG42Info table when breaking mod compat doesn't matter?
	{ "GetMG42Info", gmfGetMG42Info, NULL },
	{ "GetMountedPlayerOnMG42", gmfGetMountedPlayerOnMG42, NULL },

	{ "IsMG42Repairable", gmfIsMG42Repairable, NULL },
	{ "TeamLandminesAvailable", gmfTeamLandminesAvailable, NULL },
};

static gmFunctionEntry s_ExtendedBotLib [] =
{
	{ "IsWaitingForMedic", gmfIsWaitingForMedic, NULL },
	{ "GetCabinetData", gmfGetCabinetData, NULL },
};

void ET_Game::InitScriptBinds( gmMachine *_machine )
{
	// Register the bot functions.
	_machine->RegisterLibrary( s_ExtendedBotLib, sizeof( s_ExtendedBotLib ) / sizeof( s_ExtendedBotLib[ 0 ] ) );
	_machine->RegisterTypeLibrary( gmBot::GetType(), s_ExtendedBotTypeLib, sizeof( s_ExtendedBotTypeLib ) / sizeof( s_ExtendedBotTypeLib[ 0 ] ) );

	// Register additional bot properties

	// var: TargetBreakableDistance
	//		The distance the bot will target breakable entities. Targets beyond this range will be ignored.
	gmBot::RegisterAutoProperty( "TargetBreakableDist", GM_FLOAT, offsetof( ET_Client, mBreakableTargetDistance ), 0 );
}