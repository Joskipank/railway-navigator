#include "graph.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

bool visited[100005];

// DFS для подграфа железной дороги
void dfs_rail(const Graph& g, int v, vector<int>& component){
    visited[v] = true;
    component.push_back(v);

    for(const Edge& e : g.adj[v]){ 
        if(e.mode == MODE_RAIL && !visited[e.to]){
            dfs_rail(g, e.to, component);
        }
    }
}

// поиск компонент связности для железной дороги
vector<vector<int>> railway_components(const Graph& g){
    memset(visited,false,sizeof(visited));
    vector<vector<int>> components;

    for(int v = 1; v <= g.n; ++v){
        if(!visited[v]){
            vector<int> component;
            dfs_rail(g,v,component);

            if(!component.empty()){
                sort(component.begin(),component.end());
                components.push_back(component);
            }
        }
    }
    return components;
}

// выделение изолированных зон
vector<vector<int>> isolated_zones(vector<vector<int>>& components){
    if(components.empty()) return {};

    sort(components.begin(),components.end(),
         [](const vector<int>& a,const vector<int>& b){ return a.size() > b.size(); });

    vector<vector<int>> isolated;
    for(size_t i = 1; i < components.size(); ++i){
        isolated.push_back(components[i]);
    }
    return isolated;
}
