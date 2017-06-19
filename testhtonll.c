#include  <stdio.h> 
#include  <stdlib.h> 
#include  <string.h> 
#include  <arpa/inet.h> 
#include  <inttypes.h> 
uint64_t htonll(uint64_t val) { 
     return  (((uint64_t) htonl(val))  <<   32 )  +  htonl(val  >>   32 ); 
} 

uint64_t ntohll(uint64_t val) { 
     return  (((uint64_t) ntohl(val))  <<   32 )  +  ntohl(val  >>   32 ); 
}

uint64_t htonll16(uint64_t val) { 
     return  (((uint64_t) htonl(val))  <<   16 )  +  htonl(val  >>   16 ); 
} 

uint64_t ntohll16(uint64_t val) { 
     return  (((uint64_t) ntohl(val))  <<   16 )  +  ntohl(val  >>   16 ); 
}  

int  main() { 
    uint64_t hll  =   0x1122334455667788 ; 
	printf( " PRIu64: %s, PRIX64: %s\n", PRIu64, PRIX64); 
    printf( " uint64: % " PRIu64 " \n " , hll); 
    printf( " 0x% " PRIX64 " \n " , hll); 
    printf( " htonll(hll) = 0x% " PRIX64 " \n " , htonll(hll)); 
    printf( " ntohll(htonll(hll)) = 0x% " PRIX64 " \n " , ntohll(htonll(hll))); 
    printf( " ntohll(hll) = 0x% " PRIX64 " \n " , ntohll(hll));  //  no change     

	printf( " htonll16(hll) = 0x% " PRIX64 " \n " , htonll16(hll));  //  no change
	printf( " ntohll16(hll) = 0x% " PRIX64 " \n " , ntohll16(hll));  	
	return   1 ; 
} 