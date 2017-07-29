#ifndef CMD_HK_FIVE_CARD_HEAD_FILE
#define CMD_HK_FIVE_CARD_HEAD_FILE

//////////////////////////////////////////////////////////////////////////
//�����궨��

#define KIND_ID							6									//��Ϸ I D
#define GAME_PLAYER						5									//��Ϸ����
#define GAME_NAME						TEXT("թ��")						//��Ϸ����
#define MAX_COUNT						3									//�˿���Ŀ
#define VERSION_SERVER			    	PROCESS_VERSION(6,0,3)				//����汾
#define VERSION_CLIENT				    PROCESS_VERSION(6,1,0)				//����汾

//����ԭ��
#define GER_NO_PLAYER					0x10								//û�����
#define GER_COMPARECARD					0x20								//���ƽ���
#define GER_OPENCARD					0x30								//���ƽ���

//��Ϸ״̬
////#define GS_T_FREE					GAME_STATUS_FREE									//�ȴ���ʼ
////#define GS_T_SCORE					GAME_STATUS_PLAY								//�з�״̬
////#define GS_T_PLAYING				GAME_STATUS_PLAY+1							//��Ϸ����

//////////////////////////////////////////////////////////////////////////
//����������ṹ

#define SUB_S_GAME_START				100									//��Ϸ��ʼ
#define SUB_S_ADD_SCORE					101									//��ע���
#define SUB_S_GIVE_UP					102									//������ע
#define SUB_S_COMPARE_CARD				105									//���Ƹ�ע
#define SUB_S_LOOK_CARD					106									//���Ƹ�ע
#define SUB_S_SEND_CARD					103									//������Ϣ
#define SUB_S_GAME_END					104									//��Ϸ����
#define SUB_S_PLAYER_EXIT				107									//�û�ǿ��
#define SUB_S_OPEN_CARD					108									//������Ϣ
#define SUB_S_WAIT_COMPARE				109									//�ȴ�����

//��Ϸ״̬
struct CMD_S_StatusFree
{
	LONGLONG							lCellScore;							//��������
};

//��Ϸ״̬
struct CMD_S_StatusPlay
{
	//��ע��Ϣ
	LONGLONG							lMaxCellScore;						//��Ԫ����
	LONGLONG							lCellScore;							//��Ԫ��ע
	LONGLONG							lCurrentTimes;						//��ǰ����
	LONGLONG							lUserMaxScore;						//�û���������

	//״̬��Ϣ
	WORD								wBankerUser;						//ׯ���û�
	WORD				 				wCurrentUser;						//��ǰ���
	BYTE								cbPlayStatus[GAME_PLAYER];			//��Ϸ״̬
	bool								bMingZhu[GAME_PLAYER];				//����״̬
	LONGLONG							lTableScore[GAME_PLAYER];			//��ע��Ŀ

	//�˿���Ϣ
	BYTE								cbHandCardData[3];					//�˿�����

	//״̬��Ϣ
	bool								bCompareState;						//����״̬								
};

//��Ϸ��ʼ
struct CMD_S_GameStart
{
	//��ע��Ϣ
	LONGLONG							lMaxScore;							//�����ע
	LONGLONG							lCellScore;							//��Ԫ��ע
	LONGLONG							lCurrentTimes;						//��ǰ����
	LONGLONG							lUserMaxScore;						//��������

	//�û���Ϣ
	WORD								wBankerUser;						//ׯ���û�
	WORD				 				wCurrentUser;						//��ǰ���

	BYTE								wPlayStatus[GAME_PLAYER];////////////////////
};

//�û���ע
struct CMD_S_AddScore
{
	WORD								wCurrentUser;						//��ǰ�û�
	WORD								wAddScoreUser;						//��ע�û�
	WORD								wCompareState;						//����״̬
	LONGLONG							lAddScoreCount;						//��ע��Ŀ
	LONGLONG							lCurrentTimes;						//��ǰ����
};

//�û�����
struct CMD_S_GiveUp
{
	WORD								wGiveUpUser;						//�����û�
};

//�������ݰ�
struct CMD_S_CompareCard
{
	WORD								wCurrentUser;						//��ǰ�û�
	WORD								wCompareUser[2];					//�����û�
	WORD								wLostUser;							//�����û�
};

//�������ݰ�
struct CMD_S_LookCard
{
	WORD								wLookCardUser;						//�����û�
	BYTE								cbCardData[MAX_COUNT];				//�û��˿�
};

//�����˿�
struct CMD_S_SendCard
{
	BYTE								cbCardData[GAME_PLAYER][MAX_COUNT];	//�û��˿�
};

//�������ݰ�
struct CMD_S_OpenCard
{
	WORD								wWinner;							//ʤ���û�
};

//��Ϸ����
struct CMD_S_GameEnd
{
	LONGLONG							lGameTax;							//��Ϸ˰��
	LONGLONG							lGameScore[GAME_PLAYER];			//��Ϸ�÷�
	BYTE								cbCardData[GAME_PLAYER][3];			//�û��˿�
	WORD								wCompareUser[GAME_PLAYER][4];		//�����û�
	WORD								wEndState;							//����״̬
};

//�û��˳�
struct CMD_S_PlayerExit
{
	WORD								wPlayerID;							//�˳��û�
};

//�ȴ�����
struct CMD_S_WaitCompare
{
	WORD								wCompareUser;						//�����û�
};

//////////////////////////////////////////////////////////////////////////

//�ͻ�������ṹ
#define SUB_C_ADD_SCORE					1									//�û���ע
#define SUB_C_GIVE_UP					2									//������Ϣ
#define SUB_C_COMPARE_CARD				3									//������Ϣ
#define SUB_C_LOOK_CARD					4									//������Ϣ
#define SUB_C_OPEN_CARD					5									//������Ϣ
#define SUB_C_WAIT_COMPARE				6									//�ȴ�����
#define SUB_C_FINISH_FLASH				7									//��ɶ���

//�û���ע
struct CMD_C_AddScore
{
	LONGLONG							lScore;								//��ע��Ŀ
	WORD								wState;								//��ǰ״̬
};

//�������ݰ�
struct CMD_C_CompareCard
{	
	WORD								wCompareUser;						//�����û�
};

//////////////////////////////////////////////////////////////////////////

#endif
