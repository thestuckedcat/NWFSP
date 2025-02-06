#pragma once

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
class HACO {
	PARAMETERS::Params param;
	std::string str;
	POP<Solution_Type> pop;
	long long total_time;
	std::vector<int> time_record;

	int scenario_idx;
	DataLogger logger;
	DataLogger logger_BSN;
	DataLogger logger_WSM;
	DataLogger logger_BestSolution;
	DataLogger logger_generationRelated;
	// the BSN trend and WSM trend of the chosen solution
	std::vector<int> last_BSN;
	std::vector<int> last_WSM;
	long long last_best_PT;

	int SameBSNCount;
public:
	std::vector<int> Best;
	/*
		Use Params to construct a Bipopulation_memetic algorithm
		The all members of Params should be defined

	*/
	void print_algorithm_info() {
		logger << "*****************************************************\n";
		logger << "The parameter setting is " << std::endl;
		logger << "alpha = \t" << param.alpha << std::endl;
		logger << "gamma = \t" << param.gamma << std::endl;
		logger << "selection_rate = \t" << param.selection_rate << std::endl;
		logger << "epsilon = \t" << param.epsilon << std::endl;
		logger << "beta = \t" << param.beta << std::endl;
		logger << "max_gen =\t " << param.max_gen << std::endl;
		logger << "machine_num =\t " << param.machine_num << std::endl;
		logger << "pop_size =\t " << param.pop_size << std::endl;
		logger << "Threshold =\t " << param.Threshold << std::endl;
		logger << "scenario_num =\t " << param.scenario_num << std::endl;
		logger << "P = \t" << param.P << std::endl;
		logger << "***************************************************** " << std::endl << std::endl;
	}


	HACO(PARAMETERS::Params _param, std::string _str)
		:logger(_str, "Log"), logger_BSN(_str, "BSNComparison"), logger_WSM(_str, "WSMComparison"), logger_BestSolution(_str, "BestSolution"), logger_generationRelated(_str, "generationRelated")
	{
		ASSERT_MSG(!_param.scenario.empty(), "没有场景加工时间信息");
		ASSERT_MSG(_param.Threshold, "没有计算场景Threshold");
		param = _param;
		total_time = 0;
		str = _str;
		print_algorithm_info();
	}

	void run() {


		POP<Solution_Type> pop(param);

		framework(pop);
		logger << "Total time: " << total_time << " ms" << std::endl;



		// print q table
		PRINT_2VECTOR_LOGGER(q_table, "state", logger);

		logger << "The population bad scenario num is" << std::endl;
		for (int i = 0; i < param.pop_size; i++) {
			logger << pop.population[i].SI.bad_scenario_num << " ";
		}
		logger << std::endl;

		std::vector<int> best_makespans = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
			return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
			})->scenario_makespan;

		PRINT_VECTOR_LOGGER(best_makespans, "best makespan is as follow", logger);
		Extract_to_File(time_record, str, "timerecord");
		PRINT_VECTOR_LOGGER(Best, str, logger);
		str += "-" + std::to_string(total_time);
		Extract_to_File(Best, str, "trend");


		for (int i = 0; i < param.pop_size; i++) {
			logger << "The BSN trend of " << i << "-th individual is\n";
			logger << pop.population[i].BSN;
			logger << "The WSM trend of " << i << "-th individual is\n";
			logger << pop.population[i].WSM;
		}


		logger_BSN << last_BSN;
		logger_WSM << last_WSM;

		auto best = min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
			return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
			});
		logger_BestSolution << best->SI.Penalty_of_TBS << std::endl << best->SI.bad_scenario_num << std::endl << (float)(total_time / 1000.0) << std::endl;

	}

	void framework(POP<Solution_Type>& pop) {
		// solutions index who is chosen
		std::vector<int> chosen_solution_index;
		std::vector<std::vector<float>> eta(param.job_num,std::vector<int>(param.job_num, 0));//eta[i][j] : the mean makespan increment for job j after i
		std::vector<std::vector<float>> PheromoneMatrix(param.job_num, std::vector<int>(param.job_num, 1));
		{
			for (int i = 0; i < param.job_num; i++) {
				for (int j = 0; j < param.job_num; j++) {
					if (i == j) {
						eta[i][j] = 0;
						continue;
					}
					float inc = 0;
					for (int s = 0; s < param.scenario_num; s++) {
						inc += param.generate_jobTojobInfo[s][i + 1][j + 1];
					}
					inc /= param.scenario_num;
					eta[i][j] = 1 / inc;
				}
			}
		}
		// Generate Initial Solutions
		INITIALIZATION::SCENARIO_NEH_DOUBLE<POP, Solution_Type>()(param, pop);
		//std::cout << pop << std::endl;
		{
			auto tmp = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			last_BSN.push_back(tmp->SI.bad_scenario_num);
			last_WSM.push_back(tmp->SI.worst_scenario_makespan);
			last_best_PT = tmp->SI.Penalty_of_TBS;
		}
		SameBSNCount = 0;

		for (int g = 0; g <= param.max_gen; g++) {
			int state = 0;
			if (param.is_cpu_terminated && total_time >= param.cpu_terminate_time) {
				logger << "Terminated generation is " << g << std::endl;
				break;
			}

			//long long start = (long long)get_thread_cpu_time();
			auto start = std::chrono::high_resolution_clock::now();
			scenario_idx = generate_random_int(0, param.scenario_num);
			


			pop.clear();
			chosen_solution_index.clear();
			//GLOBAL_SEARCH::SELECTION<POP, Solution_Type>()(pop, chosen_solution_index, param,"TTS");
			GLOBAL_SEARCH::SELECTION<POP, Solution_Type>()(pop, chosen_solution_index, param);
			//PRINT_VECTOR(chosen_solution_index,"被选择的解");
			LOCAL_SEARCH::ACO<POP, Solution_Type>()(pop, chosen_solution_index, PheromoneMatrix, eta, param);

			//pop.print_current_state(chosen_solution_index);

			LS_Version1::CRITICAL_PATH_version1<POP, Solution_Type>()(pop, chosen_solution_index, param, scenario_idx);

			pop.Improve_pop();


			LS_Version3::Update_Qtable(last_BSN, last_WSM, q_table, state, scenario_idx, pop, last_best_PT, param, g + 1, SameBSNCount);
			//--------------------------------------------------

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = (end - start);
			if (g >= 0) {
				total_time += elapsed.count() / traverse_time;
				/*long long end = (long long)get_thread_cpu_time();
				long long elapsed = end - start;
				total_time += elapsed;*/
				if (g >= 0)	time_record.push_back(total_time);
				//-----------------------------------------------------

				auto bestsolution = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
					return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
					});
				long long best = bestsolution->SI.Penalty_of_TBS;

				Best.push_back(best);

				if (g % 500 == 0) {
					logger_generationRelated << g << "\t" << best << "\ttime\t" << total_time << "\t BSN\t" << bestsolution->SI.bad_scenario_num << std::endl;

				}
			}




			//std::cout << std::endl;
		}

	}



};