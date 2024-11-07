#pragma once
#include <stdexcept>
#include <vector>
#include <iostream>
#include "myutils.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <algorithm>



namespace PARAMETERS {
	// 使用scenario初始化，获得各项参数
	struct Params {

		// for Q-table
		float alpha;
		float gamma;
		// selection rate
		float selection_rate;
		// trace decay rate
		float theta;

		/*
			Population partition rate based on BSN

			BSN < beta * scenario_num的交给UN
			BSN >= beta * scenario_num的交给LN
		*/
		float beta;

		int max_gen;
		int job_num;
		int machine_num;
		int pop_size;
		int Threshold;
		int scenario_num;
		std::vector<std::vector<std::vector<int>>> scenario;
		bool use_dynamic_beta;
		Params(float _selection_rate, float _alpha, float _gamma, float _theta, float _beta,
			int _max_gen, int _job_num, int _machine_num, int _pop_size, int _scenario_num)
			:alpha{ _alpha }, gamma{ _gamma }, theta{ _theta }, beta{ _beta } {
			selection_rate = _selection_rate;
			max_gen = _max_gen;
			job_num = _job_num;
			machine_num = _machine_num;
			pop_size = _pop_size;
			scenario_num = _scenario_num;
			Threshold = 0;
			use_dynamic_beta = false;
		}
		Params() {
			alpha = 0.1;
			gamma = 0.9;
			beta = 0.5;

			selection_rate = 0.5;

			max_gen = 10000;
			job_num = 20;
			machine_num = 5;
			pop_size = 10;
			scenario_num = 20;
			Threshold = 0;
			use_dynamic_beta = false;
		}

		// scenario related

		void load_scenario(std::vector<std::vector<std::vector<int>>>& sc) {
			scenario = sc;
			Threshold = calculateThreshold();
			std::cout << "Current threshold is " << Threshold << std::endl;
		}

		void generate_scenario() {
			if (!scenario.empty()) {
				std::cout << "Scenario Already " << std::endl;
				PRINT_CURRENT_LOCATION();
				return;
			}
			scenario = std::vector<std::vector<std::vector<int>>>(scenario_num,
				std::vector<std::vector<int>>(job_num,
					std::vector<int>(machine_num)));

			for (int i = 0; i < scenario_num; i++) {
				for (int j = 0; j < job_num; j++) {
					for (int m = 0; m < machine_num; m++) {
						scenario[i][j][m] = generate_random_int(10, 100);
					}
				}
			}
			//if (job_num <= 20) generate_threshold();
			//else
			{
				Threshold = calculateThreshold();
				std::cout << "Current threshold is " << Threshold << std::endl;
			}
			saveParamToFile();
			return;
		}


	private:
		// threshold related
		void generate_threshold() {
			if (scenario.empty()) {
				std::cout << "Scenario not exist" << std::endl;
				PRINT_CURRENT_LOCATION();
				return;
			}

			if (Threshold) {
				std::cout << "Threshold already exist" << std::endl;
				PRINT_CURRENT_LOCATION();
				return;
			}

			int totalSum = 0;

			for (const auto& i : scenario) {
				for (const auto& job : i) {
					totalSum += std::accumulate(job.begin(), job.end(), 0);
				}
			}
			std::cout << "total sum is " << totalSum << std::endl;
			Threshold = (int)(0.37 *  totalSum / scenario_num);
			std::cout << "Current threshold is " <<  Threshold << std::endl;
			return;

		}

		// 生成随机序列的辅助函数
		std::vector<int> generateRandomSequence(int job_num) {
			std::vector<int> sequence(job_num);
			std::iota(sequence.begin(), sequence.end(), 0);  // 生成 [0, 1, 2, ..., job_num-1]
			std::shuffle(sequence.begin(), sequence.end(), std::mt19937{ std::random_device{}() });
			return sequence;
		}

		int calculateMakespan(std::vector<std::vector<int>>& processing_time, std::vector<int>& seq) {
			std::vector<std::vector<node>> sequence_info_(machine_num, std::vector<node>(job_num));


			// Calculate the completion times for the first job in the sequence
			int this_job = seq[0];
			sequence_info_[0][0] = node(0, processing_time[this_job][0], this_job);
			for (int j = 1; j < machine_num; ++j) {
				sequence_info_[j][0] = node(sequence_info_[j - 1][0].end, processing_time[this_job][j], this_job);

			}
			//for (int j = 0; j < num_machines; j++) {
			//	std::cout << sequence_info_[j][0].begin << " " << sequence_info_[j][0].end << std::endl;;
			//}



			// Calculate the completion times for the rest of the jobs
			for (int j = 1; j < job_num; ++j) {
				int job = seq[j];
				sequence_info_[0][j] = node(sequence_info_[0][j - 1].end, processing_time[job][0], job);

				// 应该向后推的距离
				int max_push = 0;
				// Initial
				for (int m = 1; m < machine_num; ++m) {
					// no-wait constraint
					sequence_info_[m][j] = node(sequence_info_[m - 1][j].end, processing_time[job][m], job);


					// flowshop constraint
					int need_push = sequence_info_[m][j - 1].end - sequence_info_[m][j].begin;
					if (need_push > max_push) {
						max_push = need_push;
					}
				}
				// flowshop constraint
				if (max_push)
					for (int m = 0; m < machine_num; m++) {
						sequence_info_[m][j].update(max_push);
					}
			}

			return sequence_info_[machine_num-1][job_num-1].end;
		}

		// 计算 threshold 的主函数
		int calculateThreshold() {

			long long makespan_sum = 0.0;
			int sequence_count = 10;

			// 生成5个随机序列并计算每个序列在每个场景的makespan
			for (int i = 0; i < sequence_count; ++i) {
				std::vector<int> sequence = generateRandomSequence(job_num);

				for (int j = 0; j < scenario_num; j++) {
					int makespan = calculateMakespan(scenario[j], sequence);
					makespan_sum += makespan;
				}
			}

			// 计算 makespan 的平均值，并乘以 90% 得到 threshold
			double average_makespan = makespan_sum / (scenario_num * sequence_count);
			return average_makespan * 0.965;
		}





		/*DEBUGGING TOOL*/
		template<typename T>
		void writeToFile(std::ofstream& file, const std::string& name, const T& value) {
			file << name << "=" << value << std::endl;
		}

		void saveParamToFile() {
			std::string filePath = "../data/paramInfo.txt";
			std::ofstream file(filePath);

			if (!file.is_open()) {
				std::cerr << "Error opening file: " << filePath << std::endl;
				return;
			}

			// 将每个成员变量写入文件
			writeToFile(file, "alpha", alpha);
			writeToFile(file, "gamma", gamma);
			writeToFile(file, "selection_rate", selection_rate);
			writeToFile(file, "max_gen", max_gen);
			writeToFile(file, "job_num", job_num);
			writeToFile(file, "machine_num", machine_num);
			writeToFile(file, "pop_size", pop_size);
			writeToFile(file, "Threshold", Threshold);
			writeToFile(file, "scenario_num", scenario_num);


			file.close();
			std::cout << "Param information saved to " << filePath << std::endl;
		}

	};
}