
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

// Electric switch implementation: routing, buffering, and traffic demand reporting.

#include "Switch.h"
#include "socket_client.h"
#include <Windows.h>
#include <fstream>
#include "MEMS/MEMS_Controller.h"
#include <map>  

namespace dragonflyplus {

// Global MEMS topology: MEMS_ID -> list of (srcGroup, dstGroup) optical connections
std::unordered_map<int,std::vector<std::pair<int,int>>> gobalTopo;

Define_Module(Switch);

void Switch::initialize()
{
    dealDelay = par("dealDelay");
    selflid = par("gobalId");
    controlPort = par("controlPort");
    isleafswitch = par("isleafswitch");
    selfgroupnumber = getSelfGroupNumber();
    allocateInterval = par("allocateInterval");
    numOfGroups = par("numOfGroups");
    numOfMEMS = par("numOfMEMS");
    numOfSwitch = par("numOfSwitch"); 
    numOfNode = par("numOfNode");
    switchInPod = par("switchInPod");
    numOfPorts = par("numOfPorts");
    portEnable.assign(numOfPorts,false);
    Q_out.resize(numOfPorts);
    finishFlag.resize(numOfPorts);
    portSend.assign(numOfPorts,0);
    portReceive.assign(numOfPorts,0); 

    inpodmsg=0;
    outpodmsg=0;
    for(int i=0;i<numOfPorts;i++){
        char pname[20];
        sprintf(pname, "outport%d", i);
        SWFLAG *tmp = new SWFLAG(pname,SW_FLAG);
        tmp->setOutPortNum(i);
        finishFlag[i] = tmp;
    }
    if(isleafswitch){
        for(int i=0;i<numOfPorts;i++){
            portEnable[i] = true;
        }
    }
    if(!isleafswitch){
        for(int i=0;i<controlPort;i++){
            portEnable[i] = true;
        }
        trafficDemand.assign(numOfGroups,0);
        allocateSignal = new cMessage("allocateSignal");
        // Spine switch periodically uploads traffic demand to MEMS controller
        scheduleAt(simTime() + allocateInterval, allocateSignal);

    }
}


void Switch::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()){
        if(msg==allocateSignal){
            if(isleafswitch)
                error("leafswitch receive allocateSignal");
            upLoadTrafficMatrix();
            scheduleAt(simTime() + allocateInterval,allocateSignal);
        }
        else if(msg->getKind()==SW_FLAG){
            // Link finished transmitting; dequeue and send the next flit on this port
            SWFLAG* flagMsg = dynamic_cast<SWFLAG*>(msg);
            int outPort = flagMsg->getOutPortNum();
            cancelEvent(flagMsg);
            sendDataMsg(outPort);
        }
        else{
            error("unknown selfmsg!!");
        }
    }
    else{
        short kind = msg->getKind();
        if(kind==DATA_MSG){
            handleDataMsg(dynamic_cast<dataMsg*>(msg));
        }
        else if(kind==FC_MSG){

        }
        else if(kind==SETUP_MSG){
            handleSetup(dynamic_cast<setupMsg*>(msg));
        }
        else{
            error("receive unknown msg %s",getFullPath().c_str());
        }
    }
}

void Switch::printTuple(const std::tuple<int, int, float>& t) {
    std::cout << "(" << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << ")";

}

// Determine output port for a given destination logical ID.
// Intra-group traffic uses local leaf/spine routing; inter-group traffic
// uses BFS over the current MEMS logical topology plus load balancing.
int Switch::routing(int dst){
    int outputNum = -1;
    int dstGroup = (dst - (numOfSwitch * 2)) / ( numOfNode / numOfGroups);

    if(dstGroup==selfgroupnumber){
        // Destination is in the same pod/group: route via leaf switches
        int numInLeafSwitch = numOfNode / numOfGroups / switchInPod;
        int dstLeafSwitch = numOfSwitch + (dst - numOfNode) / numInLeafSwitch;
        if(isleafswitch){
            if(selflid==dstLeafSwitch)
                outputNum = switchInPod + dst % 2;
            else{
                outputNum = 0;
                for(int i=1;i<switchInPod;i++){
                    if(Q_out[i].getLength()<Q_out[outputNum].getLength())
                        outputNum = i;
                }
            }
        }
        else{
            outputNum = dstLeafSwitch % switchInPod;
  
        }
    }
  
    else{
        // Inter-group routing: build group-level adjacency from MEMS topology
        std::vector<std::vector<int>> graphThroughGroup(numOfGroups,std::vector<int>(numOfGroups,0));
        for(auto it=gobalTopo.begin();it!=gobalTopo.end();it++){
            for(unsigned int i=0;i<it->second.size();i++){
                graphThroughGroup[it->second[i].first][it->second[i].second] = 1; 
            }
        }


        std::vector<int> nextHop;
        std::vector<std::tuple<int, int, float>> item_nexthop;  
        int minLength = INT_MAX;
        int hop_num=0;

        while(bfs(graphThroughGroup,nextHop,selfgroupnumber,dstGroup,minLength,item_nexthop)){
        }


        auto itNextHop = nextHopTable.find({selfgroupnumber, dstGroup});
        if (itNextHop != nextHopTable.end()) {

            nextHop = itNextHop->second; 
        }
        auto itItemNexthop = nexthopInfoTable.find({selfgroupnumber, dstGroup});
        if (itItemNexthop != nexthopInfoTable.end()) {

            item_nexthop = itItemNexthop->second;
        }
        if (nextHop.empty()) {
     
            while(bfs(graphThroughGroup,nextHop,selfgroupnumber,dstGroup,minLength,item_nexthop)){
                    }

        }
        std::vector<int> outputPorts;   
        std::vector<std::tuple<int, int, float>> item_outputPorts; 
        for(unsigned int i=0;i<nextHop.size();i++){
           
            for(unsigned int j=0;j<routingTable[nextHop[i]].size();j++){
                hop_num++;
                if(hop_num>1){
                    break;
                }
              
                int portIdx = routingTable[nextHop[i]][j].second;
                if(find(outputPorts.begin(),outputPorts.end(),portIdx)==outputPorts.end()){
                 
                    outputPorts.emplace_back(portIdx);
                    std::tuple<int, int, float> tmp = item_nexthop[i];
                    
                    std::get<0>(tmp) = portIdx;
//                  
                    item_outputPorts.emplace_back(tmp);
                }
            }
        }


        if(outputPorts.size()!=0){
            // Weighted random selection among next-hop ports, then pick shortest queue
            float random_value=0.5;
            float cumulative_probability = 0;
            for (const auto& item : item_outputPorts) {
                cumulative_probability += std::get<2>(item);
                if (random_value < cumulative_probability) {
                    outputNum=std::get<0>(item);
                    break;
                }
            }
            if(outputNum==-1){
            outputNum = outputPorts[0];
            }

            for(unsigned int i=1;i<outputPorts.size();i++){
                if(Q_out[outputPorts[i]].getLength()<Q_out[outputNum].getLength())
                    outputNum = outputPorts[i];

            }
       }

        else{
            if(isleafswitch) error("error in route!!");
            else{
                outputNum = 0;

                for(int i=1;i<switchInPod;i++){
                    if(Q_out[i].getLength()<Q_out[outputNum].getLength())
                        outputNum = i;
                }
            }
        }

    }

    return outputNum;
}


// Apply a MEMS reconfiguration message: update routing table and port enable flags
void Switch::handleSetup(setupMsg *msg){
    if(isleafswitch) error("leafswitch receive error msg");
    bool flag = msg->getFlag();
    bool ini = msg->getIni();
    if(ini){
        // Initial topology setup: register MEMS connections and propagate to leaf switches
        std::vector<int> configuration = msg->getConfiguration();
        int MEMSID = msg->getMEMSID();// MEMSID
        if(configuration[selfgroupnumber]==-1) return;
        int portIdx = controlPort + 1 + msg->getMEMSID()/switchInPod;
        portEnable[portIdx] = true;

        std::pair<int,int> temp(selfgroupnumber,configuration[selfgroupnumber]);
        gobalTopo[MEMSID].emplace_back(temp);

        temp.first = MEMSID;
        temp.second = portIdx;
        routingTable[configuration[selfgroupnumber]].emplace_back(temp);

        for(int i=0;i<switchInPod;i++){
            cGate* sw_gate = this->gate("portx$o",i)->getPathEndGate();
            Switch* leaf_sw = dynamic_cast<Switch *>(sw_gate->getOwnerModule());

            leaf_sw->setRoutingTable(msg->dup(),selflid%4);

        }

    }
    else{
        if(flag){
            // Re-enable port after MEMS reconfiguration completes
            int portIdx = controlPort + 1 + msg->getMEMSID()/switchInPod;
            portEnable[portIdx] = true;
        }
        else{
            // Tear down old MEMS routes and install new configuration
            std::vector<int> configuration = msg->getConfiguration();
            if(configuration[selfgroupnumber]==-1) return;
            int MEMSID = msg->getMEMSID();// MEMSID
            for(auto it_=routingTable.begin();it_!=routingTable.end();it_++){
                for(int i=0;i<int(it_->second.size());){
                    if(it_->second[i].first==MEMSID)
                        it_->second.erase(it_->second.begin()+i);
                    else{
                        i++;
                    }
                }
            }
            int portIdx = controlPort + 1 + msg->getMEMSID()/switchInPod;
            portEnable[portIdx] = false;
    
            std::pair<int,int> temp(selfgroupnumber,configuration[selfgroupnumber]);
            gobalTopo[MEMSID].emplace_back(temp);

       
            temp.first = MEMSID;
            temp.second = portIdx;
            routingTable[configuration[selfgroupnumber]].emplace_back(temp);
  
            for(int i=0;i<switchInPod;i++){
                cGate* sw_gate = this->gate("portx$o",i)->getPathEndGate();
                Switch* leaf_sw = dynamic_cast<Switch *>(sw_gate->getOwnerModule());
  
                leaf_sw->setRoutingTable(msg->dup(),selflid%4);
  
            }

        }
    }
    cancelAndDelete(msg);
}


void Switch::handleDataMsg(dataMsg* msg){
    updateTrafficDemand(msg);
    std::pair<int,int> temp;
    temp.first = msg->getSrcLid();
    temp.second = msg->getMsgId();
    int outPort = -1;
 
        for (int i = 0; i < numOfPorts; i++) {
            cGate* inputGate = this->gate("portx$i", i);
         
            if (msg->getArrivalGate() == inputGate) {
                portReceive[i]++;
                break; 
            }
        }

    outPort = routing(msg->getDstLid());
    if(ifEnable(outPort)){
           flowTable[temp] = outPort;
           Q_out[outPort].insert(msg);
           if(!finishFlag[outPort]->isScheduled())
               sendDataMsg(outPort);
       }

}

int Switch::getSelfGroupNumber(){
    int temp = -1;
    int numOfSpineSwitch = par("numOfSwitch");
    int switchInPod = par("switchInPod");
    if(selflid>=numOfSpineSwitch*2) error("not electric switch!!!");
    if(isleafswitch){
        temp = (selflid - numOfSpineSwitch)/switchInPod;
    }
    else{
        temp = selflid / switchInPod;
    }
    return temp;
}

// Send aggregated inter-group traffic demand to the MEMS controller
void Switch::upLoadTrafficMatrix(){
    BufferLength *pkt = new BufferLength("bufferlength",BUFLEN_MSG);
    pkt->setSrcGroupIdx(selfgroupnumber);
    pkt->setBufLen(trafficDemand);
    trafficDemand.assign(numOfGroups,0);
//    cout<<"send something please!"<<endl;
    send(pkt,"portx$o",4);
}

// Count inter-group traffic passing through this spine switch
void Switch::updateTrafficDemand(dataMsg* msg){
    int srcGroup = (msg->getSrcLid() - numOfNode) / (numOfNode / numOfGroups);
    int dstGroup = (msg->getDstLid() - numOfNode) / (numOfNode / numOfGroups);
    if(!isleafswitch && selfgroupnumber==srcGroup && selfgroupnumber!=dstGroup){
        trafficDemand[dstGroup]++;
    }
}


bool Switch::ifEnable(int outPort){
    if(outPort<0||outPort>=numOfPorts) error("Invalid port %d!!",outPort);
    if((!isleafswitch) && outPort==controlPort) error("controlPort can not send data msg");
    return true;
}

// Pop head of egress queue and forward with optional store-and-forward delay
void Switch::sendDataMsg(int outPort){
    if(finishFlag[outPort]->isScheduled()) error(" port %d is busy now!",outPort);
    if(Q_out[outPort].isEmpty() || portEnable[outPort] == false) return;
    dataMsg* msg = dynamic_cast<dataMsg*>(Q_out[outPort].pop());
    int msg_length=msg->getPathLength();

    simtime_t storeTime = simTime() - msg->getArrivalTime();
    if(storeTime.dbl()>dealDelay){
        send(msg,"portx$o",outPort);
    }
    else{
        sendDelayed(msg,dealDelay,"portx$o",outPort);
    }
    portSend[outPort]++;

    scheduleAt(gate("portx$o",outPort)->getTransmissionChannel()->getTransmissionFinishTime(),finishFlag[outPort]);
}

// BFS on group-level graph to find the next hop toward dstGroup
bool Switch::bfs(std::vector<std::vector<int>>& graphThroughGroup,std::vector<int>& nextHop,int src,int dst,int& minLength,std::vector<std::tuple<int, int, float>>& item_nexthop){

    int length = 1;
    int max_length = 2;
    std::vector<int> path(numOfGroups,-1);
    std::vector<bool> visited(numOfGroups,false);
    std::queue<int> q;
    q.push(src);
    visited[src] = true;
    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v = 0; v < numOfGroups; v++) {
            if(u==v) continue;
            if(visited[v] == false && graphThroughGroup[u][v] > 0){
                q.push(v);
                path[v] = u;
                visited[v] = true;
            }
        }
    }
    if(visited[dst]){
        int u=dst;
        for(;path[u]!=src;u=path[u]){
            length++;
        }

        if(length<=minLength){
            minLength=length;
            nextHop.emplace_back(u);
            item_nexthop.emplace_back(std::make_tuple(u, length, 0.88));
            graphThroughGroup[selfgroupnumber][u] = 0;
            return true;

        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}

// Leaf switch: update local routing table entry from spine setup message
void Switch::setRoutingTable(setupMsg *msg,int portIdx)
{
    bool ini = msg->getIni();
    if(!ini){
        int MEMSID = msg->getMEMSID();// MEMSID
        for(auto it_=routingTable.begin();it_!=routingTable.end();it_++){
            for(int i=0;i<int(it_->second.size());){
                if(it_->second[i].first==MEMSID)
                    it_->second.erase(it_->second.begin()+i);
                else{
                    i++;
                }
            }
        }
    }
    std::vector<int> configuration = msg->getConfiguration();
    if(configuration[selfgroupnumber]==-1) 
        return;
    
    int MEMSID = msg->getMEMSID();
    std::pair<int,int> temp(MEMSID,portIdx);
    routingTable[configuration[selfgroupnumber]].emplace_back(temp);
    cancelAndDelete(msg);
}


void Switch::finish()
{
    for(int i=0;i<numOfPorts;i++){
        cancelAndDelete(finishFlag[i]);
    }
    finishFlag.clear();
    std::string baseNameT = "PortSend_";
    std::string baseNameR = "PortReceive_";
    for(unsigned int i=0;i<portSend.size();i++){
        std::string temp = baseNameT + std::to_string(i);
        recordScalar(temp.c_str(),portSend[i]);
    }
    for(unsigned int i=0;i<portReceive.size();i++){
            std::string temp = baseNameR + std::to_string(i);
           recordScalar(temp.c_str(),portReceive[i]);
    }
}


}; // namespace



