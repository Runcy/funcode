#include "IProcess.h"
#include "Configure.h"
#include "BaseClientHandler.h"
#include "HallManager.h"
#include "Logger.h"
#include "ErrorMsg.h"
#include "AllocSvrConnect.h"
#include "CoinConf.h"
#include "GameUtil.h"
#include "ProtocolServerId.h"
#include "ProtocolServerId.h"
#include "GameApp.h"

int IProcess::serverPushLeaveInfo(Table* table, Player* leavePlayer,short retno)
{
	if(table == NULL || leavePlayer == NULL)
		return -1;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	
	_LOG_DEBUG_("<==[serverPushLeaveInfo] Push [0x%04x]\n", SERVER_MSG_KICKOUT);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	int sendnum = 0;
	int i = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(table->player_array[i])
		{
			OutputPacket response;
			response.Begin(SERVER_MSG_KICKOUT,table->player_array[i]->id);
			response.WriteShort(0);
			short result = 0;
			if((table->player_array[i]->id == leavePlayer->id) && (!leavePlayer->checkMoney(result)))
			{
				response.WriteString(ErrorMsg::getInstance()->getErrMsg(2));
			}
			else
			{
				response.WriteString(ErrorMsg::getInstance()->getErrMsg(retno));
			}
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

int IProcess::GameStart(Table* table)
{
	if(table == NULL)
		return -1;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;

	
	_LOG_DEBUG_("<==[GameStart] Push [0x%04x]\n", GMSERVER_GAME_START);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	int sendnum = 0;
	int i,j;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(table->player_array[i])
		{
			OutputPacket response;
			response.Begin(GMSERVER_GAME_START, table->player_array[i]->id);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(table->player_array[i]->id);
			response.WriteShort(table->player_array[i]->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteByte(Configure::getInstance().call_time - 2);
			response.WriteByte(Configure::getInstance().open_time > 0 ? Configure::getInstance().open_time : 0);
			response.WriteByte(table->player_array[i]->m_byHandCardData[0]);
			response.WriteByte(table->player_array[i]->m_byHandCardData[1]);
			response.WriteByte(table->player_array[i]->m_byHandCardData[2]);
			response.WriteByte(table->player_array[i]->m_byHandCardData[3]);
			response.WriteByte(table->player_array[i]->m_byHandCardData[4]);
			response.WriteByte(table->m_nCountPlayer);
			for(j = 0; j < GAME_PLAYER; ++j)
			{
				if(table->player_array[j])
				{
					response.WriteInt(table->player_array[j]->id);
					response.WriteShort(table->player_array[j]->m_nStatus);
					response.WriteByte(table->player_array[j]->m_nTabIndex);
				}
			}
			response.WriteShort((table->player_array[i]->m_pTask != NULL) ? table->player_array[i]->m_nGetRoll : 0);
			response.WriteString((table->player_array[i]->m_pTask != NULL) ? table->player_array[i]->m_pTask->taskname : "");
			response.WriteInt(table->m_nAnte);
			table->player_array[i]->getTimeTask();
			response.WriteShort(table->player_array[i]->m_nComTimeTaskFlag);//0表示还在倒计时 1为完成了但是没有领取 2为全部完成了
			response.WriteShort(table->player_array[i]->m_nLeftTimeCom);
			response.End();
			HallManager::getInstance()->sendToHall(table->player_array[i]->m_nHallid, &response, false);
			sendnum++;

			if(table->player_array[i]->source == 30)
			{
				sendToRobotCard(table->player_array[i], table);
			}
		}
	}

	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tm_nStatus=[%d]\n", table->m_nStatus);
	return 0;
}

int IProcess::sendToRobotCard(Player* player, Table* table)
{
	if(player==NULL || table == NULL)
		return 0;
	if(player->source != 30)
		return 0;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	OutputPacket response;
	response.Begin(SERVER_MSG_ROBOT, player->id);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteByte(table->m_nCountPlayer);
	int i = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* otherplayer = table->player_array[i];
		if(otherplayer)
		{
			response.WriteInt(otherplayer->id);
			for(int j = 0; j < CARD_MAX; j++)
				response.WriteByte(otherplayer->m_byHandCardData[j]);
		}
	}
	response.End();
	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[sendToRobotCard] Send To Uid[%d] Error!\n", player->id);
	return 0;
}

int IProcess::OpenCard(Table* table)
{
	if(table == NULL)
		return -1;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	
	_LOG_DEBUG_("<==[OpenCard] Push [0x%04x]\n", GMSERVER_MSG_OPEN);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	int sendnum = 0;
	int i,j,m;
	i = j = m = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(table->player_array[i])
		{
			OutputPacket response;
			response.Begin(GMSERVER_MSG_OPEN, table->player_array[i]->id);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(table->player_array[i]->id);
			response.WriteShort(table->player_array[i]->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteByte(Configure::getInstance().open_time - 2);
			response.WriteByte(table->player_array[i]->m_byHandCardData[3]);
			response.WriteByte(table->player_array[i]->m_byHandCardData[4]);
			response.End();
			HallManager::getInstance()->sendToHall(table->player_array[i]->m_nHallid, &response, false);
			sendnum++;
		}
	}
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tm_nStatus=[%d]\n", table->m_nStatus);
	return 0;
}

int IProcess::GameOver(Table* table)
{
	if(table == NULL)
		return -1;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	
	_LOG_DEBUG_("<==[GameOver] Push [0x%04x]\n", GMSERVER_MSG_GAMEOVER);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	int i = 0;
	short overnum = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = table->player_array[i];
		if(player && player->isGameOver())
		{
			overnum++;
			if(player->m_lFinallGetCoin >= 0)
				player->m_nWin++;

			if(player->m_lFinallGetCoin < 0)
				player->m_nLose++;
			player->m_bCompleteTask = TaskManager::getInstance()->calcPlayerComplete(player, table);

			if(player->m_bFinalComFlag == 1)
			{
				if(table->m_bRewardType == 2)
				{
					player->m_nRoll += player->m_nComGetRoll;	
					updateDB_UserRoll(player, table, player->m_nComGetRoll, true);
				}
				else
					player->m_lMoney += player->m_nComGetCoin;
			}

			updateDB_UserCoin(player, table);

			int64_t winmoney = player->m_lFinallGetCoin;
			std::string strTrumpt;

			if (GameUtil::getDisplayMessage(strTrumpt, GAME_ID, Configure::getInstance().m_nLevel, player->name, winmoney, Configure::getInstance().wincoin1,
				Configure::getInstance().wincoin2, Configure::getInstance().wincoin3, Configure::getInstance().wincoin4))
			{
				AllocSvrConnect::getInstance()->trumptToUser(PRODUCT_TRUMPET, strTrumpt.c_str(), player->pid);
			}
		}
	}

	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(table->player_array[i])
		{
			OutputPacket response;
			response.Begin(GMSERVER_MSG_GAMEOVER,table->player_array[i]->id);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(table->player_array[i]->id);
			response.WriteShort(table->player_array[i]->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteByte(Configure::getInstance().ready_time);
			response.WriteByte(overnum);
			short opnum = 0;
			for(int j = 0; j < GAME_PLAYER; j++)
			{
				if(opnum == overnum)
					break;
				Player *player = table->player_array[j];
				if(player && player->isGameOver())
				{
					response.WriteInt(player->id);
					response.WriteInt64(player->m_lFinallGetCoin);
					response.WriteInt64(player->m_lMoney);
					response.WriteInt(player->m_nWin);
					response.WriteInt(player->m_nLose);
					opnum++;
					_LOG_DEBUG_("overnum:%d uid:%d m_lFinallGetCoin:%ld m_lMoney:%ld\n", overnum, player->id, player->m_lFinallGetCoin, player->m_lMoney);
				}
			}
			response.WriteByte(table->player_array[i]->m_bCompleteTask ? 1 : 0);
			response.WriteShort(table->player_array[i]->m_bCompleteTask ? table->player_array[i]->m_nGetRoll : 0);
			response.WriteByte(table->player_array[i]->m_bFinalComFlag);
			response.WriteByte(table->m_bRewardType);
			if(table->m_bRewardType == 2)
				response.WriteInt(table->player_array[i]->m_nComGetRoll);
			else
				response.WriteInt(table->player_array[i]->m_nComGetCoin);
			response.WriteInt(table->player_array[i]->m_nRoll);
			_LOG_INFO_("[GameOver] uid[%d] m_lFinallGetCoin[%ld] m_lMoney[%ld] m_bFinalComFlag[%d] m_bRewardType[%d] m_nComGetCoin[%d] m_nComGetRoll[%d]\n",table->player_array[i]->id,
				table->player_array[i]->m_lFinallGetCoin, table->player_array[i]->m_lMoney, table->player_array[i]->m_bFinalComFlag, table->m_bRewardType, table->player_array[i]->m_nComGetCoin, table->player_array[i]->m_nComGetRoll);
			Player* player = table->player_array[i];
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

			
			//局数任务完成则清空标志
			if(player->m_bFinalComFlag == 1)
			{
				player->m_bFinalComFlag = 0;
				player->m_nComGetCoin = 0;
				player->m_nComGetRoll = 0;
			}

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
					if(table->m_bRewardType == 2)
					{
						if(player->m_nComGetRoll == 0)
							player->m_nComGetRoll = table->m_nGetRollLow3 + rand()%((table->m_nGetRollHigh3 - table->m_nGetRollLow3) <= 0 ? 1 : (table->m_nGetRollHigh3 - table->m_nGetRollLow3+1));
					}
					else
					{
						if(player->m_nComGetCoin == 0)
							player->m_nComGetCoin = table->m_nGetCoinLow3 + rand()%((table->m_nGetCoinHigh3 - table->m_nGetCoinLow3) <= 0 ? 1 : (table->m_nGetCoinHigh3 - table->m_nGetCoinLow3+1));
					}
				}
				//完成第一个局数
				else if (comflag & 1)
				{
					response.WriteByte(roundnum - table->m_nRoundNum1);
					response.WriteByte(table->m_nRoundNum2);
					if(table->m_bRewardType == 2)
					{
						if(player->m_nComGetRoll == 0)
							player->m_nComGetRoll = table->m_nGetRollLow2 + rand()%((table->m_nGetRollHigh2 - table->m_nGetRollLow2) <= 0 ? 1 : (table->m_nGetRollHigh2 - table->m_nGetRollLow2+1));
					}
					else
					{
						if(player->m_nComGetCoin == 0)
							player->m_nComGetCoin = table->m_nGetCoinLow2 + rand()%((table->m_nGetCoinHigh2 - table->m_nGetCoinLow2) <= 0 ? 1 : (table->m_nGetCoinHigh2 - table->m_nGetCoinLow2+1));
					}
				}
				//所有的都没有完成
				else
				{
					response.WriteByte(roundnum);
					response.WriteByte(table->m_nRoundNum1);
					if(table->m_bRewardType == 2)
					{
						if(player->m_nComGetRoll == 0)
							player->m_nComGetRoll = table->m_nGetRollLow1 + rand()%((table->m_nGetRollHigh1 - table->m_nGetRollLow1) <= 0 ? 1 : (table->m_nGetRollHigh1 - table->m_nGetRollLow1+1));
					}
					else
					{
						if(player->m_nComGetCoin == 0)
							player->m_nComGetCoin = table->m_nGetCoinLow1 + rand()%((table->m_nGetCoinHigh1 - table->m_nGetCoinLow1) <= 0 ? 1 : (table->m_nGetCoinHigh1 - table->m_nGetCoinLow1+1));
					}
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
					if(table->m_bRewardType == 2)
					{
						if(player->m_nComGetRoll == 0)
							player->m_nComGetRoll = table->m_nGetRollLow2 + rand()%((table->m_nGetRollHigh2 - table->m_nGetRollLow2) <= 0 ? 1 : (table->m_nGetRollHigh2 - table->m_nGetRollLow2+1));
					}
					else
					{
						if(player->m_nComGetCoin == 0)
							player->m_nComGetCoin = table->m_nGetCoinLow2 + rand()%((table->m_nGetCoinHigh2 - table->m_nGetCoinLow2) <= 0 ? 1 : (table->m_nGetCoinHigh2 - table->m_nGetCoinLow2+1));
					}
				}
				//所有的都没有完成
				else
				{
					response.WriteByte(roundnum);
					response.WriteByte(table->m_nRoundNum1);
					if(table->m_bRewardType == 2)
					{
						if(player->m_nComGetRoll == 0)
							player->m_nComGetRoll = table->m_nGetRollLow1 + rand()%((table->m_nGetRollHigh1 - table->m_nGetRollLow1) <= 0 ? 1 : (table->m_nGetRollHigh1 - table->m_nGetRollLow1+1));
					}
					else
					{
						if(player->m_nComGetCoin == 0)
							player->m_nComGetCoin = table->m_nGetCoinLow1 + rand()%((table->m_nGetCoinHigh1 - table->m_nGetCoinLow1) <= 0 ? 1 : (table->m_nGetCoinHigh1 - table->m_nGetCoinLow1+1));
					}
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
					if(table->m_bRewardType == 2)
					{
						if(player->m_nComGetRoll == 0)
							player->m_nComGetRoll = table->m_nGetRollLow1 + rand()%((table->m_nGetRollHigh1 - table->m_nGetRollLow1) <= 0 ? 1 : (table->m_nGetRollHigh1 - table->m_nGetRollLow1+1));
					}
					else
					{
						if(player->m_nComGetCoin == 0)
							player->m_nComGetCoin = table->m_nGetCoinLow1 + rand()%((table->m_nGetCoinHigh1 - table->m_nGetCoinLow1) <= 0 ? 1 : (table->m_nGetCoinHigh1 - table->m_nGetCoinLow1+1));
					}
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
			
			response.End();
			HallManager::getInstance()->sendToHall(table->player_array[i]->m_nHallid, &response, false);
		}
	}

	table->startResetTimer(Configure::getInstance().reset_time);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tm_nStatus=[%d]\n", table->m_nStatus);
	return 0;
}

int IProcess::serverWarnPlayerKick(Table* table, Player* warnner, short timeCount)
{
	if(table == NULL || warnner == NULL)
		return -1;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	
	_LOG_DEBUG_("<==[serverWarnPlayerKick] Push [0x%04x]\n", GMSERVER_WARN_KICK);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	OutputPacket response;
	response.Begin(GMSERVER_WARN_KICK,warnner->id);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(warnner->id);
	response.WriteShort(warnner->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteShort(timeCount);
	response.End();
	HallManager::getInstance()->sendToHall(warnner->m_nHallid, &response, false);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] warnner=[%d]\n", warnner->id);
	_LOG_DEBUG_("[Data Response] timeCount=[%d]\n", timeCount);
	return 0;
}

int IProcess::serverComTask(Table* table, Player* player)
{
	if(table == NULL || player == NULL)
		return -1;
	int svid = Configure::getInstance().m_nServerId;
	int tid = (svid << 16)|table->id;
	
	_LOG_DEBUG_("<==[serverComTask] Push [0x%04x]\n", GMSERVER_COM_TASK);
	_LOG_DEBUG_("[Data Response] err=[0], errmsg[]\n");
	OutputPacket response;
	response.Begin(GMSERVER_COM_TASK,player->id);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.End();
	HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] player=[%d]\n", player->id);
	return 0;
}

int IProcess::sendCallInfo(Table* table, Player *player,BYTE isLastOne,short seq)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	int i= 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* other = table->player_array[i];
		if(other)
		{
			OutputPacket response;
			response.Begin(CLIENT_MSG_CALL_BANKER, other->id);
			if(other->id == player->id)
				response.SetSeqNum(seq);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(other->id);
			response.WriteShort(other->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteInt(player->id);
			response.WriteByte(player->m_bisCall);
			response.WriteByte(isLastOne);
			response.End();
			if(HallManager::getInstance()->sendToHall(other->m_nHallid, &response, false) < 0)
				_LOG_ERROR_("[CallBankerProc] Send To Uid[%d] Error!\n", player->id);
		}
	}

	_LOG_DEBUG_("<==[CallBankerProc] Push [0x%04x]\n", CLIENT_MSG_CALL_BANKER);
	_LOG_DEBUG_("[Data Response] push to uid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] push to m_nStatus=[%d]\n", player->m_nStatus);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] caller=[%d] isCall=[%d]\n", player->id, player->m_bisCall);
	return 0;
}

int IProcess::sendSetMulInfo(Table* table, Player *player, short seq)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	int i= 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* other = table->player_array[i];
		if(other)
		{
			OutputPacket response;
			response.Begin(CLIENT_MSG_SET_NUL, other->id);
			if(other->id == player->id)
				response.SetSeqNum(seq);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(other->id);
			response.WriteShort(other->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteInt(player->id);
			response.WriteByte(player->m_nMul);
			response.WriteShort(player->m_nMul);
			response.End();
			if(HallManager::getInstance()->sendToHall(other->m_nHallid, &response, false) < 0)
				_LOG_ERROR_("[sendSetMulInfo] Send To Uid[%d] Error!\n", player->id);
		}
	}

	_LOG_DEBUG_("<==[sendSetMulInfo] Push [0x%04x]\n", CLIENT_MSG_SET_NUL);
	_LOG_DEBUG_("[Data Response] push to uid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] push to m_nStatus=[%d]\n", player->m_nStatus);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] SetMuluid=[%d] m_nMul=[%d]\n", player->id, player->m_nMul);
	return 0;
}
int IProcess::sendOpenCardInfo(Table* table, Player *player, BYTE hasbull, BYTE card1, BYTE card2, BYTE card3, BYTE card4, BYTE card5,short seq)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	int i= 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* other = table->player_array[i];
		if(other)
		{
			OutputPacket response;
			response.Begin(CLIENT_MSG_OPEN_CARD, other->id);
			if(other->id == player->id)
				response.SetSeqNum(seq);
			response.WriteShort(0);
			response.WriteString("ok");
			response.WriteInt(other->id);
			response.WriteShort(other->m_nStatus);
			response.WriteInt(tid);
			response.WriteShort(table->m_nStatus);
			response.WriteInt(player->id);
			response.WriteByte(hasbull);
			response.WriteByte(hasbull >= 1 ? player->m_bCardType : 0);
			response.WriteByte(card1);
			response.WriteByte(card2);
			response.WriteByte(card3);
			response.WriteByte(card4);
			response.WriteByte(card5);
			response.End();
			LOGGER(E_LOG_INFO) << "send open card msg to player = " << other->id << " card1 = "
				<< card1 << " card2 = " << card2 << " card3 = " << card3 << " card4 = " << card4
				<< " card5 = " << card5;
			if(HallManager::getInstance()->sendToHall(other->m_nHallid, &response, false) < 0)
				_LOG_ERROR_("[sendOpenCardInfo] Send To Uid[%d] Error!\n", player->id);
		}
	}

	_LOG_DEBUG_("<==[sendOpenCardInfo] Push [0x%04x]\n", CLIENT_MSG_OPEN_CARD);
	_LOG_DEBUG_("[Data Response] push to uid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] push to m_nStatus=[%d]\n", player->m_nStatus);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] m_bCardType=[%d]\n", hasbull >= 1 ? player->m_bCardType : 0);
	_LOG_DEBUG_("[Data Response] SetMuluid=[%d] hasbull=[%s]\n", player->id, hasbull== 1 ? "true":"false");
	
	//else
		//_LOG_DEBUG_("[CallBankerProc] Send To Uid[%d] Success\n", player->id);
	return 0;
}

int IProcess::updateDB_UserCoin(Player* player, Table* table)
{
	if(table->isAllRobot())
		return 0;
	if(player->source != 30)
	{
		OutputPacket respone;
		respone.Begin(UPDATE_MONEY);
		respone.WriteInt(player->id); 
		respone.WriteByte(1);
		//扣除台费、提示，加上局数任务完成获得的金币
		if(player->m_bFinalComFlag == 1)
			respone.WriteInt64(player->m_lFinallGetCoin - player->m_nUseHelpCoin);
		else
			respone.WriteInt64(player->m_lFinallGetCoin - player->m_nUseHelpCoin);
		respone.End();
		if(MoneyServer()->Send(&respone) < 0)
			_LOG_ERROR_("Send request to MoneyServer Error\n" );

		OutputPacket outPacket;
		outPacket.Begin(ROUND_SET_INFO);
		outPacket.WriteInt(player->id); 
		outPacket.WriteInt(player->m_nWin );
		outPacket.WriteInt(player->m_nLose);
		outPacket.WriteInt(player->m_nTie);
		outPacket.WriteInt(player->m_nRunAway);
		outPacket.End();
		if (RoundServer()->Send( &outPacket ) < 0)
			_LOG_ERROR_("Send request to RoundServerConnect Error\n" );
	}

	int cmd = 0x0120;
	OutputPacket outputPacket;
	outputPacket.Begin(cmd);
	outputPacket.WriteInt(player->id);  
	outputPacket.WriteInt64(player->m_lFinallGetCoin);
	outputPacket.WriteInt(player->source);  
	outputPacket.WriteInt(player->cid);  
	outputPacket.WriteInt(player->sid);  
	outputPacket.WriteInt(player->pid);  
	outputPacket.WriteInt(player->m_nTax);	//台费
	outputPacket.WriteInt(table->m_nAnte); //底注
	outputPacket.WriteInt(Configure::getInstance().m_nServerId);  
	outputPacket.WriteInt(table->id);  
	outputPacket.WriteInt(table->m_nType);  
	outputPacket.WriteString(table->getGameID());  
	outputPacket.WriteInt(0);  
	outputPacket.WriteInt64(player->m_lMoney); 
	outputPacket.WriteInt(table->getStartTime());  
	outputPacket.WriteInt(table->getEndTime());
	outputPacket.WriteInt(player->m_bCardType);
	outputPacket.End();
	if(MySqlServer()->Send( &outputPacket )==0)
	{
		_LOG_DEBUG_("Transfer request to BackServer OK\n" );
		return 0;
	}
	else
	{
		 _LOG_ERROR_("Send request to BackServer Error\n" );
		 return -1;
	}
	return 0;
}

int IProcess::updateDB_UserRoll(Player* player, Table* table, int nroll, bool isboardtask)
{
	if (table->isAllRobot())
		return 0;

	OutputPacket respone;
	respone.Begin(UPDATE_ROLL);
	respone.WriteInt(player->id); 
	respone.WriteByte(1);   
	respone.WriteInt(nroll);
	respone.End();
	if(MoneyServer()->Send(&respone) < 0)
		_LOG_ERROR_("Send request to MoneyServer Error\n" );

	int sedcmd = 0x0122;
	OutputPacket outputPacket;
	outputPacket.Begin(sedcmd);
	outputPacket.WriteInt(player->source);
	outputPacket.WriteInt(player->cid);  
	outputPacket.WriteInt(player->pid);  
	outputPacket.WriteInt(player->sid);  
	outputPacket.WriteInt(player->id);  
	outputPacket.WriteInt(nroll); 
	//获得乐券的类型
	outputPacket.WriteInt(isboardtask ? 3 : 4);
	outputPacket.WriteInt(player->m_nRoll); 
	//是局数任务直接写64就可以，因为64这个数字牌型任务不可能用到
	if (player->m_pTask != NULL) {
		outputPacket.WriteInt64(isboardtask ? 64 : player->m_pTask->taskid);
	} else {
		outputPacket.WriteInt64(isboardtask ? 64 : 0);
	}	
	outputPacket.WriteInt(Configure::getInstance().m_nLevel);
	outputPacket.WriteInt(time(NULL));
	outputPacket.End();

	_LOG_DEBUG_("==>[updateDB] [0x%04x] [updateDB_UserRoll]\n", sedcmd);

	if(MySqlServer()->Send( &outputPacket )==0)
	{
		_LOG_DEBUG_("Transfer request to BackServer OK\n" );
		return 0;
	}
	else
	{
		 _LOG_ERROR_("Send request to BackServer Error\n" );
		 return -1;
	}
	return 0;
}

int IProcess::UpdateDBActiveTime(Player* player)
{
	if(player->source == 30)
		return 0;
	OutputPacket outputPacket;
	outputPacket.Begin(ROUND_SET_ONLINETIME);
	outputPacket.WriteInt(player->id);  
	outputPacket.WriteInt(time(NULL) - player->getEnterTime());
	outputPacket.End();	
 
	_LOG_INFO_("==>[updateDB] [0x%04x] [UpdateDBActiveTime]\n", ROUND_SET_ONLINETIME);
 
	if(RoundServer()->Send( &outputPacket )==0)
	{
		_LOG_DEBUG_("Transfer request to RoundServerConnect OK\n" );
		return 0;
	}
	else
	{
		 _LOG_ERROR_("Send request to RoundServerConnect Error\n" );
		 return -1;
	}
	return 0;
}

