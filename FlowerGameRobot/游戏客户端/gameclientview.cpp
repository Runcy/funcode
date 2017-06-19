#include "StdAfx.h"
#include "Resource.h"
#include "GameClientEngine.h"
#include "GameClientView.h"
#include ".\gameclientview.h"

//////////////////////////////////////////////////////////////////////////
//�궨��

//��ť��ʶ
#define IDC_START						100								//��ʼ��ť
#define IDC_MAX_SCORE					104								//���ť
#define IDC_MIN_SCORE					105								//���ٰ�ť
#define IDC_CONFIRM						106								//ȷ����ť
#define	IDC_CANCEL_ADD					119								//ȡ����ť
#define IDC_ADD_SCORE					110								//��ע��ť
#define IDC_LOOK_CARD					111								//������Ϣ
#define IDC_COMPARE_CARD				112								//������Ϣ
#define IDC_FOLLOW						113								//������Ϣ
#define IDC_GIVE_UP						114								//������Ϣ
#define	IDC_OPEN_CARD					115								//������Ϣ

//��ʱ����ʶ
#define IDI_CARTOON						99								//������ʱ��
#define	TIME_CARTOON					100								//����ʱ��
#define IDI_MOVE_JETTON					101								//�ƶ����붨ʱ��
#define IDI_MOVE_NUMBER					102								//��������

//�����ٶ�  //(SPEED)10 (TIME_CARTOON)70 /CPU UP
#define	SPEED							7								//�����ٶȲ���
#define	ROCK							10								//�񶯸���
#define TIME_MOVE_JETTON				20								//�ƶ�����ʱ��
#define TIME_MOVE_NUMBER				50								//��������ʱ��

//λ�ñ���
#define	MY_SELF_CHAIRID					2								//�Լ�λ��

//���Ʊ���
#define VSCARD_TIME						3								//���ƴ���
#define VSCARD_COUNT					6								//������Ŀ
#define COMPARE_RADII					115								//���ƿ��

//���Ʊ���
#define SEND_PELS						30								//��������
#define LESS_SEND_COUNT					2								//���ʹ���
//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameClientView, CGameFrameViewGDI)							  
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

//���캯��
CGameClientView::CGameClientView()
{
	//��������
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_lDrawGold[i].RemoveAll();
		m_ptKeepJeton[i].RemoveAll();
		m_lStopUpdataScore[i] = 0;
		m_wFlashUser[i]=INVALID_CHAIR;	
	}
	m_bCartoon =false;
	m_SendCardPos.SetPoint(0,0);
	m_SendCardItemArray.RemoveAll();

	//���Ʊ���
	m_wConmareIndex=0;
	m_wConmareCount=0;
	m_wLoserUser=INVALID_CHAIR;
	ZeroMemory(m_wCompareChairID,sizeof(m_wCompareChairID));

	//���ݱ���
	ZeroMemory(m_tcBuffer,sizeof(m_tcBuffer));
	ZeroMemory(m_lTableScore,sizeof(m_lTableScore));
	ZeroMemory(m_bCompareUser,sizeof(m_bCompareUser));
	m_wBankerUser=INVALID_CHAIR;
	m_lMaxCellScore=0;
	m_lCellScore=0;	
	m_wFalshCount=0;
	m_bStopDraw=false;
	m_bFalsh=false;
	m_bInvest=false;		
	m_bShow=false;
	m_bCompareCard=false;
	m_wWaitUserChoice=INVALID_CHAIR;
	ZeroMemory(m_cbPlayStatus,sizeof(m_cbPlayStatus));

	//����λͼ
	HINSTANCE hInstance=AfxGetInstanceHandle();
	//m_ImageJeton.LoadFromResource(hInstance,IDB_GOLD);
	m_ImageCard.LoadFromResource(hInstance,IDB_CARD);
	//m_ImageTitle.LoadFromResource(hInstance,IDB_TITLE);
	m_ImageViewBack.LoadFromResource(hInstance,IDB_VIEW_BACK);
	m_ImageViewCenter.LoadFromResource(hInstance,IDB_VIEW_FILL);
	m_ImageBanker.LoadFromResource(hInstance,IDB_BANKER_FLAG);
	m_ImageArrowhead.LoadFromResource(hInstance,IDB_ARROWHEAD);
	//m_ImageReady.LoadFromResource(hInstance,IDB_READY);
	m_ImageNumber.LoadFromResource(hInstance,IDB_NUMBER);
	//m_ImagePocket.LoadFromResource(hInstance,IDB_POCKET);
	//m_ImageBalance.LoadFromResource(hInstance,IDB_SCORE_VIEW);

	m_nXFace=48;
	m_nYFace=48;
	m_nXTimer=65;
	m_nYTimer=69;

	return;
}

//��������
CGameClientView::~CGameClientView()
{
}

//������Ϣ
int CGameClientView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct)==-1) return -1;

	//�����ؼ�
	CRect rcCreate(0,0,0,0);
	m_ScoreView.Create(NULL,NULL,WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,rcCreate,this,10);

	//������ť
	m_btStart.Create(TEXT(""),WS_CHILD|WS_CLIPSIBLINGS,rcCreate,this,IDC_START);
	m_btMaxScore.Create(TEXT(""),WS_CHILD,rcCreate,this,IDC_MAX_SCORE);
	m_btMinScore.Create(TEXT(""),WS_CHILD,rcCreate,this,IDC_MIN_SCORE);
	m_btConfirm.Create(TEXT(""),WS_CHILD,rcCreate,this,IDC_CONFIRM);
	m_btCancel.Create(TEXT(""),WS_CHILD,rcCreate,this,IDC_CANCEL_ADD);
	m_btOpenCard.Create(TEXT(""),WS_CHILD,rcCreate,this,IDC_OPEN_CARD);

	m_btAddScore.Create(TEXT(""),WS_CHILD|WS_DISABLED,rcCreate,this,IDC_ADD_SCORE);
	m_btCompareCard.Create(TEXT(""),WS_CHILD|WS_DISABLED,rcCreate,this,IDC_COMPARE_CARD);
	m_btGiveUp.Create(TEXT(""),WS_CHILD|WS_DISABLED,rcCreate,this,IDC_GIVE_UP);
	m_btLookCard.Create(TEXT(""),WS_CHILD|WS_DISABLED,rcCreate,this,IDC_LOOK_CARD);
	m_btFollow.Create(TEXT(""),WS_CHILD|WS_DISABLED,rcCreate,this,IDC_FOLLOW);

	m_GoldControl.Create(NULL,NULL,WS_CHILD|WS_CLIPSIBLINGS,CRect(50,50,66,66),this,8);

	//����λͼ
	HINSTANCE hInstance=AfxGetInstanceHandle();
	m_btStart.SetButtonImage(IDB_START,hInstance,false,false);
	m_btMaxScore.SetButtonImage(IDB_USERCONTROL_MAX,hInstance,false,false);
	m_btMinScore.SetButtonImage(IDB_USERCONTROL_MIN,hInstance,false,false);
	m_btAddScore.SetButtonImage(IDB_USERCONTROL_INVEST,hInstance,false,false);
	m_btConfirm.SetButtonImage(IDB_CONFIRM,hInstance,false,false);
	m_btCancel.SetButtonImage(IDB_USERCONTROL_CLEAR,hInstance,false,false);
	m_btCompareCard.SetButtonImage(IDB_COMPARE_CARD,hInstance,false,false);
	m_btOpenCard.SetButtonImage(IDB_OPEN_CARD,hInstance,false,false);
	m_btGiveUp.SetButtonImage(IDB_GIVE_UP,hInstance,false,false);
	m_btLookCard.SetButtonImage(IDB_LO0K_CARD,hInstance,false,false);
	m_btFollow.SetButtonImage(IDB_FOLLOW,hInstance,false,false);

	//�����ؼ�
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_CardControl[i].Create(NULL,NULL,WS_VISIBLE|WS_CHILD,rcCreate,this,20+i);
		m_CardControl[i].SetCommandID(i,IDM_COMPARE_USER);
	}

	return 0;
}

//�����
BOOL CGameClientView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDC_START:				//��ʼ��ť
		{
			//������Ϣ
			SendEngineMessage(IDM_START,0,0);
			return TRUE;
		}
	case IDC_ADD_SCORE:			//��ע��ť
		{
			//������Ϣ
			SendEngineMessage(IDM_ADD_SCORE,0,0);
			return TRUE;
		}
	case IDC_CONFIRM:			//ȷ����ť
		{
			//������Ϣ
			SendEngineMessage(IDM_CONFIRM,0,0);
			return TRUE;
		}
	case IDC_CANCEL_ADD:		//ȡ����ť
		{
			//������Ϣ
			SendEngineMessage(IDM_CANCEL_ADD,0,0);
			return TRUE;
		}
	case IDC_FOLLOW:			//��ע��ť
		{
			//������Ϣ
			SendEngineMessage(IDM_CONFIRM,1,1);
			return TRUE;
		}
	case IDC_MIN_SCORE:			//���ٰ�ť
		{
			//������Ϣ
			SendEngineMessage(IDM_MIN_SCORE,0,0);
			return TRUE;
		}
	case IDC_MAX_SCORE:			//���ť
		{
			//������Ϣ
			SendEngineMessage(IDM_MAX_SCORE,0,0);
			return TRUE;
		}
	case IDC_COMPARE_CARD:		//���ư�ť
		{
			//������Ϣ
			SendEngineMessage(IDM_COMPARE_CARD,0,0);
			return TRUE;
		}
	case IDC_LOOK_CARD:			//���ư�ť
		{
			//������Ϣ
			SendEngineMessage(IDM_LOOK_CARD,0,0);
			return TRUE;
		}
	case IDC_OPEN_CARD:			//������Ϣ
		{
			//������Ϣ
			SendEngineMessage(IDM_OPEN_CARD,0,0);
			return TRUE;
		}
	case IDC_GIVE_UP:			//������ť
		{
			//������Ϣ
			SendEngineMessage(IDM_GIVE_UP,0,0);
			return TRUE;
		}
	}

	return __super::OnCommand(wParam, lParam);
}

//���ý���
VOID CGameClientView::ResetGameView()
{
	//ɾ��ʱ��
	KillTimer(IDI_CARTOON);
	KillTimer(IDI_MOVE_JETTON);
	KillTimer(IDI_MOVE_NUMBER);

	//��������
	m_bCartoon =false;
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_lDrawGold[i].RemoveAll();
		m_ptKeepJeton[i].RemoveAll();
		m_wFlashUser[i]=INVALID_CHAIR;
		m_lStopUpdataScore[i] = 0;
	}
	m_SendCardPos.SetPoint(0,0);
	m_SendCardItemArray.RemoveAll();

	//���Ʊ���
	m_wConmareIndex = 0;
	m_wConmareCount = 0;
	m_wLoserUser=INVALID_CHAIR;
	ZeroMemory(m_wCompareChairID,sizeof(m_wCompareChairID));

	//���ݱ���
	ZeroMemory(m_tcBuffer,sizeof(m_tcBuffer));
	ZeroMemory(m_lTableScore,sizeof(m_lTableScore));
	m_wBankerUser=INVALID_CHAIR;			
	ZeroMemory(m_bCompareUser,sizeof(m_bCompareUser));
	m_lMaxCellScore=0;					
	m_lCellScore=0;		
	m_wFalshCount=0;
	m_bStopDraw=false;
	m_bFalsh=false;
	m_bInvest=false;
	m_bShow=false;
	m_bCompareCard=false;
	m_wWaitUserChoice=INVALID_CHAIR;
	ZeroMemory(m_cbPlayStatus,sizeof(m_cbPlayStatus));

	//����ť
	m_btStart.ShowWindow(SW_HIDE);
	m_btCancel.ShowWindow(SW_HIDE);
	m_btConfirm.ShowWindow(SW_HIDE);
	m_btMaxScore.ShowWindow(SW_HIDE);
	m_btMinScore.ShowWindow(SW_HIDE);
	m_btOpenCard.ShowWindow(SW_HIDE);
	m_GoldControl.ShowWindow(SW_HIDE);
	m_btAddScore.EnableWindow(FALSE);
	m_btCompareCard.EnableWindow(FALSE);
	m_btGiveUp.EnableWindow(FALSE);
	m_btLookCard.EnableWindow(FALSE);
	m_btFollow.EnableWindow(FALSE);

	//���ؿؼ�
	for (WORD i=0;i<GAME_PLAYER;i++) 
	{
		m_NumberControl[i].ResetControl();
	}
	m_JettonControl.ResetControl();
	m_ScoreView.ShowWindow(SW_HIDE);
	for (WORD i=0;i<GAME_PLAYER;i++) m_CardControl[i].SetCardData(NULL,0);

	return;
}

//�����ؼ�
VOID CGameClientView::RectifyControl(INT nWidth, INT nHeight)
{
	//WORD w=0;
	//for(WORD i=0;i<GAME_PLAYER;i++)
	//{
	//	if(˳ʱ��)
	//	{
	//		if(i==0)w=...
	//	}
	//...
	//��������
	m_ptAvatar[0].x=nWidth/2+300;
	m_ptAvatar[0].y=nHeight/2-211;
	m_ptAvatar[1].x=nWidth/2+300;
	m_ptAvatar[1].y=nHeight/2;
	m_ptAvatar[2].x=nWidth/2-10;
	m_ptAvatar[2].y=nHeight/2+204;
	m_ptAvatar[3].x=nWidth/2-355;
	m_ptAvatar[3].y=nHeight/2;
	m_ptAvatar[4].x=nWidth/2-355;
	m_ptAvatar[4].y=nHeight/2-211;

	//ʱ��λ��
	m_ptClock[0].x=nWidth/2+155;
	m_ptClock[0].y=nHeight/2-155;
	m_ptClock[1].x=nWidth/2+155;
	m_ptClock[1].y=nHeight/2-15;
	m_ptClock[2].x=nWidth/2-60;
	m_ptClock[2].y=nHeight/2+295;
	m_ptClock[3].x=nWidth/2-160;
	m_ptClock[3].y=nHeight/2-15;
	m_ptClock[4].x=nWidth/2-160;
	m_ptClock[4].y=nHeight/2-155;

	//׼��λ��
	m_ptReady[0].x=nWidth/2+105;
	m_ptReady[0].y=nHeight/2-180;
	m_ptReady[1].x=nWidth/2+105;
	m_ptReady[1].y=nHeight/2+35;
	m_ptReady[2].x=nWidth/2+10;
	m_ptReady[2].y=nHeight/2+86;
	m_ptReady[3].x=nWidth/2-105;
	m_ptReady[3].y=nHeight/2+35;
	m_ptReady[4].x=nWidth/2-105;
	m_ptReady[4].y=nHeight/2-180;

	//�û��˿�
	m_CardControl[0].SetBenchmarkPos(nWidth/2+245,nHeight/2-150,enXCenter,enYCenter);
	m_CardControl[1].SetBenchmarkPos(nWidth/2+245,nHeight/2+69,enXCenter,enYCenter);
	m_CardControl[2].SetBenchmarkPos(nWidth/2+10,nHeight/2+203,enXCenter,enYBottom);
	m_CardControl[3].SetBenchmarkPos(nWidth/2-255,nHeight/2+69,enXCenter,enYCenter);
	m_CardControl[4].SetBenchmarkPos(nWidth/2-255,nHeight/2-150,enXCenter,enYCenter);

	//���ֿؼ�λ��
	m_NumberControl[0].SetBencbmarkPos(nWidth/2+185,nHeight/2-170,enXRight);
	m_NumberControl[1].SetBencbmarkPos(nWidth/2+185,nHeight/2+40,enXRight);
	m_NumberControl[2].SetBencbmarkPos(nWidth/2-50,nHeight/2+130,enXRight);
	m_NumberControl[3].SetBencbmarkPos(nWidth/2-185,nHeight/2+40,enXLeft);
	m_NumberControl[4].SetBencbmarkPos(nWidth/2-185,nHeight/2-170,enXLeft);

	//ׯ�ұ�־
	m_PointBanker[0].x=m_ptAvatar[0].x;
	m_PointBanker[0].y=m_ptAvatar[0].y-45;
	m_PointBanker[1].x=m_ptAvatar[1].x;
	m_PointBanker[1].y=m_ptAvatar[1].y-45;
	m_PointBanker[2].x=m_ptAvatar[2].x+45;
	m_PointBanker[2].y=m_ptAvatar[2].y+10;
	m_PointBanker[3].x=m_ptAvatar[3].x;
	m_PointBanker[3].y=m_ptAvatar[3].y-45;
	m_PointBanker[4].x=m_ptAvatar[4].x;
	m_PointBanker[4].y=m_ptAvatar[4].y-45;

	//����λ��
	m_ptJettons[0].x=m_ptAvatar[0].x;
	m_ptJettons[0].y=m_ptAvatar[0].y+50;
	m_ptJettons[1].x=m_ptAvatar[1].x;
	m_ptJettons[1].y=m_ptAvatar[1].y+30;
	m_ptJettons[2].x=m_ptAvatar[2].x;
	m_ptJettons[2].y=m_ptAvatar[2].y-10;
	m_ptJettons[3].x=m_ptAvatar[3].x;
	m_ptJettons[3].y=m_ptAvatar[3].y+30;
	m_ptJettons[4].x=m_ptAvatar[4].x;
	m_ptJettons[4].y=m_ptAvatar[4].y+50;

	m_ptUserCard[0].x=m_CardControl[0].GetCardPos().x;
	m_ptUserCard[0].y=m_CardControl[0].GetCardPos().y;
	m_ptUserCard[1].x=m_CardControl[1].GetCardPos().x-40;
	m_ptUserCard[1].y=m_CardControl[1].GetCardPos().y+60;
	m_ptUserCard[2].x=m_CardControl[2].GetCardPos().x;
	m_ptUserCard[2].y=m_CardControl[2].GetCardPos().y-20;
	m_ptUserCard[3].x=m_CardControl[3].GetCardPos().x+40;
	m_ptUserCard[3].y=m_CardControl[3].GetCardPos().y+39;
	m_ptUserCard[4].x=m_CardControl[4].GetCardPos().x+80;
	m_ptUserCard[4].y=m_CardControl[4].GetCardPos().y+20;

	//��ť�ؼ�
	CRect rcButton;
	HDWP hDwp=BeginDeferWindowPos(32);
	const UINT uFlags=SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOSIZE;

	//��ʼ��ť
	m_btStart.GetWindowRect(&rcButton);
	DeferWindowPos(hDwp,m_btStart,NULL,nWidth/2+53,nHeight/2+258,0,0,uFlags);

	//���ư�ť
	m_btAddScore.GetWindowRect(&rcButton);
	//INT n = m_nYBorder;
	DeferWindowPos(hDwp,m_btAddScore,NULL,nWidth/2+22,nHeight/2+281,0,0,uFlags);
	DeferWindowPos(hDwp,m_btFollow,NULL,nWidth/2+95,nHeight/2+268,0,0,uFlags);
	DeferWindowPos(hDwp,m_btLookCard,NULL,nWidth/2+165,nHeight/2+244,0,0,uFlags);
	DeferWindowPos(hDwp,m_btGiveUp,NULL,nWidth/2+284,nHeight/2+156,0,0,uFlags);
	DeferWindowPos(hDwp,m_btCompareCard,NULL,nWidth/2+229,nHeight/2+204,0,0,uFlags);
	DeferWindowPos(hDwp,m_btOpenCard,NULL,nWidth/2+229,nHeight/2+204,0,0,uFlags);

	//����λ��
	m_ptUserCompare.SetPoint(nWidth/2,nHeight*2/5);

	int iX=nWidth/2+80;
	int iY=nHeight/2+200;

	m_btMaxScore.GetWindowRect(&rcButton);
	DeferWindowPos(hDwp,m_btMinScore,NULL,iX+140,iY+3,0,0,uFlags);
	DeferWindowPos(hDwp,m_btMaxScore,NULL,iX+20,iY+80,0,0,uFlags);
	DeferWindowPos(hDwp,m_btCancel,NULL,iX+73,iY+80,0,0,uFlags);
	DeferWindowPos(hDwp,m_btConfirm,NULL,iX+125,iY+80,0,0,uFlags);

	//�����ƶ�
	EndDeferWindowPos(hDwp);

	//��ע����
	m_GoldControl.SetBasicPoint(iX,iY);

	//������ͼ
	CRect rcControl;
	m_ScoreView.GetWindowRect(&rcControl);
	m_ScoreView.SetWindowPos(NULL,(nWidth-rcControl.Width())/2,nHeight/2-190,0,0,SWP_NOZORDER|SWP_NOSIZE);

	//����ؼ�
	m_JettonControl.SetBenchmarkPos(nWidth/2,nHeight/2-35);
}

//�滭����
VOID CGameClientView::DrawGameView(CDC * pDC, INT nWidth, INT nHeight)
{
	//����Ч��
	if(m_bFalsh)
	{
		int xValue=rand()%ROCK;
		int yValue=rand()%ROCK;
		if(xValue%2==0)xValue*=(-1);
		if(yValue%2==0)yValue*=(-1);
		m_ImageViewBack.TransDrawImage(pDC,(nWidth/2-512)+xValue,(nHeight/2-397)+yValue,
			m_ImageViewBack.GetWidth(),m_ImageViewBack.GetHeight(),0,0,RGB(255,0,255));
	}
	else 
	{
		m_ImageViewBack.TransDrawImage(pDC,(nWidth/2-512+6),(nHeight/2-383),
			m_ImageViewBack.GetWidth(),m_ImageViewBack.GetHeight(),0,0,RGB(255,0,255));

		//�滭����
		DrawViewImage(pDC,m_ImageViewCenter,DRAW_MODE_SPREAD);//������󻯴���
		DrawViewImage(pDC,m_ImageViewBack,DRAW_MODE_CENTENT);
	}

	if(!m_bStopDraw)
	{
		//�滭�û�
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			//��������
			WORD wUserTimer=GetUserClock(i);
			IClientUserItem * pClientUserItem=GetClientUserItem(i);
#ifdef _DEBUG
			//���Դ���
			//tagUserData Obj;
			//Obj.cbGender = 1;
			//_sntprintf(Obj.szName,sizeof(Obj.szName),TEXT("�û�������"));
			//Obj.cbUserStatus=US_READY;
			////Obj.wFaceID = 2;
			//pClientUserItem = &Obj;

			////ׯ�ұ�־
			//CImageHandle ImageHandleBanker(&m_ImageBanker);
			//m_ImageBanker.TransDrawImage(pDC,m_PointBanker[i].x,m_PointBanker[i].y,m_ImageBanker.GetWidth(),m_ImageBanker.GetHeight(),0,0,RGB(255,0,255));
#endif
			//�滭�û�
			if (pClientUserItem!=NULL)
			{
				//�û�����
				pDC->SetTextAlign(TA_CENTER);
				pDC->SetTextColor((wUserTimer>0)?RGB(250,250,250):RGB(220,220,220));
				DrawTextString(pDC,pClientUserItem->GetNickName(),RGB(255,255,255),RGB(0,0,0),m_ptAvatar[i].x+25,m_ptAvatar[i].y+55);

				//�û����
				TCHAR szBuffer[64]=TEXT("");
				LONGLONG lTempScore=((m_ScoreView.IsWindowVisible()==TRUE || m_NumberControl[i].GetScore()!=0)?(0):(m_lTableScore[i]));
				if(m_btStart.IsWindowVisible()==TRUE || m_cbPlayStatus[i]==FALSE)lTempScore=0;
				LONGLONG lLeaveScore=pClientUserItem->GetUserScore()-lTempScore;
				if(m_lStopUpdataScore[i]>0)lLeaveScore=m_lStopUpdataScore[i]-m_lTableScore[i];
				_sntprintf(szBuffer,sizeof(szBuffer),TEXT("%I64d"),lLeaveScore);
				DrawTextString(pDC,szBuffer,RGB(255,255,255),RGB(0,0,0),m_ptAvatar[i].x+20,m_ptAvatar[i].y+68);

				//������Ϣ
				//wUserTimer =1;
				if (wUserTimer!=0) 
				{
					if(m_CardControl[MY_SELF_CHAIRID].GetCardCount()==0)
						DrawUserClock(pDC,m_ptClock[i].x,m_ptClock[i].y,wUserTimer);
					else DrawUserClock(pDC,m_ptClock[i].x,m_ptClock[i].y,wUserTimer);
				}

				//׼����־
				if (pClientUserItem->GetUserStatus()==US_READY) 
				{
					DrawUserReady(pDC,m_ptReady[i].x,m_ptReady[i].y);
				}
				DrawUserAvatar(pDC,m_ptAvatar[i].x,m_ptAvatar[i].y,pClientUserItem);
			}
		}

		//ׯ����Ϣ
		if (m_wBankerUser<5)
		{
			//ׯ�ұ�־
			m_ImageBanker.TransDrawImage(pDC,m_PointBanker[m_wBankerUser].x,m_PointBanker[m_wBankerUser].y,m_ImageBanker.GetWidth(),m_ImageBanker.GetHeight(),0,0,RGB(255,0,255));
		}

		//�滭����
		m_JettonControl.DrawJettonControl(pDC);

		//������ע
		LONGLONG lTableScore=0L;
		for (WORD i=0;i<GAME_PLAYER;i++) lTableScore+=m_lTableScore[i];

		//��ע��Ϣ
		if (lTableScore>0L)
		{	
			int iCount=0;
			LONGLONG lCell[11];
			ZeroMemory(lCell,sizeof(lCell));
			while(lTableScore>0L)
			{
				LONGLONG lScore=lTableScore%10;
				lCell[iCount++] = lScore;
				lTableScore=lTableScore/10;
			}
			int iTemp=0;
			int iX=m_ImageNumber.GetWidth();
			for (int i=iCount-1;i>=0;i--)
			{
				m_ImageNumber.TransDrawImage(pDC,nWidth/2+39+(iX/10)*(iTemp++),nHeight/2-296,
					iX/10,m_ImageNumber.GetHeight(),iX/10*(int)lCell[i],0,RGB(255,0,255));
			}
		}

		//�÷����ֹ�������
		for( WORD i = 0; i < GAME_PLAYER; i++ )
		{
			m_NumberControl[i].DrawNumberControl(pDC);
		}

		//�滭����
		for (BYTE i=0;i<GAME_PLAYER;i++)
		{
			if (m_lTableScore[i]==0L) continue;

			//�滭��Ŀ
			TCHAR szBuffer[64];
			_sntprintf(szBuffer,CountArray(szBuffer),TEXT("%I64d"),m_lTableScore[i]);

			//����λ��
			CRect rcDrawRect;
			rcDrawRect.top=m_CardControl[i].GetCardPos().y;
			rcDrawRect.left=m_CardControl[i].GetCardPos().x+((i>2)?(135):(-50));
			rcDrawRect.right=rcDrawRect.left+30;
			rcDrawRect.bottom=rcDrawRect.top+12;

			//��������
			CFont ViewFont;
			ViewFont.CreateFont(-14,0,0,0,700,0,0,0,134,3,2,1,2,TEXT("����"));

			//��������
			CFont * pOldFont=pDC->SelectObject(&ViewFont);

			//�滭��Ŀ
			pDC->SetTextAlign(TA_LEFT);
			DrawTextString(pDC,szBuffer,RGB(250,200,40),RGB(0,0,0),&rcDrawRect,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);
			
			//��������
			pDC->SelectObject(pOldFont);
			ViewFont.DeleteObject();
		}

		//�滭��Ϣ
		if(m_lCellScore>0)
		{
			//��������
			CFont ViewFont;
			ViewFont.CreateFont(-12,0,0,0,700,0,0,0,134,3,2,1,2,TEXT("����"));

			//��������
			CFont * pOldFont=pDC->SelectObject(&ViewFont);

			pDC->SetTextAlign(TA_LEFT);
			pDC->SetTextColor(RGB(0,0,0));

			//����λ��
			CRect rcDrawRect;

			//�滭��Ŀ
			TCHAR szBuffer[64];
			_sntprintf(szBuffer,CountArray(szBuffer),TEXT("�ⶥ��%I64d"),m_lMaxCellScore);
			rcDrawRect.top=nHeight/2-338;
			rcDrawRect.left=nWidth/2-233;
			rcDrawRect.right=rcDrawRect.left+50;
			rcDrawRect.bottom=rcDrawRect.top+12;
			DrawText(pDC,szBuffer,lstrlen(szBuffer),&rcDrawRect,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

			_sntprintf(szBuffer,CountArray(szBuffer),TEXT("��ע��%I64d"),m_lCellScore);
			rcDrawRect.top=nHeight/2-318;
			rcDrawRect.left=nWidth/2-230;
			rcDrawRect.right=rcDrawRect.left+50;
			rcDrawRect.bottom=rcDrawRect.top+12;
			DrawText(pDC,szBuffer,lstrlen(szBuffer),&rcDrawRect,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

			//��������
			pDC->SelectObject(pOldFont);		
			ViewFont.DeleteObject();
		}

		//ѡ�ȱ�־
		if(m_bCompareCard)
		{
			for (int i=0;i<GAME_PLAYER;i++)
			{
				if(m_bCompareUser[i]==TRUE)
				{
					CPoint cPost=m_CardControl[i].GetCardPos();
					m_ImageArrowhead.TransDrawImage(pDC,cPost.x,cPost.y-m_ImageArrowhead.GetHeight(),
						m_ImageArrowhead.GetWidth(),m_ImageArrowhead.GetHeight(),0,0,RGB(255,0,255));
				}
			}
		}
	}

	//���ƻ滭
	if (m_SendCardItemArray.GetCount()>0)
	{
		//��������
		tagSendCardItem * pSendCardItem=&m_SendCardItemArray[0];

		//��ȡ��С
		int nItemWidth=m_ImageCard.GetWidth()/13;
		int nItemHeight=m_ImageCard.GetHeight()/5;

		//�滭�˿�
		int nXPos=m_SendCardPos.x-nItemWidth/2;
		int nYPos=m_SendCardPos.y-nItemHeight/2+10;
		m_ImageCard.TransDrawImage(pDC,nXPos,nYPos,nItemWidth,nItemHeight,nItemWidth*2,nItemHeight*4,RGB(255,0,255));
	}

	//�����˿�
	if (m_wLoserUser!=INVALID_CHAIR)
	{
		//����״̬
		if (m_wConmareIndex<m_wConmareCount)
		{
			//�滭�˿�
			for (BYTE i=0;i<2;i++)
			{
				//��������
				CPoint PointCard;
				CPoint PointStart=m_ptUserCard[m_wCompareChairID[i]];

				//����λ��
				INT nQuotiety=(i==0)?-1:1;
				PointCard.y=PointStart.y+(m_ptUserCompare.y-PointStart.y)*m_wConmareIndex/m_wConmareCount;
				PointCard.x=PointStart.x+(m_ptUserCompare.x-PointStart.x+COMPARE_RADII*nQuotiety)*m_wConmareIndex/m_wConmareCount;

				//�滭�˿�
				DrawCompareCard(pDC,PointCard.x,PointCard.y,false);
			}
		}

		//����״̬
		if ((m_wConmareIndex>=m_wConmareCount)&&((m_wConmareIndex<m_wConmareCount+VSCARD_COUNT*VSCARD_TIME)))
		{
			//�滭�˿�
			for (BYTE i=0;i<2;i++)
			{
				//��������
				CPoint PointCard;
				CPoint PointStart=m_ptUserCard[m_wCompareChairID[i]];

				//����λ��
				INT nQuotiety=(i==0)?-1:1;
				PointCard.y=m_ptUserCompare.y;
				PointCard.x=m_ptUserCompare.x+COMPARE_RADII*nQuotiety;

				//�滭�˿�
				DrawCompareCard(pDC,PointCard.x,PointCard.y,false);
			}

			//���Ʊ�־
			CSize SizeVSCard;
			CPngImage ImageVSCard;
			ImageVSCard.LoadImage(AfxGetInstanceHandle(),TEXT("VS_CARD"));
			SizeVSCard.SetSize(ImageVSCard.GetWidth()/VSCARD_COUNT,ImageVSCard.GetHeight());

			//�滭��־
			INT nImageIndex=(m_wConmareIndex-m_wConmareCount)%VSCARD_COUNT;
			ImageVSCard.DrawImage(pDC,m_ptUserCompare.x-SizeVSCard.cx/2,m_ptUserCompare.y-SizeVSCard.cy/2,
				SizeVSCard.cx,SizeVSCard.cy,nImageIndex*SizeVSCard.cx,0);

			//����Ч��
			CGameClientEngine * pGameClientEngine=CONTAINING_RECORD(this,CGameClientEngine,m_GameClientView);
			pGameClientEngine->PlayGameSound(AfxGetInstanceHandle(),TEXT("COMPARE_CARD"));

		}

		//����״̬
		if (m_wConmareIndex>=m_wConmareCount+VSCARD_COUNT*VSCARD_TIME)
		{
			//��������
			WORD wConmareIndex=m_wConmareIndex-m_wConmareCount-VSCARD_COUNT*VSCARD_TIME;

			//�滭�˿�
			for (BYTE i=0;i<2;i++)
			{
				//��������
				CPoint PointCard;
				CPoint PointStart=m_ptUserCard[m_wCompareChairID[i]];

				//����λ��
				INT nQuotiety=(i==0)?-1:1;
				PointCard.y=m_ptUserCompare.y+(PointStart.y-m_ptUserCompare.y)*wConmareIndex/m_wConmareCount;
				PointCard.x=m_ptUserCompare.x+COMPARE_RADII*nQuotiety+(PointStart.x-m_ptUserCompare.x-COMPARE_RADII*nQuotiety)*wConmareIndex/m_wConmareCount;

				//�滭�˿�
				DrawCompareCard(pDC,PointCard.x,PointCard.y,(m_wLoserUser==m_wCompareChairID[i])?true:false);
			}

			//����Ч��
			if(m_wConmareIndex==m_wConmareCount+VSCARD_COUNT*VSCARD_TIME)
			{
				CGameClientEngine * pGameClientEngine=CONTAINING_RECORD(this,CGameClientEngine,m_GameClientView);
				pGameClientEngine->PlayGameSound(AfxGetInstanceHandle(),TEXT("CENTER_SEND_CARD"));
			}
		}
	}

	//�ȴ�ѡ��
	if(m_wWaitUserChoice!=INVALID_CHAIR)
	{
		TCHAR szBuffer[64]=TEXT("");

		//��������
		CFont ViewFont;
		ViewFont.CreateFont(-15,0,0,0,700,0,0,0,134,3,2,1,2,TEXT("����"));

		//��������
		CFont * pOldFont=pDC->SelectObject(&ViewFont);
		pDC->SetTextAlign(TA_CENTER);

		//�����Ϣ
		if(m_wWaitUserChoice==TRUE)_sntprintf(szBuffer,sizeof(szBuffer),TEXT("�ȴ����ѡ�����"));
		else _sntprintf(szBuffer,sizeof(szBuffer),TEXT("��ѡ����ҽ��б���"));
		DrawTextString(pDC,szBuffer,RGB(50,50,50),RGB(0,170,0),nWidth/2+10,nHeight/2+45);

		//��������
		pDC->SelectObject(pOldFont);
		ViewFont.DeleteObject();
	}

	return;
}

//��ʼ���ֹ���
void CGameClientView::BeginMoveNumber()
{
	BOOL bSuccess = FALSE;
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		if( m_NumberControl[i].BeginScrollNumber() )
			bSuccess = TRUE;
	}
	if( bSuccess )
	{
		SetTimer( IDI_MOVE_NUMBER,TIME_MOVE_NUMBER,NULL );
		RefreshGameView( );
	}

	return;
}

//ֹͣ���ֹ���
void CGameClientView::StopMoveNumber()
{
	BOOL bMoving = FALSE;
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		if( m_NumberControl[i].FinishScrollNumber() )
			bMoving = TRUE;
	}
	if( bMoving )
	{
		KillTimer( IDI_MOVE_NUMBER );
		RefreshGameView(  );
	}

	return;
}

//������ע
void CGameClientView::SetUserTableScore(WORD wChairID, LONGLONG lTableScore,LONGLONG lCurrentScore)
{
	//��������
	if (wChairID!=INVALID_CHAIR) 
	{
		m_lTableScore[wChairID]=lTableScore;
		m_JettonControl.AddScore(lCurrentScore,m_ptJettons[wChairID]);
	}
	else 
	{
		ZeroMemory(m_tcBuffer,sizeof(m_tcBuffer));
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			m_lDrawGold[i].RemoveAll();
			m_ptKeepJeton[i].RemoveAll();
		}
		ZeroMemory(m_lTableScore,sizeof(m_lTableScore));
	}

	//���½���
	RefreshGameView();

	return;
}

//��ʼ���붯��
void CGameClientView::BeginMoveJettons()
{
	if( m_JettonControl.BeginMoveJettons() )
	{
		SetTimer( IDI_MOVE_JETTON,TIME_MOVE_JETTON,NULL );

		//���½���
		RefreshGameView();
	}
	return;
}

//ֹͣ���붯��
void CGameClientView::StopMoveJettons()
{
	if( m_JettonControl.FinishMoveJettons() )
	{
		KillTimer( IDI_MOVE_JETTON );

		//���½���
		RefreshGameView(  );
	}
	return;
}

//ʤ�����
void CGameClientView::SetGameEndInfo( WORD wWinner )
{
	if( wWinner == INVALID_CHAIR ) return ;

	m_JettonControl.RemoveAllScore( m_ptJettons[wWinner] );
	if( m_JettonControl.BeginMoveJettons() )
		SetTimer( IDI_MOVE_JETTON,TIME_MOVE_JETTON,NULL );

	//���½���
	RefreshGameView(  );
	return;
}

//�ȴ�ѡ��
void CGameClientView::SetWaitUserChoice(WORD wChoice)
{
	m_wWaitUserChoice = wChoice;

	//���½���
	RefreshGameView();

	return;
}

//ׯ�ұ�־
void CGameClientView::SetBankerUser(WORD wBankerUser)
{
	m_wBankerUser=wBankerUser;

	//���½���
	RefreshGameView();
	return;
}

////�ȴ���־
//void CGameClientView::SetWaitInvest(bool bInvest)
//{
//	if(m_bInvest!=bInvest)
//	{
//		m_bInvest=bInvest;
//
//		//���½���
//		RefreshGameView(NULL);
//	}
//	return;
//}

//������Ϣ
void CGameClientView::SetScoreInfo(LONGLONG lTurnMaxScore,LONGLONG lTurnLessScore)
{
	m_lMaxCellScore=lTurnMaxScore;
	m_lCellScore=lTurnLessScore;

	//���½���
	RefreshGameView();
	return;
}

////��ʾ����
//void CGameClientView::DisplayType(bool bShow)
//{
//	if(m_bShow!=bShow)
//	{
//		m_bShow=bShow;
//
//		//���½���
//		RefreshGameView(NULL);
//	}
//}

//���Ʊ�־
void CGameClientView::SetCompareCard(bool bCompareCard,BOOL bCompareUser[])
{
	m_bCompareCard=bCompareCard;
	if(bCompareUser!=NULL)
	{
		CopyMemory(m_bCompareUser,bCompareUser,sizeof(m_bCompareUser));
		for(int i=0;i<GAME_PLAYER;i++)
		{
			if(m_bCompareUser[i]==TRUE)m_CardControl[i].SetCompareCard(true);
			else m_CardControl[i].SetCompareCard(false);
		}
	}
	//���½���
	RefreshGameView();

	if(!bCompareCard)
	{
		for(int i=0;i<GAME_PLAYER;i++)
		{
			m_CardControl[i].SetCompareCard(false) ;
		}
	}

	return;
}

//ֹͣ����
void CGameClientView::StopUpdataScore(bool bStop)
{
	//��������
	ZeroMemory(m_lStopUpdataScore,sizeof(m_lStopUpdataScore));

	//������Ŀ
	if(bStop)
	{
		//��ȡ�û�
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			//��������
			IClientUserItem * pClientUserItem=GetClientUserItem(i);
			if(pClientUserItem!=NULL)
			{
				m_lStopUpdataScore[i]=pClientUserItem->GetUserScore();
			}
		}
	}

	//���½���
	RefreshGameView();

	return;
}

//�滭�˿�
void CGameClientView::DrawCompareCard(CDC * pDC, INT nXPos, INT nYPos, bool bChapped)
{
	//������Դ
	CSize SizeGameCard(m_ImageCard.GetWidth()/13,m_ImageCard.GetHeight()/5);

	//����λ��
	nYPos-=SizeGameCard.cy/2;
	nXPos-=(SizeGameCard.cx+DEFAULT_PELS*(MAX_COUNT-1))/2;

	//�滭�˿�
	for (BYTE i=0;i<MAX_COUNT;i++)
	{
		INT nXDrawPos=nXPos+DEFAULT_PELS*i;
		m_ImageCard.TransDrawImage(pDC,nXDrawPos,nYPos,SizeGameCard.cx,SizeGameCard.cy,
			SizeGameCard.cx*((bChapped==true)?4:2),SizeGameCard.cy*4,RGB(255,0,255));
	}

	return;
}

//��������
void CGameClientView::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos)
{
	//��������
	int nStringLength=lstrlen(pszString);
	int nXExcursion[8]={1,1,1,0,-1,-1,-1,0};
	int nYExcursion[8]={-1,0,1,1,1,0,-1,-1};

	//�滭�߿�
	pDC->SetTextColor(crFrame);
	for (int i=0;i<CountArray(nXExcursion);i++)
	{
		TextOut(pDC,nXPos+nXExcursion[i],nYPos+nYExcursion[i],pszString,nStringLength);
	}

	//�滭����
	pDC->SetTextColor(crText);
	TextOut(pDC,nXPos,nYPos,pszString,nStringLength);

	return;
}

//������ͼ
void CGameClientView::RefreshGameView()
{
	CRect rect;
	GetClientRect(&rect);
	InvalidGameView(rect.left,rect.top,rect.Width(),rect.Height());

	return;
}

//��������
void CGameClientView::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, LPRECT lpRect,UINT nFormat)
{
	//��������
	int nStringLength=lstrlen(pszString);
	int nXExcursion[8]={1,1,1,0,-1,-1,-1,0};
	int nYExcursion[8]={-1,0,1,1,1,0,-1,-1};

	//�滭�߿�
	pDC->SetTextColor(crFrame);
	CRect rcDraw;
	for (int i=0;i<CountArray(nXExcursion);i++)
	{
		rcDraw.CopyRect(lpRect);
		rcDraw.OffsetRect(nXExcursion[i],nYExcursion[i]);
		DrawText(pDC,pszString,nStringLength,&rcDraw,nFormat);
	}

	//�滭����
	rcDraw.CopyRect(lpRect);
	pDC->SetTextColor(crText);
	DrawText(pDC,pszString,nStringLength,&rcDraw,nFormat);

	return;
}

//���ƶ���
void CGameClientView::PerformCompareCard(WORD wCompareUser[2], WORD wLoserUser)
{
	//Ч��״̬
	ASSERT(m_wLoserUser==INVALID_CHAIR);
	if (m_wLoserUser!=INVALID_CHAIR) return;

	//���ñ���
	m_wConmareIndex=0;
	m_wLoserUser=wLoserUser;
	m_wConmareCount=LESS_SEND_COUNT;

	//�Ա��û�
	m_wCompareChairID[0]=wCompareUser[0];
	m_wCompareChairID[1]=wCompareUser[1];

	//��������
	for (BYTE i=0;i<2;i++)
	{
		//λ�Ƽ���
		INT nXCount=abs(m_ptUserCard[wCompareUser[i]].x-m_ptUserCompare.x)/SEND_PELS;
		INT nYCount=abs(m_ptUserCard[wCompareUser[i]].y-m_ptUserCompare.y)/SEND_PELS;

		//��������
		m_wConmareCount=__max(m_wConmareCount,__max(nXCount,nYCount));
	}

	//�����˿�
	m_CardControl[wCompareUser[0]].SetCardData(NULL,0);
	m_CardControl[wCompareUser[1]].SetCardData(NULL,0);

	//����ʱ��
	if (!m_bCartoon)
	{
		m_bCartoon=true;
		SetTimer(IDI_CARTOON,TIME_CARTOON,NULL);
	}

	//����Ч��
	CGameClientEngine * pGameClientEngine=CONTAINING_RECORD(this,CGameClientEngine,m_GameClientView);
	pGameClientEngine->PlayGameSound(AfxGetInstanceHandle(),TEXT("CENTER_SEND_CARD"));
	return;
}

//���ƶ���
void CGameClientView::DispatchUserCard(WORD wChairID, BYTE cbCardData)
{
	//Ч�����
	ASSERT(wChairID<GAME_PLAYER);
	if (wChairID>=GAME_PLAYER) return;

	//���ñ���
	tagSendCardItem SendCardItem;
	SendCardItem.wChairID=wChairID;
	SendCardItem.cbCardData=cbCardData;
	m_SendCardItemArray.Add(SendCardItem);

	//��������
	if (m_SendCardItemArray.GetCount()==1) 
	{
		//��ȡλ��
		CRect rcClient;
		GetClientRect(&rcClient);

		//����λ�� /2
		m_SendCardPos.x=rcClient.Width()/2+200;
		m_SendCardPos.y=rcClient.Height()/2-308;
		m_KeepPos.x =m_SendCardPos.x;
		m_KeepPos.y=m_SendCardPos.y;

		if(!m_bCartoon)
		{
			m_bCartoon = true;
			//���ö�ʱ��
			SetTimer(IDI_CARTOON,TIME_CARTOON/2,NULL);
		}

		//��������
		CGameClientEngine * pGameClientEngine=CONTAINING_RECORD(this,CGameClientEngine,m_GameClientView);
		pGameClientEngine->PlayGameSound(AfxGetInstanceHandle(),TEXT("SEND_CARD"));
	}

	return;
}

//���ƶ���
bool CGameClientView::bFalshCard(WORD wFalshUser[])
{
	for(int i=0;i<GAME_PLAYER;i++)
	{
		if(wFalshUser[i]<GAME_PLAYER)m_wFlashUser[i] = wFalshUser[i];
	}

	//���ö�ʱ��
	m_bStopDraw=true;

	if(!m_bCartoon)
	{
		m_bCartoon = true;
		SetTimer(IDI_CARTOON,TIME_CARTOON,NULL);
	}

	return true;
}

//���ƴ���
bool CGameClientView::SendCard()
{
	//�����ж�
	if (m_SendCardItemArray.GetCount()==0) return false;

	//��ȡλ��
	CRect rcClient;
	GetClientRect(&rcClient);

	//��������
	int nXExcursion[GAME_PLAYER]={-1,1,-1,-1,-1};
	int nYExcursion[GAME_PLAYER]={0,1,1,1,1};

	//����λ��
	tagSendCardItem * pSendCardItem=&m_SendCardItemArray[0];
	WORD bID=pSendCardItem->wChairID;
	BYTE bTimes;
	if(bID==0)bTimes=4;
	else if(bID==1)bTimes=3;
	else if(bID==2)bTimes=2;
	else if(bID>=3)bTimes=2;

	m_SendCardPos.x+=nXExcursion[bID]*(abs(m_ptUserCard[bID].x-m_KeepPos.x)/SPEED*bTimes);
	m_SendCardPos.y+=nYExcursion[bID]*(abs(m_ptUserCard[bID].y-m_KeepPos.y)/SPEED*bTimes);

	//���½���
	RefreshGameView();

	//ֹͣ�ж�
	int bTempX=0,bTempY=0;
	bTempX=m_ptUserCard[bID].x;
	bTempY=m_ptUserCard[bID].y;
	if((pSendCardItem->wChairID==0 && m_SendCardPos.x<=bTempX)||(pSendCardItem->wChairID>=1 && m_SendCardPos.y>=bTempY))
	{
		//��ȡ�˿�
		BYTE cbCardData[MAX_COUNT];
		WORD wChairID=pSendCardItem->wChairID;
		BYTE cbCardCount=(BYTE)m_CardControl[wChairID].GetCardData(cbCardData,CountArray(cbCardData));

		//�����˿�&& pSendCardItem->cbCardData!=0
		if (cbCardCount<MAX_COUNT)
		{
			cbCardData[cbCardCount++]=pSendCardItem->cbCardData;
			m_CardControl[wChairID].SetCardData(cbCardData,cbCardCount);
		}

		//ɾ������
		m_SendCardItemArray.RemoveAt(0);

		//��������
		if (m_SendCardItemArray.GetCount()>0)
		{
			//����λ�� /2
			GetClientRect(&rcClient);
			m_SendCardPos.x=rcClient.Width()/2+200;
			m_SendCardPos.y=rcClient.Height()/2-308;

			//��������
			CGameClientEngine * pGameClientEngine=CONTAINING_RECORD(this,CGameClientEngine,m_GameClientView);
			pGameClientEngine->PlayGameSound(AfxGetInstanceHandle(),TEXT("SEND_CARD"));

			return true;
		}
		else
		{
			//��ɴ���
			SendEngineMessage(IDM_SEND_CARD_FINISH,0,0);

			return false;
		}
	}

	return true;
}

//���ƴ���
bool CGameClientView::FlashCard()
{	
	//�����ж�
	for(int i=0;i<GAME_PLAYER;i++) if(m_wFlashUser[i]<GAME_PLAYER)break;
	if(i==GAME_PLAYER)return false;

	//���ñ���
	m_wFalshCount++;
	WORD wTimes=m_wFalshCount*TIME_CARTOON;

	//���½���
	RefreshGameView();
	m_bFalsh=!m_bFalsh;

	//�˿˱���
	WORD wValue=(rand()%2==0)?((m_wFalshCount+1)%6):((m_wFalshCount+2)%6);
	for(int i=0;i<GAME_PLAYER;i++)
	{
		if(m_wFlashUser[i]<GAME_PLAYER)m_CardControl[m_wFlashUser[i]].SetCardColor(wValue);
	}

	//ֹͣ����
	if(2200 <= wTimes)
	{
		//���ò���
		m_bFalsh=false;
		m_bStopDraw=false;
		m_wFalshCount=0;
		for(int i=0;i<GAME_PLAYER;i++)
		{
			if(m_wFlashUser[i]<GAME_PLAYER)
			{
				m_CardControl[m_wFlashUser[i]].SetCardColor(INVALID_CHAIR);
				m_wFlashUser[i]=INVALID_CHAIR;
			}
		}

		//��ɴ���
		SendEngineMessage(IDM_FALSH_CARD_FINISH,0,0);

		//���½���
		RefreshGameView();

		return false; 
	}

	return true; 
}

//���ƴ���
bool CGameClientView::CompareCard()
{
	//�����ж�
	if (m_wLoserUser==INVALID_CHAIR) return false;

	//��������
	m_wConmareIndex++;

	//���½���
	RefreshGameView();

	//ֹͣ�ж�
	if (m_wConmareIndex>=((m_wConmareCount*2)+VSCARD_COUNT*VSCARD_TIME))
	{
		//�����˿�
		BYTE bTemp[]={0,0,0};
		m_CardControl[m_wLoserUser].SetCardColor(2);
		m_CardControl[m_wCompareChairID[0]].SetCardData(bTemp,MAX_COUNT);
		m_CardControl[m_wCompareChairID[1]].SetCardData(bTemp,MAX_COUNT);

		//���ñ���
		m_wConmareIndex=0;
		m_wConmareCount=0;
		m_wLoserUser=INVALID_CHAIR;
		ZeroMemory(m_wCompareChairID,sizeof(m_wCompareChairID));

		//������Ϣ
		SendEngineMessage(IDM_FALSH_CARD_FINISH,0,0);

		return false;
	}

	return true;
}

//ֹͣ����
void CGameClientView::StopFlashCard()
{
	//�����ж�
	for(int i=0;i<GAME_PLAYER;i++) if(m_wFlashUser[i]<GAME_PLAYER)break;
	if(i==GAME_PLAYER)return ;

	//���ò���
	m_bFalsh=false;
	m_bStopDraw=false;
	m_wFalshCount=0;
	for(int i=0;i<GAME_PLAYER;i++)
	{
		if(m_wFlashUser[i]<GAME_PLAYER)
		{
			m_CardControl[m_wFlashUser[i]].SetCardColor(INVALID_CHAIR);
			m_wFlashUser[i]=INVALID_CHAIR;
		}
	}

	//��ɴ���
	SendEngineMessage(IDM_FALSH_CARD_FINISH,0,0);

	return ;
}

//ֹͣ����
void CGameClientView::StopCompareCard()
{
	if(m_wLoserUser>GAME_PLAYER)return ;

	//�����˿�
	BYTE bTemp[]={0,0,0};
	m_CardControl[m_wLoserUser].SetCardColor(2);
	m_CardControl[m_wCompareChairID[0]].SetCardData(bTemp,MAX_COUNT);
	m_CardControl[m_wCompareChairID[1]].SetCardData(bTemp,MAX_COUNT);

	//���ñ���
	m_wConmareIndex=0;
	m_wConmareCount=0;
	m_wLoserUser=INVALID_CHAIR;
	ZeroMemory(m_wCompareChairID,sizeof(m_wCompareChairID));

	//������Ϣ
	SendEngineMessage(IDM_FALSH_CARD_FINISH,0,0);

	return ;
}

//��ɷ���
void CGameClientView::FinishDispatchCard()
{
	//����ж�
	if (m_SendCardItemArray.GetCount()==0) return;

	//��ɶ���
	for (INT_PTR i=0;i<m_SendCardItemArray.GetCount();i++)
	{
		//��ȡ����
		tagSendCardItem * pSendCardItem=&m_SendCardItemArray[i];

		//��ȡ�˿�
		BYTE cbCardData[MAX_COUNT];
		WORD wChairID=pSendCardItem->wChairID;
		BYTE cbCardCount=(BYTE)m_CardControl[wChairID].GetCardData(cbCardData,CountArray(cbCardData));

		//�����˿�
		if (cbCardCount<MAX_COUNT)
		{
			cbCardData[cbCardCount++]=pSendCardItem->cbCardData;
			m_CardControl[wChairID].SetCardData(cbCardData,cbCardCount);
		}
	}

	//ɾ������
	m_SendCardItemArray.RemoveAll();

	//��ɴ���
	SendEngineMessage(IDM_SEND_CARD_FINISH,0,0);

	//���½���
	RefreshGameView();

	return;
}

//��ʱ����Ϣ
void CGameClientView::OnTimer(UINT nIDEvent)
{
	if (nIDEvent==IDI_CARTOON)		//������ʱ
	{
		//��������
		bool bKillTimer=true;

		//���ƶ���
		if(m_bStopDraw && (FlashCard()==true))
		{
			bKillTimer=false;
		}

		//���ƶ���
		if ((m_wLoserUser!=INVALID_CHAIR)&&(CompareCard()==true))
		{
			bKillTimer=false;
		}

		//���ƶ���
		if ((m_SendCardItemArray.GetCount()>0)&&(SendCard()==true))
		{
			bKillTimer=false;
		}

		//ɾ��ʱ��
		if (bKillTimer==true)
		{
			m_bCartoon=false;
			KillTimer(nIDEvent);
		}

		return;
	}

	//���붯��
	if( IDI_MOVE_JETTON==nIDEvent)
	{
		if( !m_JettonControl.PlayMoveJettons() )
		{
			KillTimer(IDI_MOVE_JETTON);

			////������Ϣ
			//AfxGetMainWnd()->PostMessage(IDM_MOVE_JET_FINISH,0,0);

			////��ʾ���������Ϣ
			//if( m_lJettonScore < m_JettonControl.GetScore() )
			//{
			//	SetTimer( IDI_ACTION_ADD_JETTON,TIME_ACTION_ADD_JETTON,NULL );
			//}

			////��������
			//if( m_bPlayingActionSound )
			//	m_bMovingJetton = true;
			//else
			//{
			//	CGameClientEngine * pGameDlg=(CGameClientEngine *)AfxGetMainWnd();
			//	pGameDlg->PlayGameSound(AfxGetInstanceHandle(),TEXT("ADD_SCORE"));
			//}

			RefreshGameView(  );

			return;
		}

		//��ȡ��������
		CRect rcDraw;
		m_JettonControl.GetDrawRect(rcDraw);
		RefreshGameView();

		return;
	}

	//���ֹ���
	if( IDI_MOVE_NUMBER==nIDEvent )
	{
		BOOL bMoving = FALSE;
		for( INT i = 0; i < GAME_PLAYER; i++ )
		{
			if( m_NumberControl[i].PlayScrollNumber() )
			{
				bMoving = TRUE;
				//��ȡ��������
				CRect rc;
				m_NumberControl[i].GetDrawRect(rc);
				RefreshGameView();
			}
		}
		if( !bMoving )
			KillTimer(IDI_MOVE_NUMBER);
		return ;
	}

	__super::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////

