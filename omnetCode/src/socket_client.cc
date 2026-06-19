#include <WINSOCK2.H>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include "socket_client.h"
#include <map>  
#include <sstream> 

using namespace std;

// Initialize Winsock and allocate topology storage
bool serverClient::init(int numOfGroups,int numOfMEMS){
    isFinish = false;
    topology.resize(numOfMEMS);
    
    for(unsigned int i=0;i<topology.size();i++){
        topology[i].assign(numOfGroups,-1);
    }

    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA data;
    if(WSAStartup(sockVersion, &data)!=0)
       
    {
        return false;
    }
    else{
        return true;
    }
}

// Connect to localhost Python server, send request, receive and parse response
void serverClient::communication(const string &data, int socketPort){

    SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(sclient == INVALID_SOCKET)
            {
                cout << "invalid socket!" << endl;
                return;
            }
            sockaddr_in serAddr;
            serAddr.sin_family = AF_INET;//protocols
            serAddr.sin_port = htons(socketPort);
            serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
            if(connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
            {
                printf("Connection Disconnection!");
                closesocket(sclient);
                return;
            }

cout<<"start to send"<<endl;
       send(sclient, data.c_str(), data.length(), 0);

       if (!isFinish) {
           char recData[8192];
           cout << "communication" << endl;
           while (1) {
               int ret = recv(sclient, recData, sizeof(recData) - 1, 0);
               if (ret > 0) {
                   recData[ret] = 0x00; // null terminate
                   break; 
               } else if (ret == 0) {
                   
                   cout << "Connection closed by peer." << endl;
                   break;
               } else {
                  
                   cout << "recv error: " << WSAGetLastError() << endl;
                   break;
               }
           }
           handleRecv(recData); 

       }

       closesocket(sclient); 
        WSACleanup();
        return;
}

// Parse response by message type: "topomsg" (MEMS config) or "routemsg" (next-hop table)
void serverClient::handleRecv(char recData[]){

    string msgType;
    int i = 0;

    while (recData[i] != ';' && i < strlen(recData)) {
        msgType += recData[i];
        i++;
    }
    i++;  

    if (msgType == "topomsg") {
        // Format: comma-separated peer groups per MEMS row, semicolon-separated rows
        int src = 0;
        int dst = 0;
        int MEMS_ID = 0;
        for (; i < strlen(recData); i++) {
            if (recData[i] == ',') {
                topology[MEMS_ID][src] = dst;
                dst = 0;
                src++;
            } else if (recData[i] == ';') {
                topology[MEMS_ID][src] = dst;
                src = 0;
                dst = 0;
                MEMS_ID++;
            } else {
                dst = dst * 10 + recData[i] - '0';
            }
        }
    } else if (msgType == "routemsg") {
        // Format per entry: src,dst,path,prob;  path uses '-' separated group IDs
        handleRouteMsg(recData + i);  
    }
        else {
        cout << "Unknown message type received: " << msgType << endl;
    }
}

// Parse routing entries and populate next-hop lookup tables
void serverClient::handleRouteMsg(const char* msgContent) {


    istringstream iss(msgContent);
    string entry;


    while (getline(iss, entry, ';')) {
        istringstream entryStream(entry);
        string srcStr, dstStr, pathStr, probStr;

        getline(entryStream, srcStr, ',');
        getline(entryStream, dstStr, ',');
        getline(entryStream, pathStr, ',');
        getline(entryStream, probStr, ',');

        int src = stoi(srcStr);
        int dst = stoi(dstStr);
        float probability = stof(probStr);


        istringstream pathStream(pathStr);
        string nodeStr;
        vector<int> path;
        while (getline(pathStream, nodeStr, '-')) {
            path.push_back(stoi(nodeStr));
        }

        int next_hop = path[1];
        int path_length = path.size() - 1; 

        nextHopTable_socekt[{src, dst}].push_back(next_hop);
        nexthopInfoTable_socekt[{src, dst}].emplace_back(make_tuple(next_hop, path_length, probability));


   }
}


// Serialize traffic matrices, topology, fault info, and path-length stats for DRL agent
string serverClient::handleSend(const vector<vector<int>>& trafficDemandMatrix,const vector<vector<int>>& trafficMatrix, const vector<vector<int>>& topo, const vector<vector<int>>&fault,const vector<double>& rewardForMEMS, int numOfCommunication,int stepLength,int needConfigureNum,double totalAverageLength){
    string ans = "";

    if(numOfCommunication>=(stepLength)){
        ans += "True;";
        isFinish = true;

    }
    else{
        ans += "False;";
    }
    ans += "elsemsg;";

       for(unsigned int i=0;i<trafficDemandMatrix.size();i++){
           for(unsigned int j=0;j<trafficDemandMatrix[i].size();j++){
               ans = ans + to_string(trafficDemandMatrix[i][j]);
               if(j+1==trafficDemandMatrix[i].size())
                   ans += ";";
               else{
                   ans += ",";
               }
           }
       }

    for(unsigned int i=0;i<trafficMatrix.size();i++){
        for(unsigned int j=0;j<trafficMatrix[i].size();j++){
            ans = ans + to_string(trafficMatrix[i][j]);
            if(j+1==trafficMatrix[i].size())
                ans += ";";
            else{
                ans += ",";
            }
        }
    }

    for(unsigned int i=0;i<topo.size();i++){
        for(unsigned int j=0;j<topo[i].size();j++){
            ans += to_string(topo[i][j]);
            if(j+1==topo[i].size()) ans += ";";
            else{
                ans += ",";
            }
        }
    }

    for(unsigned int i=0;i<fault.size();i++){
               for(unsigned int j=0;j<fault[i].size();j++){
                   ans = ans + to_string(fault[i][j]);
                   if(j+1==fault[i].size())
                       ans += ";";
                   else{
                       ans += ",";
                   }
               }
           }


    ans +=to_string(totalAverageLength);
    return ans;
}

vector<vector<int>> serverClient::getResult(){
    return topology; 
}


map<pair<int, int>, vector<int>> serverClient::getNextHopTable() {
    return nextHopTable_socekt;
}

map<pair<int, int>, vector<tuple<int, int, float>>> serverClient::getNexthopInfoTable() {
    return nexthopInfoTable_socekt;
}


// Send current logical topology to Python routing service and request next-hop info
string serverClient::sendLogicTopoToPython(const vector<vector<int>>& logicTopo) {
    string ans = "";
    ans += "False;";
    ans += "topomsg;";

           for(unsigned int i=0;i<logicTopo.size();i++){
               for(unsigned int j=0;j<logicTopo[i].size();j++){
                   ans = ans + to_string(logicTopo[i][j]);
                   if(j+1==logicTopo[i].size())
                       ans += ";";
                   else{
                       ans += ",";
                   }
               }
           }
   cout <<"socket:ans()" <<ans << endl;
   return ans;
}



