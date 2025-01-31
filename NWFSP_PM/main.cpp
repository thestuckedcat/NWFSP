//#include"Swarm_Intelligence.h"
#include"solution.h"
#include"Bi-population.h"
#include"Parameter.h"
#include "ParallelRunner.h"
#include "myutils.h"
#include "Bi-population-v1.h"
#include "Bi-population-v2.h"
#include "Bi-population-v3.h"
#include "Bi-population-v4.h"


int main() {
	//set_process_priority();
	
	// use CPU TERMINATION
	// param.set_to_cpu_terminate();
	

	// Bind Assigner to TaskQueue
	TaskQueue task_queue;
	TaskAssigner dispatcher(task_queue);
	
	// run 20 times for each task
	int run_count = 1;

	//{
	//	//std::vector<int> job{100,200 };
	//	//std::vector<int> machine{ 5};
	//	std::vector<int> job{ 10,20,30,40,50,60 };
	//	std::vector<int> machine{ 5,10,15,20 };
	//	//std::vector<int> machine{ 15 };
	//	//std::vector<int> machine{ 15,20 };
	//	std::vector<float> bbeta{ 0 };

	//	for (auto n : job) {
	//		for (auto m : machine) {
	//			std::string str = "J_" + std::to_string(n) + "_M_" + std::to_string(m);
	//			PARAMETERS::Params param = PARAMETERS::Params(0.5, // selection rate
	//				0.2, // alpha
	//				0.6, // gamma
	//				0.85,//0.85,	 // epsilon
	//				0,	 // beta	
	//				6000,  // max_gen 3000 6000
	//				n,	 // job_num	
	//				m,   // machine_num
	//				100, // pop_size
	//				20, // scenario_num
	//				10); // P
	//			param.load_or_generate();
	//			///param.set_to_cpu_terminate();
	//			//param.generate_scenario();
	//			for (auto b : bbeta) {
	//				param.beta = b;

	//				std::string str1 = str + "_B_" + std::to_string(b);

	//				dispatcher.assign_tasks(param, str1, run_count, "SQMA");


	//				//for (int run_id = 1; run_id <= run_count; run_id++)
	//				//{
	//				//	/*param.max_gen = 12000;
	//				//	Bipopulation_memetic_version4<NWFSP_Solution, Population> bipopulation2(param, str1 + "_RandomLocalinstance_" + std::to_string(run_id));
	//				//	bipopulation2.run();*/
	//				//	param.max_gen = 6000;
	//				//	Bipopulation_memetic_version3<NWFSP_Solution, Population> bipopulation(param, str1 + "_instance_" + std::to_string(run_id));
	//				//	bipopulation.run();


	//				//	
	//				//	
	//				//}

	//			}

	//		}
	//	}


	//}
	{
		//std::vector<int> job{100,200 };
		//std::vector<int> machine{ 5};
		std::vector<int> job{ 600,500,400,300,200,100 };
		std::vector<int> machine{20,40,60};
		//std::vector<int> machine{ 15 };
		//std::vector<int> machine{ 15,20 };
		std::vector<float> bbeta{ 0};

		for (auto n : job) {
			for (auto m : machine) {
				std::string str = "J_" + std::to_string(n) + "_M_" + std::to_string(m);
				PARAMETERS::Params param = PARAMETERS::Params(0.5, // selection rate
					0.2, // alpha
					0.6, // gamma
					0.75,//0.85,//0.85,	 // epsilon
					0,	 // beta	
					6000,  // max_gen 3000 6000
					n,	 // job_num	
					m,   // machine_num
					100, // pop_size
					20, // scenario_num
					10); // P
				param.load_or_generate();
				///param.set_to_cpu_terminate();
				//param.generate_scenario();
				for (auto b : bbeta) {
					param.beta = b;

					std::string str1 = str + "_B_" + std::to_string(b);

					dispatcher.assign_tasks(param, str1, run_count, "SQMA");


					//for (int run_id = 1; run_id <= run_count; run_id++)
					//{
					//	/*param.max_gen = 12000;
					//	Bipopulation_memetic_version4<NWFSP_Solution, Population> bipopulation2(param, str1 + "_RandomLocalinstance_" + std::to_string(run_id));
					//	bipopulation2.run();*/
					//	param.max_gen = 6000;
					//	Bipopulation_memetic_version3<NWFSP_Solution, Population> bipopulation(param, str1 + "_instance_" + std::to_string(run_id));
					//	bipopulation.run();


					//	
					//	
					//}

				}

			}
		}


	}
	


	//{
	//	//std::vector<int> job{100,200 };
	//	//std::vector<int> machine{ 5};
	//	std::vector<int> job{ 600,500,400,300,200,100};
	//	std::vector<int> machine{ 20,40,60 };
	//	//std::vector<int> machine{ 15 };
	//	//std::vector<int> machine{ 15,20 };
	//	std::vector<float> bbeta{ 0 };

	//	for (auto n : job) {
	//		for (auto m : machine) {
	//			if (n == 600 && (m == 20 || m == 40)) continue;
	//			std::string str = "J_" + std::to_string(n) + "_M_" + std::to_string(m);
	//			PARAMETERS::Params param = PARAMETERS::Params(0.6, // selection rate
	//				0.2, // alpha
	//				0.6, // gamma
	//				0.85,//0.85,	 // epsilon
	//				0,	 // beta	
	//				12000,  // max_gen
	//				n,	 // job_num	
	//				m,   // machine_num
	//				100, // pop_size
	//				20, // scenario_num
	//				10); // P
	//			param.load_or_generate();
	//			param.set_to_cpu_terminate();
	//			//param.generate_scenario();
	//			for (auto b : bbeta) {
	//				param.beta = b;

	//				std::string str1 = str + "_B_" + std::to_string(b);

	//				dispatcher.assign_tasks(param, str1, run_count, "SQMAR");

	//				
	//				//for (int run_id = 1; run_id <= run_count; run_id++)
	//				//{
	//				//	param.max_gen = 6000;
	//				//	Bipopulation_memetic_version3<NWFSP_Solution, Population> bipopulation(param, str1 + "_instance_" + std::to_string(run_id));
	//				//	bipopulation.run();


	//				//	//param.max_gen = 24000;
	//				//	//Bipopulation_memetic_version4<NWFSP_Solution, Population> bipopulation2(param, str1 + "_RandomLocalinstance_" + std::to_string(run_id));
	//				//	//bipopulation2.run();
	//				//}
	//				
	//			}

	//		}
	//	}


	//}




	
	////Threshold Tuning---------------------------------------------------------------------------------------
	//{
	//	std::vector<float> bbeta{ 0};

	//	/*
	//	std::vector<std::pair<int, int>> instances{
	//		{30,5},
	//		{40,5},{40,10},{40,15},{40,20},
	//		{50,5},{50,10},{50,15},
	//		{100,20},
	//		{200,20},
	//		{300,20},
	//		{400,20},{400,60},
	//		{500,40},{500,60},
	//		{600,40}
	//	};*/
	//	std::vector<std::pair<int, int>> instances{

	//		{300,20},{100,40},


	//	};
	//	for (auto p : instances){
	//		int n, m; 
	//		n = p.first;
	//		m = p.second;
	//		std::string str = "J_" + std::to_string(n) + "_M_" + std::to_string(m);
	//		PARAMETERS::Params param = PARAMETERS::Params(1, // selection rate
	//			0.2, // alpha
	//			0.6, // gamma
	//			0.85,//0.85,	 // epsilon
	//			0,	 // beta	
	//			6000,  // max_gen
	//			n,	 // job_num	
	//			m,   // machine_num
	//			100, // pop_size
	//			20, // scenario_num
	//			10); // P
	//		param.load_or_generate();
	//		//param.generate_scenario();
	//		for (auto b : bbeta) {
	//			param.beta = b;

	//			std::string str1 = str + "_B_" + std::to_string(b);

	//			dispatcher.assign_tasks(param, str1, run_count);
	//				
	//				

	//			/*
	//			for (int run_id = 1; run_id <= run_count; run_id++)
	//			{
	//				param.max_gen = 6000;
	//				Bipopulation_memetic_version3<NWFSP_Solution, Population> bipopulation(param, str1 + "_instance_" + std::to_string(run_id));
	//				bipopulation.run();


	//				param.max_gen = 24000;
	//				Bipopulation_memetic_version4<NWFSP_Solution, Population> bipopulation2(param, str1 + "_RandomLocalinstance_" + std::to_string(run_id));
	//				bipopulation2.run();
	//			}
	//			*/
	//				
	//		}

	//	}


	//}
	
	
	//---------------------------------------------------------------------------------------

	
	//TAGUCHI Parameter finding
	/*
	{
		PARAMETERS::Params param = PARAMETERS::Params(	1, // selection rate
														0.8, // alpha
														0.6, // gamma
														0.85,	 // epsilon
														0,	 // beta	
														2000,  // max_gen
														100,	 // job_num	
														5,   // machine_num
														100, // pop_size
														20, // scenario_num
														10); // P

		std::vector<std::vector<float>> TaguchiMethod{ 
			{0.2,	0.2,	0.95,	10},
			{0.2,	0.4,	0.9,	20},
			{0.2,	0.6,	0.85,	50},
			{0.2,	0.8,	0.8,	100},
			{0.4,	0.2,	0.9,	50},
			{0.4,	0.4,	0.85,	100},
			{0.4,	0.6,	0.8,	10},
			{0.4,	0.8,	0.95,	20},
			{0.6,	0.2,	0.85,	100},
			{0.6,	0.4,	0.8,	50},
			{0.6,	0.6,	0.95,	20},
			{0.6,	0.8,	0.9,	10},
			{0.8,	0.2,	0.8,	20},
			{0.8,	0.4,	0.95,	10},
			{0.8,	0.6,	0.9,	100},
			{0.8,	0.8,	0.85,	50} };

		param.generate_scenario();
		param.set_to_cpu_terminate();
		for (int i = 0; i < TaguchiMethod.size(); i++) {
			param.alpha = TaguchiMethod[i][0];
			param.gamma = TaguchiMethod[i][1];
			param.epsilon = TaguchiMethod[i][2];
			param.P = (int)TaguchiMethod[i][3];
			std::string str = "afa_" + std::to_string(param.alpha) 
							+ "_gama_" + std::to_string(param.gamma) 
							+ "_ep_" + std::to_string(param.epsilon) 
							+ "_P_" + std::to_string(param.P);
			
			//std::string curstr = str + "_instance_" + std::to_string(j);
			dispatcher.assign_tasks(param, str, run_count);
			//Bipopulation_memetic_version3<NWFSP_Solution, Population> bipopulation(param, curstr);
			//bipopulation.run();
		}
	}
	*/
	

	/*
	std::vector<int> job{ 50,80,100,200,20 };
	std::vector<int> machine{ 5,10,20 };
	std::vector<float> bbeta{ 0,0.2,0.4,0.6,1 };
	for (auto n : job) {
		for (auto m : machine) {
			std::string str = "J_" + std::to_string(n) + "_M_" + std::to_string(m);
			PARAMETERS::Params param = PARAMETERS::Params(0.2, // selection rate
															0.8, // alpha
															0.6, // gamma
															0.85,	 // epsilon
															0,	 // beta	
															2000,  // max_gen
															n,	 // job_num	
															m,   // machine_num
															100, // pop_size
															20); // scenario_num
			param.generate_scenario();
			for (auto b : bbeta) {
				param.beta = b;
				
				std::string str1 = str + "_B_" + std::to_string(b);

				dispatcher.assign_tasks(param, str1, run_count);
			}

		}
	}
	

	
	// wait for all tasks done
	task_queue.wait_for_completion();
	*/


	/*
	PARAMETERS::Params param = PARAMETERS::Params(	1, // selection rate
													0.8, // alpha
													0.6, // gamma
													0.85,	 // epsilon
													0,	 // beta	
													2000,  // max_gen
													100,	 // job_num	
													5,   // machine_num
													20, // pop_size
													20, // scenario_num
													10	); // P

	param.generate_scenario();




	for (int i = 0; i < 20; i++)
	{
		Bipopulation_memetic<NWFSP_Solution, Population> test2(param, "myalgorithm_instance_" + std::to_string(i));
		test2.run();
		Bipopulation_memetic_version1<NWFSP_Solution, Population> test(param, "VERSION1_instance_"+std::to_string(i));
		test.run();
		Bipopulation_memetic_version2<NWFSP_Solution, Population> test3(param, "VERSION2_instance_"+std::to_string(i));
		test3.run();

		Bipopulation_memetic_version3<NWFSP_Solution, Population> test4(param, "VERSION3_instance_"+std::to_string(i));
		test4.run();
	}

	//std::cout << "ALL JOBS DONE\n";
	*/
}