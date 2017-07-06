#ifndef TaskManager_H
#define TaskManager_H
#include "GameLogic.h"
#include "Typedef.h"

#define MAX_TASK 128
#define EASY_MAX 32

typedef struct _Task
{
	int64_t taskid;
	char taskname[64];
	short ningotlow;
	short ningothigh;
}Task;

class Player;
class Table;
class TaskManager
{
	public:
		virtual ~TaskManager(){};
		
		static TaskManager * getInstance();
		int init();
		int loadTask();
		Task * getTask();
		Task * getRandTask();
		Task * getRandEsayTask();
		bool calcPlayerComplete(Player* player, Table* table);
		bool comCallLand(Player* player, Table* table, short callland);
		bool comSetMul(Player* player, Table* table, short setmul);
		bool comCardType(Player* player, Table* table, short cardtype);
		bool comBottomCard(Player* player, Table* table, short bottomtype);
	private:
		TaskManager();
		Task taskarry[MAX_TASK];
		Task* taskEsay[EASY_MAX];
		//��������
		short countTask;
		//��ǰ�ɷ�������±�
		short index;
		//������ĸ���
		short countEasy;
};



#endif

