#include "UpdateMoneyProc.h"
#include "ProtocolServerId.h"
#include "Room.h"
#include "Logger.h"

int UpdateMoneyProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}

int UpdateMoneyProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )
{
	short cmd = inputPacket->GetCmdType();

	_LOG_DEBUG_("==>[UpdateMoneyProc] [0x%04x] \n", cmd);
	
	int uid = inputPacket->ReadInt();
	int64_t money = inputPacket->ReadInt64();
	int64_t safemoney = inputPacket->ReadInt64();
	int roll = inputPacket->ReadInt();
	int roll1 = inputPacket->ReadInt();
	int coin = inputPacket->ReadInt();
	int exp = inputPacket->ReadInt();

	_LOG_DEBUG_("[DATA Parse] uid=[%d]\n", uid);
	_LOG_DEBUG_("[DATA Parse] money=[%ld]\n", money);
	_LOG_DEBUG_("[DATA Parse] safemoney=[%ld]\n", safemoney);
	_LOG_DEBUG_("[DATA Parse] roll=[%d]\n", roll);
	_LOG_DEBUG_("[DATA Parse] roll1=[%d]\n", roll1);
	_LOG_DEBUG_("[DATA Parse] coin=[%d]\n", coin);
	_LOG_DEBUG_("[DATA Parse] exp=[%d]\n", exp);
	
	Room* room = Room::getInstance();

	for(int i=0; i< room->getMaxUsers() ;++i)
	{
		Player * player = &room->players[i];
		if(player && player->id == uid)
		{
			player->m_lMoney += money;
//			player->m_lSafeMoney += safemoney;
//			player->m_nRoll = roll;
//			player->m_nExp = exp;
			_LOG_INFO_("Found uid=[%d] update money[%d]\n", uid, money);
			break;
		}
	}	
	return 0;	
}
