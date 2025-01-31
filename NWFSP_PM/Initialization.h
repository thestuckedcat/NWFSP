#pragma once
# include<random>
#include <algorithm>
#include<vector>
#include<set>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include "myutils.h"
#include <cstddef>
#include <cstdint>
#include"solution.h"
#include<numeric>
#include "OverloadTools.h"
#include "Parameter.h"



namespace INITIALIZATION {

    template<template<typename> class POP, typename Solution_type>
    struct SCENARIO_NEH_DOUBLE {
		/*
			Generate solution under every different scenario

			Fill the population with random generated solution 
		
		*/
		void operator()(PARAMETERS::Params &param, POP<Solution_type>& pop) {
			std::unordered_set<Solution_type> individuals;

			for (int i = 0; i < param.scenario_num; i++) {
				//individuals.insert(NEH_Generate(param.scenario[i]));
				individuals.insert(NEH_Generate(param, i));
			}

			if (individuals.size() < param.pop_size) {
				RANDOM_Generate(param.pop_size, individuals, param.job_num);
			}

			pop.Threshold = param.Threshold;

			for (auto i : individuals) {
				i.calculate_scenario_makespan(param);
				pop.population.push_back(i);
			}
			ASSERT_MSG(pop.population.size() == param.pop_size, "Not enough individuals generated");
		}

		/*
			OVERLOAD VERSION

			INPUT: pop_r, pop_t with the following INFO:

			1. pop_size

			2. job_num

			3. machine_num

			4. scenario_processing_time


			OUTPUT: pop_r, pop_t with

			std::vector<T> population

			Solution_index

			SI

		*/
		
        void operator()(POP<Solution_type>& pop_r, POP<Solution_type> & pop_t) {// pop_size for single population
			int Threshold = pop_r.Threshold;
			int pop_size = pop_r.pop_size;
			ASSERT_MSG(pop_r.pop_size == pop_t.pop_size, "Pop size error");
			ASSERT_MSG(pop_r.Threshold == pop_t.Threshold, "Threshold error");

            int scenario_num = pop_r.scenario_processing_time.size();
            assert(pop_t.scenario_processing_time.size() == pop_r.scenario_processing_time.size());

            // Generate from scenario using NEH, every scenario generate one solution
            std::unordered_set<Solution_type> individuals;
            for (int i = 0; i < scenario_num; i++) {
                //individuals.insert(NEH_Generate(pop_r.scenario_processing_time[i]));
				individuals.insert(NEH_Generate(pop_r.scenario_processing_time[i]));
            }



			// Complement individuals
			if (individuals.size() < 2 * pop_size) {
				RANDOM_Generate(2 * pop_size, individuals, pop_r.job_num);
			}


			// random parititation
			ASSERT_MSG(individuals.size() >= 2 * pop_size, "Dont have enough individuals in pool");
			
			auto it = individuals.begin();
			for (int i = 0; i < pop_size && it != individuals.end(); i++, it++) {
				// unordered_set cannot be changed directly, 
				//it->calculate_scenario_makespan(pop_r.scenario_processing_time, Threshold);
				Solution_type temp = *it;
				temp.calculate_scenario_makespan(pop_r.scenario_processing_time, Threshold);
				pop_r.population.push_back(temp);
			}
			for (int i = 0; i < pop_size && it != individuals.end(); i++, it++) {
				//it->calculate_scenario_makespan(pop_r.scenario_processing_time, Threshold);
				Solution_type temp = *it;
				temp.calculate_scenario_makespan(pop_r.scenario_processing_time, Threshold);
				pop_t.population.push_back(temp);
			}
			pop_r.Threshold = Threshold;
			pop_t.Threshold = Threshold;
			

        }

	private:
		/*
			NEH step 
	
			INPUT: Porcessing time under this scenario

			1. Sort in descending order

			2. Sequentially insert into sequence

			OUTPUT: return a SOLUTION with the following INFO
				
				1. machine_num

				2. job_num

				3. makespan

				4. sequence

				5. sequence_info

			THE FOLLOWING INFO is not dealed with:
				
				1. Solution_index

				2. SI
		*/
		void NEH(std::vector<int>& current_seq, int job_idx, std::vector<std::vector<int>>& pt, PARAMETERS::Params& param, int scenario_id) {
			std::vector<int> best_seq;
			int best_makespan = INT_MAX;
			for (int i = 0; i <= current_seq.size(); i++) {
				
				std::vector<int> temp = current_seq;
				//PRINT_VECTOR(temp, "Before insertion");

				auto it = temp.begin();
				std::advance(it, i);
				temp.insert(it, job_idx);
				//PRINT_VECTOR(temp, "After insertion");

				// decode
				Solution_type subseq(temp);
				int ms = subseq.get_makespan(param, scenario_id);
				if ( ms < best_makespan) {
					best_makespan = ms;
					best_seq = temp;
				}
			}

			//PRINT_VECTOR(best_seq, "Result insertion");
			current_seq = best_seq;
		}

		Solution_type NEH_Generate(PARAMETERS::Params& param, int scenario_id) {
			auto& pt = param.scenario[scenario_id];
			int job_num = pt.size();
			int machine_num = pt[0].size();



			// calculate total processing time of each job
			// pair: index,sum of pt[index]
			std::vector<std::pair<int, int>> sum_indicies(job_num);
			for (int i = 0; i < job_num; i++) {
				int sum = std::accumulate(pt[i].begin(), pt[i].end(), 0);
				sum_indicies[i] = { i,sum };
			}

			// sort in descending order
			std::sort(sum_indicies.begin(), sum_indicies.end(), [](const auto& a, const auto& b) {
				return a.second > b.second;
				});



			// NEH generation process, 
			// *****the insertion-decode process can be optimized later****
			std::vector<int> current_sequence;
			current_sequence.push_back(sum_indicies[0].first);
			for (int i = 1; i < job_num; i++)
			{
				NEH(current_sequence, sum_indicies[i].first, pt, param, scenario_id);
			}


			ASSERT_MSG(current_sequence.size() == job_num, "Not all the job been inserted");

			Solution_type res(current_sequence);
			return res;
		}



		/*
			Fill Random Solutions to "target" up to "num" size by shuffling permutation vector

			The inserted Solution is not decoded
		*/

		void RANDOM_Generate(int num, std::unordered_set<Solution_type>& target, int job_num) {
			std::vector<int> numbers(job_num);
			for (int i = 0; i < job_num; ++i) {
				numbers[i] = i;
			}

			std::random_device rd;
			std::mt19937 gen(rd());

			std::shuffle(numbers.begin(), numbers.end(), gen);


			while (target.size() < num) {
				target.insert(Solution_type(numbers));
				std::shuffle(numbers.begin(), numbers.end(), gen);
			}
		}
    };


	/*
		Q-table 
	*/
	template<template<typename> class POP, typename Solution_type>
	struct Q_TABLE {
		void operator()(const POP<Solution_type>& pop, std::vector<std::vector<float>>& q_table) {
			// state: 4		action: choose a single scenario
			q_table = std::vector<std::vector<float>>(4, std::vector<float>(pop.scenario_processing_time.size(), 0));
		}
		void operator()(std::vector<std::vector<float>>& q_table, PARAMETERS::Params& param) {
			q_table = std::vector<std::vector<float>>(4, std::vector<float>(param.scenario_num, 0));
		}
	};
}

