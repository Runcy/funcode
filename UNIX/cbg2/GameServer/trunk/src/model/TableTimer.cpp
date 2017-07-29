#include "TableTimer.h"
#include "IProcess.h"
#include "Table.h"
#include "Logger.h"
#include "Configure.h"
#include "HallManager.h"
//====================CTimer_Handler==================================//

int CTimer_Handler::ProcessOnTimerOut()
{
	if(handler)
		return handler->ProcessOnTimerOut(this->timeid, this->uid);
	else
		return 0;

}

void CTimer_Handler::SetTimeEventObj(TableTimer * obj, int timeid, int uid)
{
	this->handler = obj;
	this->timeid = timeid;
	this->uid = uid;
}


//==========================TableTimer==================================//

void TableTimer::init(Table* table)
{
	this->table = table;
}

void TableTimer::stopAllTimer()
{
	stopKickTimer();
}

void TableTimer::startKickTimer(int timeout)
{
	m_TableKickTimer.SetTimeEventObj(this, TABLE_KICK_TIMER);
	m_TableKickTimer.StartTimer(timeout);
}

void TableTimer::stopKickTimer()
{
	m_TableKickTimer.StopTimer();
}

void TableTimer::startOpenTimer(int timeout)
{
	m_TableNormalTimer.SetTimeEventObj(this, TABLE_OPEN_TIMER);
	m_TableNormalTimer.StartTimer(timeout);
}

void TableTimer::stopOpenTimer()
{
	m_TableNormalTimer.StopTimer();
}

void TableTimer::startResetTimer(int timeout)
{
	m_ComingRobot.SetTimeEventObj(this, RESET_TIMER);
	m_ComingRobot.StartTimer(timeout);
}

void TableTimer::stopResetTimer()
{
	m_ComingRobot.StopTimer();
}

int TableTimer::ProcessOnTimerOut(int Timerid, int uid)
{
	switch (Timerid)
	{
	case TABLE_KICK_TIMER:
		return TableKickTimeOut();
	case TABLE_OPEN_TIMER:
		return OpenTimeOut();
	case RESET_TIMER:
		return ResetTimeOut();
	default:
		return 0;
	}
	return 0;
}

int TableTimer::TableKickTimeOut()
{
	this->stopKickTimer();
	_LOG_INFO_("TableKickTimeOut tid=[%d]\n", this->table->id);
	table->unlockTable();
	if(table->isActive())
	{
		_LOG_WARN_("this table[%d] is Active\n",table->id);
		return 0;
	}

// 	if(table->m_nCountPlayer == 1)
// 	{
// 		_LOG_WARN_("this table[%d] is One Player\n",table->id);
// 		return 0;
// 	}
	int i;
	for(i = 0; i < 1/*GAME_PLAYER*/; ++i)
	{
		Player* getplayer = p;/*table->player_array[i];*/
		//把没有准备的用户踢出
		if(getplayer && !getplayer->isReady())
		{
			IProcess::serverPushLeaveInfo(table, getplayer);
			table->playerLeave(getplayer);
		}
	}

	if(table->isAllReady()&&table->m_nCountPlayer > 1)
	{
		table->GameStart();
		return 0;
	}
	else
		_LOG_ERROR_("table[%d] m_nCountPlayer[%d] not all ready\n", table->id, table->m_nCountPlayer);
	return 0;
}

int TableTimer::OpenTimeOut()
{
	this->stopOpenTimer();
	_LOG_INFO_("OpenTimeOut tid=[%d]\n", this->table->id);
	int i;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* getplayer = table->player_array[i];
		if(getplayer && getplayer->isActive())
		{
			if(!getplayer->m_bhasOpen)
			{
				getplayer->m_bhasOpen = true;
				getplayer->m_bisOpenTimeOut = true;
				
				BYTE index1,index2;
				index1 = index2 = 0;
				BYTE bCardType = table->m_GameLogic.GetCardType(getplayer->m_byHandCardData, CARD_MAX, index1, index2);

				BYTE bullcard[3];
				BYTE nobullcard[2];
				if(bCardType > 0 && bCardType < 20)
				{
					int j = 0;
					for(int i = 0; i < 5; i++)
					{
						if(i != index1 && i != index2)
							bullcard[j++] = getplayer->m_byHandCardData[i];
					}
					nobullcard[0] = getplayer->m_byHandCardData[index1];
					nobullcard[1] = getplayer->m_byHandCardData[index2];
				}
					
				getplayer->m_bCardType = bCardType;
				//用户掉线则直接给用户开牌、超时大于牛牛给用户开牌
				if (!getplayer->isonline || bCardType >= Configure::getInstance().helptype)
					getplayer->m_bCardType = table->m_GameLogic.GetCardType(getplayer->m_byHandCardData, 5);

				if(getplayer->m_bCardType > 0 && bCardType < 20)
				{
					IProcess::sendOpenCardInfo(table, getplayer, getplayer->m_bCardType, bullcard[0],bullcard[1],bullcard[2],nobullcard[0],nobullcard[1]);
				}
				else
					IProcess::sendOpenCardInfo(table, getplayer, getplayer->m_bCardType, getplayer->m_byHandCardData[0],getplayer->m_byHandCardData[1],
					getplayer->m_byHandCardData[2],getplayer->m_byHandCardData[3],getplayer->m_byHandCardData[4]);
			}
		}
	}
	table->GameOver();
	return 0;
}

int TableTimer::ResetTimeOut()
{
	this->stopResetTimer();
	if(table->isEmpty())
		return 0;
	int i = 0;
	
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = table->player_array[i];
		if(player && !player->isonline)
		{
			_LOG_INFO_("ResetTimeOut player[%d] Leave Drop Out\n", player->id);
			IProcess::serverPushLeaveInfo(table, player);
			table->playerLeave(player);
		}
	}

	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = table->player_array[i];
		if(player && player->isGameOver())
		{
			if(player->id == table->m_nBanker)
			{
				if(player->m_bisCallTimeOut && player->m_bisOpenTimeOut)
				{
					_LOG_INFO_("ResetTimeOut player[%d] Banker Leave Not Operation\n", player->id);
					IProcess::serverPushLeaveInfo(table, player);
					table->playerLeave(player);
				}
			}
			else
			{
				if(player->m_bisCallTimeOut && player->m_bisSetMulTimeOut && player->m_bisOpenTimeOut)
				{
					_LOG_INFO_("ResetTimeOut player[%d] Player Leave Not Operation\n", player->id);
					IProcess::serverPushLeaveInfo(table, player);
					table->playerLeave(player);
				}
			}
		}
	}
	if (table->isEmpty())
	{
		LOGGER(E_LOG_DEBUG) << "table is empty, no need reset table!";
		return 0;
	}
	this->table->reset();
	return 0;
}
