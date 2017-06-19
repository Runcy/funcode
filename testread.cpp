#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define MAX_LISTEN_PORT 20

//读取配置的服务器端口
bool readPortConfig(std::string confile)
{
	FILE* fp = fopen(confile.c_str(), "r+");
	if (NULL == fp)
	{
		std::cout << "open port config file failed" << std::endl;
		return false;
	}
	int _nPortArray[MAX_LISTEN_PORT];
	int nPort = 0;
	char line[80];
	char* ptr = line;
	memset(line, 0, sizeof(line));
	while (fgets(line, 80, fp) != NULL)
	{
		for (int i = 0; i < 80; i++){
			if (ptr[i] == '='){
				if (nPort < MAX_LISTEN_PORT){
					_nPortArray[nPort] = atoi(&ptr[i + 1]);
					
					printf("portindex: %d, port: %d\n", nPort, _nPortArray[nPort]);
					nPort++;
				}
			}			
		}
		memset(line, 0, sizeof(line));
	}
	return true;
}


int main()
{
	std::string filename("websocket.xml");
	
	readPortConfig("websocket.xml");
	
	return 0;
}