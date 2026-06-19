#ifndef KM_H_INCLUDED
#define KM_H_INCLUDED

// Kuhn-Munkres (Hungarian) algorithm for weighted bipartite matching.
// Used to allocate MEMS optical links based on traffic demand weights.

#include <iostream>
#include <vector>
using namespace std;

class KM {
public:
    KM() {};
    ~KM() {};
    void setBufLen(const vector<vector<int>>& buf_len);
    void init(int _numOfRack, int _numOfMEMS);
    vector<int> calculate();
    vector<vector<int>> multicalculate(int discount);


private:
    int N;                  // number of groups (racks)
    int numOfMEMS;
    double limited;         // max traffic weight (used as slack bound)

    vector<vector<int>> w;  // weight matrix: inter-group traffic demand
    vector<double> lx, ly;  // dual variables for KM algorithm
    vector<int> linky, linkx;  // matching: linkx[group] = peer group
    vector<int> visx, visy;
    vector<double> slack;
    vector<int> judge;
    vector<vector<int>> temp;  // result: temp[memsId] = matching for one MEMS

    vector<vector<vector<int>>> Dtraffic;

    int nx, ny;

protected:
    virtual bool find(int x);  // DFS augmenting path in equality subgraph
};

#endif // KM_H_INCLUDED
