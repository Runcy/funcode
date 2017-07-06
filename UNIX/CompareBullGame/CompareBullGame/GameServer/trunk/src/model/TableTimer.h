#ifndef _TableTimer_H
#define _TableTimer_H

#include <time.h>
#include "CDL_Timer_Handler.h"

#define TABLE_KICK_TIMER		20
#define TABLE_OPEN_TIMER		23
#define TABLE_COMPARE_TIMER		24
#define RESET_TIMER				25

class Table;
class Player;
class TableTimer; 
class CTimer_Handler:public CCTimer
{
	public:
		virtual int ProcessOnTimerOut();
		void SetTimeEventObj(TableTimer * obj, int timeid, int uid = 0);
		void StartTimer(long interval)
		{
			CCTimer::StartTimer(interval*1000);					
		}  
		void StartMiTimer(long interval)
		{
			CCTimer::StartTimer(interval);					
		} 
	private:
		int timeid;
		int uid;
		TableTimer * handler;
};

class TableTimer
{
	public:
		TableTimer(){} ;
		virtual ~TableTimer() {};	
		void init(Table* table);
	//操作超时函数
	public:
		void stopAllTimer();

		void startKickTimer(int timeout);
		void stopKickTimer();
		void startOpenTimer(int timeout);
		void stopOpenTimer();
		void startResetTimer(int timeout);
		void stopResetTimer();
	//发送通知函数
	public:
		int sendBetTimeOut(Player* player, Table* table, Player* timeoutplayer,Player* nextplayer);

	public:
		Table* table;
		Player* p;
		int ProcessOnTimerOut(int Timerid, int uid);
		CTimer_Handler m_TableKickTimer;
		CTimer_Handler m_TableNormalTimer;
		CTimer_Handler m_ComingRobot;
	//超时回调函数
	private:
		int TableKickTimeOut();
		int OpenTimeOut();
		int ResetTimeOut();

		friend class CTimer_Handler;


		
};

#endif
