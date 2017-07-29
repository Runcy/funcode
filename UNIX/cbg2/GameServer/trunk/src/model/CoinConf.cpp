#include "CoinConf.h"
#include "Logger.h"
#include "Configure.h"
#include "hiredis.h"
#include "Player.h"
#include "Util.h"
#include <vector>
#include "StrFunc.h"
using namespace std;

static CoinConf* instance = NULL;

CoinConf* CoinConf::getInstance()
{
	if(instance==NULL)
	{
		instance = new CoinConf();
	}
	return instance;
}

CoinConf::CoinConf()
{
}

CoinConf::~CoinConf()
{
}

bool CoinConf::GetCoinConfigData()
{
	memset(&coinCfg, 0, sizeof(coinCfg));

	map<string, string> retVal;
	vector<string> fields = Util::explode("minmoney maxmoney ante tax rateante ratetax helpcoin roundnum coinhigh1 coinlow1 coinhigh2 coinlow2 coinhigh3 coinlow3 rewardtype rollhigh1 rolllow1 rollhigh2 rolllow2 rollhigh3 rolllow3 magiccoin maxmul", " ");
	bool bRet = m_Redis.HMGET(StrFormatA("BullFight_RoomConfig:%d", Configure::getInstance().m_nLevel).c_str(), fields, retVal);
	if (bRet)
	{
		for (size_t idx = 0, max = fields.size(); idx < max; ++idx)
		{
			switch (idx)
			{
			case 0: coinCfg.minmoney = atoi(retVal[fields[idx]].c_str()); break;
			case 1: coinCfg.maxmoney = atoi(retVal[fields[idx]].c_str()); break;
			case 2: coinCfg.ante = atoi(retVal[fields[idx]].c_str()); break;
			case 3: coinCfg.tax = atoi(retVal[fields[idx]].c_str()); break;
			case 4: coinCfg.rateante = atoi(retVal[fields[idx]].c_str()); break;
			case 5: coinCfg.ratetax = atoi(retVal[fields[idx]].c_str()); break;
			case 6: coinCfg.helpcoin = atoi(retVal[fields[idx]].c_str()); break;
			case 7: coinCfg.roundnum = atoi(retVal[fields[idx]].c_str()); break;
			case 8: coinCfg.coinhigh1 = atoi(retVal[fields[idx]].c_str()); break;
			case 9: coinCfg.coinlow1 = atoi(retVal[fields[idx]].c_str()); break;
			case 10: coinCfg.coinhigh2 = atoi(retVal[fields[idx]].c_str()); break;
			case 11: coinCfg.coinlow2 = atoi(retVal[fields[idx]].c_str()); break;
			case 12: coinCfg.coinhigh3 = atoi(retVal[fields[idx]].c_str()); break;
			case 13: coinCfg.coinlow3 = atoi(retVal[fields[idx]].c_str()); break;
			case 14: coinCfg.rewardtype = atoi(retVal[fields[idx]].c_str()); break;
			case 15: coinCfg.rollhigh1 = atoi(retVal[fields[idx]].c_str()); break;
			case 16: coinCfg.rolllow1 = atoi(retVal[fields[idx]].c_str()); break;
			case 17: coinCfg.rollhigh2 = atoi(retVal[fields[idx]].c_str()); break;
			case 18: coinCfg.rolllow2 = atoi(retVal[fields[idx]].c_str()); break;
			case 19: coinCfg.rollhigh3 = atoi(retVal[fields[idx]].c_str()); break;
			case 20: coinCfg.rolllow3 = atoi(retVal[fields[idx]].c_str()); break;
			case 21: coinCfg.magiccoin = atoi(retVal[fields[idx]].c_str()); break;
			case 22: coinCfg.maxmul = atoi(retVal[fields[idx]].c_str()); break;
			}
		}
	}

	_LOG_DEBUG_("level[%d], minmoney[%d] maxmoney[%d] ante[%d] tax[%d] rateante[%d] ratetax[%d] helpcoin[%d] roundnum[%d] coinhigh1[%d] coinlow1[%d] coinhigh2[%d] coinlow2[%d] coinhigh3[%d] coinlow3[%d] rewardtype[%d] rollhigh1[%d] rolllow1[%d] rollhigh2[%d] rolllow2[%d] rollhigh3[%d] rolllow3[%d] magiccoin[%d]\n",
		coinCfg.level, coinCfg.minmoney, coinCfg.maxmoney, coinCfg.ante, coinCfg.tax, coinCfg.rateante, coinCfg.ratetax, coinCfg.helpcoin,
		coinCfg.roundnum, coinCfg.coinhigh1, coinCfg.coinlow1, coinCfg.coinhigh2, coinCfg.coinlow2, coinCfg.coinhigh3, coinCfg.coinlow3,
		coinCfg.rewardtype, coinCfg.rollhigh1, coinCfg.rolllow1, coinCfg.rollhigh2, coinCfg.rolllow2, coinCfg.rollhigh3, coinCfg.rolllow3, coinCfg.magiccoin);

	return bRet;
}

bool CoinConf::isUserInBlackList(int uid)
{
	std::string strRet = "";
	bool bRet = m_Redis.HGET("ManyBlackList", StrFormatA("%d", uid).c_str(), strRet);
	if (bRet && !strRet.empty())
	{
		return true;
	}

	return false;
}
