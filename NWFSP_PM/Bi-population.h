#pragma once
#include"Parameter.h"
#include<cassert>
#include "initialization.h"

template<typename Solution_Type, template<typename> class POP>
class Bipopulation_memetic {
	PARAMETERS::Params param;
	std::vector<std::vector<float>>	q_table;
public:
	std::vector<int> Best;
	/*
		Use Params to construct a Bipopulation_memetic algorithm
		The all members of Params should be defined

	*/
	Bipopulation_memetic(PARAMETERS::Params& _param) {
		ASSERT_MSG(!_param.scenario.empty(),"没有场景加工时间信息");
		ASSERT_MSG(_param.Threshold,"没有计算场景Threshold");
		param = _param;
	}


	void framework() {
		// solutions index who is chosen
		std::vector<int> chosen_solution_index;

		// set population: population_size,job_num, machine_num,scenario,Threshold
		POP<Solution_Type> pop(param.pop_size, param.job_num, param.machine_num, param.scenario, param.Threshold);


		// Generate Initial Solutions
		INITIALIZATION::SCENARIO_NEH_DOUBLE<POP,Solution_Type>()(param, pop);
		/*std::cout << pop << std::endl;*/






	}



};