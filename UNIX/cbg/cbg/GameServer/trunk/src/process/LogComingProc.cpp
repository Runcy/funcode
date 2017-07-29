#include "LogComingProc.h"
#include "Logger.h"
#include "HallManager.h"
#include "Room.h"
#include "ErrorMsg.h"
#include "ProcessManager.h"
#include "GameCmd.h"
#include "GameApp.h"
#include "IProcess.h"
#include "BaseClientHandler.h"
#include "ProtocolServerId.h"

REGISTER_PROCESS(CLIENT_MSG_LOGINCOMING, LogComingProc)

struct Param
{
	int uid;
	int tid;
	short source;
	char name[64];
	char json[1024];
	char userip[32];
};

LogComingProc::LogComingProc()
{
	this->name = "LogComingProc";
}

LogComingProc::~LogComingProc()
{

} 

int LogComingProc::doRequest(CDLSocketHandler* clientHandler, InputPacket* pPacket, Context* pt )
{
	//_NOTUSED(pt);
	int cmd = pPacket->GetCmdType();
	short seq = pPacket->GetSeqNum();
	short source = pPacket->GetSource();
	int uid = pPacket->ReadInt();
	string name = pPacket->ReadString();
	int tid = pPacket->ReadInt();
	uint64_t m_lMoney = pPacket->ReadInt64();
	short level = pPacket->ReadShort();
	short svid = tid >> 16;
	short realTid = tid & 0x0000FFFF;
	string json = pPacket->ReadString();
	string userip = "";
	if(source != 30)
		userip = pPacket->ReadString();
	
	_LOG_INFO_("==>[LogComingProc]  cmd[0x%04x] uid[%d]\n", cmd, uid);
	_LOG_DEBUG_("[DATA Parse] tid=[%d] svid=[%d] reallTid[%d]\n", tid, svid, realTid);
	_LOG_DEBUG_("[DATA Parse] name=[%s] \n", name.c_str());
	_LOG_DEBUG_("[DATA Parse] m_lMoney=[%lu] \n", m_lMoney);
	_LOG_DEBUG_("[DATA Parse] level=[%d] \n", level);
	_LOG_DEBUG_("[DATA Parse] json=[%s] \n", json.c_str());
	_LOG_DEBUG_("[DATA Parse] userip=[%s] \n", userip.c_str());

	BaseClientHandler* hallhandler = reinterpret_cast<BaseClientHandler*> (clientHandler);

	OutputPacket response;
	response.Begin(cmd, uid);
	response.SetSeqNum(pt->seq);
	response.WriteInt(uid);
	response.WriteShort(Configure::getInstance().m_nServerId);
	response.End();
	//把当前包的序列号设置为当前进程的序列号
	//pPacket->SetSeqNum(pt->seq);
	//这里要把当前这个包的seq放到回调里面，方便继续给前端发送包
	pt->seq = seq;

	if (source == 30)
		goto __ROBOT_LOGIN__;
	
	if (MoneyServer()->Send(&response) >= 0)
	{
		_LOG_DEBUG_("Transfer request to Back_MoneyServer OK\n");
		struct Param* param = (struct Param *) malloc (sizeof(struct Param));;
		param->uid = uid;
		param->tid = tid;
		param->source = source;
		strncpy(param->name, name.c_str(),63);	
		param->name[63] = '\0';
		//strncpy(param->password, password.c_str(), 31);
		//param->password[31] = '\0';
		strncpy(param->json, json.c_str(), 1023);
		strncpy(param->userip, userip.c_str(),31);
		param->json[1023] = '\0';
		pt->data = param;
		return 1;
	} 
	else 
	{
		_LOG_ERROR_("==>[LoginComingProc] [0x%04x] uid=[%d] ERROR:[%s]\n", cmd, uid, "Send request to BackServer Error");
		return -1;
	}


__ROBOT_LOGIN__:

	Room* room = Room::getInstance();

	Player* player = room->getAvailablePlayer();
	if(player==NULL)
	{
		_LOG_ERROR_("[Room is full,no seat for player] \n" );
		return sendErrorMsg(hallhandler, cmd, uid, -27,ErrorMsg::getInstance()->getErrMsg(-27),seq);
	}

	player->id = uid;
	player->m_lMoney = m_lMoney;
	player->m_nHallid = hallhandler->hallid;
	strncpy(player->name, name.c_str(), sizeof(player->name) - 1);
	player->name[sizeof(player->name) - 1] = '\0';
	strcpy(player->json, json.c_str());
	player->m_nStatus = STATUS_PLAYER_COMING;
	player->source = source;
	Table* table = NULL;
	table = room->getTable(realTid);
	short result = 0;
	if (!player->checkMoney(result))
	{
		_LOG_ERROR_("UID[%d] not enough money m_lMoney[%d]\n", uid, player->m_lMoney);
		sendErrorMsg(hallhandler, cmd, uid, -4,ErrorMsg::getInstance()->getErrMsg(-4),seq);
		player->m_nStatus = STATUS_PLAYER_LOGOUT;
		player->tid = -1;
		player->m_nTabIndex = -1;
		return 0;
	}

	if (table == NULL)
	{
		_LOG_WARN_("[This Table[%d] is Full] \n", realTid );
		table = room->getAvailableTable();
		if(table == NULL)
		{
			_LOG_ERROR_("[Room is full, no table for player]\n", realTid );
			return sendErrorMsg(hallhandler, cmd, uid, -28,ErrorMsg::getInstance()->getErrMsg(-28),seq);
		}
	}

	_LOG_INFO_("[LogComingProc] UID=[%d] NorLogin OK ,m_nSeatID[%d]\n", player->id,player->m_nSeatID );

	player->login();
	if(table->playerComming(player) != 0)
	{
		_LOG_WARN_("Player[%d] Coming This table[%d] Error\n", player->id, table->id);
		//如果用户进入桌子失败，则重新给用户找一个桌子进入
		table = room->getAvailableTable();
		if(table!= NULL)
		{
			_LOG_WARN_("Trans Table[%d] Coming Error, Player[%d] Coming Other table[%d]\n", tid, player->id, table->id);
			if(table->playerComming(player) != 0)
				return sendErrorMsg(hallhandler, cmd, uid, -21,ErrorMsg::getInstance()->getErrMsg(-21),seq);
		}
		else
			return sendErrorMsg(hallhandler, cmd, uid, -28,ErrorMsg::getInstance()->getErrMsg(-28),seq);
	}

	player->setActiveTime(time(NULL));
	player->setRePlayeTime(time(NULL));
	player->setTimeOutTime(time(NULL));
	int i= 0;
	int sendnum = 0;
	for(i = 0; i < GAME_PLAYER; ++i)
	{
		if(sendnum == table->m_nCountPlayer)
			break;
		if(table->player_array[i])
		{
			sendTabePlayersInfo(table->player_array[i], table, table->m_nCountPlayer, player->id, seq);
			sendnum++;
		}
	}
	
	_LOG_DEBUG_("<==[LogComingProc] Push [0x%04x]\n", CLIENT_MSG_LOGINCOMING);
	_LOG_DEBUG_("[Data Response] push to uid=[%d]\n", player->id);
	_LOG_DEBUG_("[Data Response] push to m_nStatus=[%d]\n", player->m_nStatus);
	_LOG_DEBUG_("[Data Response] tid=[%d]\n", tid);
	_LOG_DEBUG_("[Data Response] tstatus=[%d]\n", table->m_nStatus);
	_LOG_DEBUG_("[Data Response] m_nSeatID=[%d]\n", player->m_nSeatID);
	_LOG_DEBUG_("[Data Response] comuid=[%d]\n", player->id);

	return 0;
}

int LogComingProc::sendTabePlayersInfo(Player* player, Table* table, short num, int comuid, short seq)
{
	int svid = Configure::getInstance().m_nServerId;
	int tid = svid << 16|table->id;
	OutputPacket response;
	response.Begin(CLIENT_MSG_LOGINCOMING, player->id);
	if(player->id == comuid)
		response.SetSeqNum(seq);
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
	int comflag = 0;
	int roundnum = 0;
	if(Configure::getInstance().m_nLevel == LEVEL_1)
	{
		comflag = player->m_nRoundComFlag & 0x0F;
		roundnum = player->m_nRoundNum & 0x00FF;
	}
	if(Configure::getInstance().m_nLevel == LEVEL_2)
	{
		comflag = (player->m_nRoundComFlag >> 4) & 0x0F;
		roundnum = (player->m_nRoundNum >> 8) & 0x00FF;
	}
	if(Configure::getInstance().m_nLevel == LEVEL_3)
	{
		comflag = (player->m_nRoundComFlag >> 8) & 0x0F;
		roundnum = (player->m_nRoundNum >> 16) & 0x00FF;
	}

	//_LOG_DEBUG_("comflag:%d roundnum:%d\n", comflag, roundnum);

	if(table->m_nRoundNum3 != 0)
	{
		//_LOG_DEBUG_("m_nRoundNum3:%d m_nRoundNum2:%d m_nRoundNum1:%d\n", table->m_nRoundNum3, table->m_nRoundNum2, table->m_nRoundNum1);
		//都完成了
		if(comflag & 4)
		{
			response.WriteByte(0);
			response.WriteByte(0);
		}
		//完成中间的
		else if (comflag & 2)
		{
			response.WriteByte(roundnum - (table->m_nRoundNum1 + table->m_nRoundNum2));
			response.WriteByte(table->m_nRoundNum3);
			if(table->m_bRewardType == 2)
			{
				if(player->m_nComGetRoll == 0)
					player->m_nComGetRoll = table->m_nGetRollLow3 + rand()%((table->m_nGetRollHigh3 - table->m_nGetRollLow3) <= 0 ? 1 : (table->m_nGetRollHigh3 - table->m_nGetRollLow3+1));
			}
			else
			{
				if(player->m_nComGetCoin == 0)
					player->m_nComGetCoin = table->m_nGetCoinLow3 + rand()%((table->m_nGetCoinHigh3 - table->m_nGetCoinLow3) <= 0 ? 1 : (table->m_nGetCoinHigh3 - table->m_nGetCoinLow3+1));
			}
		}
		//完成第一个局数
		else if (comflag & 1)
		{
			response.WriteByte(roundnum - table->m_nRoundNum1);
			response.WriteByte(table->m_nRoundNum2);
			if(table->m_bRewardType == 2)
			{
				if(player->m_nComGetRoll == 0)
					player->m_nComGetRoll = table->m_nGetRollLow2 + rand()%((table->m_nGetRollHigh2 - table->m_nGetRollLow2) <= 0 ? 1 : (table->m_nGetRollHigh2 - table->m_nGetRollLow2+1));
			}
			else
			{
				if(player->m_nComGetCoin == 0)
					player->m_nComGetCoin = table->m_nGetCoinLow2 + rand()%((table->m_nGetCoinHigh2 - table->m_nGetCoinLow2) <= 0 ? 1 : (table->m_nGetCoinHigh2 - table->m_nGetCoinLow2+1));
			}
		}
		//所有的都没有完成
		else
		{
			response.WriteByte(roundnum);
			response.WriteByte(table->m_nRoundNum1);
			if(table->m_bRewardType == 2)
			{
				if(player->m_nComGetRoll == 0)
					player->m_nComGetRoll = table->m_nGetRollLow1 + rand()%((table->m_nGetRollHigh1 - table->m_nGetRollLow1) <= 0 ? 1 : (table->m_nGetRollHigh1 - table->m_nGetRollLow1+1));
			}
			else
			{
				if(player->m_nComGetCoin == 0)
					player->m_nComGetCoin = table->m_nGetCoinLow1 + rand()%((table->m_nGetCoinHigh1 - table->m_nGetCoinLow1) <= 0 ? 1 : (table->m_nGetCoinHigh1 - table->m_nGetCoinLow1+1));
			}
		}
	}
	else if(table->m_nRoundNum2 != 0)
	{
		//完成中间的
		if (comflag & 2)
		{
			response.WriteByte(0);
			response.WriteByte(0);
		}
		//完成第一个局数
		else if (comflag & 1)
		{
			response.WriteByte(roundnum - table->m_nRoundNum1);
			response.WriteByte(table->m_nRoundNum2);
			if(table->m_bRewardType == 2)
			{
				if(player->m_nComGetRoll == 0)
					player->m_nComGetRoll = table->m_nGetRollLow2 + rand()%((table->m_nGetRollHigh2 - table->m_nGetRollLow2) <= 0 ? 1 : (table->m_nGetRollHigh2 - table->m_nGetRollLow2+1));
			}
			else
			{
				if(player->m_nComGetCoin == 0)
					player->m_nComGetCoin = table->m_nGetCoinLow2 + rand()%((table->m_nGetCoinHigh2 - table->m_nGetCoinLow2) <= 0 ? 1 : (table->m_nGetCoinHigh2 - table->m_nGetCoinLow2+1));
			}
		}
		//所有的都没有完成
		else
		{
			response.WriteByte(roundnum);
			response.WriteByte(table->m_nRoundNum1);
			if(table->m_bRewardType == 2)
			{
				if(player->m_nComGetRoll == 0)
					player->m_nComGetRoll = table->m_nGetRollLow1 + rand()%((table->m_nGetRollHigh1 - table->m_nGetRollLow1) <= 0 ? 1 : (table->m_nGetRollHigh1 - table->m_nGetRollLow1+1));
			}
			else
			{
				if(player->m_nComGetCoin == 0)
					player->m_nComGetCoin = table->m_nGetCoinLow1 + rand()%((table->m_nGetCoinHigh1 - table->m_nGetCoinLow1) <= 0 ? 1 : (table->m_nGetCoinHigh1 - table->m_nGetCoinLow1+1));
			}
		}
	}
	else if (table->m_nRoundNum1 != 0)
	{
		//完成第一个局数
		if (comflag & 1)
		{
			response.WriteByte(0);
			response.WriteByte(0);
		}
		//所有的都没有完成
		else
		{
			response.WriteByte(roundnum);
			response.WriteByte(table->m_nRoundNum1);
			if(table->m_bRewardType == 2)
			{
				if(player->m_nComGetRoll == 0)
					player->m_nComGetRoll = table->m_nGetRollLow1 + rand()%((table->m_nGetRollHigh1 - table->m_nGetRollLow1) <= 0 ? 1 : (table->m_nGetRollHigh1 - table->m_nGetRollLow1+1));
			}
			else
			{
				if(player->m_nComGetCoin == 0)
					player->m_nComGetCoin = table->m_nGetCoinLow1 + rand()%((table->m_nGetCoinHigh1 - table->m_nGetCoinLow1) <= 0 ? 1 : (table->m_nGetCoinHigh1 - table->m_nGetCoinLow1+1));
			}
		}
	}
	else
	{
		response.WriteByte(0);
		response.WriteByte(0);
	}
	response.WriteByte(table->m_bRewardType);
	if(table->m_bRewardType == 2)
	{
		response.WriteInt(player->m_nComGetRoll);
		response.WriteInt(Configure::getInstance().rewardroll);
	}
	else
	{
		response.WriteInt(player->m_nComGetCoin);
		response.WriteInt(Configure::getInstance().rewardcoin);
	}
	response.WriteInt(table->m_nAnte);
	response.End();

	_LOG_DEBUG_("uid:%d m_nComGetCoin:%d m_nComGetRoll:%d \n", player->id, player->m_nComGetCoin, player->m_nComGetRoll);

	if(HallManager::getInstance()->sendToHall(player->m_nHallid, &response, false) < 0)
		_LOG_ERROR_("[LoginComingProc] Send To Uid[%d] Error!\n", player->id);
	return 0;
}

int LogComingProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	_NOTUSED(clientHandler);
	if(pt==NULL)
	{
       _LOG_ERROR_("[LoginComingProc:doResponse]Context is NULL\n");	
	   return -1;
	}
	if(pt->client == NULL)
	{
		_LOG_ERROR_("[LoginComingProc:doResponse]Context is client NULL\n");	
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
	string json = string(param->json);

	short cmd = inputPacket->GetCmdType();
	short retno = inputPacket->ReadShort();
	string retmsg = inputPacket->ReadString();
	string userip = string(param->userip);

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
			sendErrorMsg(hallhandler, cmd, uid, -2,ErrorMsg::getInstance()->getErrMsg(-2),pt->seq);
			_LOG_WARN_("player[%d] has in this table[%d] can't coming other table[%d]\n", player->id, player->tid, realTid);
			return 0;
		}
	}
	
	if (player == NULL)
		player = room->getAvailablePlayer();
	if(player == NULL)
	{
		_LOG_ERROR_("[Room is full,no seat for player] \n" );
		return sendErrorMsg(hallhandler, cmd, uid, -27,ErrorMsg::getInstance()->getErrMsg(-27),pt->seq);
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
	player->m_nRoll = roll + roll1;
	player->m_nExp = exp;
	strcpy(player->m_byIP, userip.c_str());

	short result = 0;
	if(!player->checkMoney(result))
	{
		_LOG_ERROR_("UID[%d] not enough money m_lMoney[%d] result=[%d]\n", uid, player->m_lMoney, result);
		if(result == 0)
			sendErrorMsg(hallhandler, cmd, uid, -4,ErrorMsg::getInstance()->getErrMsg(-4),pt->seq);
		else
			sendErrorMsg(hallhandler, cmd, uid, -29,ErrorMsg::getInstance()->getErrMsg(-29),pt->seq);
		player->m_nStatus = STATUS_PLAYER_LOGOUT;
		player->tid = -1;
		player->m_nTabIndex = -1;
		return 0;
	}

	Table* table = room->getTable(realTid);
	if(table == NULL)
	{
		_LOG_WARN_("[This Table[%d] is Null] \n", realTid );
		table = room->getAvailableTable();
		if(table == NULL)
		{
			_LOG_ERROR_("[Room is full, no table[%d] for player]\n", realTid );
			return sendErrorMsg(hallhandler, cmd, uid, -27,ErrorMsg::getInstance()->getErrMsg(-27),pt->seq);
		}
	}
	player->login();
	
	bool iscan = true;
	//相同IP不能进入同一桌
	if(Configure::getInstance().m_nLevel >= Configure::getInstance().contrllevel)
	{
		for(int i = 0; i < GAME_PLAYER; ++i)
		{
			if(table->player_array[i] && table->player_array[i]->source != 30)
			{
				if(!iscan)
					break;
				if(strlen(player->m_byIP) < 2 || strlen(table->player_array[i]->m_byIP) < 2)
					break;

				if(strcmp(table->player_array[i]->m_byIP, player->m_byIP) == 0)
				{
					iscan = false;
					break;
				}
			}
		}

		if(!iscan)
		{
			table = room->getAvailableTable(table->id);
			if(table == NULL)
			{
				_LOG_ERROR_("[Room is full,!iscan no table[%d] for player]\n", realTid );
				return sendErrorMsg(hallhandler, cmd, uid, -27,ErrorMsg::getInstance()->getErrMsg(-27),pt->seq);
			}

			_LOG_WARN_("Trans Table[%d] !iscan Coming Error, Player[%d] Coming Other table[%d]\n", tid, player->id, table->id);
		}
	}
	player->setActiveTime(time(NULL));
	_LOG_INFO_("[LoginComingProc] UID=[%d] NorLogin OK ,seat_id[%d]\n", player->id,player->m_nSeatID );
	if(table->playerComming(player) != 0)
	{
		_LOG_WARN_("Player[%d] Coming This table[%d] Error\n", player->id, table->id);
		//如果用户进入桌子失败，则重新给用户找一个桌子进入
		table = room->getAvailableTable();
		if(table!= NULL)
		{
			_LOG_WARN_("Trans Table[%d] Coming Error, Player[%d] Coming Other table[%d]\n", tid, player->id, table->id);
			if(table->playerComming(player) != 0)
				return sendErrorMsg(hallhandler, cmd, uid, -28,ErrorMsg::getInstance()->getErrMsg(-28),pt->seq);
		}
		else
			return sendErrorMsg(hallhandler, cmd, uid, -27,ErrorMsg::getInstance()->getErrMsg(-27),pt->seq);
	}

	if(table == NULL)
		return sendErrorMsg(hallhandler, cmd, uid, -27,ErrorMsg::getInstance()->getErrMsg(-27),pt->seq);

	player->setRePlayeTime(time(NULL));
	player->setTimeOutTime(time(NULL));

	//table->startReadyTimer(player->id, Configure::getInstance().ready_time);
	OutputPacket response;
	response.Begin(ROUND_GET_INFO, player->id);
	response.WriteInt(player->id);
	response.WriteShort(table->id);
	response.End();
	if(player->source != 30)
	{
		if(RoundServer()->Send(&response) < 0)
			_LOG_ERROR_("[RoundServerConnect send Error uid[%d]]\n", player->id);
	}

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
	
	_LOG_DEBUG_("<==[LogComingProc] Push [0x%04x]\n", CLIENT_MSG_LOGINCOMING);
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

	if(!table->isLock())
		//设置踢出时间
		table->setKickTimer(player);
	else
	{
		if(player->m_nStartNum == 1)
			IProcess::serverWarnPlayerKick(table, player, Configure::getInstance().kicktime - (time(NULL)-table->KickTime));
	}
	return 0;
}

int LogComingProc::sendTableInfo(Player* player, Table* table)
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

