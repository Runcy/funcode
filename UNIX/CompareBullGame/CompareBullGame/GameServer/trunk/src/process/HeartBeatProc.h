#ifndef _HeartBeatProc_H_
#define _HeartBeatProc_H_

#include "BaseProcess.h"

class HeartBeatProc :public BaseProcess
{
	public:
		HeartBeatProc();
		virtual ~HeartBeatProc();
		virtual int doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  ;
		virtual int doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )   ; 
};

#endif

