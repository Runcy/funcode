#ifndef _TABLE_H
#define _TABLE_H

#include <time.h>
#include "Player.h"
#include "GameLogic.h"
#include "Configure.h"
#include "TableTimer.h"

const int GAME_PLAYER = 5;			//5人斗牛

#define STATUS_TABLE_CLOSE	-1 //桌子关闭
#define STATUS_TABLE_EMPTY	 0 //桌子空
#define STATUS_TABLE_READY	 1 //桌子正在准备
#define STATUS_TABLE_CALL	2 //叫庄状态
#define STATUS_TABLE_MUL	3 //翻倍
#define STATUS_TABLE_OPEN	 4	//开牌状态
#define STATUS_TABLE_OVER	 5	//桌子游戏结束

#define GAMEOVER_TIME			1000



class Table
{
	public:
		Table();
		virtual ~Table();
		
		void init();
		void reset();
	//内置函数
	public:
		inline bool isClose(){return m_nStatus==STATUS_TABLE_CLOSE;};
		inline bool hasOnePlayer() {return m_nCountPlayer == 1;};
		inline bool hasTwoPlayer() {return m_nCountPlayer == 2;};
		inline bool isEmpty(){return m_nStatus==STATUS_TABLE_EMPTY || m_nCountPlayer == 0;};
		inline bool isNotFull() {return m_nCountPlayer < Configure::getInstance().numplayer;};
		inline bool isActive() {return this->m_nStatus == STATUS_TABLE_CALL || 
			this->m_nStatus == STATUS_TABLE_OPEN || 
			this->m_nStatus == STATUS_TABLE_MUL;};
		inline bool isCall() {return this->m_nStatus == STATUS_TABLE_CALL;};
		inline bool isMul() {return this->m_nStatus == STATUS_TABLE_MUL;};
		inline bool isOpen() {return this->m_nStatus == STATUS_TABLE_OPEN;};

		inline char* getPassword() { return password; }

		inline void setStartTime(time_t t){StartTime = t;}
		inline time_t getStartTime(){return StartTime;}
		inline void setEndTime(time_t t){EndTime = t;}
		inline time_t getEndTime(){return EndTime;}

		inline void setGameID(const char* id) {strcpy(GameID, id);}
        inline const char* getGameID() {return GameID;}

		inline void lockTable(){kicktimeflag = true; KickTime = time(NULL);} 
		inline void unlockTable(){kicktimeflag = false;}
		inline bool isLock(){return kicktimeflag;}
	//行为函数
	public:
		bool isAllReady();
		bool isUserInTab(int uid);
		Player* getPlayer(int uid);
		int playerComming(Player* player);
		//用户在桌子上找到一个座位
		int playerSeatInTab(Player* player);
		void playerLeave(int uid, bool senduser = true);
		void playerLeave(Player* player, bool senduser = true);
		void setSeatNULL(Player* player);
		int GetSpecialType();
		void GameStart();
		void setTableConf();
		void OpenCard();
		bool allPlayerOpen();
		void GameOver();
		void calcWinCoin();
		void setKickTimer(Player *p);
		bool haveRobot();
		bool isAllRobot();
	//时间函数
	public:	
		void startKickTimer(int timeout, Player *p);
		void stopKickTimer(Player *p);
		void startOpenTimer(int timeout);
		void stopOpenTimer();
		void startResetTimer(int timeout);
		void stopResetTimer();
	//游戏逻辑相关
	public:
		GameLogic m_GameLogic;						//游戏逻辑
		BYTE m_bTableCardArray[GAME_PLAYER][5];		//桌面扑克

	public:
		int id;
		short m_nType;
		short m_nStatus;;	//-1不可用 0 空 
		short m_nCountPlayer;
		int m_nBanker;		//庄家UID
		short m_nMul;
		int m_nRePlayTime;
		int m_nAnte;
		int m_nTax;       //百分比
		int m_nHelpCoin;
		int m_nMaxmul;		//最大下注倍数
		
		Player* player_array[GAME_PLAYER];

	public:
		short m_nRoundNum1;
		short m_nRoundNum2;
		short m_nRoundNum3;
		int m_nGetCoinHigh1;
		int m_nGetCoinLow1;
		int m_nGetCoinHigh2;
		int m_nGetCoinLow2;
		int m_nGetCoinHigh3;
		int m_nGetCoinLow3;
		BYTE m_bRewardType;
		int m_nGetRollHigh1;
		int m_nGetRollLow1;
		int m_nGetRollHigh2;
		int m_nGetRollLow2;
		int m_nGetRollHigh3;
		int m_nGetRollLow3;
		int m_nMagicCoin;							//魔法表情扣除金币

	

	//私人房信息
	public:
		int owner;
		bool haspasswd;
		char tableName[64];
		char password[64];

		//准备信息，处理第一次进入要倒计时准备的问题
	public:
		//当前桌子是否是在踢人倒计时
		bool kicktimeflag;
		time_t StartTime;//开始玩牌的时间
		time_t EndTime;//结束玩牌的时间
		time_t KickTime;
		time_t timeout_time;
	private:
		TableTimer timer ;
		char GameID[80];
};

#endif

