#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<vector>
#include<iostream>
#include<sstream>
#include <cmath>
#include <iomanip> 
using namespace std;
string MSG;
#define LOCALHOST "127.0.0.1"
#define PORTNUMBER 25642
vector<string> split(string temp)
{
  vector<string> str;
  string word;
  stringstream input;
  input << temp;
  while (input >> word){
    str.push_back(word);
  }
  return str;
}

int main(int argc, const char *argv[])
{
	if(argc > 2)
	{
		puts("input error");
		exit(-1);
	}

	int sockfd,recvfd;
	struct sockaddr_in serveraddr,myaddr;
    char message[1024];
    char serverOutput[1024];
	//创建套接子文件 create socket
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("sockfd failed");
		exit(-1);
	}
 
	
    bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr=inet_addr(LOCALHOST);
    //更改成动态的结构体地址 change to dynamic address
    serveraddr.sin_port = htons(PORTNUMBER);
    socklen_t serverlen=sizeof(serveraddr);
//    printf( "%s",argv[1]);
	//向服务器发送连接请求 send connect request to server
	if(connect(sockfd,(struct sockaddr *)&serveraddr,serverlen) < 0)
	{
		puts("connect failed");
		exit(-1);
	}
    //retrive the locally-bound name of the specified socket and store
    //it in the sockaddr structure
    bzero(&myaddr,sizeof(myaddr));
	socklen_t mylen = sizeof(myaddr);
    int getsock_check = getsockname(sockfd,(struct sockaddr *)&myaddr,&mylen);
    if(getsock_check==-1){   
     perror("getsockname");
     exit(1);
    }

	printf("The client is up and running.\n");

       memset(message,'\0',sizeof(message));

    strcpy(message,argv[1]);
    if(send(sockfd,message,sizeof(message),0)< 0){   
        perror("The client send failed.");
        exit(0);
    }
    cout <<"The client sent " << message << " to the Central server."<<endl;
    memset(serverOutput,'\0',sizeof(serverOutput));
    while(true){
       memset(serverOutput,'\0',sizeof(serverOutput));
    if (::recv(sockfd,serverOutput,sizeof(serverOutput)-1,0) <0) {
        perror("[Error] client failed to receive output from Central");
        exit(1);
    }
    serverOutput[strlen(serverOutput)]='\0';
    string s=serverOutput;
     MSG+=s;
     if(strlen(serverOutput)<1023){
      break;
     }
      memset(serverOutput,'\0',sizeof(serverOutput));
    }
    //cout << "MSG: "<< MSG << endl;
    vector<string> vec=split(MSG);
    if(vec[1]=="no"){
    	cout << MSG <<endl;
         //cout <<"Found no compatibility between "<< message <<" and "<< vec[6] << endl;
    }
    // pos1 = m.substr(m.find(delimiter));
  else{

    cout<<"Found compatiblity for "<< vec[0] <<" and " << vec[vec.size()-2]<<":"<<endl; 
     for (size_t i=0;i<vec.size()-2;i++)//输出
  {
        cout << vec[i]<<"--";

  }
    cout << vec[vec.size()-2] <<endl;
    double s=::stod(vec[vec.size()-1]);
    cout << fixed << setprecision(2);
    cout << "Matching gap : "<< s <<endl;

  }
   // MSG="";
    close(sockfd);
    return 0;
}
