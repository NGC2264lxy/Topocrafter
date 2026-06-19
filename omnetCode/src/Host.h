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

#ifndef __DRAGONFLYPLUS_HOST_H
#define __DRAGONFLYPLUS_HOST_H

// Host module: generates application flows, injects flits into the network,
// and collects end-to-end delay / FCT statistics.

#include <omnetpp.h>
#include <vector>
#include <map>
#include <fstream>
#include "packet/Packet_m.h"
#include <string>
#include <numeric>

using namespace omnetpp;
using namespace std;
long long numFlitSend;
double numFlitReceive;
double numMsgReceive; 
long long numFlitGen;
namespace dragonflyplus {

// Shared traffic matrix: trafficTable[timeSlot][src][dst] = remaining demand
extern std::vector<std::vector<std::vector<int>>> trafficTable;
extern bool isTrafficTableLoaded;

    class Host : public cSimpleModule
    {
      protected:
        int repetition;           // experiment run index (used in output file names)
        int selflid;              // global logical ID of this host
        int msgId;                // monotonically increasing application message ID
        int numOfNode;
        int numOfSwitch;
        int numOfGroups;
        cMessage* sendSignal;     // periodic timer for flow generation
        double messageinterval_us;
        std::vector<appMsg*> appMsgQueue;  // pending app-level messages awaiting flitization
        cMessage *popMsg;         // timer to send the next flit after link transmission
        int flitSn;               // current flit sequence number within a message
        int flitByteLength;
        int msgLength;
        int averagePathLength;    // running average path length reported to controller
        int numPodmsg;
        int totalPodLength;
        bool bigOrSmall;          // enable bimodal message size distribution
        double allocateInterval;
        cMessage *clearPathLength; // periodic timer to report average path length
        std::map<std::pair<int,int>,int> record; // (srcLid, msgId) -> remaining flits for FCT tracking
        
        std::string FilePath;
        simsignal_t flitEndToEndDelaySignal;

    //////////////////////////////////////////////////////////////
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        void loadTrafficTable(const std::string& filePath, int numberOfNodes);
        virtual void finish();
    //////////////////////////////////////////////////////////////
        ~Host();
        void sendDataMsg();
        dataMsg* genDataMsg(appMsg* msg);
        appMsg *genFlow(int src,int dst);
        void handleDataMsg(dataMsg* msg);
    };

}; // namespace

#endif
