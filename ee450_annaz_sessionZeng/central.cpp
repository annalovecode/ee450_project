#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <float.h>
#include <algorithm>  
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
#define LOCALHOST "127.0.0.1"
#define clientA_TCP_PORT 25642
#define clientB_TCP_PORT 26642
#define T_UDP_PORT 21642
#define S_UDP_PORT 22642
#define P_UDP_PORT 23642
#define SERVER_UDP_PORT 24642
#define BACKLOG 100 // pending connections queue
#define ERROR_FLAG -1
#define BUFLENGTH 1024
int sockfdAClient;//socket descriptor for clientA 
int sockfdBClient;//socket descriptor for clientB
int sockfdServer;//socket descriptor for central TCP connection
int sockfdUDP;
int  listenfd1;
int listenfd;
string MSG;
string MSGS;
string name;
string MSGR;
struct sockaddr_in address;
struct sockaddr_in clientA_tcp_addr;
struct sockaddr_in clientB_tcp_addr;
struct sockaddr_in server_udp_addr;
struct sockaddr_in serverS_udp_addr;
struct sockaddr_in serverT_udp_addr;
struct sockaddr_in serverP_udp_addr;
char clientAInput[BUFLENGTH]; // input data from client
char clientBInput[BUFLENGTH]; // input data from client
char sResult[BUFLENGTH]; // result returned from server S
char tResult[BUFLENGTH]; // result returned from server T
char pResult[BUFLENGTH]; // result returned from server C
char sInput[BUFLENGTH];//input from serverS
char tInput[BUFLENGTH];//input from serverT
char pInput[BUFLENGTH];//input from serverP
void handler(int sig);
void create_bind_tcp_socket();
void createUDPSocket();
void connectS();
void connectT();
void connectP_1();
void connectP_2();
void connectP_3();
void recv_from_S();
void recv_from_T();
void recv_from_P();
void receive_from_clientA();
void receive_from_clientB();
void send_to_clientA();
void send_to_clientB();
//send location to serverS
void send_to_serverS();
void send_to_serverT();
int main(){
    struct sigaction act;
    act.sa_handler=handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGCHLD, &act, 0);
    create_bind_tcp_socket();
    createUDPSocket();
    printf("The Central is up and running.\n");
    if (sigaction(SIGCHLD, &act, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
    while(true){
        socklen_t clientAAddrSize = sizeof(clientA_tcp_addr);
        socklen_t clientBAddrSize = sizeof(clientB_tcp_addr);
        sockfdAClient = ::accept(listenfd1, (struct sockaddr *) &clientA_tcp_addr, &clientAAddrSize);
        if (sockfdAClient == ERROR_FLAG) {
            perror("[ERROR] mainserver: fail to accept connection with client");
            exit(1);
        } 
        sockfdBClient = ::accept(listenfd, (struct sockaddr *) &clientB_tcp_addr, &clientBAddrSize);
        if (sockfdBClient == ERROR_FLAG) {
            perror("[ERROR] mainserver: fail to accept connection with client");
            exit(1);
        } 
        receive_from_clientA();
        receive_from_clientB();
        connectT();
        recv_from_T();
        cout << "The Central server received information from Backend-Server T using UDP over port "<< SERVER_UDP_PORT << "."<<endl;
        connectS();
        recv_from_S();
        cout << "The Central server received information from Backend-Server S using UDP over port "<< SERVER_UDP_PORT << "."<<endl;
        connectP_1();
        connectP_2();
        connectP_3();
        cout << "The Central server sent a processing request to Backend-Server P. " <<endl;
        recv_from_P();
        cout << "The Central server received information from Backend-Server P using UDP over port "<< SERVER_UDP_PORT << "."<<endl;
        send_to_clientA();
        send_to_clientB();
        MSGR="";
    }
    close(sockfdUDP); 
    close(listenfd);
    close(listenfd1);
    close(sockfdServer);
    return 0;
}

void handler(int sig){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//create TCP socket for clientA and scheduler connection
void create_bind_tcp_socket(){   
    int port = clientA_TCP_PORT;
    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    address.sin_addr.s_addr=inet_addr(LOCALHOST);
   // inet_pton( AF_INET, , &address.sin_addr );
    address.sin_port = htons( port );
    listenfd1 = socket( PF_INET, SOCK_STREAM, 0 );
    if(listenfd1<0){
        perror("socket failed");
		exit(-1);
    }
    ret = bind( listenfd1, ( struct sockaddr* )&address, sizeof( address ) );
    if(ret<0){
        perror("bind failed");
        exit(-1);
    }
    int res = listen( listenfd1, 3);
    if(res<0){
        perror("listen failed");
        exit(-1);
    }
    //create socket clientB
   // struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    address.sin_addr.s_addr=inet_addr(LOCALHOST);
   // inet_pton( AF_INET, ip, &address.sin_addr );
    port = clientB_TCP_PORT;
    address.sin_port = htons( port );
    listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    if(listenfd<0){
        perror("socket failed");
		exit(-1);
    }
     ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    if(ret<0){
        perror("bind failed");
        exit(-1);
    }
    res = listen( listenfd, 3);
    if(res<0){
        perror("listen failed");
        exit(-1);
    }
}

void createUDPSocket(){
   
    sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdUDP == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to create UDP socket");
        exit(1);
    }

    // from beej's tutorial
    memset(&server_udp_addr, 0, sizeof(server_udp_addr));
    server_udp_addr.sin_family = AF_INET;
    server_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    server_udp_addr.sin_port = htons(SERVER_UDP_PORT);
    // bind socket
    if (::bind(sockfdUDP, (struct sockaddr *) &server_udp_addr, sizeof(server_udp_addr)) == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to bind UDP socket");
        exit(1);
    }
}

void connectT(){
    string message="BOOT UP";
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff,message.c_str(), strlen(message.c_str()));
    // from beej's tutorial
    memset(&serverT_udp_addr, 0, sizeof(serverT_udp_addr));
    serverT_udp_addr.sin_family = AF_INET;
    serverT_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverT_udp_addr.sin_port = htons(T_UDP_PORT);
    socklen_t serverT_addrlength = sizeof(serverT_udp_addr);
   // setnonblocking(sockfdUDP);
     if (::sendto(sockfdUDP, dataBuff,sizeof(dataBuff), 0, (struct sockaddr *) &serverT_udp_addr,
                 serverT_addrlength) == ERROR_FLAG) {
        perror("[Error] Central: fail to send input message to server A");
        exit(1);
    }
    cout << "The Central server sent a request to Backend-Server T."<< endl;
}

void connectS(){
    string message="BOOT UP";
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff,message.c_str(), strlen(message.c_str()));
    // from beej's tutorial
    memset(&serverS_udp_addr, 0, sizeof(serverS_udp_addr));
    serverS_udp_addr.sin_family = AF_INET;
    serverS_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverS_udp_addr.sin_port = htons(S_UDP_PORT);
    socklen_t serverS_addrlength = sizeof(serverS_udp_addr);
     if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (struct sockaddr *) &serverS_udp_addr,
                 serverS_addrlength) == ERROR_FLAG) {
        perror("[Error] Central: fail to send input message to server S");
        exit(1);
    }
    cout << "The Central server sent a request to Backend-Server S."<< endl;
}

void connectP_1(){
    memset(&serverP_udp_addr, 0, sizeof(serverP_udp_addr));
    serverP_udp_addr.sin_family = AF_INET;
    serverP_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverP_udp_addr.sin_port = htons(P_UDP_PORT);
    socklen_t serverP_addrlength =sizeof(serverP_udp_addr);
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    int i;
    char *x = dataBuff;
    string::iterator sit = MSG.begin();
    int sizeResult = MSG.size();

    while (sit != MSG.end())
 {
		if(sizeResult < 1023)
		{
			break;
		}
		for(i=0;i<1023;i++)
		{
			*x =*sit;
			x++;
			sit++;
		}
		sizeResult -= 1023;
		*x = '\0';
   
		if (::sendto(sockfdUDP,dataBuff, sizeof(dataBuff),0, (const struct sockaddr *)&serverP_udp_addr,
                     serverP_addrlength) ==ERROR_FLAG)
        {
            perror("[Error] central: fail to send input Topology to serverP");
            exit(1);
        }
        memset(dataBuff, '\0', sizeof(dataBuff));
		x = dataBuff;
     
	}
	while(sit != MSG.end())
	{
		*x =*sit;
		x++;
		sit++;
      
	}

    dataBuff[strlen(dataBuff)]='\0';
    if (::sendto(sockfdUDP , dataBuff, strlen(dataBuff), 0, (const struct sockaddr *)&serverP_udp_addr,
                     sizeof(serverP_udp_addr)) < 0)
        {
       
           perror("[Error] central: fail to send input Topology1 to serverP");
           exit(1);
        }

    memset(dataBuff, '\0', strlen(dataBuff));
    MSG="";

}   

void connectP_2(){
    memset(&serverP_udp_addr, 0, sizeof(serverP_udp_addr));
    serverP_udp_addr.sin_family = AF_INET;
    serverP_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverP_udp_addr.sin_port = htons(P_UDP_PORT);
    socklen_t serverP_addrlength =sizeof(serverP_udp_addr);
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    int i;
	char *x = dataBuff;
	string::iterator sit = MSGS.begin();
	int sizeResult = MSGS.size();
	while (sit != MSGS.end())
	{
		if(sizeResult < 1023)
        {
            break;
        }
        for(i=0;i<1023;i++)
		{
			*x =*sit;
			x++;
			sit++;
		}
		sizeResult -= 1023;
		*x = '\0';
		if (::sendto(sockfdUDP,dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *)&serverP_udp_addr,
                    sizeof(serverP_udp_addr)) ==ERROR_FLAG)
        {
            perror("[Error] central: fail to send input score to serverP");
            exit(1);
        }
        memset(dataBuff, '\0', sizeof(dataBuff));
		x = dataBuff;
	}
	while(sit != MSGS.end())
	{
		*x =*sit;
		x++;
		sit++;
	}
     dataBuff[strlen(dataBuff)]='\0';
    if (::sendto(sockfdUDP,dataBuff, strlen(dataBuff), 0, (const struct sockaddr *)&serverP_udp_addr,
                    sizeof(serverP_udp_addr)) < 0)
        {
           perror("[Error] central: fail to send input score to serverP");
           exit(1);
        }
     memset(dataBuff, '\0', strlen(dataBuff));
     MSGS="";
   

}
void connectP_3(){
    char dataBuff[BUFLENGTH];
    memset(dataBuff, '\0', sizeof(dataBuff));
    strcpy(dataBuff,name.c_str());
    if (::sendto(sockfdUDP,dataBuff, strlen(dataBuff), 0, (const struct sockaddr *)&serverP_udp_addr,
                     sizeof(serverP_udp_addr)) < 0)
        {
           perror("[Error] central: fail to send input client to serverP");
           exit(1);
        }
     memset(dataBuff, '\0', sizeof(dataBuff));
     name="";
   }

void recv_from_T(){
    memset(tInput,'\0',sizeof(tInput));
    socklen_t serverTUDPLen = sizeof(serverT_udp_addr);
    while(true){
    if (::recvfrom(sockfdUDP, tInput, BUFLENGTH-1, 0, (struct sockaddr *) &serverT_udp_addr, &serverTUDPLen) == ERROR_FLAG) {
        perror("[Error] central: fail to receive input topology from severT");
        exit(1);
    }
    tInput[strlen(tInput)]='\0';
    string s=tInput;
     MSG+=s;
     if(strlen(tInput)<1023){
     	break;
     }
      memset(tInput,'\0',sizeof(tInput));
    }

}

void recv_from_S(){
    memset(sInput,'\0',sizeof(sInput));
    socklen_t serverSUDPLen = sizeof(serverS_udp_addr);
    while(true){
    if (::recvfrom(sockfdUDP, sInput, BUFLENGTH-1, 0, (struct sockaddr *) &serverS_udp_addr, &serverSUDPLen) == ERROR_FLAG) {
        perror("[Error] central: fail to receive input scores from severT");
        exit(1);
    }
    sInput[strlen(sInput)]='\0';
    string s=sInput;
    MSGS+=s;
    if(strlen(sInput)<1023){
     	break;
     }
      memset(sInput,'\0',sizeof(sInput));
    }
   // cout << "MSGS:" <<MSGS <<endl;  
}

void recv_from_P(){
    memset(pInput,'\0',sizeof(pInput));
    socklen_t serverPUDPLen = sizeof(serverP_udp_addr);
    while(true){
    if (::recvfrom(sockfdUDP, pInput, BUFLENGTH-1, 0, (struct sockaddr *) &serverP_udp_addr, &serverPUDPLen) == ERROR_FLAG) {
        perror("[Error] central: fail to receive input scores from severT");
        exit(1);
    }
    pInput[strlen(pInput)]='\0';
    string s=pInput;
   // cout << "s: " <<s <<endl;
    MSGR +=s;
    if(strlen(pInput)<1023){
        break;
     }
      memset(pInput,'\0',sizeof(pInput));

    }
    memset(pInput,'\0',sizeof(pInput));
  //  MSGR="";
  //  cout <<"MSGR: "<<MSGR<<endl;
   
}

void receive_from_clientA(){
    // from beej's tutorial
    memset(&clientAInput, 0, sizeof(clientAInput));
    // receive through child socket
    if (recv(sockfdAClient, clientAInput, BUFLENGTH-1, 0) == ERROR_FLAG) {
        perror("[ERROR] central: fail to receive input data from clientA");
        exit(1);
    }
    string name1 = clientAInput;
    name = name1+ " ";
    socklen_t len = sizeof(address);
    getsockname(sockfdAClient,(struct sockaddr *)&address,&len);
    int myport=ntohs(address.sin_port);
    cout << "The Central server received input=\""<< name1 <<"\" from the client using TCP over port " << myport << endl;
}

void receive_from_clientB(){
    // from beej's tutorial
    memset(&clientBInput, 0, sizeof(clientBInput));
    // receive through child socket
    if (recv(sockfdBClient, clientBInput, BUFLENGTH-1, 0) == ERROR_FLAG) {
        perror("[ERROR] central: fail to receive input data from clientB");
        exit(1);
    }
    string name2= clientBInput;
    name += name2;
    socklen_t len = sizeof(address);
    getsockname(sockfdBClient,(struct sockaddr *)&address,&len);
    int myport=ntohs(address.sin_port);
    cout << "The Central server received input=\""<< name2 <<"\" from the client using TCP over port " << myport << endl;
}

void send_to_clientA(){
  char clientPosition[1024];
  int i;
  char *x = clientPosition;
  string::iterator sit = MSGR.begin();
  int sizeResult = MSGR.size();
  while (sit != MSGR.end())
  {
    if(sizeResult < 1023)
    {
      break;
    }
        for(i=0;i<1023;i++)
    {
      *x =*sit;
      x++;
      sit++;
    }
    sizeResult -= 1023;
    
    *x = '\0';
    if (send(sockfdAClient,clientPosition, sizeof(clientPosition), 0) < 0)
        {
            perror("central send_to_clientA failed");
            exit(1);
        }
         memset(clientPosition, '\0', sizeof(clientPosition));
    x = clientPosition;
     }
  while(sit != MSGR.end())
  {
    *x =*sit;
    x++;
    sit++;
  }
    clientPosition[strlen(clientPosition)]='\0';
    if (send(sockfdAClient,clientPosition, strlen(clientPosition), 0) < 0)
        {
            perror("central send_to_clientA failed");
            exit(1);
        }
    string m=clientPosition;
   // cout << "clientPositionA: "<< m  <<endl;
    memset(clientPosition, '\0', sizeof(clientPosition));
    cout << "The Central server sent the results to client A." << endl;
}
void send_to_clientB(){
   char clientPosition[1024];
  int i;
  char *x = clientPosition;
  string::iterator sit = MSGR.begin();
  int sizeResult = MSGR.size();
  while (sit != MSGR.end())
  {
    if(sizeResult < 1023)
    {
      break;
    }
        for(i=0;i<1023;i++)
    {
      *x =*sit;
      x++;
      sit++;
    }
    sizeResult -= 1023;
    
    *x = '\0';
    if (send(sockfdBClient,clientPosition, sizeof(clientPosition), 0) < 0)
        {
            perror("central send_to_clientB failed");
            exit(1);
        }
         memset(clientPosition, '\0', sizeof(clientPosition));
    x = clientPosition;
     }
  while(sit != MSGR.end())
  {
    *x =*sit;
    x++;
    sit++;
  }
    clientPosition[strlen(clientPosition)]='\0';
    if (send(sockfdBClient,clientPosition, strlen(clientPosition), 0) < 0)
        {
            perror("central send_to_clientB failed");
            exit(1);
        }
         string m=clientPosition;
  //  cout << "clientPositionB: "<< m  <<endl;
        memset(clientPosition, '\0', sizeof(clientPosition));
      
    cout << "The Central server sent the results to client B." << endl;
}
