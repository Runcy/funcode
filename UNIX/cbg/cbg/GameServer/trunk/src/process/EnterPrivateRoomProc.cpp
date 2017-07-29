#include <string>
#include "EnterPrivateRoomProc.h"
#include "HallManager.h"
#include "Logger.h"
#include "Room.h"
#include "Configure.h"
#include "ErrorMsg.h"
#include "ProcessManager.h"
#include "GameCmd.h"
#include "GameApp.h"
#include "json/json.h"
#include "BaseClientHandler.h"

using namespace std;

REGISTER_PROCESS(CLIENT_ENTER_PRIVATE, EnterPrivateRoomProc)

struct Param
{
	int uid;
	int tid;
	short source;
	char name[64];
	char password[64];
	char json[1024];
};

EnterPrivateRoomProc::EnterPrivateRoomProc()
{
	this->name = "EnterPrivateRoomProc" ;
}

EnterPrivateRoomProc::~EnterPrivateRoomProc()
{
}

int EnterPrivateRoomProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket,Context* pt)
{
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	string name = pPacket->ReadString();
	int tid = pPacket->ReadInt();
	uint64_t money = pPacket->ReadInt64();
	short level = pPacket->ReadShort();
	string password = pPacket->ReadString();
	string json = pPacket->ReadString();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	_LOG_INFO_("==>[EnterPrivateRoomProc]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d]\n", tid, svid, realTid);
	_LOG_DEBUG_("[DATA Parse] name=[%s] \n", name.c_str());
	_LOG_DEBUG_("[DATA Parse] money=[%lu] \n", money);
	_LOG_DEBUG_("[DATA Parse] level=[%d] \n", level);
	_LOG_DEBUG_("[DATA Parse] json=[%s] \n", json.c_str());

	if(level != Configure::getInstance().m_nLevel)
	{
		_LOG_ERROR_("level[%d] is Not equal configure level [%d]\n", level,  Configure::getInstance().m_nLevel);
		return sendErrorMsg(clientHandler, cmd, uid, -3,ErrorMsg::getInstance()->getErrMsg(-3),seq);
	}

	OutputPacket response;
	response.Begin(cmd, uid);
	response.SetSeqNum(pt->seq);
	response.WriteInt(uid);
	response.WriteShort(Configure::getInstance().m_nServerId);
	response.End();
	pt->seq = seq;

	if (MoneyServer()->Send(&response) > 0)
	{
		_LOG_DEBUG_("Transfer request to Back_MoneyServer OK\n");
		struct Param* param = (struct Param *) malloc (sizeof(struct Param));;
		param->uid = uid;
		param->tid = tid;
		param->source = source;
		strncpy(param->name, name.c_str(),63);	
		param->name[63] = '\0';
		strncpy(param->password, password.c_str(), 63);
		param->password[63] = '\0';
		strncpy(param->json, json.c_str(), 1023);
		param->json[1023] = '\0';
		pt->data = param;
		return 1;
	} 
	else 
	{
		_LOG_ERROR_("==>[EnterPrivateRoomProc] [0x%04x] uid=[%d] ERROR:[%s]\n", cmd, uid, "Send request to BackServer Error");
		return -1;
	}

	return 0;
}

int EnterPrivateRoomProc::sendTabePlayersInfo(Player* player, Table* table, short num, int comuid, short seq)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	OutputPacket response;
	if(player->id == comuid)
	{
		response.Begin(CLIENT_ENTER_PRIVATE, player->id);
		_LOG_DEBUG_("<==[EnterPrivateRoomProc] Push [0x%04x]\n", CLIENT_ENTER_PRIVATE);
		response.SetSeqNum(seq);
	}
	else
	{
		response.Begin(CLIENT_MSG_LOGINCOMING, player->id);
		_LOG_DEBUG_("<==[LoginComingProc] Push [0x%04x]\n", CLIENT_MSG_LOGINCOMING);
	}
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteInt(comuid);
	response.WriteShort(player->m_nSeatID);
	response.WriteByte(player->m_nTabIndex);
	response.WriteByte(num);
	int sendnum = 0;
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(sendnum == num)
			break;
		Player* otherplayer = table->player_array[i];
		if(otherplayer)
		{
			response.WriteInt(otherplayer->id);
			response.WriteString(otherplayer->name);
			response.WriteShort(otherplayer->m_nStatus);
			response.WriteByte(otherplayer->m_nTabIndex);
			response.WriteInt64(otherplayer->m_lMoney);
			response.WriteString(otherplayer->json);
			sendnum++;
		}
	}
	response.WriteByte(Configure::getInstance().ready_time);
	response.WriteInt(player->m_nRoll);
	response.WriteByte(0);
	response.WriteByte(0);
	response.WriteByte(0);
	response.WriteInt(0);
	response.WriteInt(0);
	response.WriteInt(table->m_nAnte);
	short complaycount = 0;
	short level = 0;
	short playcount = player->getPlayCount(complaycount, level);
	response.WriteByte(level);
	response.WriteByte(playcount);
	response.WriteByte(complaycount);
	int maxcoin = 0;
	short index = -1;
	short comflag = player->getCoinTask(maxcoin, index);
	response.WriteByte(comflag);//0为有任务没有完成 1为有任务完成没有领取  2为所有任务都领取了
	response.WriteByte(index);
	response.WriteInt(maxcoin);
	response.End();
	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[EnterPrivateRoomProc] Send To Uid[%d] Error!\n", player->id);
	else
		_LOG_DEBUG_("[EnterPrivateRoomProc] Send To Uid[%d] Success\n", player->id);
	return 0;
}

int EnterPrivateRoomProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket ,Context* pt )  
{
	_NOTUSED(clientHandler);
	if(pt==NULL)
	{
       _LOG_ERROR_("[EnterPrivateRoomProc:doResponse]Context is NULL\n");	
	   return -1;
	}
	if(pt->client == NULL)
	{
		_LOG_ERROR_("[EnterPrivateRoomProc:doResponse]Context is client NULL\n");	
	   return -1;
	}

	BaseClientHandler* hallhandler = reinterpret_cast<BaseClientHandler*> (pt->client);

	struct Param* param = (struct Param*)pt->data;
	int uid = param->uid;
	int tid = param->tid;
	short source = param->source;

	//short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	
	string name = string(param->name);
	string password = string(param->password);
	string json = string(param->json);

	short cmd = inputPacket->GetCmdType();
	short retno = inputPacket->ReadShort();
	string retmsg = inputPacket->ReadString();

	if (retno != 0)
	{
		_LOG_ERROR_("[%s:%s:%d]Select User Score Info Error[%d]:[%s]\n", __FILE__, __FUNCTION__, __LINE__, retno, retmsg.c_str());
		sendErrorMsg(hallhandler, cmd, uid, -1, (char*)retmsg.c_str(),pt->seq);
		return -1;
	}

	uid = inputPacket->ReadInt();
	int64_t money = inputPacket->ReadInt64();
	int64_t safemoney = inputPacket->ReadInt64();
	int roll = inputPacket->ReadInt();
	int roll1 = inputPacket->ReadInt();
	int coin = inputPacket->ReadInt();
	int exp = inputPacket->ReadInt();

	_LOG_DEBUG_("[DATA Parse] money=[%ld]\n", money);
	_LOG_DEBUG_("[DATA Parse] safemoney=[%ld]\n", safemoney);
	_LOG_DEBUG_("[DATA Parse] roll=[%d]\n", roll);
	_LOG_DEBUG_("[DATA Parse] roll1=[%d]\n", roll1);
	_LOG_DEBUG_("[DATA Parse] coin=[%d]\n", coin);
	_LOG_DEBUG_("[DATA Parse] exp=[%d]\n", exp);

	Room* room = Room::getInstance();

	Player* player = room->getPlayerUID(uid);

	if(player)
	{
		Table* table = room->getTable(player->tid);
		if(table)
		{
			sendTabePlayersInfo(player, table, table->m_nCountPlayer, player->id, pt->seq);
			_LOG_WARN_("player[%d] has in this table[%d] can't coming other table[%d]\n", player->id, player->tid, realTid);
			return 0;
		}
	}

	if(player == NULL)
		player = room->getAvailablePlayer();
	if(player==NULL)
	{
		_LOG_ERROR_("[Room is full,no seat for player] \n" );
		return sendErrorMsg(hallhandler, cmd, uid, -1,ErrorMsg::getInstance()->getErrMsg(-1),pt->seq);
	}
	

	player->id = uid;
	player->m_lMoney = money;
	player->m_lSafeMoney = safemoney;
	player->m_nHallid = hallhandler->hallid;
	strncpy(player->name, name.c_str(), sizeof(player->name) - 1);
	player->name[sizeof(player->name) - 1] = '\0';
	strcpy(player->json, json.c_str());
	player->m_nStatus = STATUS_PLAYER_COMING;
	player->source = source;
	player->m_nRoll = roll;
	player->m_nExp = exp;
	Table* table = NULL;
	table = room->getTable(realTid);
	player->login();
	short result = 0;
	if(!player->checkMoney(result))
	{
		_LOG_ERROR_("UID[%d] not enough money m_lMoney[%d]\n", uid, player->m_lMoney);
		sendErrorMsg(hallhandler, cmd, uid, -4,ErrorMsg::getInstance()->getErrMsg(-4),pt->seq);
		player->leave();
		return 0;
	}

	if(table == NULL)
	{
		_LOG_ERROR_("[Room is full, no table for player uid[%d] realtid[%d]]\n", uid, realTid );
		return sendErrorMsg(hallhandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),pt->seq);
	}

	//处理当两个人都离开房间房间init了，再进入此房间已经不存在
	if(table->tableName[0] == '\0')
	{
		_LOG_ERROR_("[Private Room is Not exist uid[%d] realtid[%d]]\n", uid, realTid );
		return sendErrorMsg(hallhandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),pt->seq);
	}

	
	if(table->haspasswd && strcmp(table->getPassword(), password.c_str()) != 0)
	{
		_LOG_ERROR_("uid[%d] tid[%d] password is not correct\n", uid, realTid );
		return sendErrorMsg(hallhandler, cmd, uid, -5,ErrorMsg::getInstance()->getErrMsg(-5),pt->seq);
	}

	if(table->playerComming(player) != 0)
	{
		_LOG_WARN_("Trans Table[%d] realTid[%d] Coming Error, maybe is full Player[%d]\n", tid, realTid, player->id);
		return sendErrorMsg(hallhandler, cmd, uid, -21,ErrorMsg::getInstance()->getErrMsg(-21),pt->seq);
	}

	_LOG_INFO_("[EnterPrivateRoomProc] UID=[%d] NorLogin OK ,m_nSeatID[%d]\n", player->id,player->m_nSeatID );
	player->setActiveTime(time(NULL));
	int i= 0;
	int sendnum = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(sendnum == table->m_nCountPlayer)
			break;
		if(table->player_array[i])
		{
			sendTabePlayersInfo(table->player_array[i], table, table->m_nCountPlayer, player->id, pt->seq);
			sendnum++;
		}
	}

	_LOG_DEBUG_("<==[EnterPrivateRoomProc] Push [0x%04x]\n", CLIENT_ENTER_PRIVATE);
	_LOG_DEBUG_("[Data Response] push to uid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] push to m_nStatus=[%d]\n", player->m_nStatus);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] m_nSeatID=[%d]\n", player->m_nSeatID);
	_LOG_DEBUG_("[Data Response] comuid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] m_nAnte=[%d]\n", table->m_nAnte);


	/*if(table->isActive())
	{
		sendTableInfo(player, table);
	}*/
	return 0;
}


int EnterPrivateRoomProc::sendTableInfo(Player* player, Table* table)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	OutputPacket response;
	response.Begin(CLIENT_MSG_TABLEDET, player->id);
	response.WriteShort(0);
	response.WriteString("ok");
	response.WriteInt(player->id);
	response.WriteShort(player->m_nStatus);
	response.WriteInt(tid);
	response.WriteShort(table->m_nStatus);
	response.WriteInt(table->m_nBanker);
	short lefttime = 0;
	if(table->isCall())
	{
		lefttime = Configure::getInstance().call_time - (time(NULL) - table->timeout_time);		
	}
	if(table->isMul())
	{
		lefttime = Configure::getInstance().setmul_time - (time(NULL) - table->timeout_time);
	}

	if(table->isOpen())
	{
		lefttime = Configure::getInstance().open_time - (time(NULL) - table->timeout_time);
	}

	response.WriteByte(lefttime);
	response.WriteByte(player->m_byHandCardData[0]);
	response.WriteByte(player->m_byHandCardData[1]);
	response.WriteByte(player->m_byHandCardData[2]);
	if(table->isOpen())
	{
		response.WriteByte(player->m_byHandCardData[3]);
		response.WriteByte(player->m_byHandCardData[4]);
	}
	else
	{
		response.WriteByte(0);
		response.WriteByte(0);
	}
	response.WriteShort(1);
	response.WriteShort(1);
	response.WriteShort(1);
	response.WriteShort(1);
	response.WriteByte(table->m_nCountPlayer);
	for(int j = 0; j < GAME_PLAYER; ++j)
	{
		if(table->player_array[j])
		{
			response.WriteInt(table->player_array[j]->id);
			response.WriteShort(table->player_array[j]->m_nStatus);
			response.WriteByte(table->player_array[j]->m_nTabIndex);
			response.WriteString(table->player_array[j]->name);
			response.WriteInt64(table->player_array[j]->m_lMoney);
			response.WriteString(table->player_array[j]->json);
			response.WriteByte(table->player_array[j]->m_bisCall);
			response.WriteShort(table->player_array[j]->m_nMul);
			_LOG_DEBUG_("uid[%d] m_bhasOpen[%s]\n", table->player_array[j]->id, table->player_array[j]->m_bhasOpen ? "true" :"false");
			if(table->player_array[j]->m_bhasOpen)
			{
				response.WriteByte(1);
				response.WriteByte(table->player_array[j]->m_bCardType);
				response.WriteByte(table->player_array[j]->m_byHandCardData[0]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[1]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[2]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[3]);
				response.WriteByte(table->player_array[j]->m_byHandCardData[4]);
				
			}
			else
				response.WriteByte(0);
		}
	}
	response.WriteShort((player->m_pTask != NULL) ? player->m_nGetRoll : 0);
	response.WriteString((player->m_pTask != NULL) ? player->m_pTask->taskname : "");
	response.End();

	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[GetTableDetailProc] Send To Uid[%d] Error!\n", player->id);
	return 0;

}
