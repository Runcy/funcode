#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "TaskManager.h"
#include "Logger.h"
#include "hiredis.h"
#include "Configure.h"
#include "Table.h"
#include "Util.h"

using namespace std;

static TaskManager * Instance = NULL;

static vector<string> split(const string& src,const string& tok, bool trim , string null_subst );


TaskManager::TaskManager()
{
	countTask = 0;
	index = 0;
	memset(taskarry, 0, sizeof(taskarry));
	memset(taskEsay, 0, sizeof(taskEsay));
	countEasy = 0;
}

TaskManager * TaskManager::getInstance()
{
	if(Instance==NULL)
		Instance  = new TaskManager();
	
	return Instance;
}

int TaskManager::init()
{
// 	if(loadTask()!=0)
// 		return -1;
	return 0;
}

int TaskManager::loadTask()
{
	redisContext *m_redis = NULL;  
    redisReply *reply = NULL;
	m_redis = NULL;// Util::initRedisContext(Configure::getInstance().redis_ip, Configure::getInstance().redis_port);
	if (m_redis == NULL)
	{
		return -1;
	}

	reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "HGETALL bulltask:%d",Configure::getInstance().m_nLevel));
    if(reply == NULL)
    {
         _LOG_ERROR_("Redis can't get bulltask:%d\n",Configure::getInstance().m_nLevel);
		 redisFree(m_redis);
		 return -1;
    }
	else
	{
		if (reply->type == REDIS_REPLY_ARRAY) 
		{
			if(reply->elements==0){
				_LOG_ERROR_("Can't get bulltask:%d,task maybe not add\n",Configure::getInstance().m_nLevel);
				freeReplyObject(reply);
				redisFree(m_redis);
				return -1; 
			}
			for (unsigned short j = 0; j < reply->elements; j++) 
			{
				if(reply->element[j]->type==REDIS_REPLY_NIL){
					freeReplyObject(reply);
					redisFree(m_redis);
					return -1; 
				}
				//printf("%s\n",reply->element[j]->str);
				if(!(j & 1))
				{
					taskarry[countTask].taskid = atol(reply->element[j]->str);
					//printf("taskarry[countTask].taskid:%d countTask:%d\n", taskarry[countTask].taskid, countTask);
				}
				else
				{
					vector<std::string> vlist = split(std::string(reply->element[j]->str), "|", true, "");
					if(vlist.size() == 3)
					{
						//printf("vlist[0]:%s vlist[1]:%s\n", vlist[0].c_str(), vlist[1].c_str());
						strncpy(taskarry[countTask].taskname,vlist[0].c_str(), sizeof(taskarry[countTask].taskname));
						taskarry[countTask].taskname[sizeof(taskarry[countTask].taskname) - 1] = '\0';
						taskarry[countTask].ningotlow = atoi(vlist[1].c_str());
						taskarry[countTask].ningothigh = atoi(vlist[2].c_str());
						//简单的任务列表
						if(taskarry[countTask].taskid == 1 || taskarry[countTask].taskid == 4 || taskarry[countTask].taskid == 16)
						{
							taskEsay[countEasy++] = &taskarry[countTask];
						}
						++countTask;
					}
				}
			}
			freeReplyObject(reply);
		}
		else
		{
			freeReplyObject(reply);
			redisFree(m_redis);
			return -1;
		}
	}

	redisFree(m_redis);
	for(int i = 0; i < countTask; ++i)
	{
		printf("%ld:%s|%d----%d\n", taskarry[i].taskid, taskarry[i].taskname, taskarry[i].ningotlow, taskarry[i].ningothigh);
	}
    return 0;
}

Task * TaskManager::getTask()
{
	if(countTask == 0)
		return NULL;
	if(index >= countTask)
		index = 0;
	return &taskarry[index++];
}

Task * TaskManager::getRandTask()
{
	if(countTask == 0)
		return NULL;
	return &taskarry[rand()%countTask];
}

Task * TaskManager::getRandEsayTask()
{
	if(countEasy == 0)
		return NULL;
	return taskEsay[rand()%countEasy];
}

bool TaskManager::comCallLand(Player* player, Table* table, short callland)
{
	if(callland != 0)
	{
		if(callland == 1)
		{
			if(player->m_bisCall == PLAYER_CALL_BANKER)
				return true;
			else
				return false;
		}
	}
	return false;
}

bool TaskManager::comSetMul(Player* player, Table* table, short setmul)
{
	if(setmul != 0)
	{
		if(setmul == 1)
		{
			if((player->id != table->m_nBanker) && player->isBigestMul())
				return true;
			else
				return false;
		}
		else
			return false;
	}
	return false;
}

bool TaskManager::comCardType(Player* player, Table* table, short cardtype)
{
	if(cardtype != 0)
	{
		//无牛，炸弹牛，五花牛，五小牛特殊处理
		if(cardtype == 0x0e && player->m_bCardType == 0)
			return true;
		if(cardtype == 0x0b && player->m_bCardType == OX_FOUR_SAME)
			return true;
		if(cardtype == 0x0c && player->m_bCardType == OX_FIVEKING)
			return true;
		if(cardtype == 0x0d && player->m_bCardType == OX_FIVESMALL)
			return true;
		if(cardtype == player->m_bCardType)
			return true;
		return false;
	}
	return false;
}

bool TaskManager::comBottomCard(Player* player, Table* table, short bottomtype)
{
	if(bottomtype != 0)
	{
		BYTE bSum = table->m_GameLogic.GetCardLogicValue(player->m_byHandCardData[0]) +
				table->m_GameLogic.GetCardLogicValue(player->m_byHandCardData[1]) + 
				table->m_GameLogic.GetCardLogicValue(player->m_byHandCardData[2]);

		_LOG_DEBUG_("bSum:%d\n", bSum);
		//三个牌是10的倍数特殊处理
		if(bottomtype == 10)
		{
			if(bSum % 10 == 0)
				return true;
		}

		if(bottomtype == (bSum % 10))
			return true;

		return false;
	}
	return false;
}

bool TaskManager::calcPlayerComplete(Player* player, Table* table)
{
	if(player == NULL || table == NULL)
		return false;
	if(player->m_pTask == NULL)
		return false;
	if(player->m_lFinallGetCoin < 0)
		return false;
	//前两位为只赢取本局
	short winflag = player->m_pTask->taskid & 3;
	//抢庄
	short callland = player->m_pTask->taskid >> 2 & 3;
	//选择倍数
	short setmul = player->m_pTask->taskid >> 4 & 7;
	//最终牌型
	short cardtype = player->m_pTask->taskid >> 7 & 0x0F;
	//底牌三张
	short bottomtype = player->m_pTask->taskid >> 11 & 0x0F;
	_LOG_DEBUG_("winflag:%d callland:%d setmul:%d cardtype:%d bottomtype:%d\n",
		winflag, callland, setmul, cardtype, bottomtype);

	bool bcall, bmul, btype, bbottom;
	bcall = bmul = btype = bbottom = false;

	bcall = comCallLand(player, table, callland);
	bmul = comSetMul(player, table, setmul);
	btype = comCardType(player, table, cardtype);
	bbottom = comBottomCard(player, table, bottomtype);

	_LOG_DEBUG_("bcall:%d bmul:%d btype:%d bbottom:%d\n", bcall?1:0, bmul?1:0, btype?1:0, bbottom?1:0);

	if(winflag != 0 && winflag != 1)
		return false;
	if(callland != 0 && !bcall)
		return false;
	if(setmul != 0 && !bmul)
		return false;
	if(cardtype != 0 && !btype)
		return false;
	if(bottomtype != 0 && !bbottom)
		return false;

	return true;
}

vector<string> split(const string& src,const string& tok, bool trim , string null_subst )
{
    vector<string> v;
    if( src.empty() || tok.empty() )
        return v;
    string::size_type pre_index = 0, index = 0, len = 0;
    while( (index = src.find_first_of(tok, pre_index)) != string::npos )
    {
         if( (len = index-pre_index)!=0 )
            v.push_back(src.substr(pre_index, len));
        else if(trim==false)
             v.push_back(null_subst);
         pre_index = index+1;
    }
    string endstr = src.substr(pre_index);
     if( trim==false ) v.push_back( endstr.empty()? null_subst:endstr );
     else if( !endstr.empty() ) v.push_back(endstr);
    return v;
}



