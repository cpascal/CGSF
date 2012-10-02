#include "StdAfx.h"
#include "SFFreeForAll.h"
#include "SFRoom.h"
#include "SFRoomPlay.h"
#include "GamePacketStructure.h"
#include "SFPlayer.h"
#include "SFPacket.h"
#include "SFSendPacket.h"

SFFreeForAll::SFFreeForAll(int Mode)
: SFGameMode(Mode)
{

}

SFFreeForAll::~SFFreeForAll(void)
{
}

BOOL SFFreeForAll::OnEnter( int GameMode )
{
	return TRUE;
}

BOOL SFFreeForAll::Onleave()
{
	return TRUE;
}

BOOL SFFreeForAll::Update( DWORD dwTickcount )
{
	return TRUE;
}

BOOL SFFreeForAll::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	return TRUE;
}

BOOL SFFreeForAll::ProcessUserRequest( SFPlayer* pPlayer, SFPacket* pPacket )
{
	SFRoom* pRoom = GetOwner()->GetOwner();

	switch(pPacket->GetPacketID())
	{
	//case CGSF::MSG_PLAYER_MOVE_UPDATE:
	//case CGSF::MSG_PLAYER_LOOK_UPDATE:
	case CGSF::MSG_PLAYER_SCORE:
	case CGSF::MSG_PLAYER_WEAPON_CHANGE:
	case CGSF::MSG_PLAYER_WEAPON_CHANGING:
		{
			pRoom->BroadCast(pPacket);

		}
		break;
	case CGSF::MSG_SPAWN_PLAYER:
		{
			OnSpawnPlayer(pPlayer, pPacket);
		}
		break;
	case CGSF::MSG_PLAYER_HEALTH:
		{
			OnPlayerHealth(pPlayer, pPacket);
		}
	}

	return TRUE;
}

BOOL SFFreeForAll::OnPlayerHealth(SFPlayer* pPlayer, SFPacket* pPacket)
{
	SFRoom* pRoom = GetOwner()->GetOwner();

	SFPacketStore::MSG_PLAYER_HEALTH PktPlayerHealth;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktPlayerHealth.ParseFromZeroCopyStream(&is);

	PlayerHealthMsg msg;

	SF_GETPACKET_ARG(&msg, PktPlayerHealth.playerhealth(), PlayerHealthMsg);

	SFPlayer* pHurtPlayer = NULL;
	SFRoom::RoomMemberMap MemberMap = pRoom->GetRoomMemberMap();
	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	for(; iter != MemberMap.end();iter++)
	{
		if(iter->second->GetSerial() == msg.PlayerID)
		{
			pHurtPlayer = iter->second;
			break;
		}
	}

	if(pHurtPlayer == NULL)
		return FALSE;

	_CharacterInfo* pInfo = pHurtPlayer->GetCharacterInfo();
	
	pInfo->health = msg.health;

	if(pInfo->health == 0)
	{
		pInfo->IsAlive = false;
		pInfo->DeathTime = GetTickCount();
	}
	
	iter = MemberMap.begin();

	for(; iter != MemberMap.end();iter++)
	{
		SFPlayer* pTarget = iter->second;
		SendPlayerHealth(pTarget, pHurtPlayer);
	}

	return TRUE;
}

BOOL SFFreeForAll::OnSpawnPlayer(SFPlayer* pPlayer, SFPacket* pPacket)
{
	SFRoom* pRoom = GetOwner()->GetOwner();

	SFPacketStore::MSG_SPAWN_PLAYER PkSpawnPlayer;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PkSpawnPlayer.ParseFromZeroCopyStream(&is);

	SpawnPlayerMsg msg;

	SF_GETPACKET_ARG(&msg, PkSpawnPlayer.spawnplayer(), SpawnPlayerMsg);

	_CharacterInfo* pInfo = pPlayer->GetCharacterInfo();
	pInfo->translation = msg.translation;
	pInfo->IsAlive = true;

	SFRoom::RoomMemberMap MemberMap = pRoom->GetRoomMemberMap();
	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	for(; iter != MemberMap.end();iter++)
	{
		SFPlayer* pTarget = iter->second;
		SendSpawnPlayer(pTarget, pPlayer);
	}

	return TRUE;
}