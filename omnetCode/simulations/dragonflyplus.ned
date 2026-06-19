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

package dragonflyplus.simulations;

import dragonflyplus.Switch;
import dragonflyplus.Host;
import dragonflyplus.MEMS.MEMS_Controller;
import dragonflyplus.MEMS.MEMS_Switch;

//
// Two instances (tic and toc) of Txc connected.
//
network dragonflyplus
{
     parameters:
        int switchInPod;
        int numOfGroups;
        int numOfSwitch;
        int numOfMEMS;
        int numOfNode;
        int numOfPortsInSpine;
        int numOfPortsInLeaf;
        int controlPort;
        volatile int ODataRate @unit(Gbps);
        volatile int EDataRate @unit(Gbps);
        @display("bgb=605,379");
    types:
        channel OC extends ned.DatarateChannel
        {
            parameters:
                datarate = default(ODataRate);
        }
        channel EC extends ned.DatarateChannel
        {
            parameters:
                datarate = default(EDataRate);
        }
    submodules:
        spineSwitch[numOfSwitch]: Switch {
            parameters:
                isleafswitch = false;
                gobalId = index;
                numOfPorts = numOfPortsInSpine;
                @display("p=111,114");
            gates:
                portx[numOfPortsInSpine];
        }
        leafSwitch[numOfSwitch]: Switch {
            parameters:
                isleafswitch = true;
                gobalId = index + numOfSwitch;
                numOfPorts = numOfPortsInLeaf;
                @display("p=233,202");
            gates:
                portx[numOfPortsInLeaf];
        }
        host[numOfNode]: Host {
            parameters:
                gobalId = index + numOfSwitch * 2;
                numOfPorts = 1;
                @display("p=233,322");
        }
        openflow_switch[numOfMEMS]: MEMS_Switch {
            parameters:
                numOfGroups = numOfGroups;
                gobalId = index + numOfSwitch * 2 + numOfNode;
                @display("p=476,153");
            gates:
                MEMS_Controller;
                Pod[numOfGroups];
        }
        openflow_controller: MEMS_Controller {
            parameters:
                numOfGroups = numOfGroups;
                gobalId = numOfSwitch * 2 + numOfNode + numOfMEMS;
                @display("p=402,39");
            gates:
                Pod[numOfSwitch];
                MEMS[numOfMEMS];
        }
    connections:
        for i=0..numOfGroups-1 {
            host[8*i+0].injectionPort <--> EC <--> leafSwitch[4*i+0].portx[4];
            host[8*i+1].injectionPort <--> EC <--> leafSwitch[4*i+0].portx[5];

            host[8*i+2].injectionPort <--> EC <--> leafSwitch[4*i+1].portx[4];
            host[8*i+3].injectionPort <--> EC <--> leafSwitch[4*i+1].portx[5];

            host[8*i+4].injectionPort <--> EC <--> leafSwitch[4*i+2].portx[4];
            host[8*i+5].injectionPort <--> EC <--> leafSwitch[4*i+2].portx[5];

            host[8*i+6].injectionPort <--> EC <--> leafSwitch[4*i+3].portx[4];
            host[8*i+7].injectionPort <--> EC <--> leafSwitch[4*i+3].portx[5];
        }

        for i=0..numOfGroups-1 {
            leafSwitch[4*i+0].portx[0] <--> EC <--> spineSwitch[4*i+0].portx[0];
            leafSwitch[4*i+0].portx[1] <--> EC <--> spineSwitch[4*i+1].portx[0];
            leafSwitch[4*i+0].portx[2] <--> EC <--> spineSwitch[4*i+2].portx[0];
            leafSwitch[4*i+0].portx[3] <--> EC <--> spineSwitch[4*i+3].portx[0];

            leafSwitch[4*i+1].portx[0] <--> EC <--> spineSwitch[4*i+0].portx[1];
            leafSwitch[4*i+1].portx[1] <--> EC <--> spineSwitch[4*i+1].portx[1];
            leafSwitch[4*i+1].portx[2] <--> EC <--> spineSwitch[4*i+2].portx[1];
            leafSwitch[4*i+1].portx[3] <--> EC <--> spineSwitch[4*i+3].portx[1];

            leafSwitch[4*i+2].portx[0] <--> EC <--> spineSwitch[4*i+0].portx[2];
            leafSwitch[4*i+2].portx[1] <--> EC <--> spineSwitch[4*i+1].portx[2];
            leafSwitch[4*i+2].portx[2] <--> EC <--> spineSwitch[4*i+2].portx[2];
            leafSwitch[4*i+2].portx[3] <--> EC <--> spineSwitch[4*i+3].portx[2];

            leafSwitch[4*i+3].portx[0] <--> EC <--> spineSwitch[4*i+0].portx[3];
            leafSwitch[4*i+3].portx[1] <--> EC <--> spineSwitch[4*i+1].portx[3];
            leafSwitch[4*i+3].portx[2] <--> EC <--> spineSwitch[4*i+2].portx[3];
            leafSwitch[4*i+3].portx[3] <--> EC <--> spineSwitch[4*i+3].portx[3];
        }

        for i=0..numOfGroups-1, for j=0..3 {
            spineSwitch[i*4+j].portx$o[5] --> OC --> openflow_switch[j].Pod$i[i];
            openflow_switch[j].Pod$o[i] --> spineSwitch[i*4+j].portx$i[5];

            spineSwitch[i*4+j].portx$o[6] --> OC --> openflow_switch[j+4].Pod$i[i];
            openflow_switch[j+4].Pod$o[i] --> spineSwitch[i*4+j].portx$i[6];
        }

        for i=0..numOfMEMS-1 {
            openflow_controller.MEMS[i] <--> EC <--> openflow_switch[i].MEMS_Controller;
        }
        for i=0..numOfSwitch-1 {
            spineSwitch[i].portx[controlPort] <--> EC <--> openflow_controller.Pod[i];
        }
}