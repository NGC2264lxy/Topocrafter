//
// TCP socket client for communicating with external Python DRL/routing agents.
// Message format: "<finishFlag>;<msgType>;<payload...>"
//

#include <WINSOCK2.H>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>  
#include <sstream>  

using namespace std;

class serverClient
{
public:
    bool init(int numOfGroups,int numOfMEMS);
    void communication(const string &data, int socketPort);
    void handleRecv(char recData[]);
    vector<vector<int>> getResult(); // returned MEMS topology from Python agent

    // Pack simulation state into a string for the DRL training agent
    string handleSend(const vector<vector<int>>& trafficDemandMatrix,const vector<vector<int>>& trafficMatrix, const vector<vector<int>>& topo, const vector<vector<int>>&fault, const vector<double>& rewardForMEMS, int numOfCommunication,int stepLength,int needConfigureNum,double totalAverageLength);
    string sendLogicTopoToPython(const vector<vector<int>>& logicTopo);
   
    map<pair<int, int>, vector<int>> getNextHopTable();
    map<pair<int, int>, vector<tuple<int, int, float>>>getNexthopInfoTable() ;
    void handleRouteMsg(const char* msgContent);
    map<pair<int, int>, vector<int>> nextHopTable_socekt;
    map<pair<int, int>, vector<std::tuple<int, int, float>>> nexthopInfoTable_socekt;

private:

protected:
    bool isFinish;              // true when DRL training step limit is reached
    vector<vector<int>> topology;  // MEMS configuration: topology[memsId][groupId] = peer group
    vector<vector<float>> routingProb;
};

