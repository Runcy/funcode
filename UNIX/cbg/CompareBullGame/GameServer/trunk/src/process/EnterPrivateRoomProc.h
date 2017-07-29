#ifndef _EnterPrivateRoomProc_H_
#define _EnterPrivateRoomProc_H_

#include "BaseProcess.h"

class Table;
class Player;

class EnterPrivateRoomProc :public BaseProcess
{
	public:
		EnterPrivateRoomProc();
		virtual ~EnterPrivateRoomProc();
		virtual int doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )  ;
		virtual int doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket ,Context* pt )   ;

	private:
		int sendTabePlayersInfo(Player* player, Table* table, short num, int comuid, short seq);
		int sendTableInfo(Player* player, Table* table);
};

#endif

