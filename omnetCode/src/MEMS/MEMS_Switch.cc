// MEMS optical switch: port mapping, forwarding, and fault simulation.

#include "MEMS_Switch.h"
#include <cstdlib>  // std::rand, std::srand
#include <ctime>    // std::time
#include <cmath>    // std::log, std::sqrt, std::cos
#include <algorithm>  // std::max
#include <iostream>  

namespace dragonflyplus {
std::vector<std::vector<int>> faultMatrix;

Define_Module(MEMS_Switch);

    void MEMS_Switch::initialize()
    {
       
        selfid = par("gobalId");
        numOfGate = gateSize("Pod");
        numOfNode = par("numOfNode");
        numOfSwitch = par("numOfSwitch");
        outputGate.assign(numOfGate, -1);
        portSend.assign(numOfGate, 0);
        pod_traffic.resize(numOfGate);
        for (int i = 0; i < numOfGate; i++) {
            pod_traffic[i].assign(numOfGate, 0);
        }


        portActive.assign(numOfGate, true); 
        blinkInterval = par("blinkInterval");        
        blinkDuration = par("blinkDuration");         
        blinkDistribution = par("blinkDistribution");

        blinkMessage = new cMessage("blink");
       
        faultMatrix.resize(8, std::vector<int>(8, 0));
      
    }

    void MEMS_Switch::handleMessage(cMessage *msg)
    {
        if (msg == blinkMessage) {
            // Alternate between fault injection and port recovery phases
            bool hasInactive = std::any_of(portActive.begin(), portActive.end(), [](bool active)
                                           { return !active; });

            if (hasInactive) {
              
                togglePortState(true);
                scheduleAt(simTime() + blinkDuration, blinkMessage); 
            } else {
              
                togglePortState(false);
                scheduleAt(simTime() + blinkInterval, blinkMessage); 
            }
           return;
        }

        if (msg->arrivedOn("MEMS_Controller$i")) {
            // Apply new port mapping from MEMS controller
            setupMsg *pkt = dynamic_cast<setupMsg *>(msg);
            std::vector<int> temp = pkt->getConfiguration();
            for (int i = 0; i < numOfGate; i++) {
                outputGate[i] = temp[i];
            }
            delete pkt;
        } else {
            // Forward data flit from input port to configured output port
            int inGateIndex = msg->getArrivalGate()->getIndex();
            portSend[inGateIndex]++;
          
            if (!portActive[inGateIndex]) {
                // Drop flit if input port is faulted
                dataMsg *pkt = dynamic_cast<dataMsg *>(msg);

                delete msg;  
                return;
            }

            int outGateIndex = outputGate[inGateIndex];
            pod_traffic[inGateIndex][outGateIndex]++;
            dataMsg *pkt = dynamic_cast<dataMsg *>(msg);
            int length = pkt->getPathLength();
            pkt->setPathLength(length + 1);
            if (outGateIndex < 0 || outGateIndex >= numOfGate)
                error("Error in MEMS!!! Cannot send this msg %s", msg->getName());
            send(msg, "Pod$o", outGateIndex);
        }
    }

    // isRecovering=true: restore all ports; false: randomly fault one port
    void MEMS_Switch::togglePortState(bool isRecovering)
    {
        if (isRecovering) {
           
               for (int i = 0; i < numOfGate; ++i) {
                   portActive[i] = true;
               }
              
               return;
           }


        if ((std::rand() % 100) >= 30) {  
        }

        int portIndex = std::rand() % numOfGate;
        int memsId = selfid - numOfSwitch * 2 - numOfNode;

        portActive[portIndex] = false;  
        faultMatrix[memsId][portIndex]++; 

        
    }


    double MEMS_Switch::getNextBlinkInterval()
    {
        if (blinkDistribution ==1) {
            double lambda = 1.0 / blinkInterval;  
            double randomU = static_cast<double>(std::rand()) / RAND_MAX; 
            return -std::log(1.0 - randomU) / lambda;
        } else if (blinkDistribution == 2) {
            double min = 0.5 * blinkInterval;
            double max = 1.5 * blinkInterval;
            return min + static_cast<double>(std::rand()) / RAND_MAX * (max - min);
        } else if (blinkDistribution ==3) {
           
            double u1 = static_cast<double>(std::rand()) / RAND_MAX;
            double u2 = static_cast<double>(std::rand()) / RAND_MAX;
            double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
            double mean = blinkInterval;
            double stddev = 0.1 * blinkInterval;
            return std::max(0.0, mean + z * stddev); 
        } else {
            error("Unsupported blink distribution" );

        }
    }

    double MEMS_Switch::getNextBlinkDuration()
    {
        if (blinkDistribution == 1) {
            double lambda = 1.0 / blinkDuration;  
            double randomU = static_cast<double>(std::rand()) / RAND_MAX; 
            return -std::log(1.0 - randomU) / lambda;
        } else if (blinkDistribution == 2) {
            double min = 0.5 * blinkDuration;
            double max = 1.5 * blinkDuration;
            return min + static_cast<double>(std::rand()) / RAND_MAX * (max - min);
        } else if (blinkDistribution == 3) {
            
            double u1 = static_cast<double>(std::rand()) / RAND_MAX;
            double u2 = static_cast<double>(std::rand()) / RAND_MAX;
            double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
            double mean = blinkDuration;
            double stddev = 0.1 * blinkDuration;
            return std::max(0.0, mean + z * stddev); 
        } else {
            error("Unsupported blink distribution" );
        }
    }

    void MEMS_Switch::resetPortSend()
    {
        portSend.assign(numOfGate,0);
    }
    void MEMS_Switch::resetPortTraffic()
    {
        for(int i=0;i<numOfGate;i++){
            pod_traffic[i].assign(numOfGate,0);
            }

    }
    std::vector<int> MEMS_Switch::getPortSend()
    {
        return portSend;
    }

    std::vector<std::vector<int>> MEMS_Switch::getPodTraffic()
    {
        return pod_traffic;
    }

    void MEMS_Switch::finish()
    {
        std::string baseName = "PortSend_";

            for(unsigned int i=0;i<portSend.size();i++){
                std::string temp = baseName + std::to_string(i);
                recordScalar(temp.c_str(),portSend[i]);
            }
    }

};  // namespace dragonflyplus
