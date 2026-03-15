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

    std::vector<int> get_random_start() override {
        std::vector<int> path(n);
        
        // Заполняем массив path возрастающими числами - индексами городов
        std::iota(path.begin(), path.end(), 0);
        
        // Для перемешивания массива используем генератор std::mt19937 
        std::shuffle(path.begin(), path.end(), std::mt19937{std::random_device{}()});
        
        return path;
    }
};


// Класс решателя (локальный поиск)
class Solver {
public:
    std::vector<int> solve(Task& task) {
        std::vector<int> path = task.get_random_start();
        
        double current_sum = task.estimate(path);
        
        int n = path.size();
        
        std::vector<Point> coords = task.get_coords();
        
        bool improved = true;

        // Ищем улучшающих соседей при помощи удаления-добавления двух реб
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                    
                int i_prev = (i - 1 + n) % n;
                int j_next = (j + 1) % n;

                double old_edges = dist(coords, path[i_prev], path[i]) + dist(coords, path[j], path[j_next]);
                double new_edges = dist(coords, path[i_prev], path[j]) + dist(coords, path[i], path[j_next]);
        
                if (new_edges < old_edges) {
                    std::reverse(path.begin() + i, path.begin() + j + 1);
            
                    current_sum -= (old_edges - new_edges);
                }
            }
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