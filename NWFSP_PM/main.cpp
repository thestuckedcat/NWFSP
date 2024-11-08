//#include"Swarm_Intelligence.h"
#include"solution.h"
#include"Bi-population.h"
#include"Parameter.h"
int main() {
	/*
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
	*/

	PARAMETERS::Params param = PARAMETERS::Params(0.2, 0.1, 0.9, 0, 0, 2000, 100, 5, 100, 20);
	param.generate_scenario();



	// beta = 0
	Bipopulation_memetic<NWFSP_Solution, Population> test1(param,"pureLN");
	test1.run();


	
	// beta = 0.2
	param.beta = 0.2;
	Bipopulation_memetic<NWFSP_Solution, Population> test2(param, "beta02");
	test2.run();

	// beta = 0.4
	param.beta = 0.4;
	Bipopulation_memetic<NWFSP_Solution, Population> test3(param, "beta04");
	test3.run();

	// beta = 0.6
	param.beta = 0.6;
	Bipopulation_memetic<NWFSP_Solution, Population> test6(param, "beta06");
	test6.run();
	// beta = 0.8
	param.beta = 0.8;
	Bipopulation_memetic<NWFSP_Solution, Population> test7(param, "beta08");
	test7.run();
	// beta = 1
	param.beta = 1;
	Bipopulation_memetic<NWFSP_Solution, Population> test4(param, "pureUN");
	test4.run();



	// dynamic_beta
	param.use_dynamic_beta = true;
	Bipopulation_memetic<NWFSP_Solution, Population> test5(param, "dynamicBeta");
	test5.run();
	



	



}