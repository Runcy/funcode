#ifndef _OpenCardProc_H_
#define _OpenCardProc_H_

#include "BaseProcess.h"

class OpenCardProc :public BaseProcess
{
	public:
		OpenCardProc();
		virtual ~OpenCardProc();
		virtual int doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  ;
		virtual int doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )   ; 
	private:
};

#endif

