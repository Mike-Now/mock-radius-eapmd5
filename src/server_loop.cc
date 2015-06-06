#include <iostream>
#include "server_loop.h"
#include <vector>
#include "packets/Packet.h"

using std::vector;
    const int BUFLEN = 1000;
    const int PORT = 32000;
	SOCKET s;
	bool isRunning;
void startServer(const char *addr) {


    struct sockaddr_in server, dest_addr;
	int slen;


    WSADATA wsa;
 
    slen = sizeof(dest_addr) ;
    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Initialised.\n");
     
    //Create a socket
    if((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
    printf("Socket created.\n");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );
     
    //Bind
    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
	isRunning=true;
    puts("Bind done");
	}

void stopServer()
	{ 
	isRunning=false;
    closesocket(s);
    WSACleanup();
}


radius::packets::Packet reciveData(SOCKET s)
{
	    printf("Waiting for data...");
        fflush(stdout);
	    int slen , recv_len;
	    vector<char> buf(BUFLEN,'\0');
		struct sockaddr_in dest_addr;
		slen = sizeof(dest_addr) ;
	        //try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, &buf[0], BUFLEN, 0, (struct sockaddr *) &dest_addr, &slen)) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d" , WSAGetLastError());
				exit(EXIT_FAILURE);
			}
		//vector<byte> buffr(&buf[0],&buf[BUFLEN]);
		//wyslij pakiet do servera
		vector<byte> buffr(&buf[0],&buf[BUFLEN]);
        radius::packets::Packet rec_pack(buffr,dest_addr);
		//	rad.recvPacket(rec_pack);
return rec_pack;
}
void sendData(SOCKET s,radius::packets::Packet sen_pack)
{
			int slen , recv_len;
			sockaddr_in dest_addr =sen_pack.addr;
			slen = sizeof(dest_addr);
			vector<char> buf(&(sen_pack.bytes[0]),&(sen_pack.bytes[BUFLEN]));
			recv_len=BUFLEN*sizeof(byte);//??czy to zadziala?
			//now reply the client with the same data
			if (sendto(s, &buf[0], recv_len, 0, (struct sockaddr*) &dest_addr, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d" , WSAGetLastError());
				exit(EXIT_FAILURE);
			}
}
