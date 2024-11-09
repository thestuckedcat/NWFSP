#pragma once
#include "Parameter.h"
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include "Bi-population.h"

struct Task {
	PARAMETERS::Params param;
	std::string algorithmName;

	
	int run_id;

	Task() {}
	Task(PARAMETERS::Params p, const std::string& s, int id) {
		param = p;
		algorithmName = s;
		run_id = id;
	}
};





class TaskQueue {
	std::queue<Task> tasks;					// Job queue
	std::vector<std::thread> threads;		// worker threads
	
	std::condition_variable completion_condition;// wating job to complete
	std::condition_variable condition_;		// for worker thread, when new job comes in, they will be waken
	std::mutex mutex_;						// for queue and active_threads


	std::atomic<int> active_threads{ 0 };	// current running thread number
	bool stop_flag;							// if all threads should be exit


	void run_task(const Task& task) {
		Bipopulation_memetic<NWFSP_Solution, Population> bipopulation(task.param, task.algorithmName + "_instance_" + std::to_string(task.run_id));
		bipopulation.run();
	}


	void worker() {
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

				if (tasks.empty() && active_threads == 0) {
					completion_condition.notify_all();
				}
			}
			
		}
	}
public:
	TaskQueue(int num_threads) {
		// create thread pool

		for (int i = 0; i < num_threads; i++) {
			threads.emplace_back([this]() {this->worker(); });
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

	void assign_tasks(PARAMETERS::Params param, const std::string& str, int run_count) {
		for (int i = 0; i < run_count; i++) {
			Task task(param, str, i + 1);
			task_queue.add_task(task);
		}
	}

};