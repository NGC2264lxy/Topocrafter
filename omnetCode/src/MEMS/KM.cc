#include "KM.h"
#include <algorithm>
#include <limits.h>

// Set weight matrix from buffer-length (traffic demand); diagonal set to -limited
void KM::setBufLen(const vector<vector<int>>& buf_len) {
    w = buf_len;
    for(unsigned int i=0;i<w.size();i++){
        for(unsigned int j=0;j<w[i].size();j++){
            limited = limited>w[i][j]?limited:w[i][j];
        }
    }

    for(unsigned int i=0;i<w.size();i++){
        w[i][i] = (-1)*limited;
    }
}

void KM::init(int _numOfRack, int _numOfMEMS)
{
    N = _numOfRack;
    nx = N;
    ny = N;
    numOfMEMS = _numOfMEMS;

    temp.resize(numOfMEMS);
    for (int i = 0; i < numOfMEMS; i++)
        temp[i].assign(N, -1);
}

bool KM::find(int x)
    visx[x] = true;
    for (int y = 0; y < ny; y++)
    {
        if (visy[y])
            continue;
        double t = lx[x] + ly[y] - w[x][y];

        if (t == 0)
        {
            visy[y] = true;
            if (linky[y] == -1 || find(linky[y]))
            {
                linky[y] = x;
                linkx[x] = y;
                return true;
            }

        }

        else if (slack[y] > t)
            slack[y] = t;

    }

    return false;

}

// Run KM algorithm once to find optimal group-to-group matching
vector<int> KM::calculate()
{
    linkx.assign(N, -1);
    linky.assign(N, -1);
    ly.assign(N, 0);
    lx.assign(N, -limited);
    for (int i = 0; i < nx; i++) 
    {
        for (int j = 0; j < ny; j++)
        {
            if (w[i][j] > lx[i])
                lx[i] = w[i][j];
        }
    }
  
    for (int x = 0; x < nx; x++)
    {
        slack.assign(ny, limited);
        while (1)
        {
            visx.assign(N, 0);
            visy.assign(N, 0);
            if (find(x))
                break;

            double d = limited;
           
            for (int i = 0; i < ny; i++) 
            {
                if (!visy[i] && d > slack[i])
                    d = slack[i];    //d= min{x[x]-ly[y]-w(x,y)}
            }
            for (int i = 0; i < nx; i++) 
            {
                if (visx[i])
                    lx[i] -= d;
            }
            for (int i = 0; i < ny; i++) 
            {
                if (visy[i])
                    ly[i] += d;
                else
                    slack[i] -= d;
            }
        }
    }
    // cout<<result<<endl;
    return linkx;

}
// Compute matching for each MEMS switch; reduce matched edge weights by discount
vector< vector<int> > KM::multicalculate(int discount)
{
    for (int i = 0; i < numOfMEMS; i++)
    {
        //   vector < vector<int> > set;
        calculate();

        temp[i] = linkx;
        for (int ToR_ID = 0; ToR_ID < N; ToR_ID++)
        {
            if (linkx[ToR_ID] > -1)
            {
                w[ToR_ID][linkx[ToR_ID]] = w[ToR_ID][linkx[ToR_ID]] - discount;
            }
        }
    }
    return temp;
}


