#pragma once
# include<iostream>
# include<vector>
# include<unordered_set>
# include<cassert>
# include<random>
#include <algorithm>
# include<queue>
# include<stack>
# include<chrono>
# include "myutils.h"
# include "solution.h"
# include<tuple>
# include<set>
# include"OverloadTools.h"
# include "SingleSceanrioStructure.h"



std::vector<int> generate_unique_random_numbers(int n, int m) {
	assert(m <= n);
	// n is range, m is needed number
	std::vector<int> numbers(n);
	for (int i = 0; i < n; ++i) {
		numbers[i] = i;
	}

	std::random_device rd;
	std::mt19937 gen(rd());

	std::shuffle(numbers.begin(), numbers.end(), gen);

	// pick first m elements
	return std::vector<int>(numbers.begin(), numbers.begin() + m); 
}


/*
// input: A solution
// output: A new solution
struct graphNode {
	int machine_id;
	int job_id;
	int pos;
	// the accessable operations, including (machine_idx, pos_idx)
	std::vector<std::pair<int,int>> neighbor;
	graphNode() {};
	graphNode(int m, int pos,int j)
		: machine_id{ m }, job_id{ j }, pos{ pos } {

	}

	void add_neighbor(int m, int pos) {
		neighbor.push_back(std::make_pair(m, pos));
	}
};
*/




namespace LOCAL_SEARCH {
	template<template<typename> class POP, typename Solution_type>
	struct CRITICAL_PATH{
		/*
		void operator()(POP<Solution_type>& pop, 
						const std::vector<int>& chosen_solution,
						SCENARIO_STRUCTURE &SS,
						int generation,
						int max_gen) 
		{
			if (SS.str == "LN") {
				LN_structure(pop, chosen_solution, SS.q_table, generation, max_gen);
			}

			if (SS.str == "UN") {
				UN_structure(pop, chosen_solution);
			}
		}
		*/

		
		//for bi-population
		void operator()(POP<Solution_type>& pop,
			const std::vector<int>& chosen_solution,
			std::vector<std::vector<float>>& q_table,
			const int currentGeneration,
			PARAMETERS::Params& param) {

			for (int i = 0; i < chosen_solution.size(); i++) {
				pop.population[chosen_solution[i]].calculate_transition_scenario_makespan(param);
			}

			// sort and repartition
			std::sort(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				SCENARIO_INFO aa = a.SI_trans.is_empty() ? a.SI : a.SI_trans;
				SCENARIO_INFO bb = b.SI_trans.is_empty() ? b.SI : b.SI_trans;

				return aa.bad_scenario_num < bb.bad_scenario_num;
			});
			/*
			// FOR DEBUGGING
			{
				std::cout << "The BSN after sort is ";
				for (auto i : pop.population) {
					if (i.SI_trans.is_empty()) {
						std::cout << i.SI.bad_scenario_num << " ";
					}
					else {
						std::cout << "trans" << i.SI_trans.bad_scenario_num << " ";
					}

				}
				std::cout << std::endl;
			}
			*/


			int demarcation = int(param.beta * param.scenario_num);
			
			auto it = std::upper_bound(pop.population.begin(), pop.population.end(), demarcation,
				[](const int demarcation, const auto& individual) {
					SCENARIO_INFO scenario = individual.SI_trans.is_empty() ? individual.SI : individual.SI_trans;
					return demarcation < scenario.bad_scenario_num;
				});


			// [idx,end] for LN
			// [0,idx) for  UN
			int idx = std::distance(pop.population.begin(), it);

			if(param.beta) // not work for pureLN
			{
				bool have_job = false;
				for (int i = 0; i < idx; i++) {
					if (!pop.population[i].SI_trans.is_empty()) {
						have_job = true;
						break;
					}
				}

				if (!have_job) {
					//if no job is handled by UN, force to align one

					for (int i = idx; i < param.pop_size; i++) {
						if (!pop.population[i].SI_trans.is_empty()) {
							idx = i + 1;
							break;
						}
					}
				}

			}
			
			

			std::pair<int, int> range_UN{ 0,idx };
			std::pair<int, int> range_LN{ idx,param.pop_size };
			/*
			// FOR DEBUGGING: IF LN FUNCTION PROPERLY
			{
				for (int i = 0; i < param.pop_size; i++) {
					if (pop.population[i].trans_sequence.empty()) {
						std::cout << "**************************\n this solution is not chosen \n *************************************\n";
						continue;
					}
					std::cout << "The " << i << "-th solution is:\n";
					for (int j = 0; j < param.job_num; j++) {
						std::cout << pop.population[i].trans_sequence[j] << " ";
					}
					std::cout << "\n and its objective is" << pop.population[i].SI_trans.Penalty_of_TBS << "\n\n";
				}
			}
			*/

			LN_structure(pop, range_LN, q_table, currentGeneration, param);
			/*
			// FOR DEBUGGING: IF LN FUNCTION PROPERLY
			//PRINT_2VECTOR(q_table, "q_Table is as follow");

			{
				for (int i = 0; i < param.pop_size; i++) {
					if (pop.population[i].trans_sequence.empty()) {
						std::cout << "**************************\n this solution is not chosen \n *************************************\n";
						continue;
					}
					std::cout << "The " << i << "-th solution after LN is:\n";
					for (int j = 0; j < param.job_num; j++) {
						std::cout << pop.population[i].trans_sequence[j] << " ";
					}

					std::cout << "\n and its objective is" << pop.population[i].SI_trans.Penalty_of_TBS << "\n\n";
				}
			}
			*/
			UN_structure(pop, range_UN, param);


			/*
				此时population中应该包含了如下信息

				未被选中的个体仅有SI和sequence的相关信息
				被选中的个体还拥有SI_trans和trans_sequence的相关信息
			*/
		}


		void LN_structure(	POP<Solution_type>& pop,
							const std::pair<int, int>& range,
							std::vector<std::vector<float>>& q_table,
							const int gen,
							PARAMETERS::Params& param)
		{
			if (range.second <= range.first) {
				return;
			}
			std::vector<std::vector<int>> neighbors;

			if (gen == 0) {
				for (int i = range.first; i < range.second ; i++) {
				
					//if is not be chosen 
					if (pop.population[i].SI_trans.is_empty()) {
						continue;
					}
					Solution_type& target = pop.population[i];
					int chosen_scenario = generate_random_int(0, param.scenario_num);
					neighbors = NEIGHBOUR::CriticalPath_based_neighbor(target, chosen_scenario, param);

					find_best_neighbor_and_update_q_table(neighbors, pop, target, q_table, gen, 0, chosen_scenario, param);
				}
			}
			else {
				for (int i = range.first; i < range.second; i++) {
					
					if (pop.population[i].SI_trans.is_empty()) {
						continue;
					}
					

					float epsilon = generate_random_float();
					Solution_type& target = pop.population[i];

					int state = target.last_state;
					int action_scen;

					// choose best scenario
					//if (epsilon > param.epsilon) {
					if (epsilon < (0.1+(float)gen/param.max_gen)) {
						auto it = std::max_element(q_table[state].begin(), q_table[state].end());
						action_scen = std::distance(q_table[state].begin(), it);
						ASSERT_MSG(action_scen >= 0 && action_scen < param.scenario_num, "Wrong action chosen");
					}
					// randomly choose
					else {
						action_scen = generate_random_int(0, param.scenario_num);
					}


					neighbors = NEIGHBOUR::CriticalPath_based_neighbor(target, action_scen, param);

					find_best_neighbor_and_update_q_table(neighbors, pop, target, q_table, gen, state, action_scen, param);
				}
			}
		}

		void UN_structure(	POP<Solution_type>& pop,
							const std::pair<int, int>& range,
							PARAMETERS::Params& param)
		{
			
			for (int i = range.first; i < range.second; i++) {
				Solution_type& target = pop.population[i];

				if (target.SI_trans.is_empty()) {
					continue;
				}

				std::unordered_set<std::vector<int>> neighbors;
				for (auto& a : target.SI_trans.bad_scenario_set) {
				//for(int a = 0; a < param.scenario_num; a++){
					//auto temp = Find_Neighbor_by_CriticalPath(target, a, param);
					auto temp = NEIGHBOUR::CriticalPath_based_neighbor(target, a, param);
					neighbors.insert(temp.begin(), temp.end());
				}
				find_best_neighbor(neighbors, target, param);
			}
		}


	private:

		/*
			Find best neighbor, decode

			update next state

			update q_table

			current_state for NB_g and WSM_g

			target is to calculate new solution by
		
		*/
		
		void find_best_neighbor_and_update_q_table(	std::vector<std::vector<int>> &neighbors,
													POP<Solution_type> &pop,	
													Solution_type &target,	
													std::vector<std::vector<float>>& q_table, 
													int g, 
													int current_state,
													int current_action,
													PARAMETERS::Params& param) 
		{
			std::vector<Solution_type> neighborSolution;
			// All the generated neighbor
			for (auto& s : neighbors) {
				neighborSolution.emplace_back(s);
				neighborSolution.rbegin()->calculate_scenario_makespan(param);
			}
			// Judge if the neighbor is better than the solution after global search
			for (auto& s : neighborSolution) {
				if (s.SI.Penalty_of_TBS < target.SI_trans.Penalty_of_TBS) {
					target.improve_trans(s);
				}
			}


			//update q_table
			int DeltaNB = target.SI_trans.bad_scenario_num - target.SI.bad_scenario_num;
			int DeltaWSM = target.SI_trans.worst_scenario_makespan - target.SI.worst_scenario_makespan;
			float reward = 0;

			int nextstate = -1;

			if (DeltaNB > 0 && DeltaWSM > 0) {
				//reward = (float)DeltaNB * DeltaWSM * (2 * g + 1) / max_gen;
				reward = 2;
				nextstate = 0;
			}
			else if (DeltaNB > 0 && DeltaWSM <= 0) {
				//reward = (float)DeltaNB * (2 * g + 1) / max_gen;
				reward = 1;
				nextstate = 1;

			}
			else if (DeltaNB <= 0 && DeltaWSM > 0) {
				nextstate = 2;
				//reward = (float)DeltaWSM * (2 * g + 1) / max_gen;
				reward = 1;
			}
			else {
				nextstate = 3;
				/*
				reward = (float)DeltaWSM * DeltaNB * (2 * g + 1) / max_gen;
				if (reward > 0) reward *= -1;
				if (reward == 0) reward = -1;
				*/
				reward = -2;
			}

			ASSERT_MSG(nextstate >= 0, "Wrong next state calculated");
			// update Q-table
			q_table[current_state][current_action] = (1 - param.alpha) * q_table[current_state][current_action] +
				param.alpha * (reward + param.gamma * (*std::max_element(q_table[nextstate].begin(), q_table[nextstate].end())));
			
			target.last_state = nextstate;
		}

		/*
			For UN structure
		*/
		void find_best_neighbor(std::unordered_set<std::vector<int>>& neighbors,
								Solution_type& target,
								PARAMETERS::Params& param) 
		{
			for (auto& nei : neighbors) {
				Solution_type temp(nei);
				temp.calculate_scenario_makespan(param);
				if (temp.SI.Penalty_of_TBS < target.SI_trans.Penalty_of_TBS) {
					target.improve_trans(temp);
				}
			}
		}


	};
}


namespace LS_Version1 {
	template<template<typename> class POP, typename Solution_type>
	struct CRITICAL_PATH_version1 {
		/*
		void operator()(POP<Solution_type>& pop,
						const std::vector<int>& chosen_solution,
						SCENARIO_STRUCTURE &SS,
						int generation,
						int max_gen)
		{
			if (SS.str == "LN") {
				LN_structure(pop, chosen_solution, SS.q_table, generation, max_gen);
			}

			if (SS.str == "UN") {
				UN_structure(pop, chosen_solution);
			}
		}
		*/


		//for bi-population
		void operator()(POP<Solution_type>& pop,
			const std::vector<int>& chosen_solution,
			PARAMETERS::Params& param,
			int scenario_id) {

			for (int i = 0; i < chosen_solution.size(); i++) {
				pop.population[chosen_solution[i]].calculate_transition_scenario_makespan(param);
			}

			// sort and repartition
			std::sort(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				SCENARIO_INFO aa = a.SI_trans.is_empty() ? a.SI : a.SI_trans;
				SCENARIO_INFO bb = b.SI_trans.is_empty() ? b.SI : b.SI_trans;

				return aa.bad_scenario_num < bb.bad_scenario_num;
				});
			/*
			// FOR DEBUGGING
			{
				std::cout << "The BSN after sort is ";
				for (auto i : pop.population) {
					if (i.SI_trans.is_empty()) {
						std::cout << i.SI.bad_scenario_num << " ";
					}
					else {
						std::cout << "trans" << i.SI_trans.bad_scenario_num << " ";
					}

				}
				std::cout << std::endl;
			}
			*/


			int demarcation = int(param.beta * param.scenario_num);

			auto it = std::upper_bound(pop.population.begin(), pop.population.end(), demarcation,
				[](const int demarcation, const auto& individual) {
					SCENARIO_INFO scenario = individual.SI_trans.is_empty() ? individual.SI : individual.SI_trans;
					return demarcation < scenario.bad_scenario_num;
				});


			// [idx,end] for LN
			// [0,idx) for  UN
			int idx = std::distance(pop.population.begin(), it);

			if (param.beta) // not work for pureLN
			{
				bool have_job = false;
				for (int i = 0; i < idx; i++) {
					if (!pop.population[i].SI_trans.is_empty()) {
						have_job = true;
						break;
					}
				}

				if (!have_job) {
					//if no job is handled by UN, force to align one

					for (int i = idx; i < param.pop_size; i++) {
						if (!pop.population[i].SI_trans.is_empty()) {
							idx = i + 1;
							break;
						}
					}
				}

			}



			std::pair<int, int> range_UN{ 0,idx };
			std::pair<int, int> range_LN{ idx,param.pop_size };

			LN_structure(pop, range_LN, param, scenario_id);

			UN_structure(pop, range_UN, param);


			/*
				此时population中应该包含了如下信息

				未被选中的个体仅有SI和sequence的相关信息
				被选中的个体还拥有SI_trans和trans_sequence的相关信息
			*/
		}


		void LN_structure(POP<Solution_type>& pop,
			const std::pair<int, int>& range,
			PARAMETERS::Params& param,
			int scenario_id)
		{
			if (range.second <= range.first) {
				return;
			}
			for (int i = range.first; i < range.second; i++) {
				if (pop.population[i].SI_trans.is_empty()) {
					continue;
				}

				Solution_type& target = pop.population[i];
				int action_scen = scenario_id;

				// choose best scenario
				std::vector<std::vector<int>> neighbors = NEIGHBOUR::CriticalPath_based_neighbor(target, action_scen, param);
				find_best_neighbor(neighbors, target, param);

			}
		}

		void UN_structure(POP<Solution_type>& pop,
			const std::pair<int, int>& range,
			PARAMETERS::Params& param)
		{

			for (int i = range.first; i < range.second; i++) {
				Solution_type& target = pop.population[i];

				if (target.SI_trans.is_empty()) {
					continue;
				}

				std::unordered_set<std::vector<int>> neighbors;
				for (auto& a : target.SI_trans.bad_scenario_set) {
					//for(int a = 0; a < param.scenario_num; a++){
						//auto temp = Find_Neighbor_by_CriticalPath(target, a, param);
					auto temp = NEIGHBOUR::CriticalPath_based_neighbor(target, a, param);
					neighbors.insert(temp.begin(), temp.end());
				}
				find_best_neighbor(neighbors, target, param);
			}
		}
	private:

		void find_best_neighbor(std::unordered_set<std::vector<int>>& neighbors,
			Solution_type& target,
			PARAMETERS::Params& param)
		{
			while (neighbors.size() > 3) {
				auto it = neighbors.begin();  
				neighbors.erase(it); 
			}
			for (auto& nei : neighbors) {
				Solution_type temp(nei);
				temp.calculate_scenario_makespan(param);
				if (temp.SI.Penalty_of_TBS < target.SI_trans.Penalty_of_TBS) {
					target.improve_trans(temp);
				}
			}
		}

		void find_best_neighbor(std::vector<std::vector<int>>& neighbors,
			Solution_type& target,
			PARAMETERS::Params& param)
		{
			while (neighbors.size() > 3) {
				auto it = neighbors.begin();  
				neighbors.erase(it);  
			}
			for (auto& nei : neighbors) {
				Solution_type temp(nei);
				temp.calculate_scenario_makespan(param);
				if (temp.SI.Penalty_of_TBS < target.SI_trans.Penalty_of_TBS) {
					target.improve_trans(temp);
				}
			}
		}


	};


	int get_state(std::vector<int>& last_best_BSN, std::vector<int>& last_best_WSM) {
		//update q_table
		int last = last_best_BSN.size() - 1;
		ASSERT_MSG(last >= 1, "g <= 1");
		ASSERT_MSG(last_best_BSN.size() == last_best_BSN.size(), "The trend is not recorded correctly");
		int DeltaBSN = last_best_BSN[last] - last_best_BSN[last - 1];
		int DeltaWSM = last_best_WSM[last] - last_best_WSM[last - 1];

		if (DeltaBSN > 0 && DeltaWSM > 0) {
			return 0;
		}
		else if (DeltaBSN > 0 && DeltaWSM <= 0) {
			return 1;

		}
		else if (DeltaBSN <= 0 && DeltaWSM > 0) {
			return 2;
		}
		else {
			return 3;
		}


		/*
		ASSERT_MSG(nextstate >= 0, "Wrong next state calculated");
		// update Q-table
		q_table[current_state][current_action] = (1 - param.alpha) * q_table[current_state][current_action] +
			param.alpha * (reward + param.gamma * (*std::max_element(q_table[nextstate].begin(), q_table[nextstate].end())));
		*/
	}
	void Update_Qtable_Version1(std::vector<int>& last_BSN,
		std::vector<int>& last_WSM,
		std::vector<std::vector<float>>& q_table,
		int current_state,
		int current_action,
		Population<NWFSP_Solution>& pop,
		long long& lastPT,
		PARAMETERS::Params& param,
		int g)
	{


		auto tmp = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
			return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
			});

		last_BSN.push_back(tmp->SI.bad_scenario_num);
		last_WSM.push_back(tmp->SI.worst_scenario_makespan);

		//update q_table
		int nextstate = get_state(last_BSN, last_WSM);

		//float reward = (float)(lastPT - tmp->SI.Penalty_of_TBS) * 2 * g / param.max_gen;

		float reward = (float)(lastPT - tmp->SI.Penalty_of_TBS);

		// update Q-table
		q_table[current_state][current_action] = (1 - param.alpha) * q_table[current_state][current_action] +
			param.alpha * (reward + param.gamma * (*std::max_element(q_table[nextstate].begin(), q_table[nextstate].end())));

		lastPT = tmp->SI.Penalty_of_TBS;
	}
}





namespace LS_Version2 {
	int get_state(std::vector<int>& last_BSN, std::vector<int>& last_WSM) {
		//update q_table
		int last = last_BSN.size() - 1;
		ASSERT_MSG(last >= 1, "g <= 1");
		ASSERT_MSG(last_BSN.size() == last_BSN.size(), "The trend is not recorded correctly");
		int DeltaBSN = last_BSN[last] - last_BSN[last - 1];
		int DeltaWSM = last_WSM[last] - last_WSM[last - 1];

		if (DeltaBSN > 0 && DeltaWSM > 0) {
			return 0;
		}
		else if (DeltaBSN > 0 && DeltaWSM <= 0) {
			return 1;

		}
		else if (DeltaBSN <= 0 && DeltaWSM > 0) {
			return 2;
		}
		else {
			return 3;
		}
	}
	void Update_Qtable(std::vector<int>& last_BSN,
		std::vector<int>& last_WSM,
		std::vector<std::vector<float>>& q_table,
		int current_state,
		int current_action,
		Population<NWFSP_Solution>& pop,
		long long& lastPT,
		PARAMETERS::Params& param,
		int g)
	{

		{
			auto tmp = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			last_BSN.push_back(tmp->SI.bad_scenario_num);
			last_WSM.push_back(tmp->SI.worst_scenario_makespan);
		}
		//update q_table
		int nextstate = get_state(last_BSN, last_WSM);
		float reward;
		{
			
			auto tmp = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			//reward = (float)(lastPT - tmp->SI.Penalty_of_TBS);
			reward = (float)(lastPT - tmp->SI.Penalty_of_TBS) * 2 * g / param.max_gen;
			lastPT = tmp->SI.Penalty_of_TBS;
		}
		 

		// update Q-table
		q_table[current_state][current_action] = (1 - param.alpha) * q_table[current_state][current_action] +
			param.alpha * (reward + param.gamma * (*std::max_element(q_table[nextstate].begin(), q_table[nextstate].end())));
	}
}




namespace LS_Version3 {
	int get_state(std::vector<int>& last_BSN, std::vector<int>& last_WSM) {
		//update q_table
		int last = last_BSN.size() - 1;
		ASSERT_MSG(last >= 1, "g <= 1");
		ASSERT_MSG(last_BSN.size() == last_BSN.size(), "The trend is not recorded correctly");
		int DeltaBSN = last_BSN[last] - last_BSN[last - 1];
		int DeltaWSM = last_WSM[last] - last_WSM[last - 1];

		if (DeltaBSN > 0 && DeltaWSM > 0) {
			return 0;
		}
		else if (DeltaBSN > 0 && DeltaWSM <= 0) {
			return 1;

		}
		else if (DeltaBSN <= 0 && DeltaWSM > 0) {
			return 2;
		}
		else {
			return 3;
		}
	}
	void Update_Qtable(std::vector<int>& last_BSN,
		std::vector<int>& last_WSM,
		std::vector<std::vector<float>>& q_table,
		int current_state,
		int current_action,
		Population<NWFSP_Solution>& pop,
		long long& lastPT,
		PARAMETERS::Params& param,
		int g, 
		int &SameBSNCount)
	{

		{
			auto tmp = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			if (tmp->SI.bad_scenario_num == *last_BSN.rbegin()) {
				SameBSNCount++;
			}
			else {
				SameBSNCount = 0;
			}

			if (SameBSNCount >= param.P) {
				int currentLen = tmp->BSN.size() - 1;
				int t = generate_random_int(0, param.pop_size);
				tmp = pop.population.begin();
				std::advance(tmp, t);
			}
			last_BSN.push_back(tmp->SI.bad_scenario_num);
			last_WSM.push_back(tmp->SI.worst_scenario_makespan);
		}
		//update q_table
		int nextstate = get_state(last_BSN, last_WSM);
		float reward;
		{

			auto tmp = std::min_element(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			//reward = (float)(lastPT - tmp->SI.Penalty_of_TBS);
			//reward = (float)(lastPT - tmp->SI.Penalty_of_TBS) * 2 * g / param.max_gen;

			if (lastPT > tmp->SI.Penalty_of_TBS) {
				reward = 2;
			}
			else {
				reward = -1;
			}
			lastPT = tmp->SI.Penalty_of_TBS;
		}


		// update Q-table
		q_table[current_state][current_action] = (1 - param.alpha) * q_table[current_state][current_action] +
			param.alpha * (reward + param.gamma * (*std::max_element(q_table[nextstate].begin(), q_table[nextstate].end())));
	}
}


















namespace LS_Version4 {
	template<template<typename> class POP, typename Solution_type>
	struct CRITICAL_PATH {
		/*
		void operator()(POP<Solution_type>& pop,
						const std::vector<int>& chosen_solution,
						SCENARIO_STRUCTURE &SS,
						int generation,
						int max_gen)
		{
			if (SS.str == "LN") {
				LN_structure(pop, chosen_solution, SS.q_table, generation, max_gen);
			}

			if (SS.str == "UN") {
				UN_structure(pop, chosen_solution);
			}
		}
		*/


		//for bi-population
		void operator()(POP<Solution_type>& pop,
			const std::vector<int>& chosen_solution,
			PARAMETERS::Params& param,
			int scenario_id) {

			for (int i = 0; i < chosen_solution.size(); i++) {
				pop.population[chosen_solution[i]].calculate_transition_scenario_makespan(param);
			}

			// sort and repartition
			std::sort(pop.population.begin(), pop.population.end(), [](auto& a, auto& b) {
				SCENARIO_INFO aa = a.SI_trans.is_empty() ? a.SI : a.SI_trans;
				SCENARIO_INFO bb = b.SI_trans.is_empty() ? b.SI : b.SI_trans;

				return aa.bad_scenario_num < bb.bad_scenario_num;
				});
			/*
			// FOR DEBUGGING
			{
				std::cout << "The BSN after sort is ";
				for (auto i : pop.population) {
					if (i.SI_trans.is_empty()) {
						std::cout << i.SI.bad_scenario_num << " ";
					}
					else {
						std::cout << "trans" << i.SI_trans.bad_scenario_num << " ";
					}

				}
				std::cout << std::endl;
			}
			*/


			int demarcation = int(param.beta * param.scenario_num);

			auto it = std::upper_bound(pop.population.begin(), pop.population.end(), demarcation,
				[](const int demarcation, const auto& individual) {
					SCENARIO_INFO scenario = individual.SI_trans.is_empty() ? individual.SI : individual.SI_trans;
					return demarcation < scenario.bad_scenario_num;
				});


			// [idx,end] for LN
			// [0,idx) for  UN
			int idx = std::distance(pop.population.begin(), it);

			if (param.beta) // not work for pureLN
			{
				bool have_job = false;
				for (int i = 0; i < idx; i++) {
					if (!pop.population[i].SI_trans.is_empty()) {
						have_job = true;
						break;
					}
				}

				if (!have_job) {
					//if no job is handled by UN, force to align one

					for (int i = idx; i < param.pop_size; i++) {
						if (!pop.population[i].SI_trans.is_empty()) {
							idx = i + 1;
							break;
						}
					}
				}

			}



			std::pair<int, int> range_UN{ 0,idx };
			std::pair<int, int> range_LN{ idx,param.pop_size };

			LN_structure(pop, range_LN, param, scenario_id);

			UN_structure(pop, range_UN, param);


			/*
				此时population中应该包含了如下信息

				未被选中的个体仅有SI和sequence的相关信息
				被选中的个体还拥有SI_trans和trans_sequence的相关信息
			*/
		}


		void LN_structure(POP<Solution_type>& pop,
			const std::pair<int, int>& range,
			PARAMETERS::Params& param,
			int scenario_id)
		{
			if (range.second <= range.first) {
				return;
			}
			for (int i = range.first; i < range.second; i++) {
				if (pop.population[i].SI_trans.is_empty()) {
					continue;
				}

				Solution_type& target = pop.population[i];
				int action_scen = scenario_id;

				// choose best scenario
				std::vector<std::vector<int>> neighbors = NEIGHBOUR::VNS_neighbor(target,param);
				find_best_neighbor(neighbors, target, param);

			}
		}

		void UN_structure(POP<Solution_type>& pop,
			const std::pair<int, int>& range,
			PARAMETERS::Params& param)
		{

			for (int i = range.first; i < range.second; i++) {
				Solution_type& target = pop.population[i];

				if (target.SI_trans.is_empty()) {
					continue;
				}

				std::unordered_set<std::vector<int>> neighbors;
				for (auto& a : target.SI_trans.bad_scenario_set) {
					//for(int a = 0; a < param.scenario_num; a++){
						//auto temp = Find_Neighbor_by_CriticalPath(target, a, param);
					auto temp = NEIGHBOUR::VNS_neighbor(target,param);
					neighbors.insert(temp.begin(), temp.end());
				}
				find_best_neighbor(neighbors, target, param);
			}
		}
	private:

		void find_best_neighbor(std::unordered_set<std::vector<int>>& neighbors,
			Solution_type& target,
			PARAMETERS::Params& param)
		{
			for (auto& nei : neighbors) {
				Solution_type temp(nei);
				temp.calculate_scenario_makespan(param);
				if (temp.SI.Penalty_of_TBS < target.SI_trans.Penalty_of_TBS) {
					target.improve_trans(temp);
				}
			}
		}

		void find_best_neighbor(std::vector<std::vector<int>>& neighbors,
			Solution_type& target,
			PARAMETERS::Params& param)
		{
			for (auto& nei : neighbors) {
				Solution_type temp(nei);
				temp.calculate_scenario_makespan(param);
				if (temp.SI.Penalty_of_TBS < target.SI_trans.Penalty_of_TBS) {
					target.improve_trans(temp);
				}
			}
		}


	};

}

