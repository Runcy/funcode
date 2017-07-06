#include "OpenCardProc.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "ProcessManager.h"
#include "GameCmd.h"
#include "IProcess.h"

REGISTER_PROCESS(CLIENT_MSG_OPEN_CARD, OpenCardProc)

OpenCardProc::OpenCardProc()
{
	this->name = "OpenCardProc";
}

OpenCardProc::~OpenCardProc()
{

} 

int OpenCardProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	//short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	int tid = pPacket->ReadInt();
	BYTE hasbull = pPacket->ReadByte();
	BYTE card1 = pPacket->ReadByte();
	BYTE card2 = pPacket->ReadByte();
	BYTE card3 = pPacket->ReadByte();
	BYTE card4 = pPacket->ReadByte();
	BYTE card5 = pPacket->ReadByte();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_DEBUG_("==>[OpenCardProc]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d] hasbull=[%d]\n", tid, svid, realTid, hasbull);
	_LOG_DEBUG_("[DATA Parse] card1=[0x%02x] card2=[0x%02x] card3=[0x%02x] card4=[0x%02x] card5=[0x%02x]\n", 
		card1, card2, card3, card4, card5);

	Room* room = Room::getInstance();

	Table* table = room->getTable(realTid);

	if(table == NULL)
	{
		_LOG_ERROR_("[OpenCardProc] uid=[%d] tid=[%d] realTid=[%d] Table is NULL\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),seq);
	}

	Player* player = table->getPlayer(uid);
	if(player == NULL)
	{
		_LOG_ERROR_("[OpenCardProc] uid=[%d] tid=[%d] realTid=[%d] Your not in This Table\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),seq);
	}
	if(!player->isActive())
	{
		_LOG_ERROR_("[CallBankerProc] uid=[%d] ustatus=[%d] tid=[%d] realTid=[%d] Table status[%d] not Open\n",uid, player->m_nStatus, tid, realTid, table->m_nStatus);
		return sendErrorMsg(clientHandler, cmd, uid, -11,ErrorMsg::getInstance()->getErrMsg(-11),seq);
	}
	//有牛
	/*if(hasbull == 1)
	{
		int num = table->m_GameLogic.GetCardLogicValue(card1) + table->m_GameLogic.GetCardLogicValue(card2) + table->m_GameLogic.GetCardLogicValue(card3);
		if(num % 10 != 0)
			return sendErrorMsg(clientHandler, cmd, uid, -8,ErrorMsg::getInstance()->getErrMsg(-8),seq); 
	}*/

	_LOG_INFO_("[OpenCardProc] UID=[%d] GameID=[%s] tid=[%d] realTid=[%d] OpenCardProc OK\n", uid, table->getGameID(), tid, realTid);

	player->m_bhasOpen = true;

	BYTE index1, index2;
	index1 = index2 = 0;
	BYTE bCardType = table->m_GameLogic.GetCardType(player->m_byHandCardData, CARD_MAX, index1, index2);

	BYTE bullcard[3];
	BYTE nobullcard[2];
	if (bCardType > 0 && bCardType < 20)
	{
		int j = 0;
		for (int i = 0; i < 5; i++)
		{
			if (i != index1 && i != index2)
				bullcard[j++] = player->m_byHandCardData[i];
		}
		nobullcard[0] = player->m_byHandCardData[index1];
		nobullcard[1] = player->m_byHandCardData[index2];
	}

	player->m_bCardType = bCardType;
// 	if(hasbull >= 1)
// 	{
// 		player->m_bCardType = table->m_GameLogic.GetCardType(player->m_byHandCardData, 5);
// 	}

	if (player->m_bCardType > 0 && bCardType < 20)
	{
		IProcess::sendOpenCardInfo(table, player, player->m_bCardType, bullcard[0], bullcard[1], bullcard[2], nobullcard[0], nobullcard[1]);
	}
	else
		IProcess::sendOpenCardInfo(table, player, player->m_bCardType, player->m_byHandCardData[0], player->m_byHandCardData[1],
			player->m_byHandCardData[2], player->m_byHandCardData[3], player->m_byHandCardData[4]);

// 	if(hasbull >= 1)
// 		IProcess::sendOpenCardInfo(table, player, hasbull, bullcard[0], bullcard[1], bullcard[2], nobullcard[0], nobullcard[1], seq);
// 	else
// 		IProcess::sendOpenCardInfo(table, player, hasbull, player->m_byHandCardData[0], player->m_byHandCardData[1],
// 		player->m_byHandCardData[2], player->m_byHandCardData[3], player->m_byHandCardData[4], seq);

	player->setActiveTime(time(NULL));

	if(table->allPlayerOpen())
	{
		table->GameOver();
	}

	return 0;
}

int OpenCardProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
