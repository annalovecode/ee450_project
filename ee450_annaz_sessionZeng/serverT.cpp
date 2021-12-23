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
#include <fstream>
#include <sstream>
#include <limits.h>
#include <float.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <iterator>
#define FILE_NAME "edgelist.txt"
#define BUFLENGTH 1024
#define UDP_PORT 21642
#define client_PORT 24642
#define LOCALHOST "127.0.0.1"
using namespace std;
int sockfdUDP;                    // UDP socket
struct sockaddr_in serverAddrUDP; //serverT use to create UDP
struct sockaddr_in clientAddrUDP; //central address for UDP connection
vector<string> pointSet;
vector<vector<string>> File_save; // save each line to build graph
char clientPosition[BUFLENGTH];
string result; //record the result of the total word
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
        perror("ServerT UDP bind");
        exit(1);
    }
    cout <<"The ServerT is up and running using UDP on port " <<UDP_PORT <<"."<<endl;
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
    memset(clientPosition, '\0', sizeof(clientPosition));
    socklen_t serverT_UDP_len = sizeof(clientAddrUDP);
    if (::recvfrom(sockfdUDP, clientPosition, 1024, 0, (struct sockaddr *)&clientAddrUDP,
                   &serverT_UDP_len) < 0)
    {
        perror("serverT receive failed");
        exit(1);
    }
    printf("The ServerT received a request from Central to get the topology.\n");
}

void readFile()
{
    ifstream data_file(FILE_NAME, ios::in);
    string line_s;
    while (getline(data_file, line_s))
    {
        stringstream ss(line_s);
        string s;
        vector<string> lineArray;
        while (getline(ss, s, ' '))
        {
            if (s == "")

            {
                continue;
            }
            lineArray.push_back(s);
        }
        File_save.push_back(lineArray);
        for (int a = 0; a < lineArray.size(); a++)
        {
            pointSet.push_back(lineArray[a]);
        }
    }
    std::ostringstream vts;
    if (!pointSet.empty())
    {
        // Convert all but the last element to avoid a trailing ","
        std::copy(pointSet.begin(), pointSet.end() - 1,
                  std::ostream_iterator<string>(vts, " "));
        // Now add the last element with no delimiter
        std::copy(pointSet.end() - 1, pointSet.end(), std::ostream_iterator<string>(vts, " "));
    }
    result = vts.str();
}

void sendToCentral()
{
    int i;
    char *x = clientPosition;
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
        if (::sendto(sockfdUDP, clientPosition, sizeof(clientPosition), 0, (const struct sockaddr *)&clientAddrUDP,
                     sizeof(clientAddrUDP)) < 0)
        {
            perror("serverT response failed");
            exit(1);
        }
        memset(clientPosition, '\0', sizeof(clientPosition));
        x = clientPosition;
    }
    while (sit != result.end())
    {
        *x = *sit;
        x++;
        sit++;
    }

    clientPosition[strlen(clientPosition)] = '\0';
    if (::sendto(sockfdUDP, clientPosition, strlen(clientPosition), 0, (const struct sockaddr *)&clientAddrUDP,
                 sizeof(clientAddrUDP)) < 0)
    {
        perror("serverT response failed");
        exit(1);
    }
    memset(clientPosition, '\0', strlen(clientPosition));
    cout << "The ServerT finished sending the topology to Central." << endl;
}
int main()
{
    createUDPsocket();
    connectToCentral();
    readFile();
    while(true){
    receiveInfor();
    sendToCentral();
    }
    close(sockfdUDP);
}
