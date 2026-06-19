#include "Edmonds.h"

// Build adjacency from traffic demand: edge exists if buf_len[i][j] > 0
void Edmonds::setBufLen(const std::vector<std::vector<int>>& buf_len) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j)
                mapping[i][j] = false;
            else {
                if (buf_len[i][j] > 0)
                    mapping[i][j] = true;
                else
                    mapping[i][j] = false;
            }
        }
    }
}

// Build fully connected adjacency (all off-diagonal pairs eligible)
void Edmonds::setBufLen() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j)
                mapping[i][j] = false;
            else
                mapping[i][j] = true;
        }
    }
}

void Edmonds::init(int _numOfRack, int _numOfMEMS) {
    N = _numOfRack;
    numOfMEMS = _numOfMEMS;
    temp.resize(numOfMEMS);
    for (int i = 0; i < numOfMEMS; i++)
        temp[i].resize(N);
    cover.resize(N);
    mapping.resize(N);
    for (int i = 0; i < N; i++)
        mapping[i].resize(N);
}

bool Edmonds::Find(int i) {
    for (int j = 0; j < N; ++j) {
        if (mapping[i][j] && !cover[j]) {
            cover[j] = true;
            if (linky[j] == -1 || Find(linky[j])) {
                linkx[i] = j;
                linkx[j] = i;
                linky[i] = j;
                linky[j] = i;
                cover[i] = true;
                return true;
            }
        }
    }
    return false;
}

std::vector<int> Edmonds::calculate() {
    linkx.assign(N, -1);
    linky.assign(N, -1);
    cover.assign(N, 0);
    for (int i = 0; i < N; ++i) {
        if (cover[i]) continue;
        Find(i);
    }
    return linkx;
}

// Compute matching for each MEMS; remove used edges and resolve conflicts
std::vector<std::vector<int>> Edmonds::multicalculate() {
    for (int i = 0; i < numOfMEMS; i++) {
        calculate();
        for (int j = 0; j < N; j++) {
            if (linkx[j] > -1) {
                if (mapping[j][linkx[j]] == true)
                    mapping[j][linkx[j]] = false;
                mapping[linkx[j]][j] = false;
            }
        }
        temp[i] = linkx;
    }
    for (int j = 0; j < N; j++) {
        judge.assign(N, 0);
        for (int i = 0; i < numOfMEMS; i++) {
            if (temp[i][j] > -1) {
                if (judge[temp[i][j]] == 1 || j == temp[i][j]) {
                    temp[i][j] = -1;
                } else {
                    judge[temp[i][j]] = 1;
                }
            }
        }
    }
    return temp;
}
