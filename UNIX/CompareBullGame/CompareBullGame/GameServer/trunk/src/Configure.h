#ifndef _Configure_H
#define _Configure_H 

#include <time.h>
#include <string>
#include "Typedef.h"
#include "GameConfig.h"

//Temp solution.
#define LEVEL_1   (1)
#define LEVEL_2   (2)
#define LEVEL_3   (3)
#define LEVEL_4   (4)

#define MAX_ALLOC_NUM  8

typedef struct _checkHost
{
	char host[32];
	short port;
}checkHost;

class Configure : public GameConfig
{
public:
	Singleton(Configure);

	virtual bool LoadGameConfig();

//***********************************************************
public:
	char alloc_ip[64];
	short alloc_port;
	short numplayer;

    int monitor_time;//监控时间
    int keeplive_time;//存活时间
	short ready_time;
	short call_time;
	short setmul_time;
	short open_time;
	short loserate;
	uint16_t compare_time;	//开牌后的比较时间
	
	//控制进入桌子
	short contrllevel;
	//帮助开牌
	short helptype;

	int max_table;
	int max_user;
	//下注的超时时间
	int betcointime;
	//大于两个人准备其中有人没有准备，然后倒计时把没准备的人踢出，并且游戏开始
	int tablestarttime;
	//超时踢出没准备用户
	short kicktime;
	short reset_time;

	time_t starttime; //服务器启动时间
	time_t lasttime;  //最近活动时间

	int robotwin;	//机器人库存下限

	short robotTabNum1;
	short robotTabNum2;
	short robotTabNum3;

	short specialnum0;
	short specialnum1;
	short specialnum2;
	short specialnum3;
	short specialnum4;
	short specialnum5;
	short specialnum6;
	short bankernum;
	short rewardRate;

	int rewardcoin;
	int rewardroll;
	//机器人进入排行榜的配置
	short switchrobotrank;

	//简单任务的发放局数条件
	short esayTaskCount;
	short esayRandNum;

	//当天前n局
	short curBeforeCount;
	//当天前N局游戏获取简单任务的概率
	short esayTaskProbability;

	//赢取金币小喇叭发送
	int wincoin1;
	int wincoin2;
	int wincoin3;
	int wincoin4;
};

#endif
