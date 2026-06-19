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

// Host module implementation: flow generation, flit injection, and FCT measurement.

#include "Host.h"
#include<string>
#include <numeric>

simtime_t lastTime;  // timestamp of the most recently received flit (recorded at finish)

namespace dragonflyplus {

std::vector<std::vector<std::vector<int>>> trafficTable;
bool isTrafficTableLoaded = false;
Define_Module(Host);
extern std::vector<int> totalPathLength;  // per-host average path length, indexed by host ID

    void Host::initialize()
    {
        flitEndToEndDelaySignal = registerSignal("flitEndToEndDelay");
        msgId = 0;
        flitSn = 0;

    
        numFlitSend = 0;
        numFlitReceive = 0;
        numFlitGen = 0;
        numMsgReceive = 0;

        averagePathLength = 0;
        numPodmsg = 0;
        totalPodLength = 0;

        repetition = par("repetition");
        selflid = par("gobalId");
        numOfNode = par("numOfNode");
        numOfSwitch = par("numOfSwitch");
        numOfGroups = par("numOfGroups");
        sendSignal = new cMessage("sendSignal");
        popMsg = new cMessage("popMsg");
        clearPathLength = new cMessage("clearPathLength");
        flitByteLength = par("flitByteLength");
        messageinterval_us = par("messageinterval_us");
        bigOrSmall = par("bigOrSmall");
        msgLength = par("msgLength");
        allocateInterval = par("allocateInterval");
        scheduleAt(simTime(),sendSignal);
        scheduleAt(simTime(),popMsg);
        scheduleAt(simTime(),clearPathLength);

        std::string FilePath = par("FilePath");
        cout << "Traffic file path: " << FilePath << std::endl; 
        if (!isTrafficTableLoaded) {
            loadTrafficTable(FilePath,numOfNode);
            isTrafficTableLoaded = true;

        }

    }
    // Load a CSV traffic matrix file. Each time slice contains numberOfNodes rows;
    // each row lists comma-separated demand values to every destination host.
    void Host::loadTrafficTable(const std::string& filePath, int numberOfNodes) {
        std::ifstream inFile(filePath);
   
        cout<<"check"<<endl;
        if (!inFile) {
            std::cerr << "Error while opening input file (File not found or incorrect type)" << std::endl;
            return;  
        }
        std::string line;
        int currentTime = 0; 
        int currentRow = 0;  

   
        trafficTable.emplace_back(numberOfNodes, std::vector<int>(numberOfNodes, 0));

        while (std::getline(inFile, line)) {
            std::stringstream ss(line);
            std::string token;
  
            int src = currentRow; 
            int dst = 0;           

            while (std::getline(ss, token, ',')) {
                int traffic = std::stoi(token);
          
                trafficTable[currentTime][src][dst] = traffic;
                dst++;
            }
            currentRow++;
           
            if (currentRow == numberOfNodes) {
                currentRow = 0;
                currentTime++;
  
                trafficTable.emplace_back(numberOfNodes, std::vector<int>(numberOfNodes, 0));
            }
        }
            inFile.close();
        cout << "Traffic table loaded successfully from " << filePath << "." << endl;
    }

    void Host::handleMessage(cMessage *msg)
    {
        if(msg->isSelfMessage()){
            if(msg==sendSignal){
                // flowPattern: 1=uniform random, 2=intra/inter-pod, 3=trace-driven, 4=sequential ring
                int mode = par("flowPattern");
                switch(mode)

                {
                    case 1:  // uniform random destination (excluding self)
                    {
                        int des = intuniform(0,numOfNode-1) + numOfSwitch * 2;
                        while(des == selflid)
                            des = intuniform(0,numOfNode-1) + numOfSwitch * 2;
                        appMsg *p_new = genFlow(selflid,des);
                        appMsgQueue.emplace_back(p_new);
                        if (!popMsg->isScheduled())
                            sendDataMsg();
                        break;
                    }
                    case 2:  // 50% intra-pod, 50% inter-pod (next pod)
                    {
                        
                        int des;
                        int nodeInPod = numOfNode / numOfGroups;
                        if(uniform(0,1)<=0.5){
                        des = (((selflid - numOfSwitch * 2) / nodeInPod ) * nodeInPod + intuniform(0,nodeInPod-1)) % numOfNode + numOfSwitch * 2;
                        while(des == selflid)
                        des = (((selflid - numOfSwitch * 2) / nodeInPod ) * nodeInPod + intuniform(0,nodeInPod-1)) % numOfNode + numOfSwitch * 2;
                        }

                        else{
                            des = (((selflid - numOfSwitch * 2) / nodeInPod + 1 ) * nodeInPod + intuniform(0,nodeInPod-1)) % numOfNode + numOfSwitch * 2;
                        }
                        appMsg *p_new = genFlow(selflid,des);
                        appMsgQueue.emplace_back(p_new);
                        if (!popMsg->isScheduled())
                            sendDataMsg();
                        break;
                    }

                    case 3:  // trace-driven: pick destination from loaded traffic table
                    {

                        int des;
                        bool found = false;

                        for (int time = 0; time < trafficTable.size();time++) {

                            bool allZero = true;
                            int row_sum=std::accumulate(trafficTable[time][selflid-numOfNode].begin(),trafficTable[time][selflid-numOfNode].end(),0);

                            if(row_sum>0){
                                allZero = false;
                            }
                            if (!allZero) {


                                des = intuniform(0, numOfNode - 1) + numOfSwitch * 2;

                                while (des == selflid || trafficTable[time][selflid-numOfNode][des-numOfNode] <= 0) {
                                    des = intuniform(0, numOfNode - 1) + numOfSwitch * 2;
                                }
         
                                trafficTable[time][selflid-numOfNode][des-numOfNode]--;

                                appMsg *p_new = genFlow(selflid, des);
                                appMsgQueue.emplace_back(p_new);
                                if (!popMsg->isScheduled()) {
                                    sendDataMsg();
                                }
                                cout<<"Time"<<time<<endl;
                                found = true;
                                break; 
                            }
                        }

                        break;
                    }
                    case 4:  // sequential ring: each host sends to the next logical ID
                    {
                        int des = 0;
                        if (selflid==127)
                            {
                                des=64;
                            }

                        else{
                            des=selflid+1;

                        }
                        appMsg *p_new = genFlow(selflid,des);
                        appMsgQueue.emplace_back(p_new);
                        if (!popMsg->isScheduled())
                            sendDataMsg();
                        break;
                    }
                }
                scheduleAt(simTime() + messageinterval_us * 1e-6,sendSignal);

            }
            else if(msg==popMsg){
                // Send the next flit once the injection link becomes free
                cancelEvent(popMsg);
                sendDataMsg();
            }
            else if(msg==clearPathLength){
                // Periodically aggregate path-length samples and notify MEMS controller
                if (numPodmsg == 0){
                    averagePathLength = 0;
                   }
                else
                   {
                    //cout<<"totalPodLength"<<totalPodLength<<endl;
                    averagePathLength = totalPodLength / numPodmsg;
                   // cout<<"averagePathLength:"<<averagePathLength<<endl;
                    pathLength *pkt = new pathLength("pathlength", PATHLEN_MSG);
                    pkt->setRecPathLength(averagePathLength);
                    totalPathLength[selflid-numOfNode] = averagePathLength;
                   }
                numPodmsg = 0;
                totalPodLength = 0;
                scheduleAt(simTime() + allocateInterval,clearPathLength);
            }
            ///////////////////////////////////////////////////
            else{
                error("receive unknown selfMsg in %s!!",getFullPath().c_str());
            }
        }
        else{
            short kind = msg->getKind();
            if(kind==APP_MSG){
                error("receive appMsg in %s",getFullPath().c_str());
            }
            else if(kind==DATA_MSG){
                numFlitReceive++;
                lastTime = simTime();
                handleDataMsg(dynamic_cast<dataMsg*>(msg));
            }
            else{
                error("receive unknown msg!!");
            }
        }
    }

    // Create an application-level message; optionally apply bimodal size scaling
    appMsg *Host::genFlow(int src,int dst){
        char name[128];
        sprintf(name, "app-%s-%d", getFullPath().c_str() , msgId);
        appMsg *msg = new appMsg(name,APP_MSG);
        msg->setTimestamp(simTime());
        msg->setSrcLid(src);
        msg->setDstLid(dst);
        msg->setMsgId(msgId);
        int bytesLength = msgLength;
        if(bigOrSmall){
            int rankNum = uniform(0,100);
            if(rankNum>19)
                bytesLength = bytesLength * 0.0625;
            else{
                bytesLength = bytesLength * 4.75;
            }
        }
        int numOfFlit = bytesLength / flitByteLength;
        msg->setFlitLength(numOfFlit);
        numFlitGen = numFlitGen + numOfFlit;

        msgId++;
        return msg;
    }

    // Dequeue one flit from the head app message and inject it into the network
    void Host::sendDataMsg(){
        
        if(popMsg->isScheduled())
            return;
        if(appMsgQueue.size()!=0){
            dataMsg *curDataMsg= genDataMsg(appMsgQueue[0]);
            if(flitSn==appMsgQueue[0]->getFlitLength()){
                flitSn = 0;
                cancelAndDelete(appMsgQueue[0]);
                appMsgQueue.erase(appMsgQueue.begin());
            }
            send(curDataMsg, "injectionPort$o");
            numFlitSend++;
            scheduleAt(gate("injectionPort$o")->getTransmissionChannel()->getTransmissionFinishTime(),popMsg);

        }
    }

    dataMsg* Host::genDataMsg(appMsg* msg){
        char name[128];
        sprintf(name, "data-%d-%d-%d-%d",selflid,msg->getDstLid(),msg->getMsgId(),flitSn);
        dataMsg *data_msg = new dataMsg(name,DATA_MSG);
        data_msg->setTimestamp(simTime());
        data_msg->setSrcLid(selflid);
        data_msg->setDstLid(msg->getDstLid());
        data_msg->setMsgId(msgId);
        data_msg->setByteLength(flitByteLength);
        data_msg->setBitLength(flitByteLength*8);
        data_msg->setFlitSn(flitSn);
        data_msg->setFlitLength(msg->getFlitLength());
        data_msg->setTimestamp(msg->getTimestamp());
        data_msg->setCurTime(simTime());
        flitSn++;
        return data_msg;
    }

    // Process an incoming data flit: track delay, path length, and message completion (FCT)
    void Host::handleDataMsg(dataMsg* msg){
        std::pair<int,int> recordMsg;
        recordMsg.first = msg->getSrcLid();
        recordMsg.second = msg->getMsgId();
        std::map<std::pair<int,int>,int>::iterator it = record.find(recordMsg);
      
        simtime_t flitDelay = simTime() - msg->getTimestamp();
     
        int length = msg->getPathLength();
        if (length != 0)
        {
            numPodmsg++;
            totalPodLength = totalPodLength + length;
           
        }
        emit(flitEndToEndDelaySignal,flitDelay);
        if(it==record.end()){
            // First flit of this message: initialize remaining-flit counter
            record[recordMsg] = msg->getFlitLength() - 1;
            
        }
        else{
            record[recordMsg]--;
            if(record[recordMsg]<=3){
            // Approximate P99 FCT: record delay when last few flits arrive
            simtime_t P99FCT = simTime() - msg->getTimestamp();
                std::ofstream fout;
                fout.open("P99FCT" + std::to_string(repetition) + ".txt",std::ios::app);
                fout << P99FCT.dbl() << endl;
                fout.close();

           }
             if(record[recordMsg]<=0){
                simtime_t FCT = simTime() - msg->getTimestamp();
                std::ofstream fout;
                fout.open("FCT" + std::to_string(repetition) + ".txt",std::ios::app);
                fout << FCT.dbl() << endl;
                fout.close();
                record.erase(recordMsg);
                numMsgReceive++;
            }
        }
        delete msg;
    }
    void Host::finish()
    {
        recordScalar("lastTime",lastTime);
        recordScalar("ALL_FLIT_SEND",numFlitSend);
        recordScalar("ALL_FLIT_GEN",numFlitGen);
        recordScalar("ALL_FLIT_RECEIVE",numFlitReceive);
        recordScalar("ALL_MSG_RECEIVE",numMsgReceive);


    }

    Host::~Host() {
        if(sendSignal) cancelAndDelete(sendSignal);
        if(popMsg) cancelAndDelete(popMsg);
        if(appMsgQueue.size()!=0){
            for(unsigned int i=0;i<appMsgQueue.size();i++){
                cancelAndDelete(appMsgQueue[i]);
            }
            appMsgQueue.clear();
        }
    }
}; // namespace
