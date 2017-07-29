#ifndef PLAYER_H
#define PLAYER_H

#include <string.h>
#include <time.h>
#include <stdint.h>
#include "Typedef.h"
#include "TaskManager.h"
#include "CDL_Timer_Handler.h"
#include "TableTimer.h"
using namespace std;

#define CARD_MAX 5

#define INVAILD_SEAT_ID			-1

#define STATUS_PLAYER_LOGOUT 0
#define STATUS_PLAYER_LOGIN 1   //登陆，未入座
#define STATUS_PLAYER_COMING 2  //入座
#define STATUS_PLAYER_RSTART 3  //入座，准备开始
#define STATUS_PLAYER_ACTIVE 4  //正在打牌
#define STATUS_PLAYER_OVER 5    //游戏结束

#define PLAYER_CALL_BANKER	1
#define PLAYER_NOT_CALL		2


typedef struct _CoinCfg
{
	short level;	//等级
	int maxmoney;	//进入的最大金币
	int minmoney;	//进入的最小金币
	int ante;		//底注
	int tax;		//台费
	short rateante;	//按最小带入金币的百分几作为底注
	short ratetax;	//固定比率按低注的百分之几算台费
	int helpcoin;	//提示用金币数额
	int roundnum;	//局数任务各个场次完成获得宝箱
	int coinhigh1;	//第一获取最大金币数额
	int coinlow1;	//第一获取最小金币数额
	int coinhigh2;	//第二获取最大金币数额
	int coinlow2;	//第二获取最小金币数额
	int coinhigh3;	//第三获取最大金币数额
	int coinlow3;	//第三获取最小金币数额
	BYTE rewardtype;//局数任务奖励类型
	int rollhigh1;	//第一阶段局数任务获取最大金币数额
	int rolllow1;	//第一阶段局数任务获取最小金币数额
	int rollhigh2;	//第二阶段局数任务获取最大金币数额
	int rolllow2;	//第二阶段局数任务获取最小金币数额
	int rollhigh3;	//第三阶段局数任务获取最大金币数额
	int rolllow3;	//第三阶段局数任务获取最小金币数额
	int magiccoin;	//魔法表情扣除金币
	int maxmul;		//最大倍数
	int roundconf;			//新版局数任务的配置
	BYTE timecount;			//新版时间任务的总个数
	short timearray[8];		//新版时间任务的数值
	int rewardtime[8];		//新版时间任务的奖励
	BYTE coincount;			//新版金币任务总个数
	int cointaskarray[8];	//新版金币任务的数值
	short rewardcoin[8];	//新版金币任务的奖励
	_CoinCfg():level(0), maxmoney(0), minmoney(0), ante(100),tax(100), rateante(2),ratetax(50),helpcoin(50), roundnum(20),
		coinhigh1(10),coinlow1(0),coinhigh2(0),coinlow2(0),coinhigh3(0),coinlow3(0),rewardtype(1),rollhigh1(10),
		rolllow1(0),rollhigh2(0),rolllow2(0),rollhigh3(0),rolllow3(0),magiccoin(20), roundconf(0),timecount(0),coincount(0)
	{
		memset(timearray, 0, sizeof(timearray));
		memset(rewardtime, 0, sizeof(rewardtime));
		memset(cointaskarray, 0, sizeof(cointaskarray));
		memset(rewardcoin, 0, sizeof(rewardcoin));
	}
}CoinCfgInfo;

class Player:public CCTimer
{
	public:
		Player(){};
		virtual ~Player(){};
		void init();
	//内置函数
	public:
		inline bool isLogout(){ return m_nStatus == STATUS_PLAYER_LOGOUT; }
		inline bool isLogin(){ return m_nStatus == STATUS_PLAYER_LOGIN; }
		inline bool isComming(){ return m_nStatus == STATUS_PLAYER_COMING; }
		inline bool isReady(){ return m_nStatus == STATUS_PLAYER_RSTART; }
		inline bool isActive(){ return m_nStatus == STATUS_PLAYER_ACTIVE; }
		inline bool isGameOver(){ return m_nStatus == STATUS_PLAYER_OVER; }
		inline void setActiveTime(time_t t){active_time = t;}
        inline time_t getActiveTime(){return active_time;}
		inline void setEnterTime(time_t t){enter_time = t;}
		inline time_t getEnterTime(){return enter_time;}
		inline void setRePlayeTime(int t){replay_time = t;}
		inline int getRePlayeTime(){return replay_time;}
		inline void setTimeOutTime(int t){timeout_time = t;}
		inline int getTimeOutTime(){return timeout_time;}
	//行为函数
	public:
		void leave(bool isSendToUser = true);
		void enter();
		void login();
		void reset();
		bool isAllowMul(int mul);
		bool checkMoney(short &result);
		bool isBigestMul();
		short getPlayCount(short &comPlayCount, short &level);
		int getCoinTask(int &maxcoin, short &index);
		void setStartTime();
		void getTimeTask();
	public:
		virtual int ProcessOnTimerOut();

	public:
		BYTE m_byHandCardData[CARD_MAX];     //手牌

	public:
		int id;
		char name[64];
		char json[1024];
		char headlink[128];
		short m_nHallid;
		short m_nStatus;
		short m_nTabIndex;
		int64_t m_lMoney;
		int64_t m_lSafeMoney;
		int64_t m_lFinallGetCoin;
		short tid;
		short source;
		short m_nSeatID;
		int m_nWin;
		int m_nLose;
		int m_nRunAway;
		int m_nTie;
		BYTE m_bisCall;								//这盘是否抢了庄
		short m_nMul;
		int m_nMulArray[4];
		bool isonline;
		bool m_bhasOpen;
		BYTE m_bCardType;
		short m_nStartNum;							//准备局数处理，第一次玩的踢人动作
		int m_nRoll;
		int m_nExp;
		bool m_bhasCard;

		int m_nTax;									//消耗的台费

		char m_byIP[32];                            //IP
		TableTimer readyTimer;
	public:
		bool m_bHasUserHelp;
		int m_nUseHelpCoin;							//提示使用的金币数额
	
													
													//任务
	public:
		Task* m_pTask;
		short m_nGetRoll;
		bool m_bCompleteTask;
		///////////////////////////////局数任务/////////////////////////////////////////////////
		int m_nRoundComFlag;						//局数任务完成的标志
		int m_nRoundNum;							//局数的数量用于判断是否完成局数任务
		BYTE m_bFinalComFlag;						//游戏结束完成局数任务的标志 0 没有完成 1 完成 2所有都完成
		int m_nComGetCoin;							//完成获得的金币数
		int m_nComGetRoll;							//完成获得的乐劵数

		short m_nMagicNum;							//魔法表情使用的次数
		int m_nMagicCoinCount;						//魔法表情消耗的总金币数
		bool isbacklist;
	//新版本的任务
	public:
		short m_nTaskFlag;							//新版任务的标志
		int m_nTimeTask;							//新版时间任务的完成标志
		int m_nStartTime;							//此用户开始游戏的时刻，方便后面离开的时候计时
		short m_nComTimeTaskFlag;					//完成时间任务的标志
		short m_nLeftTimeCom;						//还剩余多少时间完成
		int m_nRewardTimeCoin;						//奖励时间任务的金币
		int m_nBoradTask;							//新版局数任务完成标志 11111111(完成标志) 11111111 1111111 11111111
		int64_t m_lCoinTask;						//新版游戏金币的累计完成的标志
	public:	
		bool m_bisCallTimeOut;
		bool m_bisSetMulTimeOut;
		bool m_bisOpenTimeOut;
	public:
		CoinCfgInfo coincfg;
	public:
		short pid;
		short cid;
		short sid;
	private:
		time_t active_time;							//客户端最近活跃时间
		time_t enter_time;							//进入游戏的时间
		time_t replay_time;							//游戏等待时间，新一盘游戏开始时计时，用于机器人判断当前等待时间
		time_t timeout_time;						//游戏给他设置超时时间的时刻

};
#endif
