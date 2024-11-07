#pragma once
#include"Parameter.h"
#include<cassert>
#include "initialization.h"
#include "Global_Search.h"
#include "myutils.h"
#include "Local_search.h"
#include <string>
#include<chrono>
template<typename Solution_Type, template<typename> class POP>
class Bipopulation_memetic {
	PARAMETERS::Params param;
	std::vector<std::vector<float>>	q_table;
	std::string str;
	POP<Solution_Type> pop;
	long long total_time;
	std::vector<int> time_record;
public:
	std::vector<int> Best;
	/*
		Use Params to construct a Bipopulation_memetic algorithm
		The all members of Params should be defined

	*/
	void print_algorithm_info() {
		std::cout << "***************************************************** " << std::endl;
		std::cout << "The parameter setting is " << std::endl;
		std::cout << "alpha = \t" << param.alpha << std::endl;
		std::cout << "gamma = \t" << param.gamma << std::endl;
		std::cout << "selection_rate = \t" << param.selection_rate << std::endl;
		std::cout << "theta = \t" << param.theta << std::endl;
		std::cout << "beta = \t" << param.beta << std::endl;
		std::cout << "max_gen =\t " << param.max_gen << std::endl;
		std::cout << "machine_num =\t " << param.machine_num << std::endl;
		std::cout << "pop_size =\t " << param.pop_size << std::endl;
		std::cout << "Threshold =\t " << param.Threshold << std::endl;
		std::cout << "scenario_num =\t " << param.scenario_num << std::endl;
		std::cout << "***************************************************** " << std::endl << std::endl;
	}
	

	Bipopulation_memetic(PARAMETERS::Params _param, std::string _str) {
		ASSERT_MSG(!_param.scenario.empty(),"没有场景加工时间信息");
		ASSERT_MSG(_param.Threshold,"没有计算场景Threshold");
		param = _param;
		str = _str;
		total_time = 0;
		print_algorithm_info();
	}
	/*
	void framework() {
		std::int64_t accum_time = 0;
		//CPU_TIME(accum_time, Mainframework);
		std::cout << "CPU Time is " << accum_time << std::endl;
	}
	*/


	void run() {
		POP<Solution_Type> pop(param);
		
		framework(pop);
		std::cout << "Total time: " << total_time << " ms" << std::endl;



		// print q table
		PRINT_2VECTOR(q_table, "state");

		for (int i = 0; i < param.pop_size; i++) {
			std::cout << pop.population[i].SI.bad_scenario_num << " ";
		}
		std::cout << std::endl;

		std::vector<int> best_makespans = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
			return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
			})->scenario_makespan;

		PRINT_VECTOR(best_makespans, "best makespan is as follow");
		Extract_to_File(time_record, str + "-timerecord");
		PRINT_VECTOR(Best, str);
		str += "-" + std::to_string(total_time);
		Extract_to_File(Best, str);
		
	}

	void framework(POP<Solution_Type>& pop) {
		// solutions index who is chosen
		std::vector<int> chosen_solution_index;
		INITIALIZATION::Q_TABLE<POP, Solution_Type>()(q_table, param);
		// Generate Initial Solutions
		INITIALIZATION::SCENARIO_NEH_DOUBLE<POP,Solution_Type>()(param, pop);
		//std::cout << pop << std::endl;
		for (int g = 0; g < param.max_gen; g++) {
			
			auto start = std::chrono::high_resolution_clock::now();

			if (param.use_dynamic_beta) {
				param.beta = (float)(g + 1) / param.max_gen;
			}
			pop.clear();
			chosen_solution_index.clear();
			GLOBAL_SEARCH::SELECTION<POP,Solution_Type>()(pop, chosen_solution_index, param);
			//PRINT_VECTOR(chosen_solution_index,"被选择的解");

			GLOBAL_SEARCH::CROSSOVER<POP, Solution_Type>()(pop, chosen_solution_index, param);

			//pop.print_current_state(chosen_solution_index);

			GLOBAL_SEARCH::MUTATION<POP, Solution_Type>()(pop, chosen_solution_index, param);
			
			//pop.print_current_state(chosen_solution_index);

			LOCAL_SEARCH::CRITICAL_PATH<POP, Solution_Type>()(pop, chosen_solution_index, q_table, g, param);
			
			pop.Improve_pop();

			//--------------------------------------------------
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;
			total_time += elapsed.count();
			time_record.push_back(elapsed.count());
			//-----------------------------------------------------

			int best = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;

			Best.push_back(best);

			//std::cout << std::endl;
		}

	}



};