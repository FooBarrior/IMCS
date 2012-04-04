#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <iostream>

using namespace std;

#define VI vector<int>
#define VS vector<string>
#define MII map<int, int>
#define MSI map<string, int>
#define MIS map<int, string>
#define MSS map<string, string>

#define IT(T) T::iterator
#define MIII IT(MII)
#define MSII IT(MSI)
#define MISI IT(MIS)
#define MSSI IT(MSS)

#define N 100

int n;
int s[N], h = 0;
int bests[N], besth = 0;
bool adj[N][N];

void clique(int v){
    if(v > n){
        if(h > besth){
            besth = h;
            for(int i = 0; i < h; i++) bests[i] = s[i];
        }
        return;
    }
    bool can = true;
    for(int i = 0; i < h; i++) can = can && adj[v][s[i]];
    if(can){
        s[h++] = v;
        clique(v+1);
        h--;
    }
    clique(v+1);
}

int main(){
//    freopen("f.in", "r", stdin);
//    freopen("f.out", "w", stdout);
    int m;
    cin >> n >> m;
    for(int i = 0; i < m; i++){
        int a, b;
        cin >> a >> b;
        adj[a][b] = adj[b][a] = true;
    }
    clique(1);
    for(int i = 0; i < besth; i++) cout << bests[i] << endl;
    return 0;
}

