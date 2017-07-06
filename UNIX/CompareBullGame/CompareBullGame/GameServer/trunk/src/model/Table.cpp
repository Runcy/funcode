#include <stdio.h>
#include <stdlib.h>
#include "Table.h"
#include "Logger.h"
#include "IProcess.h"
#include "Configure.h"
#include "AllocSvrConnect.h"
#include "CoinConf.h"
#include "GameUtil.h"
#include "Util.h"

Table::Table():m_nStatus(-1)
{
}
Table::~Table()
{
}

void Table::init()
{
	timer.init(this);
	this->m_nStatus = STATUS_TABLE_EMPTY;
	memset(m_bTableCardArray, 0, sizeof(m_bTableCardArray));
	memset(player_array, 0, sizeof(player_array));
	m_nType = Configure::getInstance().m_nLevel;
	m_nBanker = 0;
	m_nAnte = 1;
	m_nTax = 0;
	m_nRoundNum1 = 0;
	m_nRoundNum2 = 0;
	m_nRoundNum3 = 0;
	m_nGetCoinHigh1 = 0;
	m_nGetCoinLow1 = 0;
	m_nGetCoinHigh2 = 0;
	m_nGetCoinLow2 = 0;
	m_nGetCoinHigh3 = 0;
	m_nGetCoinLow3 = 0;
	m_bRewardType = 1;
	m_nGetRollHigh1 = 0;
	m_nGetRollLow1 = 0;
	m_nGetRollHigh2 = 0;
	m_nGetRollLow2 = 0;
	m_nGetRollHigh3 = 0;
	m_nGetRollLow3 = 0;
	m_nMagicCoin = 5;
}

void Table::reset()
{
	memset(m_bTableCardArray, 0, sizeof(m_bTableCardArray));
	m_nBanker = 0;
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = player_array[i];
		if(player)
			player->reset();
	}
	m_nAnte = 1;
	m_nTax  = 0;       
	m_nRoundNum1 = 0;
	m_nRoundNum2 = 0;
	m_nRoundNum3 = 0;
	m_nGetCoinHigh1 = 0;
	m_nGetCoinLow1 = 0;
	m_nGetCoinHigh2 = 0;
	m_nGetCoinLow2 = 0;
	m_nGetCoinHigh3 = 0;
	m_nGetCoinLow3 = 0;
	m_bRewardType = 1;
	m_nGetRollHigh1 = 0;
	m_nGetRollLow1 = 0;
	m_nGetRollHigh2 = 0;
	m_nGetRollLow2 = 0;
	m_nGetRollHigh3 = 0;
	m_nGetRollLow3 = 0;
	m_nMaxmul = 0;
	this->setTableConf();
}

bool Table::isUserInTab(int uid)
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = player_array[i];
		if(player && player->id == uid)
			return true;
	}
	return false;
}

Player* Table::getPlayer(int uid)
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = player_array[i];
		if(player && player->id == uid)
			return player;
	}
    return NULL;
}

int Table::playerSeatInTab(Player* complayer)
{
	if(player_array[0] == NULL)
	{
		player_array[0] = complayer;
		complayer->m_nStatus = STATUS_PLAYER_COMING;
		complayer->tid = this->id;
		complayer->m_nTabIndex = 0;
		return 0;
	}
	if(player_array[1] == NULL)
	{
		player_array[1] = complayer;
		complayer->m_nStatus = STATUS_PLAYER_COMING;
		complayer->tid = this->id;
		complayer->m_nTabIndex = 1;
		return 0;
	}

	if(player_array[2] == NULL)
	{
		player_array[2] = complayer;
		complayer->m_nStatus = STATUS_PLAYER_COMING;
		complayer->tid = this->id;
		complayer->m_nTabIndex = 2;
		return 0;
	}

	if(player_array[3] == NULL)
	{
		player_array[3] = complayer;
		complayer->m_nStatus = STATUS_PLAYER_COMING;
		complayer->tid = this->id;
		complayer->m_nTabIndex = 3;
		return 0;
	}

	if(player_array[4] == NULL)
	{
		player_array[4] = complayer;
		complayer->m_nStatus = STATUS_PLAYER_COMING;
		complayer->tid = this->id;
		complayer->m_nTabIndex = 4;
		return 0;
	}
	return -1;
}

int Table::playerComming(Player* player)
{
	if(player == NULL)
		return -1;
	//如果此用户已经在此桌子则把另外那个player清除，替代为新的player
	if(isUserInTab(player->id))
	{
		for(int i = 0; i < GAME_PLAYER; ++i)
		{
			if(player_array[i] && player_array[i]->id == player->id)
			{
				player_array[i]->readyTimer.stopKickTimer();
				player->m_nStatus = player_array[i]->m_nStatus;
				player->tid = player_array[i]->tid;
				player->m_nTabIndex = player_array[i]->m_nTabIndex;
				memcpy(player->m_byHandCardData,player_array[i]->m_byHandCardData, CARD_MAX);
				player->m_bisCall = player_array[i]->m_bisCall;
				player->m_nMul = player_array[i]->m_nMul;
				player->m_bhasOpen = player_array[i]->m_bhasOpen;
				player->m_bCardType = player_array[i]->m_bCardType;
				player->m_bhasCard = player_array[i]->m_bhasCard;
				player->m_bisCall = player_array[i]->m_bisCall;
				memcpy(player->m_nMulArray,player_array[i]->m_nMulArray, 4*sizeof(player->m_nMulArray[0]));
				player_array[i]->init();
				player_array[i] = player;
			}
		}
		return 0;
	}
	//说明此房间这个用户进入不成功，返回-2之后重新给此用户分配房间
	if(playerSeatInTab(player) < 0)
		return -2;
	if (!this->isActive())
		this->m_nStatus = STATUS_TABLE_READY;
	//这里设置用户进入的标志，并且设置状态
	player->enter();
	//当前用户数加1
	++this->m_nCountPlayer;
	if (this->m_nCountPlayer == 1)
	{
		this->setTableConf();
	}
	AllocSvrConnect::getInstance()->updateTableUserCount(this);
	return 0;
}

bool Table::isAllReady()
{
	int readyCount=0;
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(player_array[i] && player_array[i]->isReady())
			readyCount++;
		if(readyCount == this->m_nCountPlayer)
			return true;
	}
	return false;
}

void Table::setSeatNULL(Player* player)
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(player_array[i] == player)
			player_array[i] = NULL;
	}
}

void Table::playerLeave(int uid, bool senduser /*= true*/)
{
	Player*  player = this->getPlayer(uid);
	if(player)
	{
		this->playerLeave(player, senduser);
	}
}

void Table::playerLeave(Player* player, bool senduser /*= true*/)
{
	if(player == NULL)
		return;
	if (!isUserInTab(player->id))
	{
		return;
	}
	_LOG_WARN_("Tid[%d] Player[%d] Leave\n", this->id, player->id);
	//this->stopReadyTimer(player->id);
	stopKickTimer(player);
	//清空上一个赢牌的用户
	player->leave(senduser);
	player->init();
	this->setSeatNULL(player);
	//如果桌子不是在玩牌则状态变为等待准备状态
	if(!this->isActive())
	{
		this->m_nStatus = STATUS_TABLE_READY;
	}
	//当前用户减一
	--this->m_nCountPlayer;
	if (m_nCountPlayer <= 1)
	{
		this->unlockTable();
	}

	if(this->isEmpty())
	{
		this->init();
	}


	AllocSvrConnect::getInstance()->updateTableUserCount(this);
}

int Table::GetSpecialType()
{
	//20%  => 8
	//20%  => 9
	//20%  => 牛牛
	//20%  => 炸弹牛
	//10%  => 五花牛
	//10%  => 五小牛
	//int num = 0;
	static int dwRandCount=0;
	if(dwRandCount > 10000000)
		dwRandCount = 0;
	srand((unsigned)time(NULL)+dwRandCount++);
	int rnd = rand()%100;
	if(rnd >= 0 && rnd < 20) //20
		return 8;
	if(rnd >= 20 && rnd < 40) //40
		return 9;
	if(rnd >= 40 && rnd < 60) //60
		return 10;
	if(rnd >= 60 && rnd < 80)
		return OX_FOUR_SAME;
	if(rnd >= 80 && rnd < 90)
		return OX_FIVEKING;
	if(rnd >= 90 && rnd < 100)
		return OX_FIVESMALL;
	return 10;
}

bool Table::isAllRobot()
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(player_array[i] && player_array[i]->source != 30)
		{
			//排除掉还没有准备的情况
			if(player_array[i]->isComming())
				continue;
			if(!player_array[i]->isReady())
				return false;
		}
	}
	return true;
}

bool Table::haveRobot()
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(player_array[i] && player_array[i]->source == 30)
			return true;
	}
	return false;
}

void Table::GameStart()
{
	m_GameLogic.RandCardSpecialType(m_bTableCardArray[0],sizeof(m_bTableCardArray)/sizeof(m_bTableCardArray[0][0]), 0);	
	int cardindex = 0;
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(player_array[i])
		{
			this->stopKickTimer(player_array[i]);
			if(!player_array[i]->m_bhasCard)
			{
				memcpy(player_array[i]->m_byHandCardData,m_bTableCardArray[cardindex++], sizeof(m_bTableCardArray[0]));
				player_array[i]->m_bhasCard = true;
			}
			char cardbuff[64]={0};
			for (int j = 0; j < 5; ++j)
			{
				sprintf(cardbuff+5*j, "0x%02x ",player_array[i]->m_byHandCardData[j]);
			}
			_LOG_INFO_("[gameStart] tid=[%d] uid=[%d] cardarray:[%s] cardType:%d, mul:%d \n", this->id, player_array[i]->id, cardbuff, m_GameLogic.GetCardType(player_array[i]->m_byHandCardData, 5), m_GameLogic.GetTimes(player_array[i]->m_byHandCardData, 5));
			player_array[i]->m_nStatus = STATUS_PLAYER_ACTIVE;
			player_array[i]->setStartTime();
		}
	}
	this->m_nStatus = STATUS_TABLE_CALL;
	this->setTableConf();
	this->setStartTime(time(NULL));

	time_t t;
	time(&t);
	char time_str[32]={0};
    struct tm* tp= localtime(&t);
    strftime(time_str,32,"%Y%m%d%H%M%S",tp);
    char gameId[80] ={0};
	short tlevel = this->m_nType;
    sprintf(gameId, "%s|%02d|%02d|%d|%d|%d|%d|%d", time_str,tlevel,this->m_nCountPlayer,
		player_array[0]?player_array[0]->id:0,
		player_array[1]?player_array[1]->id:0,
		player_array[2]?player_array[2]->id:0,
		player_array[3]?player_array[3]->id:0,
		player_array[4]?player_array[4]->id:0);
    this->setGameID(gameId);
	LOGGER(E_LOG_INFO) << "[gameStart] tid=" << this->id << " ante=" << this->m_nAnte << " ntax=" << this->m_nTax << " gameid=" << this->getGameID();
	IProcess::GameStart(this);
	this->m_nStatus = STATUS_TABLE_OPEN;
	this->stopResetTimer();
	this->startOpenTimer(Configure::getInstance().open_time);
	this->timeout_time = time(NULL);
}

void Table::setTableConf()
{
	CoinCfgInfo* coincfg = CoinConf::getInstance()->getCoinCfg();

	if (coincfg->tax > 100 || coincfg->tax < 0)
		coincfg->tax = 5;
	this->m_nTax = coincfg->tax;

	this->m_nAnte		  = coincfg->ante > 0 ? coincfg->ante : 10;
	this->m_nHelpCoin	  = 0;	//提示不用扣分
	this->m_nRoundNum1    = coincfg->roundnum & 0x00FF;
	this->m_nRoundNum2    = coincfg->roundnum>>8 & 0x00FF;
	this->m_nRoundNum3    = coincfg->roundnum>>16 & 0x00FF;
	this->m_nGetCoinHigh1 = coincfg->coinhigh1;
	this->m_nGetCoinLow1  = coincfg->coinlow1;
	this->m_nGetCoinHigh2 = coincfg->coinhigh2;
	this->m_nGetCoinLow2  = coincfg->coinlow2;
	this->m_nGetCoinHigh3 = coincfg->coinhigh3;
	this->m_nGetCoinLow3  = coincfg->coinlow3;
	this->m_bRewardType   =	coincfg->rewardtype;
	this->m_nGetRollHigh1 = coincfg->rollhigh1;
	this->m_nGetRollLow1  = coincfg->rolllow1;
	this->m_nGetRollHigh2 = coincfg->rollhigh2;
	this->m_nGetRollLow2  = coincfg->rolllow2;
	this->m_nGetRollHigh3 = coincfg->rollhigh3;
	this->m_nGetRollLow3  = coincfg->rolllow3;
	this->m_nMagicCoin    = coincfg->magiccoin;
	this->m_nMaxmul = coincfg->maxmul > 0 ? coincfg->maxmul : 99;
	if(rand()%100 < Configure::getInstance().rewardRate)
		this->m_bRewardType = 1;
	else
		this->m_bRewardType = 2;
	LOGGER(E_LOG_DEBUG) << "m_nAnte=" << m_nAnte << " m_nTax=" << m_nTax << " m_nMaxMul=" << this->m_nMaxmul;
}

void Table::OpenCard()
{
	this->m_nStatus = STATUS_TABLE_OPEN;
	IProcess::OpenCard(this);

	this->startOpenTimer(Configure::getInstance().open_time);
	this->timeout_time = time(NULL);
}

bool Table::allPlayerOpen()
{
	int i;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = this->player_array[i];
		if(player && player->isActive())
		{
			if(!player->m_bhasOpen)
				return false;
		}
	}
	return true;
}

void Table::GameOver()
{
	this->stopOpenTimer();
	this->m_nStatus = STATUS_TABLE_OVER;
	this->setEndTime(time(NULL));
	int i;
	calcWinCoin();
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = this->player_array[i];
		if(player && player->isActive())
		{
			player->m_nStatus = STATUS_PLAYER_OVER;
			player->m_nStartNum++;
		}
	}
	IProcess::GameOver(this);

	for (i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = this->player_array[i];
		if (player)
		{
			this->startKickTimer(Configure::getInstance().kicktime, player);
		}
	}
	this->timeout_time = time(NULL);
}

void Table::calcWinCoin()
{
	Player* winner = NULL;
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = this->player_array[i];
		if(player && player->isActive())
		{
			if (winner == NULL)
			{
				winner = player;
				continue;
			}
			if (this->m_GameLogic.CompareCard(player->m_byHandCardData, winner->m_byHandCardData, CARD_MAX,
				player->m_bCardType > 0 ? 1 : 0, winner->m_bCardType > 0 ? 1 : 0, this->m_nType == 4 ? true : false))
			{
				winner = player;
			}
		}
	}
	if (winner == NULL)
	{
		LOGGER(E_LOG_ERROR) << "all player is NULL!";
		return;
	}
	//bool isOx = m_GameLogic.GetOxCard(winner->m_byHandCardData, CARD_MAX); //是否为牛牛
	int mul = m_GameLogic.GetTimes(winner->m_byHandCardData, CARD_MAX);
	//if (isOx)
	{
	//	mul = 2;
	}
	int64_t outmoney = this->m_nAnte * mul;
	int64_t winMoney = 0;
	for (int i = 0; i < GAME_PLAYER; i++)
	{
		Player* player = this->player_array[i];
		if (player && player->isActive())
		{
			if (player == winner)
			{
				continue;
			}
			player->m_lFinallGetCoin = -outmoney;
			winMoney += outmoney;
		}
	}
	GameUtil::CalcSysWinMoney(winMoney, this->m_nTax, winner->m_nTax);

	for (int i = 0; i < GAME_PLAYER; i++)
	{
		Player* player = this->player_array[i];
		if (player && player->isActive())
		{
			if (player == winner)
			{
				player->m_lMoney += winMoney;
				player->m_lFinallGetCoin += winMoney;
				continue;
			}
			player->m_lMoney += player->m_lFinallGetCoin;
		}
	}
}

void Table::setKickTimer(Player *p)
{
	if(this->isActive() /*|| this->m_nCountPlayer <= 1*/)
		return;

	this->startKickTimer(Configure::getInstance().ready_time, p);
	this->lockTable();
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		Player* player = this->player_array[i];
		if(player && (!player->isActive() || !player->isReady())&& player->m_nStartNum == 1)
		{
			IProcess::serverWarnPlayerKick(this, player, Configure::getInstance().kicktime - 2);
		}
	}
	this->unlockTable();
}


void Table::startKickTimer(int timeout, Player *p)
{
	_LOG_DEBUG_("=========[startKickTimer]\n");
	//timer.startKickTimer(timeout);
	p->readyTimer.init(this);
	p->readyTimer.p = p;
	p->readyTimer.startKickTimer(timeout);
}

void Table::stopKickTimer(Player *p)
{
	_LOG_DEBUG_("=========[stopKickTimer]\n");
	p->readyTimer.stopKickTimer();
}

void Table::startOpenTimer(int timeout)
{
	_LOG_DEBUG_("=========[startOpenTimer]\n");
	timer.startOpenTimer(timeout);
}

void Table::stopOpenTimer()
{
	_LOG_DEBUG_("=========[stopOpenTimer]\n");
	timer.stopOpenTimer();
}

void Table::startResetTimer(int timeout)
{
	timer.startResetTimer(timeout);
}

void Table::stopResetTimer()
{
	timer.stopResetTimer();
}


