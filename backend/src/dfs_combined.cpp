#include <vector>
#include <algorithm>
#include <cstring>

#include "graph.hpp"
#include "dfs_bus.cpp"
#include "dfs_metro.cpp"
#include "dfs_railway.cpp"

using namespace std;


// Объединённый DFS через существующие DFS по типам
void dfs_combined(const Graph& g, int v, vector<int>& used, vector<int>& component)
{
    if (used[v])
        return;

    used[v] = true;
    component.push_back(v);

    // DFS по автобусам
    dfs_bus(g, v, used);

    // DFS по метро
    dfs_metro(g, v, used);

    // DFS по железной дороге
    dfs_rail(g, v, used);

    for (int i = 0; i < g.n; ++i)
    {
        if (used[i] &&
            find(component.begin(), component.end(), i) == component.end())
        {
            dfs_combined(g, i, used, component);
        }
    }
}

// Поиск компонент связности всего графа
vector<vector<int>> find_components(const Graph& g)
{
    vector<int> used(g.n, false);
    vector<vector<int>> components;

    for (int v = 0; v < g.n; ++v)
    {
        if (!used[v])
        {
            vector<int> component;
            dfs_combined(g, v, used, component);
            components.push_back(component);
        }
    }

    return components;
}

// Сортировка и вывод компонент
void print_sorted_zones(const Graph& g) {
    // Получаем компоненты
    vector<vector<int>> comps = find_components(g);
    int zones = comps.size();
    
    if (zones == 0) {
        printf("Нет изолированных зон\n");
        return;
    }
    
    // Массив для сортировки
    int* sizes = new int[zones];
    int** zones_data = new int*[zones];
    
    for (int i = 0; i < zones; i++) {
        sizes[i] = comps[i].size();
        zones_data[i] = new int[sizes[i]];
        for (int j = 0; j < sizes[i]; j++) {
            zones_data[i][j] = comps[i][j];
        }
    }
    
    // Сортировка пузырьком по убыванию размера
    for (int i = 0; i < zones - 1; i++) {
        for (int j = 0; j < zones - i - 1; j++) {
            if (sizes[j] < sizes[j + 1]) {
                // Меняем местами
                int temp_size = sizes[j];
                sizes[j] = sizes[j + 1];
                sizes[j + 1] = temp_size;
                
                int* temp_ptr = zones_data[j];
                zones_data[j] = zones_data[j + 1];
                zones_data[j + 1] = temp_ptr;
            }
        }
    }
    
    // Вывод заголовка
    printf("\nИЗОЛИРОВАННЫЕ ЗОНЫ (отсортировано по размеру):\n");
    printf("Всего зон: %d\n", zones);
    printf("-----------------------------\n");
    
    // Вывод результатов
    for (int i = 0; i < zones; i++) {
        printf("%d. %d станций: ", i + 1, sizes[i]);
        
        // Показываем первые 5 станций
        int show = sizes[i] > 5 ? 5 : sizes[i];
        for (int j = 0; j < show; j++) {
            printf("%d", zones_data[i][j]);
            if (j < show - 1) printf(", ");
        }
        
        if (sizes[i] > 5) printf(", ...");
        printf("\n");
    }
    
    // Статистика
    int total = 0;
    for (int i = 0; i < zones; i++) total += sizes[i];
    printf("\nСтатистика:\n");
    printf("- Охвачено станций: %d из %d\n", total, g.n);
    printf("- Самая большая зона: %d станций\n", sizes[0]);
    printf("- Самая маленькая: %d станций\n", sizes[zones - 1]);
    
    // Очистка
    for (int i = 0; i < zones; i++) {
        delete[] zones_data[i];
    }
    delete[] zones_data;
    delete[] sizes;
}

// Быстрый вызов
void show_zones(const Graph& g) {
    vector<vector<int>> comps = find_components(g);
    
    // Сортировка исходного вектора
    for (int i = 0; i < comps.size(); i++) {
        for (int j = i + 1; j < comps.size(); j++) {
            if (comps[i].size() < comps[j].size()) {
                vector<int> temp = comps[i];
                comps[i] = comps[j];
                comps[j] = temp;
            }
        }
    }
    
    printf("\nЗоны (по размеру):\n");
    for (int i = 0; i < comps.size(); i++) {
        printf("%d. %d ст.: [", i + 1, comps[i].size());
        for (int j = 0; j < comps[i].size(); j++) {
            printf("%d", comps[i][j]);
            if (j < comps[i].size() - 1) printf(" ");
        }
        printf("]\n");
    }
}
