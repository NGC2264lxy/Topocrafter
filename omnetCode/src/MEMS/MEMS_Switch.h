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

#ifndef __DRAGONFLYPLUS_MEMS_SWITCH_H
#define __DRAGONFLYPLUS_MEMS_SWITCH_H

// MEMS optical circuit switch: forwards flits according to configured
// port-to-port mapping and simulates port fault/blink behavior.

#include <omnetpp.h>
#include <vector>
#include "../packet/Packet_m.h"
#include <algorithm> 
using namespace omnetpp;
using namespace std;

namespace dragonflyplus {

extern std::vector<std::vector<int>> faultMatrix;  // faultMatrix[memsId][portIdx] = fault count
class MEMS_Switch : public cSimpleModule
{
  public:
    void resetPortSend();
    void resetPortTraffic();
    std::vector<int> getPortSend();
    std::vector<std::vector<int>> getPodTraffic();


  protected:
    int numOfGroups;
    int numOfGate;
    int numOfMEMS;
    int selfid;
    int numOfNode;
    int numOfSwitch;
    std::vector<int> outputGate;           // input port -> output port mapping
    std::vector<int> portSend;             // per-port flit send counters
    std::vector<std::vector<int>> pod_traffic;  // traffic matrix between pod ports

        std::vector<bool> portActive;      // false when port is in fault/blink state
        cMessage *blinkMessage;
        double blinkInterval;              // mean interval between fault events
        double blinkDuration;              // duration of port recovery
        int blinkDistribution;             // 1=exponential, 2=uniform, 3=normal




////////////////////////////////////////////////////////////////////////
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    double getNextBlinkInterval();
    double getNextBlinkDuration();
////////////////////////////////////////////////////////////////////////

  
        void togglePortState(bool isRecovring);        
};

}; // namespace

#endif
