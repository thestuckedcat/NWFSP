#pragma once
#include"Parameter.h"
#include<cassert>
#include "initialization.h"
#include "Global_Search.h"
#include "myutils.h"
#include "Local_search.h"
#include <string>
template<typename Solution_Type, template<typename> class POP>
class Bipopulation_memetic {
	PARAMETERS::Params param;
	std::vector<std::vector<float>>	q_table;
	std::string str;
public:
	std::vector<int> Best;
	/*
		Use Params to construct a Bipopulation_memetic algorithm
		The all members of Params should be defined

	*/
	Bipopulation_memetic(PARAMETERS::Params& _param, std::string _str) {
		ASSERT_MSG(!_param.scenario.empty(),"没有场景加工时间信息");
		ASSERT_MSG(_param.Threshold,"没有计算场景Threshold");
		param = _param;
		str = _str;
	}


	void framework() {
		
		// solutions index who is chosen
		std::vector<int> chosen_solution_index;
		INITIALIZATION::Q_TABLE<POP, Solution_Type>()(q_table, param);
		// set population: population_size,job_num, machine_num,scenario,Threshold
		POP<Solution_Type> pop(param);


		// Generate Initial Solutions
		INITIALIZATION::SCENARIO_NEH_DOUBLE<POP,Solution_Type>()(param, pop);
		//std::cout << pop << std::endl;
		for (int g = 0; g < param.max_gen; g++) {
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


			int best = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;

			Best.push_back(best);

			//std::cout << std::endl;
		}


		// print q table
		PRINT_2VECTOR(q_table, "state");

		for (int i = 0; i < param.pop_size; i++) {
			std::cout << pop.population[i].SI.bad_scenario_num << " ";
		}
		std::cout << std::endl;

		Extract_to_File(Best, str);


	}



};