#include "GetIdleTableProc.h"
#include "ProtocolServerId.h"
#include "Room.h"
#include "Logger.h"
#include "ProcessManager.h"

REGISTER_PROCESS(ADMIN_GET_IDLE_TABLE, GetIdleTableProc)

int GetIdleTableProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )
{
	short cmd = inputPacket->GetCmdType();

	_LOG_DEBUG_("==>[GetIdleTableProc] [0x%04x] \n", cmd);
	Room* room = Room::getInstance();

	if(room->getStatus()==-1)
	{
		return sendErrorMsg(clientHandler, cmd, 0, -1, "room.Status=-1, Server Wait For Stop") ;
	}

	int total= 64; //总记录数
	Table* tables = room->getAllTables();

	Table* tableList[64];
	int size = 0;
	for(int i=0; i<total; ++i )
	{
		Table* table = &tables[i];

		if(Configure::getInstance().m_nLevel == LEVEL_1)
		{
			if(table->isEmpty() && table->id < Configure::getInstance().robotTabNum1)
			{
				if(size < 8)
					tableList[size++] = table;
			}
		}
		else if (Configure::getInstance().m_nLevel == LEVEL_2)
		{
			if(table->isEmpty() && table->id < Configure::getInstance().robotTabNum2)
			{
				if(size < 8)
					tableList[size++] = table;
			}
		}
		else if (Configure::getInstance().m_nLevel == LEVEL_3)
		{
			if(table->isEmpty() && table->id < Configure::getInstance().robotTabNum3)
			{
				if(size < 8)
					tableList[size++] = table;
			}
		}

		if(table->isNotFull() && !table->isEmpty())
		{
			for(int j = 0; j < GAME_PLAYER; ++j)
			{
				if(table->player_array[j] && table->player_array[j]->isonline && table->player_array[j]->source != 30)
				{
					table->m_nRePlayTime = table->player_array[j]->getRePlayeTime();
					tableList[size++] = table;
					break;
				}
			}
		}
		if(table->hasOnePlayer())
		{
			for(int j = 0; j < GAME_PLAYER; ++j)
			{
				if(table->player_array[j] && table->player_array[j]->source == 30)
				{
					table->m_nRePlayTime = table->player_array[j]->getRePlayeTime();
					tableList[size++] = table;
					break;
				}
			}
		}
	}

	OutputPacket response;
	response.Begin(ADMIN_GET_IDLE_TABLE);
	response.WriteShort(0);
	response.WriteString("");

	response.WriteInt(time(NULL));
	response.WriteShort(size);
	
	for(int i=0; i<size; ++i )
	{
		Table* table = tableList[i];
		int svid = Configure::getInstance().m_nServerId;
		int tid = (svid << 16)|table->id;
		response.WriteInt(tid);
		response.WriteShort(table->m_nStatus);
		response.WriteShort(Configure::getInstance().m_nLevel);
		response.WriteShort(table->m_nCountPlayer);
		response.WriteInt(table->m_nRePlayTime);
	}
	response.End();

	_LOG_DEBUG_("<==Free Table Count=[%d] \n", size);
	this->send(clientHandler,&response);
	return 0;
}

int GetIdleTableProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )
{
	_NOTUSED(clientHandler);
	_NOTUSED(inputPacket);
	_NOTUSED(pt);
	return 0;
}
