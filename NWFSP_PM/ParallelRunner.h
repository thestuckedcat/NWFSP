#pragma once
#include "Parameter.h"
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include "Bi-population.h"
#include<Windows.h>
#include "myutils.h"
#include "Bi-population-v3.h"
#include "Bi-population-v4.h"
#include "Compare_PIG.h"
#include "Compare_CMAQ.h"
struct Task {
	PARAMETERS::Params param;
	std::string algorithmName;
	std::string algorithm;
	
	int run_id;

	Task() {}
	Task(PARAMETERS::Params p, const std::string s, int id, std::string alg) {
		param = p;
		algorithmName = s;
		run_id = id;
		algorithm = alg;
	}
};





class TaskQueue {
	std::queue<Task> tasks;					// Job queue
	std::vector<std::thread> threads;		// worker threads
	
	std::condition_variable completion_condition;// wating job to complete
	std::condition_variable condition_;		// for worker thread, when new job comes in, they will be waken
	std::mutex mutex_;						// for queue and active_threads


	std::atomic<int> active_threads{ 0 };	// current running thread number
	bool stop_flag{ false };							// if all threads should be exit
	std::atomic<int> completed_tasks{ 0 };
	int total_tasks{ 0 };

	void run_task(const Task& task) {
		if (task.algorithm == "SQMA" || task.algorithm == "SUMA")
		{
			Bipopulation_memetic_version3<NWFSP_Solution, Population> bipopulation(task.param, task.algorithmName + "_instance_" + std::to_string(task.run_id));
			bipopulation.run();
		}

		else if (task.algorithm == "GAVNS") {
			Bipopulation_memetic_version4<NWFSP_Solution, Population> bipopulation(task.param, task.algorithmName + "_instance_" + std::to_string(task.run_id));
			bipopulation.run();
		}
		else if (task.algorithm == "PIG") {
			PIG<NWFSP_Solution, Population> pig(task.param, task.algorithmName + "_instance_" + std::to_string(task.run_id));
			pig.run();
		}
		else if (task.algorithm == "CMAQ") {
			CMAQ<NWFSP_Solution, Population> cmaq(task.param, task.algorithmName + "_instance_" + std::to_string(task.run_id));
			cmaq.run();
		}
		else	
		{
			std::cout << "No such algorithm" << std::endl;
		}
	}


	void worker(int core_id) {
		//set_thread_priority();
		while (true) {
			Task task;

			{
				std::unique_lock<std::mutex> lock(mutex_);// get lock, if others is handling it ,block this thread
				// manually stopped OR has job to do
				condition_.wait(lock, [this]() {return stop_flag || !tasks.empty(); });

				if (stop_flag && tasks.empty()) {
					return;
				}


				// assign job
				task = tasks.front();
				tasks.pop();
				++active_threads;
			}

			run_task(task);

			{
				std::unique_lock<std::mutex> lock(mutex_);

				--active_threads;
				++completed_tasks;
				std::cout << "\rProgress: " << completed_tasks << "/" << total_tasks << " tasks completed." << std::flush;
				if (tasks.empty() && active_threads == 0) {
					completion_condition.notify_all();
				}
			}
			
		}
	}
public:
	TaskQueue() {
		// create thread pool

		/*for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
			threads.emplace_back([this,i]() {this->worker(i); });
		}*/
		for (int i = 0; i < 5; i++) {
			threads.emplace_back([this, i]() {this->worker(i); });
		}
	}


	~TaskQueue() {
		{
			std::unique_lock<std::mutex> lock(mutex_);
			stop_flag = true;
		}
		condition_.notify_all();

		for (std::thread& thread : threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	}


	void add_task(const Task& task) {
		{
			std::unique_lock<std::mutex> lock(mutex_);
			tasks.push(task);
			total_tasks++; // Increase total task count
		}

		condition_.notify_one();
	}


	void wait_for_completion() {
		std::unique_lock<std::mutex> lock(mutex_);

		completion_condition.wait(lock, [this]() {return tasks.empty() && !active_threads; });
	}




};





class TaskAssigner {
	TaskQueue& task_queue;
public:
	// bind
	TaskAssigner(TaskQueue& task_queue) :task_queue(task_queue) {};

	void assign_tasks(PARAMETERS::Params param, std::string str, int run_count, std::string algorithm) {
		str = algorithm + "_" + str;
		for (int i = 0; i < run_count; i++) {
			Task task(param, str, i + 1, algorithm);
			task_queue.add_task(task);
		}
	}

};