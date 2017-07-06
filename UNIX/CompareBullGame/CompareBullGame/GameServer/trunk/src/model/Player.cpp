#include <json/json.h>
#include "Player.h"
#include "Configure.h"
#include "AllocSvrConnect.h"
#include "Logger.h"
#include "GameCmd.h"
#include "GameApp.h"
#include "CoinConf.h"
#include "Room.h"
#include "IProcess.h"

int pase_json(Json::Value& value, const char* key, int def)
{
	int v = def;
	try{
		v = value[key].asInt();
	}
	catch(...)
	{
		 v = def;
	}
	return v;
}

std::string pase_json(Json::Value& value, const char* key, std::string def)
{
	std::string  v = def;
	try{
		v = value[key].asString();
	}
	catch(...)
	{
		 v = def;
	}
	return v;
}


void Player::init()
{
	id = 0;
	memset(m_byHandCardData, 0, sizeof(m_byHandCardData));	//当前手牌
	memset(m_nMulArray, 0, sizeof(m_nMulArray));
	memset(name, 0, sizeof(name));
	memset(json, 0, sizeof(json));
	memset(headlink, 0, sizeof(headlink));
	memset(m_byIP, 0, sizeof(m_byIP));
	m_nStatus = 0;
	m_nTabIndex = -1;
	m_nHallid = 0;
	tid = -1;
	source = 0;
	m_bisCall = 0;
	isonline = true;
	m_nMul = 0;
	m_bhasOpen = false;
	m_nStartNum = 0;
	m_bCardType = 0;
	m_lFinallGetCoin = 0;
	m_bisCallTimeOut = false;
	m_bisSetMulTimeOut = false;
	m_bisOpenTimeOut = false;
	m_nUseHelpCoin = 0;
	m_bHasUserHelp = false;
	m_pTask = NULL;
	m_nGetRoll = 0;
	m_bCompleteTask = false;
	m_nRoundComFlag = 0;
	m_nRoundNum = 0;
	m_bFinalComFlag = 0;
	m_nComGetCoin = 0;
	m_nComGetRoll = 0;
	m_bhasCard = false;
	m_nMagicCoinCount = 0;
	m_nMagicNum = 0;
	m_nBoradTask = 0;
	m_nTimeTask = 0;
	m_lCoinTask = 0;
	m_lSafeMoney = 0;
	m_nStartTime = 0;
	m_nComTimeTaskFlag = 0;
	m_nLeftTimeCom = 0;
	m_nRewardTimeCoin = 0;
	m_nTaskFlag = 0;
	isbacklist = false;
}

void Player::reset()
{
	memset(m_byHandCardData, 0, sizeof(m_byHandCardData));	//当前手牌
	memset(m_nMulArray, 0, sizeof(m_nMulArray));
	m_bisCall = 0;
	m_nMul = 0;
	m_bhasOpen = false;
	m_bCardType = 0;
	m_lFinallGetCoin = 0;
	m_bisCallTimeOut = false;
	m_bisSetMulTimeOut = false;
	m_bisOpenTimeOut = false;
	m_nUseHelpCoin = 0;
	m_bHasUserHelp = false;
	m_pTask = NULL;
	m_nGetRoll = 0;
	m_bCompleteTask = false;
	m_bhasCard = false;
	if(this->isGameOver())
	{
		_LOG_DEBUG_("player:%d is GameOver\n", this->id);
		m_nMagicCoinCount = 0;
		m_nMagicNum = 0;
	}
	isbacklist = false;
}

void Player::login()
{
	isonline = true;

	if(strlen(json)>=2 && json[0]=='{')
	{
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(json, value))
		{   
			this->cid = pase_json(value,"cid",0);
			this->sid = pase_json(value,"sid",0); 
			this->pid = pase_json(value,"pid",0); 
			m_nTaskFlag = pase_json(value,"task",0);
			int totalNum = pase_json(value,"sum",20);
			int winNum = pase_json(value,"win",5);
			this->m_nWin = winNum;
			this->m_nLose = totalNum - winNum;
			string link =  pase_json(value,"picUrl", "");
			strcpy(this->headlink, link.c_str());
		} 
	}
	else
	{
		_LOG_ERROR_("json parse error [%s]\n", json);   
	}
	m_nStartNum = 1;

	AllocSvrConnect::getInstance()->userEnterGame(this);
	this->setEnterTime(time(NULL));
}

void Player::leave(bool isSendToUser)
{

	if(isSendToUser)
	{
		AllocSvrConnect::getInstance()->userLeaveGame(this);
	}
	IProcess::UpdateDBActiveTime(this);
	OutputPacket respone;
	respone.Begin(UPDATE_LEAVE_GAME_SERVER);
	respone.WriteInt(this->id);
	respone.End();
	if(this->source != 30)
	{
		if(MoneyServer()->Send(&respone) < 0)
			_LOG_ERROR_("Send request to MoneyServer Error\n" );
	}
	IProcess::UpdateDBActiveTime(this);
	//用户离开处理时间问题
	if(m_nStartTime != 0)
	{
		int comflag = m_nTimeTask >> 16;
		int onlinetime = m_nTimeTask & 0x0FFFF;
		m_nTimeTask = comflag << 16 | (onlinetime + (time(NULL) - m_nStartTime));
	}
	this->StopTimer();
	this->m_nStatus = STATUS_PLAYER_LOGOUT;
	this->tid = -1;
	this->m_nTabIndex = -1;
}

void Player::enter()
{
	AllocSvrConnect::getInstance()->userUpdateStatus(this, this->m_nStatus);
}

bool Player::isAllowMul(int mul)
{
	for(int i = 0; i < 4; ++i)
	{
		if(m_nMulArray[i] == mul)
			return true;
	}
	return false;
}

bool Player::isBigestMul()
{
	if(this->m_nMul == m_nMulArray[3])
		return true;
	return false;
}

bool Player::checkMoney(short &result)
{
	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();

	short clevel = Configure::getInstance().m_nLevel;
	coincfg->level = clevel;

	if (this->m_lMoney == 0)
	{
		return false;
	}

	if(coincfg->maxmoney>0 && this->m_lMoney>=coincfg->minmoney&& this->m_lMoney<=coincfg->maxmoney)
	{
		return true;
	}

	else if(coincfg->maxmoney<=0 && this->m_lMoney>=coincfg->minmoney)
	{
		return true;
	}

	else
	{
		if(coincfg->maxmoney > 0 && this->m_lMoney > coincfg->maxmoney)
			result = 1;
		return false;
	}
	return false;
}

short Player::getPlayCount(short &comPlayCount, short &level)
{
	//if(Configure::getInstance()->type == LEVEL_5 || Configure::getInstance()->type == LEVEL_4)
	//	return 0;

	if(m_nTaskFlag != 1)
		return 0;

	int ntask = this->m_nBoradTask;

	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();
	
	int nlow = ntask & 0x00FF;
	int nmid = ntask>>8 & 0x00FF;
	int nhigh = ntask>>16 & 0x00FF;
	int ncomplete = ntask >> 24;
	int playCount1 = coincfg->roundconf & 0x00FF;
	int playCount2 = coincfg->roundconf >> 8 & 0x00FF;
	int playCount3 = coincfg->roundconf >> 16 & 0x00FF;
	//int nret = 0;
	if(Configure::getInstance().m_nLevel == LEVEL_1)
	{
		if(ncomplete & 1)
		{
			if(ncomplete & 2)
			{
				if(ncomplete & 4)
				{
					return 0;
				}
				else
				{
					level = 3;
					comPlayCount = nhigh;
					return playCount3;
				}
			}
			else
			{
				level = 2;
				comPlayCount = nmid;
				return playCount2;
			}
		}
		else
		{
			level = 1;
			comPlayCount = nlow;
			return playCount1;
			
		}
	}

	if(Configure::getInstance().m_nLevel == LEVEL_2)
	{
		if(ncomplete & 2)
		{
			if(ncomplete & 4)
			{
				return 0;
			}
			else
			{
				level = 3;
				comPlayCount = nhigh;
				return playCount3;
			}
		}
		else
		{
			level = 2;
			comPlayCount = nmid;
			return playCount2;
		}
	}

	if(Configure::getInstance().m_nLevel == LEVEL_3)
	{
		if(ncomplete & 4)
		{
			return 0;
		}
		else
		{
			level = 3;
			comPlayCount = nhigh;
			return playCount3;
		}
	}
	return 0;
}

int Player::getCoinTask(int &maxcoin, short &index)
{
	if(m_nTaskFlag != 1)
		return 0;
	int64_t winMoney = this->m_lCoinTask & 0x0FFFFFFFF;
	int comflag = this->m_lCoinTask >> 32;
	int i = 0;
	int64_t sumMoney = this->m_lMoney + this->m_lSafeMoney;

	int64_t tempMoney = winMoney > sumMoney ? winMoney : sumMoney;
	
	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();
	maxcoin = coincfg->cointaskarray[0];

	int tempflag = 1;
	for(i = coincfg->coincount - 1; i >= 0; --i)
	{
		tempflag = tempflag | (1<<i);
	}
	_LOG_DEBUG_("uid:%d comflag:%d winMoney:%d tempflag:%d \n", this->id, comflag, winMoney, tempflag);
	
	//全部完成了
	if(comflag >= tempflag)
		return 2;

	for(i = 0; i < coincfg->coincount; ++i)
	{
		bool btemp = comflag & (1<<i);
		if(tempMoney >= coincfg->cointaskarray[i] && !btemp)
		{
			//完成但是没有领取
			index = i;
			maxcoin = coincfg->cointaskarray[i];
			return 1;
		}
	}

	for(i = 0; i < coincfg->coincount; ++i)
	{
		bool btemp = comflag & (1<<i);
		if(tempMoney >= coincfg->cointaskarray[i] && btemp)
		{
			if(i+1 < coincfg->coincount)
			{
				index = i+1;
				maxcoin = coincfg->cointaskarray[i+1];
			}
		}
	}
	return 0;
}

void Player::getTimeTask()
{
	if(m_nTaskFlag != 1)
		return;
	int comflag = m_nTimeTask >> 16;
	int onlinetime = m_nTimeTask & 0x0FFFF;
	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();
	//全部完成就不要开启定时器去处理任务了
	if(comflag >= coincfg->timecount)
	{
		m_nComTimeTaskFlag = 2;
		return;
	}
	//完成了但是没有领取宝箱
	if(onlinetime >= coincfg->timearray[comflag])
	{
		m_nComTimeTaskFlag = 1;
		m_nRewardTimeCoin = coincfg->rewardtime[comflag];
		return;
	}

	if(m_nStartTime != 0)
	{
		m_nComTimeTaskFlag = 0;
		m_nLeftTimeCom = coincfg->timearray[comflag] - (onlinetime + (time(NULL) - m_nStartTime));
		m_nRewardTimeCoin = coincfg->rewardtime[comflag];
	}
	else
	{
		m_nComTimeTaskFlag = 3;
		m_nLeftTimeCom = coincfg->timearray[comflag] - onlinetime;
		m_nRewardTimeCoin = coincfg->rewardtime[comflag];
	}
}

void Player::setStartTime()
{
	if(m_nTaskFlag != 1)
		return;
	//当等于0说明先前没有玩牌，现在才开始玩牌的
	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();
	if(this->m_nStartTime == 0 && this->isActive())
	{
		int comflag = m_nTimeTask >> 16;
		int onlinetime = m_nTimeTask & 0x0FFFF;
		//全部完成就不要开启定时器去处理任务了
		if(comflag >= coincfg->timecount)
			return;
		//完成了但是没有领取宝箱
		if(onlinetime >= coincfg->timearray[comflag])
			return;
		this->m_nStartTime = time(NULL);
		int timeOut = coincfg->timearray[comflag] - onlinetime;
		this->StartTimer(timeOut*1000);
	}
}

int Player::ProcessOnTimerOut()
{
	int comflag = m_nTimeTask >> 16;
	int onlinetime = m_nTimeTask & 0x0FFFF;
	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();
	onlinetime = coincfg->timearray[comflag];
	_LOG_INFO_("Time Task TimeOut uid=[%d] onlinetime=[%d]\n", this->id, onlinetime);
	m_nTimeTask = comflag << 16 | onlinetime;
	m_nLeftTimeCom = 0;
	this->m_nStartTime = 0;
	Room* room = Room::getInstance();
	Table* table = room->getTable(this->tid);
	IProcess::serverComTask(table, this);
	return 0;
}


