#include <iterator>
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
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#define FILE_NAME "scores.txt"
using namespace std;
int sockfdUDP;                    // UDP socket
struct sockaddr_in serverAddrUDP; //serverS use to create UDP
struct sockaddr_in clientAddrUDP; //central address for UDP connection
vector<string> File_save;         // save each line to build graph
char serverSInput[1024];
string result;
#define BUFLENGTH 1024
#define UDP_PORT 22642
#define client_PORT 24642
#define LOCALHOST "127.0.0.1"
int createUDPsocket()
{
    sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
    //test if create a socket successfully
    if (sockfdUDP < 0)
    {
        perror("socket failed");
        exit(1);
    }
    // from beej's tutorial
    memset(&serverAddrUDP, 0, sizeof(serverAddrUDP));
    serverAddrUDP.sin_family = AF_INET;
    serverAddrUDP.sin_port = htons(UDP_PORT);
    serverAddrUDP.sin_addr.s_addr = inet_addr(LOCALHOST);
    // bind socket
    if (::bind(sockfdUDP, (struct sockaddr *)&serverAddrUDP, sizeof(serverAddrUDP)) < 0)
    {
        perror("ServerS UDP bind failed.");
        exit(1);
    }
    cout<<"The ServerS is up and running using UDP on port " <<UDP_PORT<<"."<<endl;
}

void connectToCentral()
{
    // from beej's tutorial
    memset(&clientAddrUDP, 0, sizeof(clientAddrUDP));
    clientAddrUDP.sin_family = AF_INET;
    clientAddrUDP.sin_addr.s_addr = inet_addr(LOCALHOST);
    clientAddrUDP.sin_port = htons(client_PORT);
}

void receiveInfor()
{
    memset(serverSInput, '\0', sizeof(serverSInput));
    socklen_t serverS_UDP_len = sizeof(clientAddrUDP);
    if (::recvfrom(sockfdUDP, serverSInput, 1024, 0, (struct sockaddr *)&clientAddrUDP,
                   &serverS_UDP_len) < 0)
    {
        perror("serverS receive failed");
        exit(1);
    }
    printf("The ServerS received a request from Central to get the scores.\n");
    memset(serverSInput, '\0', sizeof(serverSInput));
}

void readFile()
{
    ifstream data_file(FILE_NAME, ios::in);
    string line_s;
    while (getline(data_file, line_s))
    {
        File_save.push_back(line_s);
    }
    std::ostringstream vts;
    if (!File_save.empty())
    {
        // Convert all but the last element to avoid a trailing " "
        std::copy(File_save.begin(), File_save.end() - 1,
                  std::ostream_iterator<string>(vts, " "));
        // Now add the last element with no delimiter
        vts << File_save.back();
    }
    result = vts.str();
    //cout << result << endl;
}
void sentToCentral()
{
    int i;
    char *x = serverSInput;
    string::iterator sit = result.begin();
    int sizeResult = result.size();
    while (sit != result.end())
    {
        if (sizeResult < 1024)
        {
            break;
        }
        for (i = 0; i < 1023; i++)
        {
            *x = *sit;
            x++;
            sit++;
        }
        sizeResult -= 1023;

        *x = '\0';
        if (::sendto(sockfdUDP, serverSInput, sizeof(serverSInput), 0, (const struct sockaddr *)&clientAddrUDP,
                     sizeof(clientAddrUDP)) < 0)
        {
            perror("serverS response failed");
            exit(1);
        }
        memset(serverSInput, '\0', strlen(serverSInput));
        x = serverSInput;
    }
    while (sit != result.end())
    {
        *x = *sit;
        x++;
        sit++;
    }
    serverSInput[strlen(serverSInput)] = '\0';
    if (::sendto(sockfdUDP, serverSInput, strlen(serverSInput), 0, (const struct sockaddr *)&clientAddrUDP,
                 sizeof(clientAddrUDP)) < 0)
    {
        perror("serverS response failed");
        exit(1);
    }
    memset(serverSInput, '\0', sizeof(serverSInput));
    cout << "The ServerS finished sending the scores to Central." << endl;
}

int main()
{
    createUDPsocket();
    connectToCentral();
    readFile();
    while(true){
    receiveInfor();
    sentToCentral();
    }
    close(sockfdUDP);
}
