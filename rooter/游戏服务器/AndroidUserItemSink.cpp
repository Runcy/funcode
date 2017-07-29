#include "Stdafx.h"
#include "AndroidUserItemSink.h"

//////////////////////////////////////////////////////////////////////////

//辅助时间
#define TIME_LESS					2									//最少时间

//游戏时间
#define IDI_START_GAME				(IDI_ANDROID_ITEM_SINK+0)			//开始定时器
#define IDI_USER_ADD_SCORE			(IDI_ANDROID_ITEM_SINK+1)			//加注定时器
#define IDI_USER_COMPARE_CARD		(IDI_ANDROID_ITEM_SINK+2)			//选比牌用户定时器
#define IDI_USER_FINISH_FLASH		(IDI_ANDROID_ITEM_SINK+3)			///////

//时间标识
#define TIME_START_GAME				3									//开始定时器
#define TIME_USER_ADD_SCORE			3									//加注定时器
#define	TIME_USER_COMPARE_CARD		3									//比牌定时器
#define	TIME_USER_FINISH_FLASH		4									//////

#define IDI_ANDROID_ITEM_SINK		500//////////////////

//////////////////////////////////////////////////////////////////////////

//构造函数
CAndroidUserItemSink::CAndroidUserItemSink()
{
	//接口变量
	m_pIAndroidUserItem=NULL;

	//游戏变量
	WORD m_wCurrentUser=INVALID_CHAIR;							//当前用户
	WORD m_wBankerUser=INVALID_CHAIR;							//庄家用户/////////

	//用户状态
	ZeroMemory(m_cbPlayStatus,sizeof(m_cbPlayStatus));			//游戏状态
	ZeroMemory(m_lTableScore,sizeof(m_lTableScore));			//下注数目
	ZeroMemory(m_cbHandCardData,sizeof(m_cbHandCardData));		//桌面扑克///////////////
	m_cbCardType=0;												//牌型///////////////////
	m_bMingZhu=false;											//看牌动作///////////////

	//下注信息
	m_lMaxScore=0;							//封顶数目
	m_lCellScore=0;							//单元下注
	m_lCurrentTimes=1;						//当前倍数
	m_lUserMaxScore=0;						//最大分数

	return;
}

//析构函数
CAndroidUserItemSink::~CAndroidUserItemSink()
{
}

//接口查询
void * CAndroidUserItemSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IAndroidUserItemSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IAndroidUserItemSink,Guid,dwQueryVer);
	return NULL;
}

//初始接口
bool CAndroidUserItemSink::Initialization(IUnknownEx * pIUnknownEx)
{
	//查询接口
	////m_pIAndroidUserItem=GET_OBJECTPTR_INTERFACE(pIUnknownEx,IAndroidUserItem);
	m_pIAndroidUserItem=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAndroidUserItem);
	if (m_pIAndroidUserItem==NULL) return false;

	return true;
}

//重置接口
bool CAndroidUserItemSink::RepositionSink()
{
	//游戏变量
	WORD m_wCurrentUser=INVALID_CHAIR;							//当前用户
	WORD m_wBankerUser=INVALID_CHAIR;							//庄家用户/////////

	//用户状态
	ZeroMemory(m_cbPlayStatus,sizeof(m_cbPlayStatus));			//游戏状态
	ZeroMemory(m_lTableScore,sizeof(m_lTableScore));			//下注数目
	ZeroMemory(m_cbHandCardData,sizeof(m_cbHandCardData));		//桌面扑克///////////////
	m_cbCardType=0;												//牌型///////////////////
	m_bMingZhu=false;											//看牌动作///////////////

	//////下注信息
	m_lMaxScore=0;							//封顶数目
	m_lCellScore=0;							//单元下注
	m_lCurrentTimes=1;						//当前倍数
	m_lUserMaxScore=0;						//最大分数

	return true;
}

//时间消息
bool CAndroidUserItemSink::OnEventTimer(UINT nTimerID)
{
	switch (nTimerID)
	{
	case IDI_START_GAME:		//开始定时器
		{
			//发送准备
			m_pIAndroidUserItem->SendUserReady(NULL,0);

			return true;
		}
	case IDI_USER_ADD_SCORE:	//加注定时器
		{
			m_pIAndroidUserItem->KillGameTimer(IDI_USER_ADD_SCORE);

			ASSERT(m_pIAndroidUserItem->GetChairID()==m_wCurrentUser);
			if(m_pIAndroidUserItem->GetChairID()!=m_wCurrentUser)return false;
			//放弃
			//加注
			//比牌
			//开牌
			//跟注
			//看牌

			BYTE wTemp=0;
			LONGLONG lCurrentScore=0;
			WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
			LONGLONG lAndroidMaxScore=m_pIAndroidUserItem->GetMeUserItem()->GetUserScore();
			if(lAndroidMaxScore<m_lUserMaxScore)
				lAndroidMaxScore=m_lUserMaxScore;

			//其他用户数目
			BYTE bPlayerCount=0;
			for(BYTE i=0;i<GAME_PLAYER;i++)
			{
				////if(i==wMeChairID)continue;
				if(m_cbPlayStatus[i]==TRUE)
					bPlayerCount++;
			}
			if(bPlayerCount==0 || bPlayerCount>=GAME_PLAYER)
				bPlayerCount=4;


			if(m_cbPlayStatus[wMeChairID]==TRUE && m_lTableScore[wMeChairID]==0)	//其他闲家首次操作	//加注//跟注//看牌
			{

				//是否看牌 80%看牌
				if(!m_bMingZhu)
				{
					wTemp=rand()%10;
					if(wTemp<8)
					{
						m_bMingZhu=true;
						m_pIAndroidUserItem->SendSocketData(SUB_C_LOOK_CARD,NULL,0);

						return true;
					}
				}
				else	//单牌随机放弃 20%放弃
				{
					if(m_cbCardType==0)
					{
						m_pIAndroidUserItem->SendSocketData(SUB_C_GIVE_UP,NULL,0);

						return true;
					}
					else if(m_cbCardType==1)
					{
						wTemp=rand()%10;
						if(wTemp<2)
						{
							m_pIAndroidUserItem->SendSocketData(SUB_C_GIVE_UP,NULL,0);

							return true;
						}
					}
				}

				if(m_wBankerUser==wMeChairID && m_bMingZhu)
				{
					wTemp=rand()%10;

					if(m_cbCardType==1 && wTemp<5)
					{
						//(庄家和上家)比牌 
						lCurrentScore=m_lCurrentTimes*m_lCellScore*2*2;

						//发送消息
						CMD_C_AddScore AddScore;
						AddScore.wState=TRUE;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

						//查找上家
						WORD wMeChairID=m_pIAndroidUserItem->GetChairID();

						//构造变量
						CMD_C_CompareCard CompareCard;
						ZeroMemory(&CompareCard,sizeof(CompareCard));
						CompareCard.wCompareUser=wMeChairID;

						for (LONG i=wMeChairID-1;;i--)
						{
							if(i==-1)i=GAME_PLAYER-1;
							if(m_cbPlayStatus[i]==TRUE)
							{
								CompareCard.wCompareUser=(WORD)i;
								break;
							}
						}
		
						ASSERT(CompareCard.wCompareUser!=wMeChairID);

						//发送消息
						m_pIAndroidUserItem->SendSocketData(SUB_C_COMPARE_CARD,&CompareCard,sizeof(CompareCard));

						return true;
					}
				}

				//加注//跟注//(庄家和上家)比牌 不比
				ASSERT(m_lCurrentTimes<=10 && m_lCurrentTimes>=0);
				lCurrentScore=m_lCellScore*(m_lCurrentTimes+rand()%(11-m_lCurrentTimes));

				//明注加倍
				if(m_bMingZhu)lCurrentScore*=2;

				//发送消息
				CMD_C_AddScore AddScore;
				AddScore.wState=0;
				AddScore.lScore=lCurrentScore;
				m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

				return true;
			}
			else	//普通操作
			{
				if(!m_bMingZhu)	//暗牌	//加注//跟注//比牌
				{
					//暗牌 10%看牌
					wTemp=rand()%10;
					if(wTemp<1)
					{
						m_bMingZhu=true;
						m_pIAndroidUserItem->SendSocketData(SUB_C_LOOK_CARD,NULL,0);

						return true;
					}

					if(m_lTableScore[wMeChairID]< m_lMaxScore*2 && (4*bPlayerCount*m_lMaxScore+m_lCellScore+m_lTableScore[wMeChairID])<lAndroidMaxScore)	//0加注//1跟注 //暗牌下注小于[封顶数目*2]加注 大于比牌
					{
						ASSERT(m_lCurrentTimes<=10 && m_lCurrentTimes>=0);
						lCurrentScore=m_lCellScore*(m_lCurrentTimes+rand()%(11-m_lCurrentTimes));
						//发送消息
						CMD_C_AddScore AddScore;
						AddScore.wState=0;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

						return true;
					}
					else	//比牌
					{
						////lCurrentScore=(m_bMingZhu)?(m_lCurrentTimes*m_lCellScore*4):(m_lCurrentTimes*m_lCellScore*2);
						lCurrentScore=m_lCurrentTimes*m_lCellScore*2;

						//发送消息
						CMD_C_AddScore AddScore;
						AddScore.wState=1;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));
					
						//构造变量
						CMD_C_CompareCard CompareCard;
						ZeroMemory(&CompareCard,sizeof(CompareCard));


						BYTE bCount=0;
						for (WORD i=0;i<GAME_PLAYER;i++)if(m_cbPlayStatus[i]==TRUE)bCount++;

						if(bCount>2)
						{
							m_pIAndroidUserItem->SendSocketData(SUB_C_WAIT_COMPARE,&CompareCard,sizeof(CompareCard));
						}
						else
						{
							//查找上家
							for (LONG i=wMeChairID-1;;i--)
							{
								if(i==-1)i=GAME_PLAYER-1;
								if(m_cbPlayStatus[i]==TRUE && i!=wMeChairID)
								{
									CompareCard.wCompareUser=(WORD)i;
									break;
								}
							}

							m_pIAndroidUserItem->SendSocketData(SUB_C_COMPARE_CARD,&CompareCard,sizeof(CompareCard));
						}

						return true;
					}
				}
				else	//明牌	//加注//跟注//比赛//放弃
				{
					if(m_cbCardType==0)
					{
						m_pIAndroidUserItem->SendSocketData(SUB_C_GIVE_UP,NULL,0);

						return true;
					}
					else if(m_cbCardType==1 && m_lTableScore[wMeChairID]<=m_lMaxScore*5)	//单牌并且下注少于[封顶数目*5]随机放弃 20%放弃
					{
						wTemp=rand()%10;
						if(wTemp<1)
						{
							m_pIAndroidUserItem->SendSocketData(SUB_C_GIVE_UP,NULL,0);

							return true;
						}
					}


					//加注//跟注 //明牌下注小于[封顶数目*牌型]加注 大于比牌
					if((m_cbCardType<4 && m_lTableScore[wMeChairID]<(1 * 2 * m_lMaxScore * m_cbCardType) || 
						m_cbCardType==4 && m_lTableScore[wMeChairID]<(3 * 2 * m_lMaxScore * m_cbCardType) || 
						m_cbCardType>4 && m_lTableScore[wMeChairID]<(5 * 2 * m_lMaxScore * m_cbCardType)) && 
					////if(
						(4*bPlayerCount*m_lMaxScore+m_lCellScore+m_lTableScore[wMeChairID])<lAndroidMaxScore)
					{
						ASSERT(m_lCurrentTimes<=10 && m_lCurrentTimes>=0);
						//0加注//1跟注
						lCurrentScore=m_lCellScore*(m_lCurrentTimes+rand()%(11-m_lCurrentTimes));

						//明注加倍
						if(m_bMingZhu)lCurrentScore*=2;

						//发送消息
						CMD_C_AddScore AddScore;
						AddScore.wState=0;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

						return true;
					}
					else	//比牌
					{
						lCurrentScore=m_lCurrentTimes*m_lCellScore*4;

						//发送消息
						CMD_C_AddScore AddScore;
						AddScore.wState=1;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));
					
						//构造变量
						CMD_C_CompareCard CompareCard;
						ZeroMemory(&CompareCard,sizeof(CompareCard));


						BYTE bCount=0;
						for (WORD i=0;i<GAME_PLAYER;i++)if(m_cbPlayStatus[i]==TRUE)bCount++;

						if(bCount>2)
						{
							m_pIAndroidUserItem->SendSocketData(SUB_C_WAIT_COMPARE,&CompareCard,sizeof(CompareCard));
						}
						else
						{
							//查找上家
							for (LONG i=wMeChairID-1;;i--)
							{
								if(i==-1)i=GAME_PLAYER-1;
								if(m_cbPlayStatus[i]==TRUE && i!=wMeChairID)
								{
									CompareCard.wCompareUser=(WORD)i;
									break;
								}
							}

							m_pIAndroidUserItem->SendSocketData(SUB_C_COMPARE_CARD,&CompareCard,sizeof(CompareCard));
						}
			
						return true;
					}

					return false;
				}
			}

			return true;
		}
	case IDI_USER_COMPARE_CARD:	//选择定时器
		{
			//查找上家
			WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
			BYTE wTemp=5;

			//构造变量
			CMD_C_CompareCard CompareCard;
			ZeroMemory(&CompareCard,sizeof(CompareCard));
			CompareCard.wCompareUser=wMeChairID;

			////for (LONG i=wMeChairID-1;;i--)
			////{
			////	if(i==-1)i=GAME_PLAYER-1;
			////	if(m_cbPlayStatus[i]==TRUE)
			////	{
			////		CompareCard.wCompareUser=(WORD)i;
			////		break;
			////	}
			////}
			while(true)
			{
				wTemp=rand()%GAME_PLAYER;
				if(wTemp!=wMeChairID && m_cbPlayStatus[wTemp]==TRUE)
				{
					CompareCard.wCompareUser=wTemp;
					break;
				}
			}
			ASSERT(CompareCard.wCompareUser!=wMeChairID);

			//发送消息
			m_pIAndroidUserItem->SendSocketData(SUB_C_COMPARE_CARD,&CompareCard,sizeof(CompareCard));

			return true;
		}
	case IDI_USER_FINISH_FLASH:	
		{
			m_pIAndroidUserItem->SendSocketData(SUB_C_FINISH_FLASH,NULL,0);
			return true;
		}
	}

	return false;
}

//游戏消息
bool CAndroidUserItemSink::OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_GAME_START:		//游戏开始
		{
			//消息处理
			return OnSubGameStart(pData,wDataSize);
		}
	case SUB_S_ADD_SCORE:		//用户下注
		{
			//消息处理
			return OnSubAddScore(pData,wDataSize);
		}
	case SUB_S_LOOK_CARD:		//看牌消息
		{
			//消息处理
			return OnSubLookCard(pData,wDataSize);
		}
	case SUB_S_COMPARE_CARD:	//比牌消息
		{
			//消息处理
			return OnSubCompareCard(pData,wDataSize);
		}
	case SUB_S_OPEN_CARD:		//开牌消息
		{
			//消息处理
			return OnSubOpenCard(pData,wDataSize);
		}
	case SUB_S_GIVE_UP:			//用户放弃
		{
			//消息处理
			return OnSubGiveUp(pData,wDataSize);
		}
	case SUB_S_PLAYER_EXIT:		//用户强退
		{
			//消息处理
			return OnSubPlayerExit(pData,wDataSize);
		}
	case SUB_S_GAME_END:		//游戏结束
		{
			//消息处理
			return OnSubGameEnd(pData,wDataSize);
		}
	case SUB_S_WAIT_COMPARE:	//等待比牌
		{
			//消息处理
			if (wDataSize!=sizeof(CMD_S_WaitCompare)) return false;
			CMD_S_WaitCompare * pWaitCompare=(CMD_S_WaitCompare *)pData;	
			ASSERT(pWaitCompare->wCompareUser==m_wCurrentUser);

			//重新定时
			if(m_pIAndroidUserItem->GetChairID()==m_wCurrentUser)
			{
				//比牌时间
				UINT nElapse=rand()%TIME_USER_COMPARE_CARD+TIME_LESS;
				m_pIAndroidUserItem->SetGameTimer(IDI_USER_COMPARE_CARD,nElapse);
			}

			return true;
		}
	}

	//错误断言
	ASSERT(FALSE);

	return true;
}

//游戏消息
bool CAndroidUserItemSink::OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize)
{
	return true;
}

//场景消息
bool CAndroidUserItemSink::OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pBuffer, WORD wDataSize)
{
	switch (cbGameStatus)
	{
	case GAME_STATUS_FREE:		//空闲状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusFree)) return false;

			//开始时间
			UINT nElapse=rand()%TIME_START_GAME+TIME_LESS;
			m_pIAndroidUserItem->SetGameTimer(IDI_START_GAME,nElapse);

			return true;
		}
	case GAME_STATUS_PLAY:	//游戏状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusPlay)) return false;
			CMD_S_StatusPlay * pStatusPlay=(CMD_S_StatusPlay *)pBuffer;
			WORD wMeChairID=m_pIAndroidUserItem->GetChairID();

			//加注信息
			m_lCellScore=pStatusPlay->lCellScore;
			m_lMaxScore=pStatusPlay->lMaxCellScore;//////////////
			m_lCurrentTimes=pStatusPlay->lCurrentTimes;
			m_lUserMaxScore=pStatusPlay->lUserMaxScore;

			m_wBankerUser=pStatusPlay->wBankerUser;						//庄家用户

			//设置变量
			m_wCurrentUser=pStatusPlay->wCurrentUser;
			m_bMingZhu=pStatusPlay->bMingZhu[wMeChairID];
			CopyMemory(m_lTableScore,pStatusPlay->lTableScore,sizeof(pStatusPlay->lTableScore));
			CopyMemory(m_cbPlayStatus,pStatusPlay->cbPlayStatus,sizeof(pStatusPlay->cbPlayStatus));
			////CopyMemory(m_cbHandCardData,pStatusPlay->cbHandCardData, sizeof(pStatusPlay->cbHandCardData));/////////////

			////if((!IsLookonMode()) && wMeChairID==m_wCurrentUser)
			if(wMeChairID==m_wCurrentUser)
			{
				//判断控件
				if(!(pStatusPlay->bCompareState))
				{
					//设置时间
					UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS;
					m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
				}
				else 
				{
					//等待比牌
					m_pIAndroidUserItem->SendSocketData(SUB_C_WAIT_COMPARE,NULL,0);

					//设置时间
					UINT nElapse=rand()%TIME_USER_COMPARE_CARD+TIME_LESS;
					m_pIAndroidUserItem->SetGameTimer(IDI_USER_COMPARE_CARD,nElapse);
				}
			}
			return true;
		}
	}

	ASSERT(FALSE);

	return false;
}

//用户进入
void CAndroidUserItemSink::OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户离开
void CAndroidUserItemSink::OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户积分
void CAndroidUserItemSink::OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户状态
void CAndroidUserItemSink::OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户段位
void CAndroidUserItemSink::OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//游戏开始
bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_GameStart)) return false;
	CMD_S_GameStart * pGameStart=(CMD_S_GameStart *)pBuffer;

	//数据信息
	m_lCellScore=pGameStart->lCellScore;
	m_lMaxScore=pGameStart->lMaxScore;///////////////
	m_lCurrentTimes=pGameStart->lCurrentTimes;
	m_wCurrentUser=pGameStart->wCurrentUser;
	m_lUserMaxScore=pGameStart->lUserMaxScore;
	m_bMingZhu=false;
	m_cbCardType=0;												//牌型///////////////////
	
	m_wBankerUser=pGameStart->wBankerUser;						//庄家用户

	//设置变量
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		ASSERT(m_pIAndroidUserItem!=NULL);
		//获取用户
		if (m_pIAndroidUserItem->GetTableUserItem(i)!=NULL) ////??无法获取用户
		{
			//参数信息
			m_lTableScore[i]=m_lCellScore;
			m_cbPlayStatus[i]=TRUE;
		}
		else 
		{
			m_lTableScore[i]=0;
			m_cbPlayStatus[i]=0;
		}
		////m_cbPlayStatus[i]=pGameStart->wPlayStatus[i];///////////
	}

	CopyMemory(m_cbPlayStatus,pGameStart->wPlayStatus,sizeof(pGameStart->wPlayStatus));////////////

	//多人游戏延时倍数////////////
	BYTE bCount=0;
	for(BYTE i=0;i<GAME_PLAYER;i++)
	{
		if(m_cbPlayStatus[i]==TRUE)
			bCount++;
	}
	if(bCount<2)
		bCount=2;

	//设置时间
	if(m_wCurrentUser==m_pIAndroidUserItem->GetChairID())
	{
		//下注时间
		UINT nElapse=rand()%TIME_USER_ADD_SCORE+1*bCount;
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
	}

	return true;
}

//用户放弃
bool CAndroidUserItemSink::OnSubGiveUp(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_GiveUp)) return false;
	CMD_S_GiveUp * pGiveUp=(CMD_S_GiveUp *)pBuffer;

	//设置变量
	m_cbPlayStatus[pGiveUp->wGiveUpUser]=FALSE;

	return true;
}

//用户下注
bool CAndroidUserItemSink::OnSubAddScore(const void * pBuffer, WORD wDataSize)
{
	//变量定义
	WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
	CMD_S_AddScore * pAddScore=(CMD_S_AddScore *)pBuffer;

	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_AddScore));
	if (wDataSize!=sizeof(CMD_S_AddScore)) return false;
	ASSERT(pAddScore->wCurrentUser < GAME_PLAYER);
	ASSERT(pAddScore->lCurrentTimes<=m_lMaxScore/m_lCellScore);

	//当前用户
	m_lCurrentTimes = pAddScore->lCurrentTimes;
	m_wCurrentUser=pAddScore->wCurrentUser;
	m_lTableScore[pAddScore->wAddScoreUser]+=pAddScore->lAddScoreCount;

	//设置时间
	if(pAddScore->wCompareState==FALSE && m_wCurrentUser==wMeChairID)
	{
		//下注时间
		UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS;
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
	}

	return true;
}

//用户看牌
bool CAndroidUserItemSink::OnSubLookCard(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_LookCard)) return false;
	CMD_S_LookCard * pLookCard=(CMD_S_LookCard *)pBuffer;

	WORD wMeChairID=m_pIAndroidUserItem->GetChairID();

	//重新定时
	if(m_wCurrentUser==wMeChairID)
	{
		m_cbCardType=m_GameLogic.GetCardType(pLookCard->cbCardData,MAX_COUNT);/////////////////////获取牌型

		if(m_cbCardType<2)
		{
			for(BYTE i=0;i<MAX_COUNT;i++)
			{
				m_cbCardType=0;
				if(m_GameLogic.GetCardLogicValue(pLookCard->cbCardData[i])>11)
				{
					m_cbCardType=1;
					break;
				}
			}
		}
		//下注时间
		UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS;
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
	}

	return true;
}

//用户比牌
bool CAndroidUserItemSink::OnSubCompareCard(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_CompareCard)) return false;
	CMD_S_CompareCard * pCompareCard=(CMD_S_CompareCard *)pBuffer;

	//设置变量
	WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
	m_cbPlayStatus[pCompareCard->wLostUser]=FALSE;

	//当前用户
	m_wCurrentUser=pCompareCard->wCurrentUser;

	//玩家人数
	BYTE bCount=0;
	for (WORD i=0;i<GAME_PLAYER;i++)if(m_cbPlayStatus[i]==TRUE)bCount++;

	//判断结束
	if(bCount>1)
	{
		//控件信息
		////if(!IsLookonMode() && wMeChairID==m_wCurrentUser)
		if(m_pIAndroidUserItem->GetChairID()==m_wCurrentUser)
		{
			//下注时间
			UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS+2;		//比牌动画时间
			m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
		}
	}
	else if(wMeChairID==pCompareCard->wCompareUser[0] || wMeChairID==pCompareCard->wCompareUser[1])
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_FINISH_FLASH,TIME_USER_FINISH_FLASH);//////////

	return true;
}

//用户开牌
bool CAndroidUserItemSink::OnSubOpenCard(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_OpenCard)) return false;

	return true;
}

//用户强退
bool CAndroidUserItemSink::OnSubPlayerExit(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_PlayerExit)) return false;
	CMD_S_PlayerExit * pPlayerExit=(CMD_S_PlayerExit *)pBuffer;

	//游戏信息
	ASSERT(m_cbPlayStatus[pPlayerExit->wPlayerID]==TRUE);
	m_cbPlayStatus[pPlayerExit->wPlayerID]=FALSE;

	return true;
}

//游戏结束
bool CAndroidUserItemSink::OnSubGameEnd(const void * pBuffer, WORD wDataSize)
{
	//效验参数
	if (wDataSize!=sizeof(CMD_S_GameEnd)) return false;

	m_pIAndroidUserItem->KillGameTimer(IDI_START_GAME);
	m_pIAndroidUserItem->KillGameTimer(IDI_USER_ADD_SCORE);
	m_pIAndroidUserItem->KillGameTimer(IDI_USER_COMPARE_CARD);

	//开始按钮
	////if (IsLookonMode()==false)
	////{
		//下注时间
		UINT nElapse=rand()%TIME_START_GAME+TIME_LESS;
		m_pIAndroidUserItem->SetGameTimer(IDI_START_GAME,nElapse);
	////}

	return true;
}

//////////////////////////////////////////////////////////////////////////
