#include "StartGameProc.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "Util.h"
#include "AllocSvrConnect.h"
#include "ProcessManager.h"
#include "GameCmd.h"
#include "GameUtil.h"
#include "GameApp.h"
#include "ProtocolServerId.h"

REGISTER_PROCESS(CLIENT_MSG_START_GAME, StartGameProc)

StartGameProc::StartGameProc()
{
	this->name = "StartGameProc";
}

StartGameProc::~StartGameProc()
{

} 

int StartGameProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	//short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	int tid = pPacket->ReadInt();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_DEBUG_("==>[StartGameProc]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d]\n", tid, svid, realTid);

	Room* room = Room::getInstance();

	Table* table = room->getTable(realTid);

	if(table == NULL)
	{
		_LOG_ERROR_("[StartGameProc] uid=[%d] tid=[%d] realTid=[%d] Table is NULL\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),seq);
	}

	Player* player = table->getPlayer(uid);
	if(player == NULL)
	{
		_LOG_ERROR_("[StartGameProc] uid=[%d] tid=[%d] realTid=[%d] Your not in This Table\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),seq);
	}

	if(player->isReady() || player->isActive())
	{
		_LOG_ERROR_("[StartGameProc] uid=[%d] ustatus=[%d] tid=[%d] realTid=[%d] Table status[%d]\n",uid, player->m_nStatus, tid, realTid, table->m_nStatus);
		return sendErrorMsg(clientHandler, cmd, uid, -8,ErrorMsg::getInstance()->getErrMsg(-8),seq);
	}

	short result = 0;
	if(!player->checkMoney(result))
	{
		_LOG_ERROR_("UID[%d] not enough money m_lMoney[%d]\n", uid, player->m_lMoney);
		if(result == 0)
		{
			//if(player->m_nTaskFlag == 1)
			//{
			sendErrorMsg(clientHandler, cmd, uid, -4, ErrorMsg::getInstance()->getErrMsg(-4), seq);
			//}
			//else
			//	sendErrorMsg(hallhandler, cmd, uid, -4,ErrorMsg::getInstance()->getErrMsg(-4),seq);
			//返回给客户端描述信息不够钱了
			//sendErrorMsg(hallhandler, cmd, uid, -1, ErrorMsg::getInstance()->getErrMsg(2), seq);
		}
		else
			sendErrorMsg(clientHandler, cmd, uid, -29,ErrorMsg::getInstance()->getErrMsg(-29),seq);
		return 0;
	}

	_LOG_INFO_("[StartGameProc] UID=[%d] tid=[%d] realTid=[%d] StartGame OK\n", uid, tid, realTid);

	player->m_nStatus = STATUS_PLAYER_RSTART;
	player->setActiveTime(time(NULL));
	player->setRePlayeTime(time(NULL));
	table->stopKickTimer(player);

	int i= 0;
	int sendnum = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(sendnum == table->m_nCountPlayer)
			break;
		if(table->player_array[i])
		{
			sendTabePlayersInfo(table->player_array[i], table, table->m_nCountPlayer, player, seq);
			sendnum++;
		}
	}

	if (table->m_nCountPlayer < GAME_PLAYER)
	{
		std::string		strTrumpt;

		if (GameUtil::getInvitePlayerTrumpt(strTrumpt, GAME_ID, Configure::getInstance().m_nLevel, realTid, table->m_nCountPlayer, GAME_PLAYER))
			AllocSvrConnect::getInstance()->trumptToUser(PRODUCT_TRUMPET, strTrumpt.c_str(), player->pid);
	}

	if(table->isAllReady() && table->m_nCountPlayer >= 2)
	{
		table->GameStart();
		return 0;
	}
	
	if(table->isActive())
		return 0;

	return 0;
}

int StartGameProc::sendTabePlayersInfo(Player* player, Table* table, short num, Player* starter, short seq)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	OutputPacket response;
	response.Begin(CLIENT_MSG_START_GAME, player->id);
	if(player->id == starter->id)
		response.SetSeqNum(seq);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteInt(starter->id);
	response.WriteShort(starter->m_nStatus);
	response.End();
	_LOG_DEBUG_("<==[StartGameProc] Push [0x%04x]\n", CLIENT_MSG_START_GAME);
	_LOG_DEBUG_("[Data Response] push to uid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] push to m_nStatus=[%d]\n", player->m_nStatus);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] startuid=[%d]\n", starter->id);
	_LOG_DEBUG_("[Data Response] startstatus=[%d]\n", starter->m_nStatus);
	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[StartGameProc] Send To Uid[%d] Error!\n", player->id);
	else
		_LOG_DEBUG_("[StartGameProc] Send To Uid[%d] Success\n", player->id);
	return 0;
}

int StartGameProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
