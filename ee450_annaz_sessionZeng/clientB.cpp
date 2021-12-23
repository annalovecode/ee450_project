#include<unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<vector>
#include<iostream>
#include<sstream>
#include<cmath>
#include <iomanip> 
using namespace std;
string MSG;
#define LOCALHOST "127.0.0.1"
#define PORTNUMBER 26642
vector<string> split(string temp)//按空格分隔字符串
{
  vector<string> str;
  string word;
  stringstream input;//利用字符串流输入
  input << temp;
  while (input >> word)//循环把缓冲区数据读出
  {
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
	//定义相关文件描述符
	int sockfd,recvfd;
	struct sockaddr_in serveraddr,myaddr;
    char message[1024];
    char serverOutput[1024];
	//创建套接子文件
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("sockfd failed");
		exit(-1);
	}
 
	//填充结构体
    bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr=inet_addr(LOCALHOST);
    //更改成动态的结构体地址
    serveraddr.sin_port = htons(PORTNUMBER);
    socklen_t serverlen=sizeof(serveraddr);
//    printf( "%s",argv[1]);
	//向服务器发送连接请求
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

        printf("The client sent %s to the Central server.\n",message);
    memset(serverOutput,'\0',sizeof(serverOutput));
    while(true){
    if (::recv(sockfd,serverOutput,sizeof(serverOutput),0) <0) {
        perror("[Error] client failed to receive output from Central.");
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
   // cout << "MSG: "<< MSG << endl;
    vector<string> vec=split(MSG);
    if(vec[1]=="no"){
         cout <<"Found no compatibility between "<< message <<" and "<< vec[4] << endl;
    }
    else{
     cout<<"Found compatiblity for "<< vec[vec.size()-2]  <<" and " << vec[0]<<":"<<endl; 
     for (size_t i=vec.size()-2;i>0;i--)//输出
  {
        cout << vec[i]<<"--";

  }
    cout << vec[0] <<endl;
    double s=::stod(vec[vec.size()-1]);
    cout << fixed << setprecision(2);
    cout << "Matching gap : "<< s <<endl;
    }
    close(sockfd);
   // MSG="";
    return 0;
}
