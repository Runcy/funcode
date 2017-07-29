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
		//任务总数
		short countTask;
		//当前派发任务的下标
		short index;
		//简单任务的个数
		short countEasy;
};



#endif

