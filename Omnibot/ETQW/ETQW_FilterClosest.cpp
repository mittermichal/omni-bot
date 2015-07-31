////////////////////////////////////////////////////////////////////////////////
//
// $LastChangedBy$
// $LastChangedDate$
// $LastChangedRevision$
//
////////////////////////////////////////////////////////////////////////////////

#include "ETQW_FilterClosest.h"
#include "ETQW_InterfaceFuncs.h"
#include "ETQW_Client.h"

ETQW_FilterClosest::ETQW_FilterClosest( Client *_client, AiState::SensoryMemory::Type _type )
	: FilterClosest( _client, _type )
{
}

bool ETQW_FilterClosest::CheckEx( const MemoryRecord &_record )
{
	// Special consideration for some entity types.
	switch ( _record.mTargetInfo.mEntInfo.mClassId )
	{
		case ETQW_CLASSEX_VEHICLE_HVY:
		{
			break;
		}
		case ETQW_CLASSEX_MG42MOUNT:
		{
			GameEntity mounted = InterfaceFuncs::GetMountedPlayerOnMG42( mClient, _record.GetEntity() );
			if ( !mounted.IsValid() || mClient->IsAllied( mounted ) )
				return false;
			break;
		}
		case ETQW_CLASSEX_BREAKABLE:
		{
			float fBreakableDist = static_cast<ETQW_Client*>( mClient )->GetBreakableTargetDist();
			float fDistance = ( mClient->GetPosition() - _record.GetLastSensedPosition() ).SquaredLength();
			if ( fDistance > ( fBreakableDist * fBreakableDist ) )
				return false;
			break;
		}
	}

	// TODO: Handle disguised covertops
	if ( _record.mTargetInfo.mEntInfo.mFlags.CheckFlag( ETQW_ENT_FLAG_DISGUISED ) && Mathf::UnitRandom() > 0.33f )
		return false;

	return true;
}