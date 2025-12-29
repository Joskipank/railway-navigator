#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

struct roads {
    int to;
    int mode; // 0-метро 1-автобус 2-железная дорога
};

int N, M;
vector<roads> graph[100005]; // смежности
bool visited[100005];

// dfs для железной дороги
void DFS(int v, vector<int>& component) {
    visited[v] = true;
    component.push_back(v);

    for (roads e : graph[v]) {
        if (e.mode == 2) { // только железная дорога
            if (!visited[e.to]) {
                DFS(e.to, component);
            }
        }
    }
}

// поиск компонент связности железгной дороги
vector<vector<int>> find_railway_components() {
    memset(visited, false, sizeof(visited));

    vector<vector<int>> components;

    for (int v = 1; v <= N; v++) {
        if (!visited[v]) {
            vector<int> component;
            DFS(v, component);

            if (!component.empty()) {
                sort(component.begin(), component.end()); // по возрастанию
                components.push_back(component);
            }
        }
    }
    return components;
}

// выделение изолированных зон
vector<vector<int>> get_isolated_zones(vector<vector<int>>& components) {
    if (components.empty()) return {};

    sort(components.begin(), components.end(),
         [](const vector<int>& a, const vector<int>& b) {
             return a.size() > b.size(); // по убыванию размера
         });

    vector<vector<int>> isolated;
    for (int i = 1; i < (int)components.size(); i++) {
        isolated.push_back(components[i]);
    }

    return isolated;
}


