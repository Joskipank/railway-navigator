#include <iostream>
#include <vector>
using namespace std;
// Глобальные переменные
vector<vector<int>> DMas; //Двухмерный массив
vector<bool> Flag;    // Помечать остановки на которых были

// Функция поиска в глубину (DFS)
void dfs(int a) {
    Flag[a] = true; // Помечаем посещенную остановку
    
// Проходим по всем остановкам
for (int sosed : DMas[a]) {
    if (!Flag[sosed]) {
        dfs(sosed);
        }
    }
}

int main() {
    cout << "Введите кол-во остановок и маршрутов: " << endl;
    int b, c;
    cin >> b >> c;
    // Инициализируем граф
    DMas.resize(b); //Меняем размер вектора
    Flag.assign(b, false); //Добавляет нов. знач. в вектор и меняет его размер

    cout << "Введите маршруты, пары остановок от 0 до " << b-1 << ":" << endl;
    for (int i = 0; i < c; ++i) {
        int d, e;
        cin >> d >> e;
        // Дороги туда и обратно
        DMas[d].push_back(d);
        DMas[e].push_back(e);
    }

    int score = 0; // Счетчик компонент связности

    // Проходим по всем остановкам (основной)
    for (int i = 0; i < b; ++i) {
        if (!Flag[i]) {
            // Если есть не посещенная остановка, то создается новая сеть
            score++;
            dfs(i); // Запускаем DFS, чтобы пометить всю сеть
        }
    }

    cout << "Количество изолированных автобусных сетей: " << score << endl;

    return 0;
}
