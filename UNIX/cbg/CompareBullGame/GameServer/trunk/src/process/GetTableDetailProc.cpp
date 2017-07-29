#include "json/json.h"
#include "GetTableDetailProc.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "AllocSvrConnect.h"
#include "ProcessManager.h"
#include "GameCmd.h"
#include "BaseClientHandler.h"

REGISTER_PROCESS(CLIENT_MSG_TABLEDET, GetTableDetailProc)

GetTableDetailProc::GetTableDetailProc()
{
	this->name = "GetTableDetailProc";
}

GetTableDetailProc::~GetTableDetailProc()
{

} 

int GetTableDetailProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	//short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	int tid = pPacket->ReadInt();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_INFO_("==>[GetTableDetailProc]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d]\n", tid, svid, realTid);

	BaseClientHandler* hallhandler = reinterpret_cast<BaseClientHandler*> (clientHandler);

	Room* room = Room::getInstance();

	Table* table = room->getTable(realTid);

	if(table == NULL)
	{
		_LOG_ERROR_("[GetTableDetailProc] uid=[%d] tid=[%d] Table is NULL\n",uid, tid);
		AllocSvrConnect::getInstance()->userLeaveGame(NULL, uid);
		return sendErrorMsg(hallhandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),seq);
	}

	Player* player = table->getPlayer(uid);
	if(player == NULL)
	{
		_LOG_ERROR_("[GetTableDetailProc] uid=[%d] tid=[%d] Your not in This Table\n",uid, tid);
		AllocSvrConnect::getInstance()->userLeaveGame(NULL, uid);
		return sendErrorMsg(hallhandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),seq);
	}

	if(player->isActive())
	{
		_LOG_INFO_("player[%d] is active\n", player->id);
	}
	player->m_nHallid = hallhandler->hallid;
	player->isonline = true;

	_LOG_INFO_("[GetTableDetailProc] UID=[%d] tid=[%d] realTid=[%d] GetTableDetailProc OK\n", uid, tid, realTid);

	OutputPacket response;
	response.Begin(cmd, player->id);
	response.SetSeqNum(seq);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteInt(table->m_nBanker);
	response.WriteByte(Configure::getInstance().ready_time);
	short lefttime = 0;
// 	if(table->isCall())
// 	{
// 		lefttime = Configure::getInstance().call_time - (time(NULL) - table->timeout_time);		
// 	}
// 	if(table->isMul())
// 	{
// 		lefttime = Configure::getInstance().setmul_time - (time(NULL) - table->timeout_time);
// 	}
// 
// 	if(table->isOpen())
// 	{
// 		lefttime = Configure::getInstance().open_time - (time(NULL) - table->timeout_time);
// 	}
// 
// 	response.WriteByte(lefttime);
	response.WriteByte(player->m_byHandCardData[0]);
	response.WriteByte(player->m_byHandCardData[1]);
	response.WriteByte(player->m_byHandCardData[2]);
	response.WriteByte(player->m_byHandCardData[3]);
	response.WriteByte(player->m_byHandCardData[4]);

	response.WriteShort(player->m_nMulArray[0]);
	response.WriteShort(player->m_nMulArray[1]);
	response.WriteShort(player->m_nMulArray[2]);
	response.WriteShort(player->m_nMulArray[3]);
	response.WriteByte(table->m_nCountPlayer);
	for(int j = 0; j < GAME_PLAYER; ++j)
	{
		if(table->player_array[j])
		{
			lefttime = Configure::getInstance().ready_time - (time(NULL) - table->player_array[j]->getEnterTime());
			response.WriteInt(table->player_array[j]->id);
			response.WriteShort(table->player_array[j]->m_nStatus);
			response.WriteByte(table->player_array[j]->m_nTabIndex);
			response.WriteString(table->player_array[j]->name);
			response.WriteInt64(table->player_array[j]->m_lMoney);
			response.WriteString(table->player_array[j]->json);
			response.WriteByte(table->player_array[j]->m_bisCall);
			response.WriteShort(table->player_array[j]->m_nMul);
			response.WriteByte(lefttime > 0 ? lefttime : 0);
			//_LOG_DEBUG_("uid[%d] m_bhasOpen[%s]\n", table->player_array[j]->id, table->player_array[j]->m_bhasOpen ? "true" :"false");
			if(table->player_array[j]->m_bhasOpen)
			{
				response.WriteByte(1);
				response.WriteByte(table->player_array[j]->m_bCardType);
				response.WriteByte(table->player_array[j]->m_byHandCardData[0]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[1]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[2]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[3]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[4]);
				
			}
			else
				response.WriteByte(0);
		}
	}
	response.WriteShort((player->m_pTask != NULL) ? player->m_nGetRoll : 0);
	response.WriteString((player->m_pTask != NULL) ? player->m_pTask->taskname : "");
	response.WriteInt(player->m_nRoll);
	int comflag = 0;
	int roundnum = 0;
	if(Configure::getInstance().m_nLevel == LEVEL_1)
	{
		comflag = player->m_nRoundComFlag & 0x0F;
		roundnum = player->m_nRoundNum & 0x00FF;
	}
	if(Configure::getInstance().m_nLevel == LEVEL_2)
	{
		comflag = (player->m_nRoundComFlag >> 4) & 0x0F;
		roundnum = (player->m_nRoundNum >> 8) & 0x00FF;
	}
	if(Configure::getInstance().m_nLevel == LEVEL_3)
	{
		comflag = (player->m_nRoundComFlag >> 8) & 0x0F;
		roundnum = (player->m_nRoundNum >> 16) & 0x00FF;
	}

	_LOG_DEBUG_("comflag:%d roundnum:%d\n", comflag, roundnum);

	if(table->m_nRoundNum3 != 0)
	{
		_LOG_DEBUG_("m_nRoundNum3:%d m_nRoundNum2:%d m_nRoundNum1:%d\n", table->m_nRoundNum3, table->m_nRoundNum2, table->m_nRoundNum1);
		//都完成了
		if(comflag & 4)
		{
			response.WriteByte(0);
			response.WriteByte(0);
		}
		//完成中间的
		else if (comflag & 2)
		{
			response.WriteByte(roundnum - (table->m_nRoundNum1 + table->m_nRoundNum2));
			response.WriteByte(table->m_nRoundNum3);
		}
		//完成第一个局数
		else if (comflag & 1)
		{
			response.WriteByte(roundnum - table->m_nRoundNum1);
			response.WriteByte(table->m_nRoundNum2);
		}
		//所有的都没有完成
		else
		{
			response.WriteByte(roundnum);
			response.WriteByte(table->m_nRoundNum1);
		}
	}
	else if(table->m_nRoundNum2 != 0)
	{
		//完成中间的
		if (comflag & 2)
		{
			response.WriteByte(0);
			response.WriteByte(0);
		}
		//完成第一个局数
		else if (comflag & 1)
		{
			response.WriteByte(roundnum - table->m_nRoundNum1);
			response.WriteByte(table->m_nRoundNum2);
		}
		//所有的都没有完成
		else
		{
			response.WriteByte(roundnum);
			response.WriteByte(table->m_nRoundNum1);
		}
	}
	else if (table->m_nRoundNum1 != 0)
	{
		//完成第一个局数
		if (comflag & 1)
		{
			response.WriteByte(0);
			response.WriteByte(0);
		}
		//所有的都没有完成
		else
		{
			response.WriteByte(roundnum);
			response.WriteByte(table->m_nRoundNum1);
		}
	}
	else
	{
		response.WriteByte(0);
		response.WriteByte(0);
	}
	response.WriteByte(table->m_bRewardType);
	if(table->m_bRewardType == 2)
	{
		response.WriteInt(player->m_nComGetRoll);
		response.WriteInt(Configure::getInstance().rewardroll);
	}
	else
	{
		response.WriteInt(player->m_nComGetCoin);
		response.WriteInt(Configure::getInstance().rewardcoin);
	}
	response.WriteInt(table->m_nAnte);
	response.End();

	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[GetTableDetailProc] Send To Uid[%d] Error!\n", player->id);
	
	return 0;
}

int GetTableDetailProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
