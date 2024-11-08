#pragma once
#pragma once

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include "solution.h"
#include "Bi-population.h"
#include "Parameter.h"

// 封装一个用于多线程执行的类
template <typename SolutionType, typename PopulationType>
class ParallelRunner {
public:
    ParallelRunner(PARAMETERS::Params param, const std::string& name, int run_count = 20)
        : param_(param), name_(name), run_count_(run_count) {}

    // 启动多次运行，每次运行调用 `run()` 方法
    void run() {
        threads_.emplace_back([this]() { this->run_multiple_times(); });
    }

    // 等待所有线程完成
    static void join_all() {
        for (std::thread& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
        threads_.clear();
    }

private:
    // 定义每个线程运行的函数，执行 `run_count_` 次 `run()` 方法
    void run_multiple_times() {
        Bipopulation_memetic<SolutionType, PopulationType> bipopulation(param_, name_);
        for (int i = 0; i < run_count_; ++i) {
            std::cout << "Running " << name_ << " iteration " << i + 1 << std::endl;
            bipopulation.run();
        }
    }

    PARAMETERS::Params param_;
    std::string name_;
    int run_count_;
    static std::vector<std::thread> threads_;
};

// 定义静态成员
template <typename SolutionType, typename PopulationType>
std::vector<std::thread> ParallelRunner<SolutionType, PopulationType>::threads_;

