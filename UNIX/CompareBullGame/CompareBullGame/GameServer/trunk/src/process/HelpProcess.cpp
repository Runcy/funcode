#include "HelpProcess.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "ProcessManager.h"
#include "GameCmd.h"

REGISTER_PROCESS(CLIENT_MSG_HELP, HelpProcess)

HelpProcess::HelpProcess()
{
	this->name = "HelpProcess";
}

HelpProcess::~HelpProcess()
{

} 

int HelpProcess::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	//short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	int tid = pPacket->ReadInt();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_INFO_("==>[HelpProcess]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d]\n", tid, svid, realTid);

	Room* room = Room::getInstance();

	Table* table = room->getTable(realTid);

	if(table == NULL)
	{
		_LOG_ERROR_("[HelpProcess] uid=[%d] tid=[%d] realTid=[%d] Table is NULL\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),seq);
	}

	Player* player = table->getPlayer(uid);
	if(player == NULL)
	{
		_LOG_ERROR_("[HelpProcess] uid=[%d] tid=[%d] realTid=[%d] Your not in This Table\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),seq);
	}

	if(!table->isOpen() || !player->isActive())
	{
		_LOG_ERROR_("[HelpProcess] uid=[%d] ustatus=[%d] tid=[%d] realTid=[%d] Table status[%d] not Open\n",uid, player->m_nStatus, tid, realTid, table->m_nStatus);
		return sendErrorMsg(clientHandler, cmd, uid, -11,ErrorMsg::getInstance()->getErrMsg(-11),seq);
	}
	player->m_nUseHelpCoin = 0;

	player->m_bHasUserHelp = true;
	player->setActiveTime(time(NULL));

	BYTE index1,index2;
	index1 = index2 = 0;
	BYTE bCardType = table->m_GameLogic.GetCardType(player->m_byHandCardData, CARD_MAX, index1, index2);

	BYTE bullcard[3];
	BYTE nobullcard[2];
	if(bCardType > 0 && bCardType < 20)
	{
		int j = 0;
		for(int i = 0; i < 5; i++)
		{
			if(i != index1 && i != index2)
				bullcard[j++] = player->m_byHandCardData[i];
		}
		nobullcard[0] = player->m_byHandCardData[index1];
		nobullcard[1] = player->m_byHandCardData[index2];
	}

	OutputPacket response;
	response.Begin(cmd, player->id);
	response.SetSeqNum(seq);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteByte(bCardType);
	if(bCardType > 0 && bCardType < 20)
	{
		response.WriteByte(bullcard[0]);
		response.WriteByte(bullcard[1]);
		response.WriteByte(bullcard[2]);
		response.WriteByte(nobullcard[0]);
		response.WriteByte(nobullcard[1]);
	}
	else
	{
		response.WriteByte(player->m_byHandCardData[0]);
		response.WriteByte(player->m_byHandCardData[1]);
		response.WriteByte(player->m_byHandCardData[2]);
		response.WriteByte(player->m_byHandCardData[3]);
		response.WriteByte(player->m_byHandCardData[4]);
	}
	response.WriteInt(0); //剩余次数
	response.WriteInt(table->m_nHelpCoin);
	response.WriteInt64(player->m_lMoney);
	response.End();

	_LOG_INFO_("[HelpProcess] UID=[%d] GameID=[%s] tid=[%d] realTid=[%d] bCardType=[%d]HelpProcess OK\n", uid, table->getGameID(), tid, realTid, bCardType);

	_LOG_DEBUG_("[Data Response] ret:%d bCardType:%d\n", 0, bCardType);
	_LOG_DEBUG_("[Data Response] nobullcard:0x%02x 0x%02x\n", nobullcard[0], nobullcard[1]);
	_LOG_DEBUG_("[Data Response] bullcard:0x%02x 0x%02x 0x%02x\n", bullcard[0], bullcard[1], bullcard[2]);
	_LOG_DEBUG_("[Data Response] m_nHelpCoin:%d\n", table->m_nHelpCoin);
	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[HelpProcess] Send To Uid[%d] Error!\n", player->id);

	return 0;
}

int HelpProcess::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
