#ifndef ANDROID_USER_ITEM_SINK_HEAD_FILE
#define ANDROID_USER_ITEM_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "GameLogic.h"

//////////////////////////////////////////////////////////////////////////

//游戏对话框
class CAndroidUserItemSink : public IAndroidUserItemSink
{
	//游戏变量
protected:
	WORD							m_wCurrentUser;							//当前用户
	WORD							m_wBankerUser;							//庄家用户////////

	//用户状态
protected:
	BYTE							m_cbPlayStatus[GAME_PLAYER];			//游戏状态
	////LONG							m_lTableScore[GAME_PLAYER];				//下注数目
	LONGLONG						m_lTableScore[GAME_PLAYER];				//下注数目

	BYTE							m_cbHandCardData[MAX_COUNT];			//桌面扑克///////////////
	BYTE							m_cbCardType;							//牌型////////

	//下注信息
protected:
	////LONG							m_lMaxScore;							//封顶数目
	////LONG							m_lCellScore;							//单元下注
	////LONG							m_lCurrentTimes;						//当前倍数
	////LONG							m_lUserMaxScore;						//最大分数
	LONGLONG						m_lMaxScore;							//封顶数目
	LONGLONG						m_lCellScore;							//单元下注
	LONGLONG						m_lCurrentTimes;						//当前倍数
	LONGLONG						m_lUserMaxScore;						//最大分数
	////bool							m_bMingZhu[GAME_PLAYER];				//看牌动作
	bool							m_bMingZhu;				//看牌动作

	//控件变量
public:
	CGameLogic						m_GameLogic;							//游戏逻辑
	IAndroidUserItem *				m_pIAndroidUserItem;					//用户接口
	
	//函数定义
public:
	//构造函数
	CAndroidUserItemSink();
	//析构函数
	virtual ~CAndroidUserItemSink();

	//基础接口
public:
	//释放对象
	////virtual bool Release() { if (IsValid()) delete this; return true; }
	virtual VOID Release() { }
	//是否有效
	virtual bool IsValid() { return AfxIsValidAddress(this,sizeof(CAndroidUserItemSink))?true:false; }
	//接口查询
	virtual void * QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//控制接口
public:
	//初始接口
	virtual bool Initialization(IUnknownEx * pIUnknownEx);
	//重置接口
	virtual bool RepositionSink();

	//游戏事件
public:
	//时间消息
	virtual bool OnEventTimer(UINT nTimerID);
	//游戏消息
	virtual bool OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//游戏消息
	virtual bool OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//场景消息
	virtual bool OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pData, WORD wDataSize);

	//用户事件
public:
	//用户进入
	virtual void OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户离开
	virtual void OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户积分
	virtual void OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户状态
	virtual void OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户段位
	virtual void OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);

	//消息处理
protected:
	//游戏开始
	bool OnSubGameStart(const void * pBuffer, WORD wDataSize);
	//用户放弃
	bool OnSubGiveUp(const void * pBuffer, WORD wDataSize);
	//用户加注
	bool OnSubAddScore(const void * pBuffer, WORD wDataSize);
	//用户看牌
	bool OnSubLookCard(const void * pBuffer, WORD wDataSize);
	//用户比牌
	bool OnSubCompareCard(const void * pBuffer, WORD wDataSize);
	//用户开牌
	bool OnSubOpenCard(const void * pBuffer, WORD wDataSize);
	//用户强退
	bool OnSubPlayerExit(const void * pBuffer, WORD wDataSize);
	//游戏结束
	bool OnSubGameEnd(const void * pBuffer, WORD wDataSize);
};

//////////////////////////////////////////////////////////////////////////

#endif
