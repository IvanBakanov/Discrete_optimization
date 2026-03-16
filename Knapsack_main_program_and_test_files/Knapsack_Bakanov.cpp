/*
Задача дискретной оптимизации

Автор: Баканов И. П. ФКИ 202
Версия от: 15.03.26
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <string>
#include <numeric>


struct Item {
    int value;
    int weight;
};


class Task {
public:
    virtual double estimate(const std::vector<int>& solution) const = 0;
    
    virtual std::vector<int> get_greedy_start() = 0;
    
    virtual std::vector<int> get_random_start() = 0;
    
    virtual bool load_data(const std::string& filename) = 0;
    
    virtual ~Task() = default;
};


class Knapsack : public Task {
private:
    int n;
    int capacity;
    std::vector<Item> items;

public:
    bool load_data(const std::string& filename) override {
        std::ifstream input(filename);
        
        if (!input.is_open()) return false;

        input >> n >> capacity;
        
        items.resize(n);
        
        for (int i = 0; i < n; ++i) {
            input >> items[i].value >> items[i].weight;
        }
        
        return true;
    }

    double estimate(const std::vector<int>& solution) const override {
        int total_weight = 0;
        
        int total_value = 0;
        
        for (int i = 0; i < n; ++i) {
            if (solution[i] == 1) {
                total_weight += items[i].weight;
                total_value += items[i].value;
            }
        }

        return (total_weight <= capacity) ? total_value : 0;
    }

    // Эвристика - жадный старт
    std::vector<int> get_greedy_start() override {
        std::vector<int> solution(n, 0);
    
        std::vector<int> indices(n);
        
        std::iota(indices.begin(), indices.end(), 0);
    
        std::sort(indices.begin(), indices.end(), [&](int a, int b) {
            double ratio_a = (double)items[a].value / items[a].weight;
            double ratio_b = (double)items[b].value / items[b].weight;
            return ratio_a > ratio_b;
        });
    
        int current_weight = 0;
        
        for (int idx : indices) {
            if (current_weight + items[idx].weight <= capacity) {
                solution[idx] = 1;
                current_weight += items[idx].weight;
            }
        }
    
        return solution;
    }
    
    std::vector<int> get_random_start() override {
        // Случайный бинарный вектор
        std::vector<int> sol(n);
        
        std::mt19937 gen{std::random_device{}()};
        
        std::uniform_int_distribution<> dist(0, 1);
        
        for(int i = 0; i < n; ++i) sol[i] = dist(gen);
        
        return sol;
    }
};


// Решатель задачи - локальный поиск + жадный старт + множественные запуски + отжиг
class Solver {
public:
    std::vector<int> solve(Task& task) {
        std::vector<int> current_sol = task.get_greedy_start();
        double current_val = task.estimate(current_sol);

        std::vector<int> best_solution = current_sol;
        double best_val = current_val;

        double T = 100.0;
        double cooling_rate = 0.99;

        while (T > 0.01) {
            for (int attempt = 0; attempt < 50; ++attempt) {
                // Выбираем случайный предмет
                size_t i = rand() % current_sol.size();
            
                // Инвертируем бит
                current_sol[i] = 1 - current_sol[i];
                
                double new_val = task.estimate(current_sol);
                
                double delta = new_val - current_val;

                // Имитация отжига
                if (delta > 0 || (std::exp(delta / T) > (double)rand() / RAND_MAX)) {
                    current_val = new_val;
                    
                    if (current_val > best_val) {
                        best_val = current_val;
                        best_solution = current_sol;
                    }
                } else {
                    // Возвращаем все как было
                    current_sol[i] = 1 - current_sol[i];
                }
            }
            T *= cooling_rate; // Остывание
        }
        return best_solution;
    }
};


int main() {
    Knapsack task;
    Solver solver;

    if (!task.load_data("KS_test_100.txt")) {
        std::cerr << "Ошибка при загрузке файла" << std::endl;
        return 1;
    }

    std::vector<int> solution = solver.solve(task);

    double total_value = task.estimate(solution);

    std::cout << "\n========================================" << std::endl;
    std::cout << "SOLUTION" << std::endl;
    std::cout << "Total value: " << total_value << std::endl;
    std::cout << "Knapsack content:" << std::endl;

    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            std::cout << "Item " << i << " is taken" << std::endl;
        }
    }
    std::cout << "========================================" << std::endl;

    return 0;
}