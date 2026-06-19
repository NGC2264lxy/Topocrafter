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

// MEMS controller: traffic collection, topology allocation, and reconfiguration.

#include "MEMS_Controller.h"
#include "MEMS_Switch.h"
#include "../socket_client.h"
#include <Windows.h>
#include <fstream>
#include <map> 

namespace dragonflyplus {

extern std::unordered_map<int,std::vector<std::pair<int,int>>> gobalTopo;
std::map<std::pair<int, int>, std::vector<int>> nextHopTable;
std::map<std::pair<int, int>, std::vector<std::tuple<int, int, float>>> nexthopInfoTable;
Define_Module(MEMS_Controller);
std::vector<int> totalPathLength;  // per-host average path length from Host modules
void MEMS_Controller::initialize()
{
    cMessage *startSignal = new cMessage("startSignal");
    flitByteLength = par("flitByteLength");
    heuristicAlgorithm = par("heuristicAlgorithm");
    repetition = par("repetition");
    numOfMEMS = par("numOfMEMS");
    numOfGroups = par("numOfGroups");
    socketPort = par("socketPort");
    socketPort_logic=par("socketPort_logic");
    messageinterval_us = par("messageinterval_us");
    msgLength = par("msgLength");
    gobalLinks = par("gobalLinks");
    switchInPod = par("switchInPod");
    numOfSamples = par("numOfSamples");
    stepLength = par("stepLength");
    rTopo = par("rTopo");
    allocateInterval = par("allocateInterval");
    totalAverageLength = 0;

    numOfCommunication = 0;
    curFinish = false;

    trafficReply = 0;
    T_night = par("T_night");
    T_nightInMEMS = par("T_nightInMEMS");
    handlePathLength = new cMessage("handlePathLength");

    allocateFlag.assign(numOfMEMS,true);//
    //allocateFlag.assign(numOfMEMS,false);

    linky.resize(numOfMEMS);
    recordConfigurationTime.assign(numOfMEMS,0);
    for(int i=0;i<numOfMEMS;i++){
        linky[i].assign(numOfGroups,-1);
    }
    trafficArray.resize(numOfGroups);
    trafficStatistics.resize(numOfGroups);
  
    spineTraffic.resize(numOfMEMS*switchInPod);
    for(int i=0;i<numOfMEMS*switchInPod;i++){
        spineTraffic[i].assign(numOfGroups,0);
       }
    trafficSend.resize(numOfGroups);
    for(int i=0;i<numOfGroups;i++){
        trafficArray[i].assign(numOfGroups,0);
        trafficStatistics[i].assign(numOfGroups,0);
        trafficSend[i].assign(numOfGroups,0);
    }
    if(numOfMEMS>0)
    scheduleAt(simTime(),startSignal);
    totalPathLength.assign(numOfHosts,0);
    scheduleAt(simTime(),handlePathLength);

}

void MEMS_Controller::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()){

        if(msg==handlePathLength){
            // Aggregate per-host path length and compute network-wide average
            cout << "/////////////////////// handle path length ///////////////////////" << endl;
            totalAverageLength = 0;
            int totalLength = 0;
            int num_nozero = 0;
            for(int i=0;i<totalPathLength.size();i++){
                if(totalPathLength[i]!=0){
                    totalLength = totalLength + totalPathLength[i];
                    num_nozero++;
                }
            }
            cout << endl;
            if (num_nozero){

                totalAverageLength = totalLength / num_nozero;
            }
            totalPathLength.assign(numOfHosts,0);
            scheduleAt(simTime() + allocateInterval, handlePathLength);
        }
        if(simTime()>0.00001){
            return;
        }
        Edmonds *initTopo = new Edmonds;
        // Compute initial MEMS topology using Edmonds matching at simulation start
        initTopo->init(numOfGroups,numOfMEMS);
        initTopo->setBufLen();
        linky = initTopo->multicalculate();
        
        cout<<"----initial MEMS Configuration--------"<<endl;
        showMEMSConfiguration();
        setupMsg *setup = new setupMsg("setup",SETUP_MSG);
        setup->setIni(true);
        setup->setFlag(true);
        for(int MEMS_Id=0;MEMS_Id<numOfMEMS;MEMS_Id++){
            setup->setMEMSID(MEMS_Id);
            std::vector<int> temp(numOfGroups,-1);
            for(int GroupId=0;GroupId<numOfGroups;GroupId++){
                temp[GroupId] = linky[MEMS_Id][GroupId];
            }
            setup->setConfiguration(temp);
           
            send(setup->dup(),"MEMS$o",MEMS_Id);
            for(int i=0;i<8;i++){
                std::cout << simTime() << " IIIIIIIIIIIIII " << std::endl;
                sendDelayed(setup->dup(),1e-9,"Pod$o",MEMS_Id%4 + i*4);

            }

           
        }

    }
    else{

        handleTrafficDemand(dynamic_cast<BufferLength*>(msg));
    }

}

// Main allocation routine: collect link utilization, then run selected algorithm
void MEMS_Controller::allocate(){
    int countTraffic = 0;
  
    for (int i = 0; i < trafficSend.size(); i++) {
          for (int j = 0; j < trafficSend[0].size(); j++) {
              trafficSend[i][j] = 0;
          }
      }
    for(int i=0;i<numOfMEMS;i++){
        cGate* sw_gate = this->gate("MEMS$o",i)->getPathEndGate();

        MEMS_Switch* MEMS_ = dynamic_cast<MEMS_Switch *>(sw_gate->getOwnerModule());
        std::vector<int> temp = MEMS_->getPortSend();
        std::vector<std::vector<int>> pod_traffic = MEMS_->getPodTraffic();
        MEMS_->resetPortSend();
        MEMS_->resetPortTraffic();
        for(int j=0;j<trafficSend.size();j++){
            for(int k=0;k<trafficSend[0].size();k++){
                trafficSend[j][k] = trafficSend[j][k] + pod_traffic[j][k];

            }
        }
        countTraffic += std::accumulate(temp.begin(),temp.end(),countTraffic);
        cout<<"countTraffic"<<countTraffic<<endl;
    }
    cout << "################## Link Utilization 2 #####################" << endl;
    for(int i=0;i<trafficSend.size();i++){
        for(int j=0;j<trafficSend[0].size();j++){
            if( trafficSend[i][j]<0){
                cout<<"errorTime:"<<simTime()<<endl;
                error("traffic send error");
            }
            cout << trafficSend[i][j] << " ";
        }
        cout << endl;
    }


 
    if(heuristicAlgorithm==0){
        // DRL-based topology reconfiguration via Python socket agent
        int needConfigureNum = 0;
        allocateFlag.assign(numOfMEMS,true);
        std::vector<double> rewardForMEMS(numOfMEMS,1.0);
        cout<<"I'm here!!!!!"<<endl;
        for(int i=0;i<numOfMEMS;i++){
            cGate* sw_gate = this->gate("MEMS$o",i)->getPathEndGate();
            MEMS_Switch* MEMS_ = dynamic_cast<MEMS_Switch *>(sw_gate->getOwnerModule());
            std::vector<int> temp = MEMS_->getPortSend();
            countTraffic += std::accumulate(temp.begin(),temp.end(),countTraffic);
            needConfigureNum += allocateFlag[i];

        }

        serverClient *DRL = new serverClient;

        while(true){
            if(DRL->init(numOfGroups,needConfigureNum)){
               
              
                string socketMsg = DRL->handleSend(trafficDemandSeq,trafficSend,linky,faultMatrix,rewardForMEMS,numOfCommunication,stepLength,needConfigureNum,totalAverageLength);// ������������
               
                DRL->communication(socketMsg,socketPort);
                if(numOfCommunication>=(stepLength)){
                    recordScalar("finish time",simTime());
                    endSimulation();
                  }
                  numOfCommunication++;

                trafficDemandSeq.clear();
              
                std::vector<std::vector<int>> newLogTopo = DRL->getResult();
               
                showLogicTopology(newLogTopo,"newLogTopo");

                std::vector<std::vector<int>> logicTopo(numOfGroups,std::vector<int>(numOfGroups,0));
                for(int i=0;i<numOfMEMS;i++){

                    if(!allocateFlag[i]){
                        for(int j=0;j<numOfGroups;j++){
                            logicTopo[j][linky[i][j]]++;
                        }
                    }
        
                    else{
                        if(newLogTopo.size()==0) error(" error in MEMS controller");
                        std::vector<int> temp = newLogTopo[0];
                        newLogTopo.erase(newLogTopo.begin());
                        for(int j=0;j<numOfGroups;j++){
                            if (temp[j]<0||temp[j]>7){
                                 continue;
                            }
                            else{
                            logicTopo[j][temp[j]]++;
                            linky[i][j]=temp[j];
                            }
                        }
                    }
                }
                cout<<"-----------------new logic topo---------"<<endl;
                showLogicTopology(logicTopo,"new logic topo");
                serverClient *DRL2= new serverClient;
               if(DRL2->init(numOfGroups,numOfMEMS)){

                   string socketMsg=DRL2->sendLogicTopoToPython(linky);

                   DRL2->communication(socketMsg,socketPort_logic);

                   nextHopTable = DRL2->getNextHopTable();
                   nexthopInfoTable = DRL2->getNexthopInfoTable();
                   cout<<"I'm well"<<endl;

                   }


                trafficStatistics.clear();
                trafficStatistics.resize(numOfGroups);
                for(int i=0;i<numOfGroups;i++){
                    trafficStatistics[i].assign(numOfGroups,0);
                }
                synchronization();
                allocateFlag.assign(numOfGroups,true);

               for (int i = 0; i < 8; ++i) {
                   for (int j = 0; j < 8; ++j) {
                       faultMatrix[i][j] = 0;
                   }
               }

                break;
            }

        }

    } 

    else if(heuristicAlgorithm==1){
        // Kuhn-Munkres (KM) weighted matching heuristic
        KM *heuristicAlgorithm_ = new KM;
        heuristicAlgorithm_->init(numOfGroups, numOfMEMS);
        heuristicAlgorithm_->setBufLen(trafficStatistics);
        int heuristicAlgorithmDiscount = 5 * 1e6 * (repetition + 1) / flitByteLength;
        linky = heuristicAlgorithm_->multicalculate(heuristicAlgorithmDiscount);
        trafficDemandSeq.clear();
        trafficStatistics.clear();
        trafficStatistics.resize(numOfGroups);
        for(int i=0;i<numOfGroups;i++){
            trafficStatistics[i].assign(numOfGroups,0);
        }
        synchronization();
    }

    else{
        // Edmonds maximum matching heuristic (default)
        Edmonds *heuristicAlgorithm_ = new Edmonds;
        heuristicAlgorithm_->init(numOfGroups, numOfMEMS);
        heuristicAlgorithm_->setBufLen(trafficStatistics);
        linky = heuristicAlgorithm_->multicalculate();
        showMEMSConfiguration();
        trafficDemandSeq.clear();
        trafficStatistics.clear();
        trafficStatistics.resize(numOfGroups);
        for(int i=0;i<numOfGroups;i++){
            trafficStatistics[i].assign(numOfGroups,0);
        }
        synchronization();
    }
}

// Push new MEMS configuration to switches with staged port enable/disable timing
void MEMS_Controller::synchronization(){
    showMEMSConfiguration();
    setupMsg *setup = new setupMsg("setup",SETUP_MSG);
    setup->setIni(false);
    for(int MEMS_Id=0;MEMS_Id<numOfMEMS;MEMS_Id++){
        if(allocateFlag[MEMS_Id]==false)
            continue;
        recordConfigurationTime[MEMS_Id] = simTime().dbl();
        auto it = gobalTopo.find(MEMS_Id);
        if(it!=gobalTopo.end()) 
            gobalTopo.erase(it);
        setup->setMEMSID(MEMS_Id);
        std::vector<int> temp(numOfGroups,-1);
        for(int GroupId=0;GroupId<numOfGroups;GroupId++){
            temp[GroupId] = linky[MEMS_Id][GroupId];
        }
        setup->setConfiguration(temp);

        sendDelayed(setup->dup(),T_nightInMEMS,"MEMS$o",MEMS_Id);
        setup->setFlag(false);
      
        for(int i=0;i<numOfGroups;i++){
          send(setup->dup(),"Pod$o",MEMS_Id%4 + i*4);
        }
       
    }
    setup->setFlag(true);
    for(int MEMS_Id=0;MEMS_Id<numOfMEMS;MEMS_Id++){
        if(allocateFlag[MEMS_Id]==false)
            continue;
        setup->setMEMSID(MEMS_Id);
        for(int i=0;i<numOfGroups;i++){

              sendDelayed(setup->dup(),T_night, "Pod$o", MEMS_Id % 4 + i * 4);
        }
    }
}

void MEMS_Controller::showMEMSConfiguration(){
    std::cout << "--------MEMS Configuration--------" << std::endl;
    for(unsigned int i=0;i<linky.size();i++){
        for(unsigned int j=0;j<linky[i].size();j++){
            std::cout << linky[i][j] << " ";
        }
        std::cout << std::endl;
    }
}


// Called when a spine switch reports its inter-group traffic demand
void MEMS_Controller::handleTrafficDemand(BufferLength *msg){
    trafficReply++;
    int srcGroup = msg->getSrcGroupIdx();
    std::vector<int> temp = msg->getBufLen();


    for(int i=0;i<numOfGroups;i++){

        trafficArray[srcGroup][i] = trafficArray[srcGroup][i] + temp[i];
        trafficStatistics[srcGroup][i] = trafficStatistics[srcGroup][i] + temp[i];

    }
    delete msg;

    if(trafficReply==gateSize("Pod")){
        showAndRecordTrafficMatrix();
        trafficReply = 0;
        numOfSamples--;
        if(numOfSamples==0){
            numOfSamples = par("numOfSamples");
            allocate();  // trigger MEMS reconfiguration when all samples collected
        }
    }
}

void MEMS_Controller::showAndRecordTrafficMatrix(){
    std::cout << "--------Traffic Demand--------" << std::endl;
    for(unsigned int i=0;i<trafficArray.size();i++){
        for(unsigned int j=0;j<trafficArray[i].size();j++){
            std::cout << trafficArray[i][j] << " ";
        }
        std::cout << std::endl;
 
        trafficDemandSeq.emplace_back(trafficArray[i]);
    }


    std::cout << "--------Traffic Demand2--------" << std::endl;
       for(unsigned int i=0;i<trafficStatistics.size();i++){
           for(unsigned int j=0;j<trafficStatistics[i].size();j++){
               std::cout << trafficStatistics[i][j] << " ";
               std::ofstream fout;
               fout.open("Traffic Demand" + std::to_string(repetition) + ".txt",std::ios::app);
              fout << trafficStatistics[i][j]<< endl;
              fout.close();
           }
           std::cout << std::endl;
           trafficDemandSeq.emplace_back(trafficStatistics[i]);
       }

    trafficArray.clear();
    trafficArray.resize(numOfGroups);
    for(int i=0;i<numOfGroups;i++){
        trafficArray[i].assign(numOfGroups,0);
    }
}

void MEMS_Controller::showLogicTopology(const std::vector<std::vector<int>>& logicTopo,const std::string &matrix_name){
    std::cout << "---------- show "<<matrix_name<<"----------" << std::endl;
    for(unsigned int i=0;i<logicTopo.size();i++){
        for(unsigned int j=0;j<logicTopo[i].size();j++){
            std::cout << logicTopo[i][j] << " ";
        }
        std::cout << std::endl;
    }
}



void MEMS_Controller::handleDRLResult(std::vector<std::vector<int>>& logicTopo){
    std::vector<std::vector<int>> recordIndirectPath(numOfGroups,std::vector<int>(numOfGroups,0));

    for(unsigned int i=0;i<logicTopo.size();i++){
        for(unsigned int j=0;j<logicTopo[i].size();j++){
            if(logicTopo[i][j]>0) recordIndirectPath[i][j] = INT_MAX;
        }
    }
    std::unordered_map<int,std::vector<int>> paths;
    for(int src=0;src<numOfGroups;src++){
        std::queue<int> q;
        for(int i=0;i<numOfGroups;i++){
            if(logicTopo[src][i]>0)
                q.push(i);
        }
        while(!q.empty()){
            int mid = q.front();
            q.pop();
            for(int i=0;i<numOfGroups;i++){
                if(logicTopo[mid][i]>0 && i!=src && recordIndirectPath[src][i]<INT_MAX && find(paths[src].begin(),paths[src].end(),i)==paths[src].end()){
                    paths[src].emplace_back(i);
                
                }
            }
        }
    }
    curFinish = false;
    recombineForTopo(logicTopo,paths,recordIndirectPath,0);
}


bool MEMS_Controller::ifKeep(int MEMSID,std::vector<std::vector<int>>& logicTopo){
    for(unsigned int i=0;i<linky[MEMSID].size();i++){
        if(linky[MEMSID][i]==-1 || logicTopo[i][linky[MEMSID][i]]<=0) return false;
    }
    return true;
}

bool MEMS_Controller::recombineFinish(const std::vector<std::vector<int>>& logicTopo){
    for(unsigned int i=0;i<logicTopo.size();i++){
        for(unsigned int j=0;j<logicTopo[i].size();j++){
            if(logicTopo[i][j]!=0)
                return false;
        }
    }
    return true;
}

// Backtracking search to assign remaining MEMS switches to valid topology links
void MEMS_Controller::recombineForTopo(std::vector<std::vector<int>>& logicTopo,std::unordered_map<int,std::vector<int>>& paths,std::vector<std::vector<int>>& recordIndirectPath,int MEMSID){

    if(MEMSID>=numOfMEMS-1){
        if(recombineFinish(logicTopo)){
        curFinish = true;
        serverClient *DRL= new serverClient;
   
        cout<<MEMSID<<endl;
        if(DRL->init(numOfGroups,numOfMEMS)){
            string socketMsg = DRL->sendLogicTopoToPython(linky);


            DRL->communication(socketMsg,socketPort_logic);

            nextHopTable = DRL->getNextHopTable();
            nexthopInfoTable = DRL->getNexthopInfoTable();

            }
        }
        return;
    }
    if(allocateFlag[MEMSID]==false){

        updateState(logicTopo,recordIndirectPath,MEMSID);
        recombineForTopo(logicTopo,paths,recordIndirectPath,MEMSID+1);
    }
    else{
        if(ifKeep(MEMSID,logicTopo)){
            cout<<"Need configuration"<<endl;
            allocateFlag[MEMSID] = false;

            updateState(logicTopo,recordIndirectPath,MEMSID);
            recombineForTopo(logicTopo,paths,recordIndirectPath,MEMSID+1);
            if(curFinish)
                return;
           
            backState(logicTopo,recordIndirectPath,MEMSID);
        }
        linky[MEMSID].assign(numOfGroups,-1);
        allocateFlag[MEMSID] = true;

        genTopo(logicTopo,paths,MEMSID,recordIndirectPath);
    }
}

// Check whether the remaining logic topology can form a valid connected matching
bool MEMS_Controller::canUse(std::vector<std::vector<int>>& tempTopo){
    if(recombineFinish(tempTopo))
        return true;
    std::vector<int> visited(numOfGroups - 1);
    std::iota(visited.begin(),visited.end(),1);
    std::queue<int> q;
    q.push(0);
    int num = 1;
    while(!q.empty() || visited.size()>0){
        if(q.empty()){
            if(num & 1) 
                return false;
            num = 1;
            q.push(visited[0]);
            visited.erase(visited.begin());
        }
        else{
            int cur = q.front();
            q.pop();
            for(int i=0;i<numOfGroups;i++){
                auto it = find(visited.begin(),visited.end(),i);
                if(it==visited.end() || tempTopo[cur][i]<=0) continue;
                q.push(i);
                visited.erase(it);
                num++;
            }
        }
    }
    if(num & 1)
        return false;
    return true;
}
// Recursive backtracking: assign one link for MEMSID in the logic topology
void MEMS_Controller::genTopo(std::vector<std::vector<int>>& logicTopo,std::unordered_map<int,std::vector<int>>& paths,int MEMSID,std::vector<std::vector<int>>& recordIndirectPath){
    if(find(linky[MEMSID].begin(),linky[MEMSID].end(),-1)==linky[MEMSID].end())
    {
        updateState(logicTopo,recordIndirectPath,MEMSID);
        if(canUse(logicTopo)){
            recombineForTopo(logicTopo,paths,recordIndirectPath,MEMSID+1);
        }
        if(!curFinish)
            backState(logicTopo,recordIndirectPath,MEMSID);
        return;
    }
    int anotherMEMS = (MEMSID >= (numOfMEMS / 2)) ? (MEMSID - (numOfMEMS / 2)) : (MEMSID + (numOfMEMS / 2));
    for(int i=0;i<numOfGroups;i++){
        if(linky[MEMSID][i]!=-1) continue;
        std::vector<int> validLinks;
      
        if(linky[anotherMEMS][i]!=-1){
            int indirectPathSrc = linky[anotherMEMS][i];
            for(unsigned int m=0;m<paths[indirectPathSrc].size();m++){
                int targetDst = paths[indirectPathSrc][m];
                if(logicTopo[i][targetDst]>0)
                    validLinks.emplace_back(targetDst);
            }
            sort(validLinks.begin(),validLinks.end(),[&](int a, int b){
                return recordIndirectPath[indirectPathSrc][a] < recordIndirectPath[indirectPathSrc][b];
            });
        }
        getValidLinks(validLinks,logicTopo,i);
        for(unsigned int m=0;m<validLinks.size();m++){
            if(linky[MEMSID][validLinks[m]]!=-1)
                continue;
   
            linky[MEMSID][i] = validLinks[m];
            linky[MEMSID][validLinks[m]] = i;
            genTopo(logicTopo,paths,MEMSID,recordIndirectPath);

            if(curFinish)
                return;
            linky[MEMSID][i] = -1;
            linky[MEMSID][validLinks[m]] = -1;
        }
    }
}


void MEMS_Controller::getValidLinks(std::vector<int>& validLinks,std::vector<std::vector<int>>& logicTopo,int groupId){
    for(int i=0;i<numOfGroups;i++){
        if(logicTopo[groupId][i]>0 && find(validLinks.begin(),validLinks.end(),i)==validLinks.end())
            validLinks.emplace_back(i);
    }
}





bool MEMS_Controller::checkDRLResult(const std::vector<std::vector<int>>& logicTopo){
    stack<int> record;
    std::vector<bool> flag(numOfGroups,false);
    record.push(0);
    flag[0] = true;
    while(!record.empty()){
        int curGroup = record.top();
        record.pop();
        for(int i=0;i<numOfGroups;i++){
            if(logicTopo[curGroup][i] > 0 && flag[i] == false){
                record.push(i);
                flag[i] = true;
            }
        }
    }

    std::vector<bool>::iterator it = find(flag.begin(),flag.end(),false);
    if(it==flag.end())
        return true;
    else{
        return false;
    }
}

double MEMS_Controller::calReward(bool flag,const std::vector<std::vector<int>>& temp){
    if(!flag)
        return -100;

    double cosNumerator = 0;
    double cosDenominator_1 = 0;
    double cosDenominator_2 = 0;
    for(unsigned int i=0;i<temp.size();i++){
        for(unsigned int j=0;j<temp[i].size();j++){
            cosNumerator = cosNumerator + temp[i][j] * trafficStatistics[i][j];
            cosDenominator_1 = cosDenominator_1 + temp[i][j] * temp[i][j];
            cosDenominator_2 = cosDenominator_2 + double(trafficStatistics[i][j]) * double(trafficStatistics[i][j]);
        }
    }
    double cosSimilarity = cosNumerator / (sqrt(cosDenominator_1 * cosDenominator_2)+1e-8);
    return cosSimilarity;
}


void MEMS_Controller::updateState(std::vector<std::vector<int>>& logicTopo,std::vector<std::vector<int>>& recordIndirectPath,int MEMSID){
    for(unsigned int j=0;j<linky[MEMSID].size();j++){
        logicTopo[j][linky[MEMSID][j]]--;
    }
    int anotherMEMS = (MEMSID >= (numOfMEMS / 2)) ? (MEMSID - (numOfMEMS / 2)) : (MEMSID + (numOfMEMS / 2));
    if(find(linky[anotherMEMS].begin(),linky[anotherMEMS].end(),-1)!=linky[anotherMEMS].end()) return;
    for(int src=0;src<numOfGroups;src++){
        int mid = linky[MEMSID][src];
        if(recordIndirectPath[src][linky[anotherMEMS][mid]]<INT_MAX){
            recordIndirectPath[src][linky[anotherMEMS][mid]]++;
        }
    }
}

void MEMS_Controller::backState(std::vector<std::vector<int>>& logicTopo,std::vector<std::vector<int>>& recordIndirectPath,int MEMSID){
    for(unsigned int j=0;j<linky[MEMSID].size();j++){
        logicTopo[j][linky[MEMSID][j]]++;
    }
    int anotherMEMS = (MEMSID >= (numOfMEMS / 2)) ? (MEMSID - (numOfMEMS / 2)) : (MEMSID + (numOfMEMS / 2));
    if(find(linky[anotherMEMS].begin(),linky[anotherMEMS].end(),-1)!=linky[anotherMEMS].end()) return;
    for(int src=0;src<numOfGroups;src++){
        int mid = linky[MEMSID][src];
        if(recordIndirectPath[src][linky[anotherMEMS][mid]]<INT_MAX){
            recordIndirectPath[src][linky[anotherMEMS][mid]]--;
        }
    }
}

bool MEMS_Controller::ifRecombineForMEMS(const std::vector<int>& portSend,int countTraffic){
    int sumOfFlow = std::accumulate(portSend.begin(),portSend.end(),0);
    return (sumOfFlow < (countTraffic / 10));
}

double MEMS_Controller::calReward_(const std::vector<int>& portSend,bool flag,int countTraffic,double timeStamp){
    int sumOfFlow = std::accumulate(portSend.begin(),portSend.end(),0);
    double reward = double(sumOfFlow / countTraffic) * 0.5 + (simTime().dbl() - timeStamp) * 0.5;
    return (flag ? reward : reward * 0.9);
}



void MEMS_Controller::finish()
{

}

}; // namespace
