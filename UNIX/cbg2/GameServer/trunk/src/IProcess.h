#ifndef __IProcess_H__
#define __IProcess_H__
#include <string>
#include <map>
#include <json/json.h>
#include "CDLSocketHandler.h"
#include "Packet.h"
#include "GameCmd.h"
#include "Table.h"

#define _NOTUSED(V) ((void) V)

class IProcess
{
	//¾²Ì¬º¯Êý
	public:

		static int GameStart(Table* table);
		static int OpenCard(Table* table);
		static int GameOver(Table* table);
		static int sendToRobotCard(Player* player, Table* table);

		static int sendCallInfo(Table* table, Player *player,BYTE isLastOne,short seq = 0);
		static int sendSetMulInfo(Table* table, Player *player, short seq = 0);
		static int sendOpenCardInfo(Table* table, Player *player, BYTE hasbull, BYTE card1, BYTE card2, BYTE card3, BYTE card4, BYTE card5,short seq = 0);

		static int serverPushLeaveInfo(Table* table, Player* leavePlayer, short retno = 1);
		static int serverWarnPlayerKick(Table* table, Player* warnner, short timeCount);
		static int serverComTask(Table* table, Player* player);
		static int updateDB_UserCoin(Player* player, Table* table);
		static int updateDB_UserRoll(Player* player, Table* table, int nroll, bool isboardtask = false);
		static int UpdateDBActiveTime(Player* player);	
};

#endif 
