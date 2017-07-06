#include "GetTaskProc.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "ProcessManager.h"
#include "GameCmd.h"

REGISTER_PROCESS(CLIENT_GET_TASK, GetTaskProc)

GetTaskProc::GetTaskProc()
{
	this->name = "GetTaskProc";
}

GetTaskProc::~GetTaskProc()
{

} 

int GetTaskProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	//short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	int tid = pPacket->ReadInt();
	//short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_DEBUG_("==>[GetTaskProc]  cmd[0x%04x] uid[%d]\n", cmd, uid);

	Room* room = Room::getInstance();

	Table* table = room->getTable(realTid);

	if(table == NULL)
	{
		_LOG_ERROR_("[GetTaskProc] uid=[%d] tid=[%d] realTid=[%d] Table is NULL\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),seq);
	}

	Player* player = table->getPlayer(uid);
	if(player == NULL)
	{
		_LOG_ERROR_("[GetTaskProc] uid=[%d] tid=[%d] realTid=[%d] Your not in This Table\n",uid, tid, realTid);
		return sendErrorMsg(clientHandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),seq);
	}

	_LOG_INFO_("[GetTaskProc] UID=[%d] GameID=[%s] tid=[%d] realTid=[%d] GetTaskProc OK\n", uid, table->getGameID(), tid, realTid);
	
	OutputPacket response;
	response.Begin(cmd, player->id);
	response.SetSeqNum(seq);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	player->getTimeTask();
	response.WriteString("幸运宝箱");
	response.WriteShort(player->m_nComTimeTaskFlag);	//0表示还在倒计时 1为完成了但是没有领取 2为全部完成了 3表示还没有开始倒计时
	response.WriteShort(player->m_nLeftTimeCom);
	char szbuff[64] = {0};
	sprintf(szbuff, "奖%d金币", player->m_nRewardTimeCoin);
	response.WriteString(szbuff);
	response.WriteString("牌型任务");
	response.WriteShort((player->m_pTask != NULL) ? player->m_nGetRoll : 0);
	response.WriteString((player->m_pTask != NULL) ? player->m_pTask->taskname : "");
	short complaycount = 0;
	short level = 0;
	short playcount = player->getPlayCount(complaycount, level);
// 	if(level == 1)
// 		response.WriteString("初级场玩牌");
// 	else if(level == 2)
// 		response.WriteString("中级场玩牌");
// 	else if(level == 3)
// 		response.WriteString("高级场玩牌");
// 	else 
 	response.WriteString("");
	response.WriteByte(level);							//0为都完成了 1初级场局数任务 2为中级场局数任务 3为高级场局数任务
	response.WriteByte(playcount);
	response.WriteByte(complaycount);
	response.WriteString("随机");
	int maxcoin = 0;
	short index = 0;
	short comflag = player->getCoinTask(maxcoin, index);
	response.WriteString("富贵逼人");
	response.WriteByte(comflag);						//0为有任务没有完成 1为有任务完成没有领取  2为所有任务都领取了
	sprintf(szbuff, "金币大于%d万", maxcoin/10000);
	response.WriteString(szbuff);
	sprintf(szbuff, "奖%d乐券", player->coincfg.rewardcoin[index]);
	response.WriteString(szbuff);
	_LOG_INFO_("type:%d uid:%d m_nComTimeTaskFlag:%d m_nLeftTimeCom:%d level:%d playcount:%d complaycount:%d comflag:%d index:%d maxcoin:%d\n", 
		Configure::getInstance().m_nLevel, player->id, player->m_nComTimeTaskFlag, player->m_nLeftTimeCom, level, playcount, complaycount, comflag, index, maxcoin);
	response.End();
	player->setActiveTime(time(NULL));
	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[GetTableDetailProc] Send To Uid[%d] Error!\n", player->id);

	return 0;
}

int GetTaskProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
