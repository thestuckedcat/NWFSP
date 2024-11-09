#pragma once
# include<iostream>
# include<queue>
# include<set>
# include<random>
# include<unordered_map>
# include<unordered_set>
# include<vector>
# include"myutils.h"
# include"Cooperative.h"
# include"Initialization.h"
# include"Global_Search.h"
# include"Local_search.h"
# include<fstream>



template<	typename Solution_Type, 
			template <typename> class POP>
class Memetic {
	auto Generate_scenario_pt(int scenario_num, int job_num, int machine_num) {
		std::vector<std::vector<std::vector<int>>> res(scenario_num,
			std::vector<std::vector<int>>(job_num,
				std::vector<int>(machine_num)));
		for (int i = 0; i < scenario_num; i++) {
			for (int j = 0; j < job_num; j++) {
				for (int m = 0; m < machine_num; m++) {
					res[i][j][m] = generate_random_int(10, 100);
				}
			}
		}

		return res;
	}

	int calculateThreshold(std::vector<std::vector<std::vector<int>>>& scen) {
		int totalSum = 0;

		for (const auto& scenario : scen) {
			for (const auto& job : scenario) {
				totalSum += std::accumulate(job.begin(), job.end(), 0);
			}
		}

		return totalSum / scen.size();
	}

	int max_gen;
	int job_num;
	int machine_num;
	int pop_size;
	int Threshold;
	int scenario_num;



public:
	Memetic(int max_gen_, int job_num_, int machine_num_, int pop_size_, int Threshold_, int scenario_num_)
		:max_gen{max_gen_}, job_num{job_num_}, machine_num{machine_num_}, pop_size{pop_size_}, Threshold{Threshold_}, scenario_num{scenario_num_}
	{
		
	}

	void LN_UN_comparison() {
		// Generate scenario processing time
		auto scenario = Generate_scenario_pt(scenario_num, job_num, machine_num);
		long long total_time_LN = 0;
		long long total_time_UN = 0;
		Threshold =(int)( 0.37*calculateThreshold(scenario));
		std::cout << Threshold << std::endl;

		CPU_TIME(total_time_LN,framework("LN", 0.1, scenario));
		CPU_TIME(total_time_UN,framework("UN", 0.1, scenario));

		std::cout << "LN is " << total_time_LN << std::endl;
		std::cout << "UN is " << total_time_UN << std::endl;
	}


	void framework(	std::string str, 
					const float selection_rate, 
					std::vector<std::vector<std::vector<int>>>& scenario) {

		// Solutions be chosen
		std::vector<int> chosen_solution_index_r;
		std::vector<int> chosen_solution_index_t;
	
		// Initialize Scenario Structure
		SCENARIO_STRUCTURE neighborStructure(str);

		// set pop : job_num, machine_num, scenario_processing_time, pop_size
		POP<Solution_Type> Pop_r(pop_size, job_num, machine_num, scenario, Threshold);
		POP<Solution_Type> Pop_t(pop_size, job_num, machine_num, scenario, Threshold);

		// Generate Initial solutions
		INITIALIZATION::SCENARIO_NEH_DOUBLE<POP, Solution_Type>()(Pop_r, Pop_t);

		// Initializing Q_table
		if(neighborStructure.str == "LN")
			INITIALIZATION::Q_TABLE<POP, Solution_Type>()(Pop_r, neighborStructure.q_table);


		// for graph
		std::vector<int> Best;
		std::vector<int> Best_in_r;
		std::vector<int> Best_in_t;

		// Loop main
		for (int g = 0; g < max_gen; g++) {
			GLOBAL_SEARCH::SELECTION<POP, Solution_Type>()(Pop_r, selection_rate, chosen_solution_index_r);
			GLOBAL_SEARCH::SELECTION<POP, Solution_Type>()(Pop_t, selection_rate, chosen_solution_index_t);


			GLOBAL_SEARCH::CROSSOVER<POP,Solution_Type>()(Pop_r, chosen_solution_index_r);
			GLOBAL_SEARCH::MUTATION<POP,Solution_Type>()(Pop_r, chosen_solution_index_r);

			GLOBAL_SEARCH::PBXCROSSOVER<POP, Solution_Type>()(Pop_t, chosen_solution_index_t);

			LOCAL_SEARCH::CRITICAL_PATH <POP,Solution_Type>()(Pop_r, chosen_solution_index_r, neighborStructure, g, max_gen);
			LOCAL_SEARCH::CRITICAL_PATH <POP,Solution_Type>()(Pop_t, chosen_solution_index_t, neighborStructure, g, max_gen);

			// Update the current sequence with best trans_seq
			Pop_r.Improve_pop(chosen_solution_index_r);
			Pop_t.Improve_pop(chosen_solution_index_t);


			// record trend
			int bestr = std::min_element(Pop_r.population.begin(), Pop_r.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;
			int bestt = std::min_element(Pop_t.population.begin(), Pop_t.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;

			Best_in_r.push_back(bestr);
			Best_in_t.push_back(bestt);
			Best.push_back(std::min(bestr, bestt));

			COOPERATIVE::Calculate_Score_and_Repartition<POP,Solution_Type>()(Pop_r, Pop_t,g);

		}

		//PRINT_VECTOR(Best, "Best trend");
		//PRINT_VECTOR(Best_in_r, "Best trend of r");
		//PRINT_VECTOR(Best_in_t, "Best trend of t");
		
		
		if (neighborStructure.str == "LN") {
			for (auto& a : neighborStructure.q_table) {
				PRINT_VECTOR(a, "state");
			}
		}
		
		std::cout << "The bad scenario num is" << std::endl;
		for (int i = 0; i < Pop_r.population.size(); i++) {
			std::cout << Pop_r.population[i].SI.bad_scenario_num << " ";
		}
		std::cout << std::endl;
		for (int i = 0; i < Pop_t.population.size(); i++) {
			std::cout << Pop_t.population[i].SI.bad_scenario_num << " ";
		}
		std::cout << std::endl;
		Extract_to_File(Best, str, "trend");
	}
};












