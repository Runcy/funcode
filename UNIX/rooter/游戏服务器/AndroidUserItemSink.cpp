#include "Stdafx.h"
#include "AndroidUserItemSink.h"

//////////////////////////////////////////////////////////////////////////

//����ʱ��
#define TIME_LESS					2									//����ʱ��

//��Ϸʱ��
#define IDI_START_GAME				(IDI_ANDROID_ITEM_SINK+0)			//��ʼ��ʱ��
#define IDI_USER_ADD_SCORE			(IDI_ANDROID_ITEM_SINK+1)			//��ע��ʱ��
#define IDI_USER_COMPARE_CARD		(IDI_ANDROID_ITEM_SINK+2)			//ѡ�����û���ʱ��
#define IDI_USER_FINISH_FLASH		(IDI_ANDROID_ITEM_SINK+3)			///////

//ʱ���ʶ
#define TIME_START_GAME				3									//��ʼ��ʱ��
#define TIME_USER_ADD_SCORE			3									//��ע��ʱ��
#define	TIME_USER_COMPARE_CARD		3									//���ƶ�ʱ��
#define	TIME_USER_FINISH_FLASH		4									//////

#define IDI_ANDROID_ITEM_SINK		500//////////////////

//////////////////////////////////////////////////////////////////////////

//���캯��
CAndroidUserItemSink::CAndroidUserItemSink()
{
	//�ӿڱ���
	m_pIAndroidUserItem=NULL;

	//��Ϸ����
	WORD m_wCurrentUser=INVALID_CHAIR;							//��ǰ�û�
	WORD m_wBankerUser=INVALID_CHAIR;							//ׯ���û�/////////

	//�û�״̬
	ZeroMemory(m_cbPlayStatus,sizeof(m_cbPlayStatus));			//��Ϸ״̬
	ZeroMemory(m_lTableScore,sizeof(m_lTableScore));			//��ע��Ŀ
	ZeroMemory(m_cbHandCardData,sizeof(m_cbHandCardData));		//�����˿�///////////////
	m_cbCardType=0;												//����///////////////////
	m_bMingZhu=false;											//���ƶ���///////////////

	//��ע��Ϣ
	m_lMaxScore=0;							//�ⶥ��Ŀ
	m_lCellScore=0;							//��Ԫ��ע
	m_lCurrentTimes=1;						//��ǰ����
	m_lUserMaxScore=0;						//������

	return;
}

//��������
CAndroidUserItemSink::~CAndroidUserItemSink()
{
}

//�ӿڲ�ѯ
void * CAndroidUserItemSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IAndroidUserItemSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IAndroidUserItemSink,Guid,dwQueryVer);
	return NULL;
}

//��ʼ�ӿ�
bool CAndroidUserItemSink::Initialization(IUnknownEx * pIUnknownEx)
{
	//��ѯ�ӿ�
	////m_pIAndroidUserItem=GET_OBJECTPTR_INTERFACE(pIUnknownEx,IAndroidUserItem);
	m_pIAndroidUserItem=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAndroidUserItem);
	if (m_pIAndroidUserItem==NULL) return false;

	return true;
}

//���ýӿ�
bool CAndroidUserItemSink::RepositionSink()
{
	//��Ϸ����
	WORD m_wCurrentUser=INVALID_CHAIR;							//��ǰ�û�
	WORD m_wBankerUser=INVALID_CHAIR;							//ׯ���û�/////////

	//�û�״̬
	ZeroMemory(m_cbPlayStatus,sizeof(m_cbPlayStatus));			//��Ϸ״̬
	ZeroMemory(m_lTableScore,sizeof(m_lTableScore));			//��ע��Ŀ
	ZeroMemory(m_cbHandCardData,sizeof(m_cbHandCardData));		//�����˿�///////////////
	m_cbCardType=0;												//����///////////////////
	m_bMingZhu=false;											//���ƶ���///////////////

	//////��ע��Ϣ
	m_lMaxScore=0;							//�ⶥ��Ŀ
	m_lCellScore=0;							//��Ԫ��ע
	m_lCurrentTimes=1;						//��ǰ����
	m_lUserMaxScore=0;						//������

	return true;
}

//ʱ����Ϣ
bool CAndroidUserItemSink::OnEventTimer(UINT nTimerID)
{
	switch (nTimerID)
	{
	case IDI_START_GAME:		//��ʼ��ʱ��
		{
			//����׼��
			m_pIAndroidUserItem->SendUserReady(NULL,0);

			return true;
		}
	case IDI_USER_ADD_SCORE:	//��ע��ʱ��
		{
			m_pIAndroidUserItem->KillGameTimer(IDI_USER_ADD_SCORE);

			ASSERT(m_pIAndroidUserItem->GetChairID()==m_wCurrentUser);
			if(m_pIAndroidUserItem->GetChairID()!=m_wCurrentUser)return false;
			//����
			//��ע
			//����
			//����
			//��ע
			//����

			BYTE wTemp=0;
			LONGLONG lCurrentScore=0;
			WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
			LONGLONG lAndroidMaxScore=m_pIAndroidUserItem->GetMeUserItem()->GetUserScore();
			if(lAndroidMaxScore<m_lUserMaxScore)
				lAndroidMaxScore=m_lUserMaxScore;

			//�����û���Ŀ
			BYTE bPlayerCount=0;
			for(BYTE i=0;i<GAME_PLAYER;i++)
			{
				////if(i==wMeChairID)continue;
				if(m_cbPlayStatus[i]==TRUE)
					bPlayerCount++;
			}
			if(bPlayerCount==0 || bPlayerCount>=GAME_PLAYER)
				bPlayerCount=4;


			if(m_cbPlayStatus[wMeChairID]==TRUE && m_lTableScore[wMeChairID]==0)	//�����м��״β���	//��ע//��ע//����
			{

				//�Ƿ��� 80%����
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
				else	//����������� 20%����
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
						//(ׯ�Һ��ϼ�)���� 
						lCurrentScore=m_lCurrentTimes*m_lCellScore*2*2;

						//������Ϣ
						CMD_C_AddScore AddScore;
						AddScore.wState=TRUE;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

						//�����ϼ�
						WORD wMeChairID=m_pIAndroidUserItem->GetChairID();

						//�������
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

						//������Ϣ
						m_pIAndroidUserItem->SendSocketData(SUB_C_COMPARE_CARD,&CompareCard,sizeof(CompareCard));

						return true;
					}
				}

				//��ע//��ע//(ׯ�Һ��ϼ�)���� ����
				ASSERT(m_lCurrentTimes<=10 && m_lCurrentTimes>=0);
				lCurrentScore=m_lCellScore*(m_lCurrentTimes+rand()%(11-m_lCurrentTimes));

				//��ע�ӱ�
				if(m_bMingZhu)lCurrentScore*=2;

				//������Ϣ
				CMD_C_AddScore AddScore;
				AddScore.wState=0;
				AddScore.lScore=lCurrentScore;
				m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

				return true;
			}
			else	//��ͨ����
			{
				if(!m_bMingZhu)	//����	//��ע//��ע//����
				{
					//���� 10%����
					wTemp=rand()%10;
					if(wTemp<1)
					{
						m_bMingZhu=true;
						m_pIAndroidUserItem->SendSocketData(SUB_C_LOOK_CARD,NULL,0);

						return true;
					}

					if(m_lTableScore[wMeChairID]< m_lMaxScore*2 && (4*bPlayerCount*m_lMaxScore+m_lCellScore+m_lTableScore[wMeChairID])<lAndroidMaxScore)	//0��ע//1��ע //������עС��[�ⶥ��Ŀ*2]��ע ���ڱ���
					{
						ASSERT(m_lCurrentTimes<=10 && m_lCurrentTimes>=0);
						lCurrentScore=m_lCellScore*(m_lCurrentTimes+rand()%(11-m_lCurrentTimes));
						//������Ϣ
						CMD_C_AddScore AddScore;
						AddScore.wState=0;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

						return true;
					}
					else	//����
					{
						////lCurrentScore=(m_bMingZhu)?(m_lCurrentTimes*m_lCellScore*4):(m_lCurrentTimes*m_lCellScore*2);
						lCurrentScore=m_lCurrentTimes*m_lCellScore*2;

						//������Ϣ
						CMD_C_AddScore AddScore;
						AddScore.wState=1;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));
					
						//�������
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
							//�����ϼ�
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
				else	//����	//��ע//��ע//����//����
				{
					if(m_cbCardType==0)
					{
						m_pIAndroidUserItem->SendSocketData(SUB_C_GIVE_UP,NULL,0);

						return true;
					}
					else if(m_cbCardType==1 && m_lTableScore[wMeChairID]<=m_lMaxScore*5)	//���Ʋ�����ע����[�ⶥ��Ŀ*5]������� 20%����
					{
						wTemp=rand()%10;
						if(wTemp<1)
						{
							m_pIAndroidUserItem->SendSocketData(SUB_C_GIVE_UP,NULL,0);

							return true;
						}
					}


					//��ע//��ע //������עС��[�ⶥ��Ŀ*����]��ע ���ڱ���
					if((m_cbCardType<4 && m_lTableScore[wMeChairID]<(1 * 2 * m_lMaxScore * m_cbCardType) || 
						m_cbCardType==4 && m_lTableScore[wMeChairID]<(3 * 2 * m_lMaxScore * m_cbCardType) || 
						m_cbCardType>4 && m_lTableScore[wMeChairID]<(5 * 2 * m_lMaxScore * m_cbCardType)) && 
					////if(
						(4*bPlayerCount*m_lMaxScore+m_lCellScore+m_lTableScore[wMeChairID])<lAndroidMaxScore)
					{
						ASSERT(m_lCurrentTimes<=10 && m_lCurrentTimes>=0);
						//0��ע//1��ע
						lCurrentScore=m_lCellScore*(m_lCurrentTimes+rand()%(11-m_lCurrentTimes));

						//��ע�ӱ�
						if(m_bMingZhu)lCurrentScore*=2;

						//������Ϣ
						CMD_C_AddScore AddScore;
						AddScore.wState=0;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));

						return true;
					}
					else	//����
					{
						lCurrentScore=m_lCurrentTimes*m_lCellScore*4;

						//������Ϣ
						CMD_C_AddScore AddScore;
						AddScore.wState=1;
						AddScore.lScore=lCurrentScore;
						m_pIAndroidUserItem->SendSocketData(SUB_C_ADD_SCORE,&AddScore,sizeof(AddScore));
					
						//�������
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
							//�����ϼ�
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
	case IDI_USER_COMPARE_CARD:	//ѡ��ʱ��
		{
			//�����ϼ�
			WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
			BYTE wTemp=5;

			//�������
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

			//������Ϣ
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

//��Ϸ��Ϣ
bool CAndroidUserItemSink::OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_GAME_START:		//��Ϸ��ʼ
		{
			//��Ϣ����
			return OnSubGameStart(pData,wDataSize);
		}
	case SUB_S_ADD_SCORE:		//�û���ע
		{
			//��Ϣ����
			return OnSubAddScore(pData,wDataSize);
		}
	case SUB_S_LOOK_CARD:		//������Ϣ
		{
			//��Ϣ����
			return OnSubLookCard(pData,wDataSize);
		}
	case SUB_S_COMPARE_CARD:	//������Ϣ
		{
			//��Ϣ����
			return OnSubCompareCard(pData,wDataSize);
		}
	case SUB_S_OPEN_CARD:		//������Ϣ
		{
			//��Ϣ����
			return OnSubOpenCard(pData,wDataSize);
		}
	case SUB_S_GIVE_UP:			//�û�����
		{
			//��Ϣ����
			return OnSubGiveUp(pData,wDataSize);
		}
	case SUB_S_PLAYER_EXIT:		//�û�ǿ��
		{
			//��Ϣ����
			return OnSubPlayerExit(pData,wDataSize);
		}
	case SUB_S_GAME_END:		//��Ϸ����
		{
			//��Ϣ����
			return OnSubGameEnd(pData,wDataSize);
		}
	case SUB_S_WAIT_COMPARE:	//�ȴ�����
		{
			//��Ϣ����
			if (wDataSize!=sizeof(CMD_S_WaitCompare)) return false;
			CMD_S_WaitCompare * pWaitCompare=(CMD_S_WaitCompare *)pData;	
			ASSERT(pWaitCompare->wCompareUser==m_wCurrentUser);

			//���¶�ʱ
			if(m_pIAndroidUserItem->GetChairID()==m_wCurrentUser)
			{
				//����ʱ��
				UINT nElapse=rand()%TIME_USER_COMPARE_CARD+TIME_LESS;
				m_pIAndroidUserItem->SetGameTimer(IDI_USER_COMPARE_CARD,nElapse);
			}

			return true;
		}
	}

	//�������
	ASSERT(FALSE);

	return true;
}

//��Ϸ��Ϣ
bool CAndroidUserItemSink::OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize)
{
	return true;
}

//������Ϣ
bool CAndroidUserItemSink::OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pBuffer, WORD wDataSize)
{
	switch (cbGameStatus)
	{
	case GAME_STATUS_FREE:		//����״̬
		{
			//Ч������
			if (wDataSize!=sizeof(CMD_S_StatusFree)) return false;

			//��ʼʱ��
			UINT nElapse=rand()%TIME_START_GAME+TIME_LESS;
			m_pIAndroidUserItem->SetGameTimer(IDI_START_GAME,nElapse);

			return true;
		}
	case GAME_STATUS_PLAY:	//��Ϸ״̬
		{
			//Ч������
			if (wDataSize!=sizeof(CMD_S_StatusPlay)) return false;
			CMD_S_StatusPlay * pStatusPlay=(CMD_S_StatusPlay *)pBuffer;
			WORD wMeChairID=m_pIAndroidUserItem->GetChairID();

			//��ע��Ϣ
			m_lCellScore=pStatusPlay->lCellScore;
			m_lMaxScore=pStatusPlay->lMaxCellScore;//////////////
			m_lCurrentTimes=pStatusPlay->lCurrentTimes;
			m_lUserMaxScore=pStatusPlay->lUserMaxScore;

			m_wBankerUser=pStatusPlay->wBankerUser;						//ׯ���û�

			//���ñ���
			m_wCurrentUser=pStatusPlay->wCurrentUser;
			m_bMingZhu=pStatusPlay->bMingZhu[wMeChairID];
			CopyMemory(m_lTableScore,pStatusPlay->lTableScore,sizeof(pStatusPlay->lTableScore));
			CopyMemory(m_cbPlayStatus,pStatusPlay->cbPlayStatus,sizeof(pStatusPlay->cbPlayStatus));
			////CopyMemory(m_cbHandCardData,pStatusPlay->cbHandCardData, sizeof(pStatusPlay->cbHandCardData));/////////////

			////if((!IsLookonMode()) && wMeChairID==m_wCurrentUser)
			if(wMeChairID==m_wCurrentUser)
			{
				//�жϿؼ�
				if(!(pStatusPlay->bCompareState))
				{
					//����ʱ��
					UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS;
					m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
				}
				else 
				{
					//�ȴ�����
					m_pIAndroidUserItem->SendSocketData(SUB_C_WAIT_COMPARE,NULL,0);

					//����ʱ��
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

//�û�����
void CAndroidUserItemSink::OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û��뿪
void CAndroidUserItemSink::OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û�����
void CAndroidUserItemSink::OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û�״̬
void CAndroidUserItemSink::OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û���λ
void CAndroidUserItemSink::OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//��Ϸ��ʼ
bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, WORD wDataSize)
{
	//Ч������
	if (wDataSize!=sizeof(CMD_S_GameStart)) return false;
	CMD_S_GameStart * pGameStart=(CMD_S_GameStart *)pBuffer;

	//������Ϣ
	m_lCellScore=pGameStart->lCellScore;
	m_lMaxScore=pGameStart->lMaxScore;///////////////
	m_lCurrentTimes=pGameStart->lCurrentTimes;
	m_wCurrentUser=pGameStart->wCurrentUser;
	m_lUserMaxScore=pGameStart->lUserMaxScore;
	m_bMingZhu=false;
	m_cbCardType=0;												//����///////////////////
	
	m_wBankerUser=pGameStart->wBankerUser;						//ׯ���û�

	//���ñ���
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		ASSERT(m_pIAndroidUserItem!=NULL);
		//��ȡ�û�
		if (m_pIAndroidUserItem->GetTableUserItem(i)!=NULL) ////??�޷���ȡ�û�
		{
			//������Ϣ
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

	//������Ϸ��ʱ����////////////
	BYTE bCount=0;
	for(BYTE i=0;i<GAME_PLAYER;i++)
	{
		if(m_cbPlayStatus[i]==TRUE)
			bCount++;
	}
	if(bCount<2)
		bCount=2;

	//����ʱ��
	if(m_wCurrentUser==m_pIAndroidUserItem->GetChairID())
	{
		//��עʱ��
		UINT nElapse=rand()%TIME_USER_ADD_SCORE+1*bCount;
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
	}

	return true;
}

//�û�����
bool CAndroidUserItemSink::OnSubGiveUp(const void * pBuffer, WORD wDataSize)
{
	//Ч������
	if (wDataSize!=sizeof(CMD_S_GiveUp)) return false;
	CMD_S_GiveUp * pGiveUp=(CMD_S_GiveUp *)pBuffer;

	//���ñ���
	m_cbPlayStatus[pGiveUp->wGiveUpUser]=FALSE;

	return true;
}

//�û���ע
bool CAndroidUserItemSink::OnSubAddScore(const void * pBuffer, WORD wDataSize)
{
	//��������
	WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
	CMD_S_AddScore * pAddScore=(CMD_S_AddScore *)pBuffer;

	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_AddScore));
	if (wDataSize!=sizeof(CMD_S_AddScore)) return false;
	ASSERT(pAddScore->wCurrentUser < GAME_PLAYER);
	ASSERT(pAddScore->lCurrentTimes<=m_lMaxScore/m_lCellScore);

	//��ǰ�û�
	m_lCurrentTimes = pAddScore->lCurrentTimes;
	m_wCurrentUser=pAddScore->wCurrentUser;
	m_lTableScore[pAddScore->wAddScoreUser]+=pAddScore->lAddScoreCount;

	//����ʱ��
	if(pAddScore->wCompareState==FALSE && m_wCurrentUser==wMeChairID)
	{
		//��עʱ��
		UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS;
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
	}

	return true;
}

//�û�����
bool CAndroidUserItemSink::OnSubLookCard(const void * pBuffer, WORD wDataSize)
{
	//Ч������
	if (wDataSize!=sizeof(CMD_S_LookCard)) return false;
	CMD_S_LookCard * pLookCard=(CMD_S_LookCard *)pBuffer;

	WORD wMeChairID=m_pIAndroidUserItem->GetChairID();

	//���¶�ʱ
	if(m_wCurrentUser==wMeChairID)
	{
		m_cbCardType=m_GameLogic.GetCardType(pLookCard->cbCardData,MAX_COUNT);/////////////////////��ȡ����

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
		//��עʱ��
		UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS;
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
	}

	return true;
}

//�û�����
bool CAndroidUserItemSink::OnSubCompareCard(const void * pBuffer, WORD wDataSize)
{
	//Ч������
	if (wDataSize!=sizeof(CMD_S_CompareCard)) return false;
	CMD_S_CompareCard * pCompareCard=(CMD_S_CompareCard *)pBuffer;

	//���ñ���
	WORD wMeChairID=m_pIAndroidUserItem->GetChairID();
	m_cbPlayStatus[pCompareCard->wLostUser]=FALSE;

	//��ǰ�û�
	m_wCurrentUser=pCompareCard->wCurrentUser;

	//�������
	BYTE bCount=0;
	for (WORD i=0;i<GAME_PLAYER;i++)if(m_cbPlayStatus[i]==TRUE)bCount++;

	//�жϽ���
	if(bCount>1)
	{
		//�ؼ���Ϣ
		////if(!IsLookonMode() && wMeChairID==m_wCurrentUser)
		if(m_pIAndroidUserItem->GetChairID()==m_wCurrentUser)
		{
			//��עʱ��
			UINT nElapse=rand()%TIME_USER_ADD_SCORE+TIME_LESS+2;		//���ƶ���ʱ��
			m_pIAndroidUserItem->SetGameTimer(IDI_USER_ADD_SCORE,nElapse);
		}
	}
	else if(wMeChairID==pCompareCard->wCompareUser[0] || wMeChairID==pCompareCard->wCompareUser[1])
		m_pIAndroidUserItem->SetGameTimer(IDI_USER_FINISH_FLASH,TIME_USER_FINISH_FLASH);//////////

	return true;
}

//�û�����
bool CAndroidUserItemSink::OnSubOpenCard(const void * pBuffer, WORD wDataSize)
{
	//Ч������
	if (wDataSize!=sizeof(CMD_S_OpenCard)) return false;

	return true;
}

//�û�ǿ��
bool CAndroidUserItemSink::OnSubPlayerExit(const void * pBuffer, WORD wDataSize)
{
	//Ч������
	if (wDataSize!=sizeof(CMD_S_PlayerExit)) return false;
	CMD_S_PlayerExit * pPlayerExit=(CMD_S_PlayerExit *)pBuffer;

	//��Ϸ��Ϣ
	ASSERT(m_cbPlayStatus[pPlayerExit->wPlayerID]==TRUE);
	m_cbPlayStatus[pPlayerExit->wPlayerID]=FALSE;

	return true;
}

//��Ϸ����
bool CAndroidUserItemSink::OnSubGameEnd(const void * pBuffer, WORD wDataSize)
{
	//Ч�����
	if (wDataSize!=sizeof(CMD_S_GameEnd)) return false;

	m_pIAndroidUserItem->KillGameTimer(IDI_START_GAME);
	m_pIAndroidUserItem->KillGameTimer(IDI_USER_ADD_SCORE);
	m_pIAndroidUserItem->KillGameTimer(IDI_USER_COMPARE_CARD);

	//��ʼ��ť
	////if (IsLookonMode()==false)
	////{
		//��עʱ��
		UINT nElapse=rand()%TIME_START_GAME+TIME_LESS;
		m_pIAndroidUserItem->SetGameTimer(IDI_START_GAME,nElapse);
	////}

	return true;
}

//////////////////////////////////////////////////////////////////////////
