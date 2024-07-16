#pragma once
#include<vector>
#include<random>
#include<utility>
#include"myutils.h"
#include<set>




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




}


