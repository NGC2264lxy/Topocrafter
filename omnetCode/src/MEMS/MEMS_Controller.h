//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __DRAGONFLYPLUS_MEMS_CONTROLLER_H
#define __DRAGONFLYPLUS_MEMS_CONTROLLER_H

// Central controller for MEMS optical switches: collects traffic demand,
// runs topology allocation (DRL / KM / Edmonds), and pushes reconfiguration.

#include <omnetpp.h>
#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <numeric>
#include "MEMS_Switch.h"
#include "Edmonds.h"
#include "KM.h"
#include "../packet/Packet_m.h"
#include <map>  

using namespace omnetpp;

namespace dragonflyplus {

// Global routing tables populated by Python routing service or BFS fallback
extern std::map<std::pair<int, int>, std::vector<int>> nextHopTable;
extern std::map<std::pair<int, int>, std::vector<std::tuple<int, int, float>>> nexthopInfoTable;
class MEMS_Controller : public cSimpleModule
{
protected:
    int numOfSamples;           // traffic samples to collect before each allocation
    int flitByteLength;
    int repetition;
    int numOfGroups;
    int numOfMEMS;
    int trafficReply;         // count of spine switches that reported demand
    int gobalLinks;
    int switchInPod;
    int numOfCommunication;     // DRL training step counter
    int stepLength;             // max DRL training steps before simulation ends
    bool curFinish;
    cMessage *handlePathLength;
    std::vector<std::vector<int>> linky;  // current MEMS config: linky[memsId][groupId] = peer group
    std::vector<std::vector<int>> trafficArray;
    std::vector<std::vector<int>> trafficDemandSeq;  // history of demand matrices for DRL
    std::vector<std::vector<int>> trafficSend;      // link utilization from MEMS port counters
    std::vector<std::vector<int>> trafficStatistics;
    std::vector<bool> allocateFlag;  // whether each MEMS needs reconfiguration this round
    std::vector<double> recordConfigurationTime;


    std::vector<std::vector<int>> spineTraffic;
    ///////////////////////
    int numOfHosts = 64;
    double allocateInterval;
    double totalAverageLength;

    int socketPort;             // port for DRL topology allocation agent
    int socketPort_logic;       // port for Python routing/next-hop service
    int heuristicAlgorithm;     // 0=DRL, 1=KM, 2=Edmonds

    bool firstTimeAllocate;
    bool rTopo;

    double T_night;             // delay before re-enabling ports after reconfiguration
    double T_nightInMEMS;
    double messageinterval_us; 
    int msgLength; 




////////////////////////////////////////////////////////////////////////
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
////////////////////////////////////////////////////////////////////////

    void synchronization();
    void handleTrafficDemand(BufferLength *msg);
    void showAndRecordTrafficMatrix();
    void showMEMSConfiguration();
    void showLogicTopology(const std::vector<std::vector<int>>& logicTopo,const std::string& matrix_name);
    void handleDRLResult(std::vector<std::vector<int>>& resultTopo);
    void allocate();

    void recombineForTopo(std::vector<std::vector<int>>& logicTopo,std::unordered_map<int,std::vector<int>>& paths,std::vector<std::vector<int>>& recordIndirectPath,int MEMSID);
    void genTopo(std::vector<std::vector<int>>& logicTopo,std::unordered_map<int,std::vector<int>>& paths,int MEMSID,std::vector<std::vector<int>>& recordIndirectPath);
    void getValidLinks(std::vector<int>& validLinks,std::vector<std::vector<int>>& logicTopo,int groupId);
    void updateState(std::vector<std::vector<int>>& logicTopo,std::vector<std::vector<int>>& recordIndirectPath,int MEMSID);
    void backState(std::vector<std::vector<int>>& logicTopo,std::vector<std::vector<int>>& recordIndirectPath,int MEMSID);

    bool checkDRLResult(const std::vector<std::vector<int>>& resultTopo);
    bool canUse(std::vector<std::vector<int>>& tempTopo);
    bool ifKeep(int MEMSID,std::vector<std::vector<int>>& logicTopo);
    bool recombineFinish(const std::vector<std::vector<int>>& logicTopo);
    bool ifRecombineForMEMS(const std::vector<int>& portSend,int countTraffic);

    double calReward(bool flag,const std::vector<std::vector<int>>& temp);
    double calReward_(const std::vector<int>& portSend,bool flag,int countTraffic,double timeStamp);


};

}; // namespace

#endif
