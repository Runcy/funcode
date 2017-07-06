#ifndef CoinConf_H
#define CoinConf_H

#include "BaseCoinConf.h"
#include "Player.h"

class CoinConf : public BaseCoinConf
{
public:
	static CoinConf* getInstance();
	virtual ~CoinConf();
	virtual bool GetCoinConfigData();

	bool isUserInBlackList(int uid);
	CoinCfgInfo *getCoinCfg() { return &coinCfg; }

private:
	CoinCfgInfo coinCfg;
	CoinConf();
};

#endif

