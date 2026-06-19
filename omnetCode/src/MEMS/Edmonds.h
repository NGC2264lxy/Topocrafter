#ifndef EDMONDS_H
#define EDMONDS_H

// Edmonds blossom algorithm for maximum bipartite matching.
// Used for initial and heuristic MEMS topology allocation.

#include <vector>

class Edmonds {
public:
    void setBufLen(const std::vector<std::vector<int>>& buf_len);
    void setBufLen();  // fully connected mapping (no traffic info)
    void init(int _numOfRack, int _numOfMEMS);
    std::vector<int> calculate();
    std::vector<std::vector<int>> multicalculate();

private:
    int N;
    int numOfMEMS;
    std::vector<int> linkx;
    std::vector<int> linky;
    std::vector<bool> cover;
    std::vector<bool> judge;
    std::vector<bool> ly;
    std::vector<std::vector<int>> temp;       // per-MEMS matching result
    std::vector<std::vector<bool>> mapping;   // adjacency: eligible group pairs

protected:
    virtual bool Find(int x);  // DFS augmenting path for matching
};

#endif // EDMONDS_H
