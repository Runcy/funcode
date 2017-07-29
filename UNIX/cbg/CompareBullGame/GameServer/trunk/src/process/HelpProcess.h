#ifndef _HelpProcess_H_
#define _HelpProcess_H_

#include "BaseProcess.h"

class HelpProcess :public BaseProcess
{
	public:
		HelpProcess();
		virtual ~HelpProcess();
		virtual int doRequest(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt)  ;
		virtual int doResponse(CDLSocketHandler* clientHandler, InputPacket* inputPacket,Context* pt )   ; 
	private:
};

#endif

