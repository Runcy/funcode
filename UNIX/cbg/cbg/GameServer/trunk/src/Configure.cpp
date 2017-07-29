#include <sys/stat.h> 
#include <signal.h> 
#include "Configure.h"
#include "Logger.h"
#include "Protocol.h"

#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <getopt.h>
#endif

Configure::Configure() : GameConfig(E_COMPALL_BULL_GAME_ID, E_MSG_SOURCE_SERVER, E_CONFIG_GAME)
{
	max_table = 0;
	max_user = 0;

	starttime = time(NULL);
	lasttime = starttime;
}

Configure::~Configure()
{
}

bool Configure::LoadGameConfig()
{
	//最大棋桌 最大人数
	max_table = ReadInt("Room" ,"MaxTable", 100);
	max_user  = ReadInt("Room" ,"MaxUser", 500);

	monitor_time = ReadInt("Room" ,"MonitorTime", 30);
	keeplive_time  = ReadInt("Room" ,"KeepliveTime", 100);
	ready_time  = ReadInt("Room" ,"ReadyTime", 10);
	call_time  = ReadInt("Room" ,"CallTime", 10);
	setmul_time  = ReadInt("Room" ,"SetMulTime", 10);
	open_time  = ReadInt("Room" ,"OpenTime", 10);
	compare_time = ReadInt("Room", "CompareTime", 5);
	kicktime = ReadInt("Room","KickTime",10);
	reset_time  = ReadInt("Room" ,"ResetTime", 2);
	wincoin1 = ReadInt("Room" ,"TrumptCoin1", 50000);
	wincoin2 = ReadInt("Room" ,"TrumptCoin2", 10000);
	wincoin3 = ReadInt("Room" ,"TrumptCoin3", 5000);
	wincoin4 = ReadInt("Room", "TrumptCoin4", 2000);
	loserate = ReadInt("Room" ,"LoseRate", 80);
	contrllevel = ReadInt("Room" ,"ContrlLevel", 1000);
	helptype = ReadInt("Room" ,"HelpType", 10);

	//匹配服务器
	char key[15];
	sprintf(key, "Alloc_%d", this->m_nLevel);
	std::string alloc_host =  ReadString(key,"Host","NULL");
	strcpy( alloc_ip, alloc_host.c_str() );
    alloc_port = ReadInt(key,"Port",0);
	numplayer = ReadInt(key,"NumPlayer",4);
	rewardcoin = ReadInt(key,"RewardCoin",1000000);
	rewardroll = ReadInt(key,"RewardRoll",1000000);
	specialnum0 = ReadInt(key,"SNum0",10);
	specialnum1 = ReadInt(key,"SNum1",20);
	specialnum2 = ReadInt(key,"SNum2",40);
	specialnum3 = ReadInt(key,"SNum3",70);
	specialnum4 = ReadInt(key,"SNum4",80);
	specialnum5 = ReadInt(key,"SNum5",90);
	specialnum6 = ReadInt(key,"SNum6",100);
	bankernum = ReadInt(key,"MaxMoneyBanker",60);
	rewardRate = ReadInt(key,"RewardRate",80);
	robotwin = ReadInt(key,"RobotWin",20000);

	esayTaskCount = ReadInt("EsayTask","PlayCount",30);
	esayRandNum = ReadInt("EsayTask","RandNum",60);

	curBeforeCount = ReadInt("EsayTask","curBeforeCount",10);
	esayTaskProbability = ReadInt("EsayTask","esayTaskProbability",80);

	robotTabNum1 = ReadInt("RobotTab","Num1",10);
	robotTabNum2 = ReadInt("RobotTab","Num2",10);
	robotTabNum3 = ReadInt("RobotTab","Num3",10);

	switchrobotrank = ReadInt("Switch","RobotRank",0);

	return true;
}
