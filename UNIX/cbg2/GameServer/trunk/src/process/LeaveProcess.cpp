#include "LeaveProcess.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "ProcessManager.h"
#include "GameCmd.h"

REGISTER_PROCESS(CLIENT_MSG_LEAVE, LeaveProcess)

LeaveProcess::LeaveProcess()
{
	this->name = "LeaveProcess";
}

LeaveProcess::~LeaveProcess()
{
} 

int LeaveProcess::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	//short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	int tid = pPacket->ReadInt();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_INFO_("==>[LeaveProcess]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d]\n", tid, svid, realTid);

	Room* room = Room::getInstance();

	Table* table = room->getTable(realTid);

	if(table == NULL)
	{
		_LOG_ERROR_("[LeaveProcess] uid=[%d] tid=[%d] realTid=[%d] Table is NULL\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),seq);
	}

	Player* player = table->getPlayer(uid);
	if(player == NULL)
	{
		_LOG_ERROR_("[LeaveProcess] uid=[%d] tid=[%d] realTid=[%d] Your not in This Table\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),seq);
	}
	if(player->isActive())
	{
		return sendErrorMsg(clientHandler, cmd, uid, -8,ErrorMsg::getInstance()->getErrMsg(-8),seq);
	}
	else
	{
		sendLeaveInfo(table, player, seq);
		table->playerLeave(player, true);

		if(table->isAllReady()&&table->m_nCountPlayer > 1)
		{
			table->GameStart();
		}
	}
	_LOG_INFO_("[LeaveProcess] UID=[%d] tid=[%d] realTid=[%d] Leave OK\n", uid, tid, realTid);
	player->isonline = false;
	return 0;
}

int LeaveProcess::sendLeaveInfo(Table* table, Player* leavePlayer, short seq)
{
	_LOG_DEBUG_("<==[LeaveProcess] Push [0x%04x]\n", CLIENT_MSG_LEAVE);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	int sendnum = 0;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	int i = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(table->player_array[i])
		{
			OutputPacket response;
			response.Begin(CLIENT_MSG_LEAVE,table->player_array[i]->id);
			if(leavePlayer->id == table->player_array[i]->id)
				response.SetSeqNum(seq);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(table->player_array[i]->id);
			response.WriteShort(table->player_array[i]->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteInt(leavePlayer->id);
			response.End();
			HallManager::getInstance()->sendToHall(table->player_array[i]->m_nHallid, &response, false);
			sendnum++;
		}
	}
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tm_nStatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] leavePlayer=[%d]\n", leavePlayer->id);
	return 0;
}

int LeaveProcess::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
