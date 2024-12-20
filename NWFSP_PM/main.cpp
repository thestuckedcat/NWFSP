//#include"Swarm_Intelligence.h"
#include"solution.h"
#include"Bi-population.h"
#include"Parameter.h"
#include "ParallelRunner.h"
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

	/*

	PARAMETERS::Params param = PARAMETERS::Params(0.2, 0.1, 0.9, 0, 0, 2000, 20, 5, 100, 20);
	param.generate_scenario();


	{
		// beta = 0
		Bipopulation_memetic<NWFSP_Solution, Population> test1(param, "pureLN");
		test1.run();
	}


	
	{
		// beta = 0.2
		param.beta = 0.2;
		Bipopulation_memetic<NWFSP_Solution, Population> test2(param, "beta02");
		test2.run();
	}

	{
		// beta = 0.4
		param.beta = 0.4;
		Bipopulation_memetic<NWFSP_Solution, Population> test3(param, "beta04");
		test3.run();
	}

	{
		// beta = 0.6
		param.beta = 0.6;
		Bipopulation_memetic<NWFSP_Solution, Population> test6(param, "beta06");
		test6.run();
	}
	{
		// beta = 0.8
		param.beta = 0.8;
		Bipopulation_memetic<NWFSP_Solution, Population> test7(param, "beta08");
		test7.run();
	}
	{
		// beta = 1
		param.beta = 1;
		Bipopulation_memetic<NWFSP_Solution, Population> test4(param, "pureUN");
		test4.run();
	}



	{
		// dynamic_beta
		param.use_dynamic_beta = true;
		Bipopulation_memetic<NWFSP_Solution, Population> test5(param, "dynamicBeta");
		test5.run();
	}
	*/





	PARAMETERS::Params param = PARAMETERS::Params(	0.2, // selection rate
													0.2, // alpha
													0.6, // gamma
													0.85,	 // epsilon
													0.4,	 // beta	
													2000,  // max_gen
													50,	 // job_num	
													5,   // machine_num
													100, // pop_size
													20); // scenario_num

	// use CPU TERMINATION
	param.set_to_cpu_terminate();
	param.generate_scenario();

	// Bind Assigner to TaskQueue
	const int num_threads = 12; 
	TaskQueue task_queue(num_threads);
	TaskAssigner dispatcher(task_queue);
	
	// run 20 times for each task
	const int run_count = 10;
	
	//TAGUCHI Parameter finding
	{
		std::vector<std::vector<float>> TaguchiMethod{ 
			{0.2,0.2,0.95,50},
			{0.2,0.4,0.9,100},
			{0.2,0.6,0.85,150},
			{0.2,0.8,0.8,200},
			{0.4,0.2,0.9,150},
			{0.4,0.4,0.95,200},
			{0.4,0.6,0.8,50},
			{0.4,0.8,0.85,100},
			{0.6,0.2,0.85,200},
			{0.6,0.4,0.8,150},
			{0.6,0.6,0.95,100},
			{0.6,0.8,0.9,50},
			{0.8,0.2,0.8,100},
			{0.8,0.4,0.85,50},
			{0.8,0.6,0.9,200},
			{0.8,0.8,0.95,150} };


		for (int i = 0; i < TaguchiMethod.size(); i++) {
			param.alpha = TaguchiMethod[i][0];
			param.gamma = TaguchiMethod[i][1];
			param.epsilon = TaguchiMethod[i][2];
			param.pop_size = (int)TaguchiMethod[i][3];
			std::string str = "afa_" + std::to_string(param.alpha) 
							+ "_gama_" + std::to_string(param.gamma) 
							+ "_ep_" + std::to_string(param.epsilon) 
							+ "_pop_" + std::to_string(param.pop_size);
			dispatcher.assign_tasks(param, str, run_count);
		}
	}
	/*
	dispatcher.assign_tasks(param, "pureLN-20job5machine", run_count);

	param.beta = 0.2;
	dispatcher.assign_tasks(param, "beta02-20job5machine", run_count);

	param.beta = 0.4;
	dispatcher.assign_tasks(param, "beta04-20job5machine", run_count);

	param.beta = 0.6;
	dispatcher.assign_tasks(param, "beta06-20job5machine", run_count);

	param.beta = 0.8;
	dispatcher.assign_tasks(param, "beta08-20job5machine", run_count);

	param.beta = 1;
	dispatcher.assign_tasks(param, "pureUN-20job5machine", run_count);
	*/
	// wait for all tasks done
	task_queue.wait_for_completion();
	
	

	std::cout << "ALL JOBS DONE\n";

}