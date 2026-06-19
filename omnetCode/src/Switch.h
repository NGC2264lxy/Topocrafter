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

#ifndef __DRAGONFLYPLUS_SWITCH_H
#define __DRAGONFLYPLUS_SWITCH_H

// Electric switch module (leaf or spine): buffers flits, performs routing,
// and reports inter-group traffic demand to the MEMS controller.

#include <omnetpp.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <limits.h>
#include <queue>
#include <algorithm>
#include "packet/Packet_m.h"
#include <map> 
#include "MEMS/MEMS_Controller.h"
using namespace omnetpp;
using namespace std;

namespace dragonflyplus {

class Switch : public cSimpleModule
{
  protected:
    int selflid;
    int numOfMEMS;
    int numOfPorts;
    int numOfGroups;
    int numOfSwitch;
    int numOfNode;
    int switchInPod;
    int selfgroupnumber;      // group index this switch belongs to
    int controlPort;          // number of host-facing ports on a spine switch
    bool isleafswitch;
    std::vector<cQueue> Q_out;  // per-output-port egress queues
    std::map<std::pair<int,int>,int> flowTable;  // (srcLid, msgId) -> assigned outPort
    std::unordered_map<int,std::vector<std::pair<int,int>>> routingTable;  // dstGroup -> [(MEMSId, portIdx), ...]
    std::vector<SWFLAG*> finishFlag;  // self-messages signaling link transmission completion
    double allocateInterval;
    double dealDelay;         // minimum store-and-forward delay before forwarding
    cMessage *allocateSignal;
    std::vector<int> trafficDemand;  // inter-group demand counters (spine switches only)
    std::vector<bool> portEnable;    // whether a MEMS-facing port is configured and active
    std::vector<double> portSend;
    std::vector<int> portReceive;
    std::vector<double> bufferLength;

    int inpodmsg;
    int outpodmsg;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();


    int getSelfGroupNumber();
    void printTuple(const std::tuple<int, int, float>& t);
    void handleDataMsg(dataMsg* msg);
    bool ifEnable(int outPort);
    int routing(int dst);
    void sendDataMsg(int outPort);
    void upLoadTrafficMatrix();
    void handleSetup(setupMsg *msg);
    bool bfs(std::vector<std::vector<int>>& graphThroughGroup,std::vector<int>& nextHop,int src,int dst,int& minLength,std::vector<std::tuple<int, int, float>>& item_nexthop);
    void updateTrafficDemand(dataMsg* msg);
  public:
    void setRoutingTable(setupMsg *msg,int portIdx);
   
};

}; // namespace

#endif
