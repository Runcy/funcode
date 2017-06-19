#include <map>
#include <string>
#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> 
#include <unistd.h>
#include <vector>
#include <functional>

typedef struct dobleInt
{
	int ad;
	int bd;
} DINT;
 
class map_finder
{
public:
       map_finder(const std::string &cmp_string):m_s_cmp_string(cmp_string){std::cout<<"enter map_finder" << std::endl;}
       bool operator ()(const std::map<int, std::string>::value_type &pair)
       {
		   std::cout<<"enter operator (): " << pair.second <<std::endl;
            return pair.second == m_s_cmp_string;
       }
private:
        const std::string &m_s_cmp_string;                    
};
 
 


int main()
{
    std::map<int, std::string> my_map;
    my_map.insert(std::make_pair(10, "china"));
    my_map.insert(std::make_pair(20, "usa"));
    my_map.insert(std::make_pair(30, "english"));
    my_map.insert(std::make_pair(40, "hongkong"));    
    
    std::map<int, std::string>::iterator it = find_if(my_map.begin(), my_map.end(), map_finder("english"));
    if (it == my_map.end())
       printf("not found\n");       
    else
       printf("found key:%d value:%s\n", it->first, it->second.c_str());
   
   
	DINT dint;
	std::vector<DINT> vdint;
	for(int i = 0; i < 50; i++)
	{
		dint.ad = i;
		dint.bd = i+100;
		vdint.push_back(dint);	
	}
	
	typedef std::vector<DINT>::iterator DITER;
	DITER iter = find_if(vdint.begin(), vdint.end(), [=](DINT& dt){return dt.ad == 34;});
	if(iter != vdint.end())
	{
		std::cout<<"it->ad: " <<iter->ad <<std::endl;
	}
	
	auto mod = [](int x, int y){return x%y;};
	std::function<int(int, int)> mod2 = [](int x, int y){return x%y;};
	
	int ret = mod(10,100);
	std::cout<<"ret: " << ret << std::endl;
	
	ret = mod2(11,100);
	std::cout<<"ret: " << ret << std::endl;
       
    return 0;        
}