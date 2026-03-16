/*
Задача дискретной оптимизации

Автор: Баканов И. П. ФКИ 202
Версия от: 14.03.26
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <ctime>


struct Point {
    double x, y;
};


// Используем евклидову метрику
double dist(std::vector<Point> coords, int i, int j) {
        return std::sqrt(std::pow(coords[i].x - coords[j].x, 2) + 
                         std::pow(coords[i].y - coords[j].y, 2));
}


class Task {
public:
    virtual std::vector<Point> get_coords() const = 0;

    virtual double estimate(const std::vector<int>& solution) const = 0;
    
    virtual std::vector<int> get_greedy_start() const = 0;
    
    virtual std::vector<int> get_random_start() = 0;
    
    virtual bool load_data(const std::string& filename) = 0;
    
    virtual ~Task() = default;
};


// Задача коммивояжера
class TSP : public Task {
private:
    int n;
    std::vector<Point> coords;

public:
    std::vector<Point> get_coords() const override {
        return coords;
    }

    bool load_data(const std::string& filename) override {
	    n = 0;

    	std::ifstream input(filename);
    
    	if (!input.is_open()) {
            std::cerr << "File not open!" << std::endl;
            return false;
    	}

    	// Чтение числа вершин в графе n
    	input >> n;
        
        std::cout << "Number of vertices: " << n << std::endl;
        
        coords.resize(n);
        
        for (int i = 0; i < n; ++i) {
            input >> coords[i].x >> coords[i].y;
        }
        
        return true;
    }

    // Вычисляем суммарный вес маршрута
    double estimate(const std::vector<int>& solution) const override {
        double total = 0;
        
        for (int i = 0; i < n; ++i) {
            total += dist(coords, solution[i], solution[(i + 1) % n]);
        }
        
        return total;
    }
    
    // Эвристика: жадный старт
    // P. S. Эксперименты показали, что он работает хуже, чем random_start
    std::vector<int> get_greedy_start() const override {
        int n = coords.size();
        
        std::vector<int> path;
        
        std::vector<bool> visited(n, false);
    
        // Начинаем с 0-го города
        int current = 0;
        
        path.push_back(current);
        
        visited[current] = true;
    
        for (int i = 1; i < n; ++i) {
            int next_city = -1;
            
            double min_dist = 1e18;
        
            for (int next = 0; next < n; ++next) {
                if (!visited[next]) {
                    double d = dist(coords, current, next);
                    
                    if (d < min_dist) {
                        min_dist = d;
                        next_city = next;
                    }
                }
            }
        
            path.push_back(next_city);
            
            visited[next_city] = true;
            
            current = next_city;
        }
        
        return path;
    }
    
    std::vector<int> get_random_start() override {
        std::vector<int> path(n);

        // Заполняем массив path возрастающими числами - индексами городов
        std::iota(path.begin(), path.end(), 0);

        // Для перемешивания массива используем генератор std::mt19937 
        std::shuffle(path.begin(), path.end(), std::mt19937{std::random_device{}()});

        return path;

    }
};


// Решатель - локальный поиск + отжиг
class Solver {
public:
    std::vector<int> solve(Task& task) {
        // Инициализируем генератор случайных чисел
        srand(42);
    
        std::vector<int> path = task.get_random_start();
        
        double current_sum = task.estimate(path);
        
        int n = path.size();
        
        std::vector<Point> coords = task.get_coords();
        
        // Параметры отжига
        double T = 100.0;
        double cooling_rate = 0.99;
        int iterations = 1000;
        
        for (int iter = 0; iter < iterations; ++iter) {
            current_sum = task.estimate(path);
            
            int i = rand() % n;
            int j = rand() % n;
            if (i > j) std::swap(i, j);
            if (i == j) continue;

            int i_prev = (i - 1 + n) % n;
            int j_next = (j + 1) % n;

            double old_edges = dist(coords, path[i_prev], path[i]) + dist(coords, path[j], path[j_next]);
            double new_edges = dist(coords, path[i_prev], path[j]) + dist(coords, path[i], path[j_next]);
            
            double delta = new_edges - old_edges;

            // Имитация отжига
            if (delta < 0 || (std::exp(-delta / T) > (double)rand() / RAND_MAX)) {
                std::reverse(path.begin() + i, path.begin() + j + 1);
            }
            
            // Уменьшаем температуру
            T *= cooling_rate;
            if (T < 0.1) T = 0.1;
        }
        
        return path;
    }
};

int main() {
    TSP task;
    Solver solver;

    if (!task.load_data("TSP_test_200.txt")) {
        std::cerr << "Ошибка при загрузке файла" << std::endl;
        return 1;
    }

    std::vector<int> best_path = solver.solve(task);

    double total_weight = task.estimate(best_path);

    std::cout << "\n========================================" << std::endl;
    std::cout << "SOLUTION" << std::endl;
    std::cout << "Path length: " << total_weight << std::endl;
    std::cout << "Path order:" << std::endl;
    
    for (size_t i = 0; i < best_path.size(); ++i) {
        std::cout << best_path[i];
        
        if (i < best_path.size() - 1) std::cout << " -> ";
    }
    
    std::cout << " -> " << best_path[0];
    
    std::cout << "\n========================================" << std::endl;

    return 0;
}