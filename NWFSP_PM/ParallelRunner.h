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

// ��װһ�����ڶ��߳�ִ�е���
template <typename SolutionType, typename PopulationType>
class ParallelRunner {
public:
    ParallelRunner(PARAMETERS::Params param, const std::string& name, int run_count = 20)
        : param_(param), name_(name), run_count_(run_count) {}

    // ����������У�ÿ�����е��� `run()` ����
    void run() {
        threads_.emplace_back([this]() { this->run_multiple_times(); });
    }

    // �ȴ������߳����
    static void join_all() {
        for (std::thread& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
        threads_.clear();
    }

private:
    // ����ÿ���߳����еĺ�����ִ�� `run_count_` �� `run()` ����
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

// ���徲̬��Ա
template <typename SolutionType, typename PopulationType>
std::vector<std::thread> ParallelRunner<SolutionType, PopulationType>::threads_;
