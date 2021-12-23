#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <vector>
#include <limits.h>
#include <map>
#include <set>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <iterator>
using namespace std;
int sockfdUDP;                    // UDP socket
struct sockaddr_in serverAddrUDP; //serverP use to create UDP
struct sockaddr_in clientAddrUDP; //central address for UDP connection
set<string> pointSet;             //use pointset to store vertex
#define BUFLENGTH 1024
#define UDP_PORT 23642
#define client_PORT 24642
#define LOCALHOST "127.0.0.1"
map<string, int> map_index; //id_to_index
vector<vector<string>> File_save;//save each line to build graph
string MessageT;//string topoloy from serverT
string MessageS;//string scores from serverS
string name1;//name get from clientA
string name2;//name get from clientB
string MessageFeedback;//string message feedback to central
map<string, int> scoreMap;//store scores for each name
map<int, int> score_index;//map for each (name index,score) pair
int n = 0;//use n to store the vertex number
double sum = 0;//the result of caculation
char serverPInput[BUFLENGTH];

int connectAndBindUDPsocket()
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
    perror("ServerP UDP bind");
    exit(1);
  }
  cout<<"The ServerP is up and running using UDP on port "<< UDP_PORT << "." << endl;

  memset(&clientAddrUDP, 0, sizeof(clientAddrUDP));
  clientAddrUDP.sin_family = AF_INET;
  clientAddrUDP.sin_addr.s_addr = inet_addr(LOCALHOST);
  clientAddrUDP.sin_port = htons(client_PORT);
  memset(serverPInput, '\0', sizeof(serverPInput));
  socklen_t serverP_UDP_len = sizeof(clientAddrUDP);
}

/** recv from central to get the total topology**/
void recvTopoplogy()
{
  socklen_t serverP_UDP_len = sizeof(clientAddrUDP);
  memset(serverPInput, '\0', sizeof(serverPInput));
 // string MessageT1;
  while (true)
  {
    if (::recvfrom(sockfdUDP, serverPInput, 1023, 0, (struct sockaddr *)&clientAddrUDP,
                   &serverP_UDP_len) < 0)
    {
      perror("serverP receive topology failed");
      exit(1);
    }
    serverPInput[strlen(serverPInput)] = '\0';
    string s1 = serverPInput;
    MessageT += s1;
    if (strlen(serverPInput) < 1023)
    {
      break;
    }
    memset(serverPInput, '\0', sizeof(serverPInput));
  }
  // MessageT1;
  //cout << "MessageT:" <<MessageT <<endl;
}

/** recv from central to get the score**/
void recvScore()
{
  socklen_t serverP_UDP_len = sizeof(clientAddrUDP);
  memset(serverPInput, '\0', sizeof(serverPInput));
//  string MessageS1;
  while (true)
  {
    if (::recvfrom(sockfdUDP, serverPInput, 1023, 0, (struct sockaddr *)&clientAddrUDP,
                   &serverP_UDP_len) < 0)
    {
      perror("serverP receive score failed");
      exit(1);
    }
    serverPInput[strlen(serverPInput)] = '\0';
    string s1 = serverPInput;
    MessageS += s1;
    if (strlen(serverPInput) < 1023)
    {
      break;
    }
    memset(serverPInput, '\0', sizeof(serverPInput));
  }
 // return MessageS1;
 // cout << "MessageS:" << MessageS <<endl;
}

/** recv from central to get the name**/
void recvName()
{
  socklen_t serverP_UDP_len = sizeof(clientAddrUDP);
  memset(serverPInput, '\0', sizeof(serverPInput));
  if (::recvfrom(sockfdUDP, serverPInput, 1023, 0, (struct sockaddr *)&clientAddrUDP,
                 &serverP_UDP_len) < 0)
  {
    perror("serverP receive name failed");
    exit(1);
  }
  string m = serverPInput;
  string delimiter = " ";
  name1 = m.substr(0, m.find(delimiter));
  name2 = m.substr(m.find(delimiter) + 1);
  //cout << name1 <<endl;
  //cout << name2 <<endl;
  memset(serverPInput, '\0', sizeof(serverPInput));
}

//split the string
vector<string> split(string temp) //split by " "
{
  vector<string> str;
  string word;
  stringstream input; //use stringstream
  input << temp;
  while (input >> word) //read the data
  {
    str.push_back(word);
  }
  return str;
}

/**push line into File_save and put points in point_set**/
void readHelper()
{
  //string message=recvTopoplogy();
  vector<string> vec = split(MessageT);
  for (size_t i = 0; i < vec.size(); i++) //print out
  {
    pointSet.insert(vec[i]);
    if (i % 2 == 0)
    {
      vector<string> lineArray;
      lineArray.resize(2);
      lineArray[0] = vec[i];
      lineArray[1] = vec[i + 1];
      File_save.push_back(lineArray);
    }
  }
  n = pointSet.size();
  MessageT="";
  //printf("sssssssssssaaaaahelper\n");
}

void readScore()
{
  
  //string message=recvScore();
  vector<string> vec = split(MessageS);
  //cout << vec.size() <<endl;
 //vector<string> vec = split(MessageS);
  for (size_t i = 0; i < vec.size(); i++) 
  {
    if (i % 2 == 0)
    {
      scoreMap.insert(pair<string, int>(vec[i], atoi(vec[i + 1].c_str())));
    }
   // cout<< i<<endl;
  }
  MessageS="";
 // printf("ssssssssssss\n");
}

// A utility function to find the vertex with minimum distance
// value, from the set of vertices not yet included in shortest
// path tree
int minDistance(int dist[], bool sptSet[], int V)
{
  // Initialize min value
  int min = INT_MAX, min_index;

  for (int v = 0; v < V; v++)
    if (sptSet[v] == false && dist[v] <= min)
      min = dist[v], min_index = v;

  return min_index;
}

// Function to print shortest path from source to j
// using parent array
void getPath(int parent[], int j, int src, map<string, int> Map)
{
 
  map<string, int>::iterator it;
  int current = j;
  if (parent[current] == -1)
    return;
  getPath(parent, parent[j], src, Map);
  if (current != src)
  {
    for (it = Map.begin(); it != Map.end(); it++)
      if ((it->second) == current)
      {
        MessageFeedback += it->first + " ";
      }
  }
}

//get score of the needed path
double getScore(int parent[], int j, map<int, int> map, double &sum)
{
  std::map<int, int>::iterator it;
  if (parent[j] < 0)
    return 0;
  double m1 = (double)(map.find(j)->second);
  double m2 = (double)(map.find(parent[j])->second);
  double s;
  if (m1 < m2)
    s = m2 - m1;
  if (m1 > m2)
    s = m1 - m2;
  double t = s / (m1 + m2);
  sum += t;
  getScore(parent, parent[j], score_index, sum);
  return sum;
}

// A utility function to get the constructed distance
void getSolution(int dist[], int n, int parent[], map<int, int> map, int V, int drc, int src)
{
  for (int i = 0; i < V; i++)
  {
    if (i == drc)
    {
      if (dist[i] <= 0 && (name1 !=name2))
      {
        MessageFeedback = "Found no compatibility between " + name1 + " and " + name2;
        break;
      }
      else
      {
        MessageFeedback = name1 + " ";
        getPath(parent, i, src, map_index);
        sum = getScore(parent, i, score_index, sum);
        MessageFeedback += " " + to_string(sum);
        sum=0;
      }
    }
  }
  //cout << MessageFeedback << endl;
}


void readToplogy()
{
  int graph[n][n];
 // cout << "n: " << n << endl;
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      graph[i][j] = 0;
    }
  }
  int index = 0;
  //build graph
  for (auto l : File_save)
  {
    if (map_index.count(l[0]) > 0 && map_index.count(l[1]) > 0)
    {
      //if both exists
      map<string, int>::iterator iter;
      iter = map_index.find(l[0]);
      int index1 = iter->second;
      map<string, int>::iterator iter2;
      iter2 = map_index.find(l[1]);
      int index2 = iter2->second;
      int val1 = scoreMap.find(l[1])->second;
      int val2 = scoreMap.find(l[0])->second;
      score_index.insert(pair<int, int>(index2, val1));
      score_index.insert(pair<int, int>(index1, val2));
      graph[index1][index2] = val1;
      graph[index2][index1] = val2;
    }
    else if (map_index.count(l[0]) > 0 && map_index.count(l[1]) == 0)
    {
      //0 yes 1 no
      map<string, int>::iterator iter;
      iter = map_index.find(l[0]);
      int index1 = iter->second;
      int index2 = index;
      map_index.insert(pair<string, int>(l[1], index));
      index++;
      int val1 = scoreMap.find(l[1])->second;
      score_index.insert(pair<int, int>(index2, val1));
      int val2 = scoreMap.find(l[0])->second;
      score_index.insert(pair<int, int>(index1, val2));
      graph[index1][index2] = val1;
      graph[index2][index1] = val2;
    }
    else if (map_index.count(l[1]) > 0 && map_index.count(l[0]) == 0)
    {
      //1 yes 0 no
      int index0 = index;
      map<string, int>::iterator iter;
      iter = map_index.find(l[1]);
      int index1 = iter->second;
      map_index.insert(pair<string, int>(l[0], index0));
      index++;
      int val1 = scoreMap.find(l[1])->second;
      int val2 = scoreMap.find(l[0])->second;
      score_index.insert(pair<int, int>(index1, val1));
      score_index.insert(pair<int, int>(index0, val2));
      graph[index0][index1] = val1;
      graph[index1][index0] = val2;
    }
    else
    {
      //both no
      int index0 = index;
      int index1 = index + 1;
      map_index.insert(pair<string, int>(l[0], index0));
      map_index.insert(pair<string, int>(l[1], index1));
      int val1 = scoreMap.find(l[1])->second;
      int val2 = scoreMap.find(l[0])->second;
      score_index.insert(pair<int, int>(index1, val1));
      score_index.insert(pair<int, int>(index0, val2));
      graph[index0][index1] = val1;
      graph[index1][index0] = val2;
      index = index + 2;
    }
  }
  // for (int i = 0; i < n; i++)
  // {
  //   cout << " " << endl;
  //   for (int j = 0; j < n; j++)
  //   {
  //     cout << graph[i][j] << " ";
  //   }
  // }
 // cout << endl;

  if (map_index.count(name1) == 0 || map_index.count(name2) == 0)
  {
    //client not in the map
    char output[BUFLENGTH];
    string str = "Found no compatibility between " + name1 + " and " + name2;
    memset(output, '\0', sizeof(output));
    strncpy(output, str.c_str(), str.length());
    output[strlen(output)]='\0';
    //send output information to central
    if (::sendto(sockfdUDP, output, strlen(output), 0, (const struct sockaddr *)&clientAddrUDP,
                 sizeof(clientAddrUDP)) < 0)
    {
      perror("serverP response failed");
      exit(1);
    }
    memset(output, '\0', sizeof(output));
   // memset(clientPosition,'\0',sizeof(clientPosition))
  }
  else
  {
    map<string, int>::iterator iter_clientA;
    iter_clientA = map_index.find(name1);
    int src = iter_clientA->second;
    int V = n;
    map<string, int>::iterator iter_clientB;
    iter_clientB = map_index.find(name2);
    int drc = iter_clientB->second;

    /**
         *dijkstra algorithm  from geeksforgeeks
        **/
    int dist[V]; // The output array. dist[i] will hold
    // the shortest distance from src to i
    // sptSet[i] will true if vertex i is included / in shortest
    // path tree or shortest distance from src to i is finalized
    bool sptSet[V];
    // Parent array to store shortest path tree
    int parent[V];
    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < V; i++)
    {
      parent[i] = -1;
      dist[i] = INT_MAX;
      sptSet[i] = false;
    }
    // Distance of source vertex from itself is always 0
    dist[src] = 0;
    // Find shortest path for all vertices
    for (int count = 0; count < V - 1; count++)
    {
      // Pick the minimum distance vertex from the set of
      // vertices not yet processed. u is always equal to src
      // in first iteration.
      int u = minDistance(dist, sptSet, V);

      // Mark the picked vertex as processed
      sptSet[u] = true;

      // Update dist value of the adjacent vertices of the
      // picked vertex.
      for (int v = 0; v < V; v++)
      {

        // Update dist[v] only if is not in sptSet, there is
        // an edge from u to v, and total weight of path from
        // src to v through u is smaller than current value of
        // dist[v]
        if (!sptSet[v] && graph[u][v] && (dist[u] + graph[u][v]) < dist[v])
        {
          parent[v] = u;
          dist[v] = dist[u] + graph[u][v];
        }
      }
    }

    // print the constructed distance array
    getSolution(dist, V, parent, score_index, V, drc, src);
    //cout << "MessageFeedback:" <<MessageFeedback << endl;
    //printf("dddddddddddd\n");
    char clientPosition[1024];
    memset(clientPosition, '\0', sizeof(clientPosition));
    int i;
    char *x = clientPosition;
    int sizeResult =  MessageFeedback.size();
    string::iterator sit = MessageFeedback.begin();
    while (sit != MessageFeedback.end())
    {
      if (sizeResult < 1023)
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
        perror("serverP sendto failed");
        exit(1);
      }
      memset(clientPosition, '\0', sizeof(clientPosition));
      x = clientPosition;
    }
    while (sit != MessageFeedback.end())
    {
      *x = *sit;
      x++;
      sit++;
    }
   // printf("ssssssssss\n");
    clientPosition[strlen(clientPosition)] = '\0';
    //string s= clientPosition;
    //cout << "s:"<< s << endl;
    if (::sendto(sockfdUDP, clientPosition, strlen(clientPosition), 0, (const struct sockaddr *)&clientAddrUDP,
                 sizeof(clientAddrUDP)) < 0)
    {
      perror("serverP sendto failed");
      exit(1);
    }
    memset(clientPosition, '\0', sizeof(clientPosition));
  }
    MessageFeedback="";
  cout << "The ServerP finished sending the results to the Central." << endl;
}

int main()
{
  connectAndBindUDPsocket();
  while(true){
  recvTopoplogy();
  recvScore();
  recvName();
  cout <<"The ServerP received the topology and score information."<<endl;
  readHelper();
 // cout <<"aaaaaaaaaa"<<endl;
  readScore();
 // cout <<"bbbbbbbbbbbb" <<endl;
  readToplogy();
  }
  close(sockfdUDP);
}
