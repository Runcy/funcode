#include "KickOutPlayerProc.h"
#include "HallHandler.h"
#include "Logger.h"
#include "Configure.h"
#include "GameServerConnect.h"
#include "PlayerManager.h"
#include "GameCmd.h"
#include "ProcessManager.h"
#include <string>
using namespace std;

REGISTER_PROCESS(SERVER_MSG_KICKOUT, KickOutPlayerProc)

KickOutPlayerProc::KickOutPlayerProc()
{
	this->name = "KickOutPlayerProc";
}

KickOutPlayerProc::~KickOutPlayerProc()
{

} 

int KickOutPlayerProc::doRequest(CDLSocketHandler* client, InputPacket* pPacket, Context* pt )
{
	return 0;
}

int KickOutPlayerProc::doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  
{
	HallHandler* hallHandler = reinterpret_cast <HallHandler*> (clientHandler);
	Player* player = PlayerManager::getInstance()->getPlayer(hallHandler->uid);
	int retcode = inputPacket->ReadShort();
	string retmsg = inputPacket->ReadString();

	if(retcode < 0 || player == NULL)
		return EXIT;
	
	int uid = inputPacket->ReadInt();
	short ustatus = inputPacket->ReadShort();
	int tid = inputPacket->ReadInt();
	if(tid != player->m_TableId)
		return EXIT;
	
	short tstatus = inputPacket->ReadShort();
	int leaver = inputPacket->ReadInt();
	if(uid == leaver)
		return EXIT;
	
	//特殊处理
	if(leaver > 1000)
		return EXIT;
	
	return 0;
}


