#ifndef ANDROID_USER_ITEM_SINK_HEAD_FILE
#define ANDROID_USER_ITEM_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "GameLogic.h"

//////////////////////////////////////////////////////////////////////////

//��Ϸ�Ի���
class CAndroidUserItemSink : public IAndroidUserItemSink
{
	//��Ϸ����
protected:
	WORD							m_wCurrentUser;							//��ǰ�û�
	WORD							m_wBankerUser;							//ׯ���û�////////

	//�û�״̬
protected:
	BYTE							m_cbPlayStatus[GAME_PLAYER];			//��Ϸ״̬
	////LONG							m_lTableScore[GAME_PLAYER];				//��ע��Ŀ
	LONGLONG						m_lTableScore[GAME_PLAYER];				//��ע��Ŀ

	BYTE							m_cbHandCardData[MAX_COUNT];			//�����˿�///////////////
	BYTE							m_cbCardType;							//����////////

	//��ע��Ϣ
protected:
	////LONG							m_lMaxScore;							//�ⶥ��Ŀ
	////LONG							m_lCellScore;							//��Ԫ��ע
	////LONG							m_lCurrentTimes;						//��ǰ����
	////LONG							m_lUserMaxScore;						//������
	LONGLONG						m_lMaxScore;							//�ⶥ��Ŀ
	LONGLONG						m_lCellScore;							//��Ԫ��ע
	LONGLONG						m_lCurrentTimes;						//��ǰ����
	LONGLONG						m_lUserMaxScore;						//������
	////bool							m_bMingZhu[GAME_PLAYER];				//���ƶ���
	bool							m_bMingZhu;				//���ƶ���

	//�ؼ�����
public:
	CGameLogic						m_GameLogic;							//��Ϸ�߼�
	IAndroidUserItem *				m_pIAndroidUserItem;					//�û��ӿ�
	
	//��������
public:
	//���캯��
	CAndroidUserItemSink();
	//��������
	virtual ~CAndroidUserItemSink();

	//�����ӿ�
public:
	//�ͷŶ���
	////virtual bool Release() { if (IsValid()) delete this; return true; }
	virtual VOID Release() { }
	//�Ƿ���Ч
	virtual bool IsValid() { return AfxIsValidAddress(this,sizeof(CAndroidUserItemSink))?true:false; }
	//�ӿڲ�ѯ
	virtual void * QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//���ƽӿ�
public:
	//��ʼ�ӿ�
	virtual bool Initialization(IUnknownEx * pIUnknownEx);
	//���ýӿ�
	virtual bool RepositionSink();

	//��Ϸ�¼�
public:
	//ʱ����Ϣ
	virtual bool OnEventTimer(UINT nTimerID);
	//��Ϸ��Ϣ
	virtual bool OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//��Ϸ��Ϣ
	virtual bool OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//������Ϣ
	virtual bool OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pData, WORD wDataSize);

	//�û��¼�
public:
	//�û�����
	virtual void OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û��뿪
	virtual void OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û�����
	virtual void OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û�״̬
	virtual void OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û���λ
	virtual void OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);

	//��Ϣ����
protected:
	//��Ϸ��ʼ
	bool OnSubGameStart(const void * pBuffer, WORD wDataSize);
	//�û�����
	bool OnSubGiveUp(const void * pBuffer, WORD wDataSize);
	//�û���ע
	bool OnSubAddScore(const void * pBuffer, WORD wDataSize);
	//�û�����
	bool OnSubLookCard(const void * pBuffer, WORD wDataSize);
	//�û�����
	bool OnSubCompareCard(const void * pBuffer, WORD wDataSize);
	//�û�����
	bool OnSubOpenCard(const void * pBuffer, WORD wDataSize);
	//�û�ǿ��
	bool OnSubPlayerExit(const void * pBuffer, WORD wDataSize);
	//��Ϸ����
	bool OnSubGameEnd(const void * pBuffer, WORD wDataSize);
};

//////////////////////////////////////////////////////////////////////////

#endif
