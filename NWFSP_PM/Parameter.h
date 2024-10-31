#pragma once
#include <stdexcept>
#include <vector>
#include <iostream>
#include "myutils.h"
#include <algorithm>
#include <numeric>
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
		}
		Params() {
			alpha = 0.1;
			gamma = 0.9;
			beta = 0.5;

			selection_rate = 0.5;

			max_gen = 2000;
			job_num = 20;
			machine_num = 5;
			pop_size = 100;
			scenario_num = 20;
			Threshold = 0;
		}

		// scenario related

		void load_scenario(std::vector<std::vector<std::vector<int>>>& sc) {
			scenario = sc;
			generate_threshold();
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
			generate_threshold();
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

			Threshold = (int)(0.37 * (float)totalSum / scenario_num);
			std::cout << "Current threshold is " <<  Threshold << std::endl;
			return;

		}



	};
}