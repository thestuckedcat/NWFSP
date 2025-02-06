#pragma once
#include<vector>
#include<random>
#include<utility>
#include"myutils.h"
#include<set>
#include"Parameter.h"



namespace GLOBAL_SEARCH {
	/*
		INPUT: Target population, selection rate
		
		OUTPUT: chosen_idx
	*/
	template<template<typename> class POP, typename Solution_type>
	struct SELECTION {
		void operator()(const POP<Solution_type>& pop, float selection_rate, std::vector<int>& chosen_idx)
		{
			chosen_idx.clear();
			for (int i = 0; i < pop.population.size(); i++) {
				if (generate_random_float() < selection_rate) {
					chosen_idx.push_back(i);
				}
			}
		}
		void operator()(const POP<Solution_type>& pop , std::vector<int>& chosen_idx, const PARAMETERS::Params& param)
		{
			chosen_idx.clear();
			for (int i = 0; i < param.pop_size; i++) {
				if (generate_random_float() < param.selection_rate) {
					chosen_idx.push_back(i);
				}
			}			
		}

		
		void operator()(const POP<Solution_type>& pop, std::vector<int>& chosen_idx, const PARAMETERS::Params& param, std::string str) {
			//Ternary Tournament Selection
			if (str == "TTS") {
				std::vector<bool> is_chosen(param.job_num,false);

				for (int i = 0; i < param.pop_size; i++) {
					int J1 = generate_random_int(0, param.job_num);
					int J2 = generate_random_int(0, param.job_num);
					int J3 = generate_random_int(0, param.job_num);

					long long PT1 = pop.population[J1].SI.Penalty_of_TBS;
					long long PT2 = pop.population[J2].SI.Penalty_of_TBS;
					long long PT3 = pop.population[J3].SI.Penalty_of_TBS;
					if (PT1 <= PT2 && PT1 <= PT3) {
						is_chosen[J1] = true;
					}
					else if (PT2 <= PT1 && PT2 <= PT3) {
						is_chosen[J2] = true;
					}
					else if (PT3 <= PT1 && PT3 <= PT2) {
						is_chosen[J3] = true;
					}
				}

				for (int i = 0; i < param.job_num; i++) {
					if (is_chosen[i]) chosen_idx.push_back(i);
				}
			}
			//std::cout << "done" << std::endl;

		}

	};





	/*
		INPUT: pop, chosen index
	
		OUTPUT: change the "transition sequence" in the corresponding solution
	*/
	template<template<typename> class POP, typename Solution_type>
	struct CROSSOVER {
		void operator()(POP<Solution_type>& pop, const std::vector<int> &chosen_solution) {
			// for each chosen solution
			for (int i = 0; i < chosen_solution.size(); i++) {
				// randomly pick another parent
				int other = generate_random_int(0,pop.population.size());

				while (other == chosen_solution[i]) {
					other = generate_random_int(0, pop.population.size());
				}

				// do crossover
				std::vector<int> Cans = crossover(pop.population[chosen_solution[i]], pop.population[other]);

				// Get transition solution
				pop.population[chosen_solution[i]].trans_sequence = std::move(Cans);
			}
		}

		void operator()(POP<Solution_type>& pop, const std::vector<int>& chosen_solution, const PARAMETERS::Params& param) {
			// for each chosen solution
			for (int i = 0; i < chosen_solution.size(); i++) {
				// randomly pick another parent
				int other = generate_random_int(0, param.pop_size);

				while (other == chosen_solution[i]) {
					other = generate_random_int(0, param.pop_size);
				}

				// do crossover, obtain new solution
				std::vector<int> Cans = crossover(pop.population[chosen_solution[i]], pop.population[other]);

				// Set transition solution
				pop.population[chosen_solution[i]].trans_sequence = std::move(Cans);
			}
		}


		

		

	private:
		/*
			crossover for the given solution
			
			return the result of crossover

			A random subsequence of target is chosen, mother's element is filled in order

		*/
		std::vector<int> crossover(Solution_type target, Solution_type mother) {
			// define an interval
			int begin = generate_random_int(0, target.sequence.size());
			int end = generate_random_int(0, target.sequence.size());
			if (begin > end) {
				std::swap(begin, end);
			}
			
			ASSERT_MSG(begin <= end && begin >= 0 && end < target.sequence.size(), "Out of Bound");
			
			// crossover
			// vector that filled in
			std::vector<int> tmp(target.sequence.size(),-1);

			std::unordered_set<int> us;
			for (int i = begin; i <= end; i++) {
				us.insert(target.sequence[i]);
				tmp[i] = target.sequence[i];
			}

			int ptr_to_tmp = 0;
			for (int i = 0; i < target.sequence.size(); i++) {
				while (ptr_to_tmp < target.sequence.size() && tmp[ptr_to_tmp] != -1 ) {
					ptr_to_tmp++;
				}
				
				if (ptr_to_tmp < target.sequence.size() && us.find(mother.sequence[i]) == us.end())
					tmp[ptr_to_tmp] = mother.sequence[i];
			}

			ASSERT_MSG(std::all_of(tmp.begin(), tmp.end(), [](int num) {
				return num != -1;
				}), "WRONG crossover result: Some positions are not filled.");

			ASSERT_MSG(std::unordered_set<int>(tmp.begin(), tmp.end()).size() == tmp.size(), 
				"WRONG crossover result: Duplicates found in result.");

			return tmp;
		}

	};





	/*
		In each chosen solution

		a random job is extracted and inserted to a random position
	*/
	template<template<typename> class POP, typename Solution_type>
	struct MUTATION {
		void operator()(POP<Solution_type>& pop, const std::vector<int>& chosen_solution) {
			for (int i = 0; i < chosen_solution.size(); i++) {
				ASSERT_MSG(!pop.population[chosen_solution[i]].trans_sequence.empty(), 
					"transition sequence donot exist");

				pop.population[chosen_solution[i]].trans_sequence = mutation(pop.population[chosen_solution[i]].trans_sequence);
			}
		}

		void operator()(POP<Solution_type>& pop, const std::vector<int>& chosen_solution, const PARAMETERS::Params& param) {
			for (int i = 0; i < chosen_solution.size(); i++) {
				ASSERT_MSG(!pop.population[chosen_solution[i]].trans_sequence.empty(),
					"transition sequence donot exist");

				pop.population[chosen_solution[i]].trans_sequence = mutation(pop.population[chosen_solution[i]].trans_sequence);
			}
		}
	private:

		std::vector<int> mutation(const std::vector<int> &seq){
			// random remove position and insert position
			int rmv = generate_random_int(0, seq.size());
			int ins = generate_random_int(0, seq.size());
			if (ins > rmv) {
				ins--;
				ASSERT_MSG(ins >= 0, "WRONG INSERT POS IN MUTATION");
			}
			// remove process
			std::vector<int> res = seq;
			std::vector<int>::iterator it_rmv = res.begin();
			std::advance(it_rmv, rmv);
			int ele = *it_rmv;
			res.erase(it_rmv);

			// insert process
			std::vector<int>::iterator it_ins = res.begin();
			std::advance(it_ins, ins);
			res.insert(it_ins,ele);
			ASSERT_MSG(res.size() == seq.size(), "Mutation remove/insert error, size changed");

			return res;
		}
	};





	/*
		PBXcrossover
	*/
	template<template<typename> class POP, typename Solution_type>
	struct PBXCROSSOVER {
		void operator()(POP<Solution_type>& pop, const std::vector<int> chosen_solution) {
			auto Bestit = std::min_element(pop.population.begin(), pop.population.end(), [](const auto& a, const auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			std::vector<int> Best = Bestit->sequence;
			auto Worstit = std::max_element(pop.population.begin(), pop.population.end(), [](const auto& a, const auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				});
			std::vector<int> Worst = Worstit->sequence;

			for (int i = 0; i < chosen_solution.size(); i++) {
				pop.population[chosen_solution[i]].trans_sequence = crossover_PBX(Worst, Best, pop.population[chosen_solution[i]].sequence);
			}
		}
	private:
		/*
			It is a Position-Based Crossover

			1. The best individual and the worst individual is first found

			2. Delete the gene has the same pos with worst individual

			3. Fill the blank with the order of best individual
		*/

		std::vector<int> crossover_PBX(const std::vector<int>& Worst,
			const std::vector<int>& Best,
			const std::vector<int>& current)
		{

			ASSERT_MSG(Worst.size() == Best.size() && Best.size() == current.size(),
				"Input solution size has something wrong ");

			std::vector<bool> is_to_fill(current.size(), false);
			std::vector<int> res = current;
			// Delete gene has same pos
			for (int i = 0; i < current.size(); i++) {
				if (Worst[i] == current[i]) {
					is_to_fill[Worst[i]] = true;
					res[i] = -1;
				}
			}

			// Fill with Best individual
			int ptr_best = 0;
			int ptr_cur = 0;
			while (ptr_cur < current.size() && ptr_best < current.size()) {

				// find first ele to fill
				while (ptr_best < current.size() && !is_to_fill[Best[ptr_best]]) {
					ptr_best++;
				}

				// find first pos to fill
				while (ptr_cur < current.size() && res[ptr_cur] != -1) {
					ptr_cur++;
				}

				if (ptr_cur < current.size() && ptr_best < current.size())
				{
					res[ptr_cur] = Best[ptr_best];
					ptr_best++;
					ptr_cur++;
				}
			}


			return res;

		}
	};


	/*
		pairwise IG
	*/

	template<template<typename> class POP, typename Solution_type>
	struct Pairwise_IG {
		void operator()(POP<Solution_type>& pop, const std::vector<int> chosen_solution, PARAMETERS::Params& param,int scenario_idx) {
			for (auto i : chosen_solution) {
				std::vector<int>& solution = pop.population[i].trans_sequence;
				pop.population[i].trans_sequence = IG_Process(solution, 0.2, param,scenario_idx);
				pop.population[i].calculate_transition_scenario_makespan(param);
			}
		}


	private:
		std::vector<int> IG_Process(std::vector<int> Originsolution, float pair_num, PARAMETERS::Params& param,int scenario_idx) {
			int pairNum = 1;
			if (generate_random_float() > pair_num) {
				return Originsolution;
			}
			else {
				pairNum = std::max((int)pair_num, pairNum);
			}
			Solution_type Origin(Originsolution);
			Origin.calculate_scenario_makespan(param);

			std::vector<int> bestsolution = Originsolution;
			long long bestPT = Origin.SI.Penalty_of_TBS;
			for (int p = 0; p < pair_num; p++) {
				std::vector<int> solution = Originsolution;
				int first = generate_random_int(0, solution.size());
				int targetJob = solution[first];
				solution.erase(solution.begin() + first);

				int second = generate_random_int(0, solution.size());
				int targetJob2 = solution[second];
				solution.erase(solution.begin() + second);

				
				std::vector<int> bestsubSolution;
				long long bestSubMakespan = MAXLONGLONG;
				for (int i = 0; i <= solution.size(); i++) {
					std::vector<int> subSolution = solution;
					subSolution.insert(subSolution.begin()+i,targetJob);
					
					Solution_type temp(subSolution);
					long long makespan = temp.get_makespan(param, scenario_idx);
					if (makespan < bestSubMakespan) {
						bestsubSolution = subSolution;
						bestSubMakespan = makespan;
					}
				}

				for (int j = 0; j <= bestsubSolution.size(); j++) {
					std::vector<int> FinalSolution = bestsubSolution;
					FinalSolution.insert(FinalSolution.begin() + j, targetJob2);

					Solution_type temp(FinalSolution);
					temp.calculate_scenario_makespan(param);
					if (temp.SI.Penalty_of_TBS > bestPT) {
						bestPT = temp.SI.Penalty_of_TBS;
						bestsolution = FinalSolution;
					}

				}
			}
			return bestsolution;
		}
	};
}


