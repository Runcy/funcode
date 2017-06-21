#include "PushTrusteePro.h"
#include "HallHandler.h"
#include "Logger.h"
#include "GameCmd.h"
#include "GameServerConnect.h"
#include "PlayerManager.h"
#include "ProcessManager.h"
#include <string>
using namespace std;

REGISTER_PROCESS(CLIENT_MSG_USERAI, PushTrusteePro)

PushTrusteePro::PushTrusteePro()
{
	this->name = "PushTrusteePro";
}

PushTrusteePro::~PushTrusteePro()
{

} 

int PushTrusteePro::doRequest(CDLSocketHandler* client, InputPacket* pPacket, Context* pt )
{
	return 0;
}

int PushTrusteePro::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	HallHandler* hallHandler = reinterpret_cast <HallHandler*> (clientHandler);
	Player* player = PlayerManager::getInstance()->getPlayer(hallHandler->uid);
	int retcode = inputPacket->ReadShort();
	string retmsg = inputPacket->ReadString();
	/*printf("Recv SendCardProc Packet From Server\n");
	printf("Data Recv: retcode=[%d]\n",retcode);
	printf("Data Recv: retmsg=[%s]\n",retmsg.c_str());*/
	if (retcode < 0 || player == NULL)
	{
		return EXIT;
	}
	return 0;
}

