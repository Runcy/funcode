#ifndef _GetTaskProc_H_
#define _GetTaskProc_H_

#include "BaseProcess.h"

class GetTaskProc :public BaseProcess
{
	public:
		GetTaskProc();
		virtual ~GetTaskProc();
		virtual int doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  ;
		virtual int doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )   ; 
	private:
};

#endif

