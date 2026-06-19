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

package dragonflyplus.MEMS;

//
// TODO auto-generated type
//
simple MEMS_Controller
{
    parameters:
        int numOfGroups;
        int numOfMEMS;
        int gobalId;
        int numOfSamples;
        int socketPort;
        int socketPort_logic;
        int msgLength;
        int gobalLinks;
        int switchInPod;
        int repetition;
        int stepLength;
        int flitByteLength;
        
        int heuristicAlgorithm;
        bool rTopo;
        double allocateInterval;
        
        double messageinterval_us;
        double T_night;
        double T_nightInMEMS;
    gates:
        inout Pod[];
        inout MEMS[];
}
