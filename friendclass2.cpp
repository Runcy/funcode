#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define __STDC_FORMAT_MACROS  
#include <inttypes.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>

using namespace std;

typedef unsigned char BYTE;

#define NTOHLL(UL) ((uint64_t)ntohl((UL)& 0xFFFFFFFF) << 16) + (ntohl((UL) >> 16))
#define HTONLL(UL) ((uint64_t)htonl((UL)& 0xFFFFFFFF) << 16) + (htonl((UL) >> 16))


#define MHTONLL(x) (((uint64_t)(htonl(x)) << 32) + htonl((x)>>32))
#define MNTOHLL(x) (((uint64_t)(ntohl(x)) << 32) + ntohl((x)>>32))

/*
class B;
class A;
void Show( A& a, B& b )
{
	cout << a.value << endl;
	cout << b .tt << endl;
}

class B
{
public:
	B( int temp = 100):tt ( temp ){}
private:
	int tt;
	friend class A;
	friend void Show( A& , B& );
};


class A
{
private:
	int value;
	friend void Show( A& , B& );

public:
	A(int temp = 200 ):value ( temp ){}

	void Show( B &b )
	{
	  cout << value << endl;
	  cout << b.tt << endl; 
	}
};

*/

class CAddr
{
private:
	CAddr(){}

public:
	static CAddr* instance()
	{
		//局部静态变量效果等同于私有成员变量，instance是唯一获取实例的接口
		static CAddr* pcaddr = NULL;
		if(NULL == pcaddr)
		{
			pcaddr = new CAddr();
			return pcaddr;
		}
	}
	
	int add(int a, int b)
	{
		return ((a)+(b));
	}
	
};

char Encode_GetChar(BYTE num)
{
    return 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "+/="[num];
}

int main()
{
	//char c=15;
	unsigned short index = 1;
	unsigned short routeid = 2;
	
	///*
	unsigned long long int x = 0x1122334455667788;
	
	printf("PRIX64: %s\n", PRIX64);
	printf("raw: 0x%" PRIX64"\n", x);
	printf("NTOHLL(UL): 0x%"PRIX64 " \n ", NTOHLL(x));
	printf("HTONLL(UL): 0x%"PRIX64 " \n ", HTONLL(x));
	printf("MHTONLL(x): 0x%"PRIX64 " \n ", MHTONLL(x));
	printf("MNTOHLL(x): 0x%"PRIX64 " \n ", MNTOHLL(x));
    //*/
	//printf("%c\n", Encode_GetChar(15));
	//printf("%c\n", Encode_GetChar(c));
	
	printf("%u\n", routeid);
	printf("%d\n", (int32_t)(index)<<16);
	printf("%d\n", (int32_t)(index<<16));
	printf("%d\n", (int32_t)(index<<16) + routeid);
	
	printf("%d\n", atoi(" 0213"));
	printf("\n");
	
	char pdot[INET_ADDRSTRLEN] = {0};
	struct ifaddrs *ifd = NULL, *pifd = NULL;
	if(getifaddrs(&ifd) == 0)
	{
		for(pifd = ifd; pifd != NULL; pifd = pifd->ifa_next)
		{
			if(pifd->ifa_addr->sa_family == AF_INET)
				if(0 == strncmp(pifd->ifa_name, "eth0", sizeof("eth0")))
				{
					printf(" %s: ", pifd->ifa_name);
					printf("%s\n", inet_ntop(AF_INET, &((struct sockaddr_in*)(pifd->ifa_addr))->sin_addr, pdot, INET_ADDRSTRLEN));
				}
					
			//memset(pdot, 0, sizeof(pdot));
			//printf("if mask: %s\n", inet_ntop(AF_INET, pifd->ifa_netmask->sin_addr.s_addr, pdot, (socklen_t)sizeof(pdot)));
			//memset(pdot, 0, sizeof(pdot));
			//if(IFF_BROADCAST == pifd->ifa_flags)
			//	printf("if mask: %s\n", inet_ntop(AF_INET, pifd->ifa_broadaddr->sin_addr.s_addr, pdot, (socklen_t)sizeof(pdot)));
			//else
			//	printf("if mask: %s\n", inet_ntop(AF_INET, pifd->ifa_dstaddr->sin_addr.s_addr, pdot, (socklen_t)sizeof(pdot)));
			printf("\n");
			
		}
		
	}
	else
	{
		printf("getifaddrs error: %s\n", strerror(errno));
	}
	
	freeifaddrs(ifd);
	
	
	int sum = CAddr::instance()->add(10, 100);
	printf("%d\n", sum);
	
	
	string abc="0123456789";
	string bc(abc,4);
	cout<<bc<<endl;
	int num=52;
	for(int i = 0; i < 52; i++)
	{
		printf("%d,", rand()%(num--));
	}
	printf("\n");
	
	int num1 = 100;
	int mun1 = 109;
	char buf[10] = {0};
	sprintf(buf, "%d#%d", num1, mun1);
	std::string str(buf);
	cout<<str << endl;
	
	string fl("123.78");
	int nt = atoi(fl.c_str());
	printf("nt: %d\n", nt);
	
	return 0;
}