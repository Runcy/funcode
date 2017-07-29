#ifndef _LogComingProc_H_
#define _LogComingProc_H_

#include "BaseProcess.h"

class Table;
class Player;

class LogComingProc :public BaseProcess
{
	public:
		LogComingProc();
		virtual ~LogComingProc();
		virtual int doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  ;
		virtual int doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )   ; 
	private:
		int sendTabePlayersInfo(Player* player, Table* table, short num, int comuid, short seq);
		int sendTableInfo(Player* player, Table* table);
};

#endif

