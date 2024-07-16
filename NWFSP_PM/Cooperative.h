#pragma once
# include<iostream>
# include<algorithm>
# include<vector>
# include"myutils.h"
namespace COOPERATIVE{
	/*
		Calculate each solution's score 

		Merge and repartitate to pop_r and pop_t
	*/
	template<template<typename> class POP, typename Solution_type>
	struct Calculate_Score_and_Repartition {
		void operator()(POP<Solution_type>& Pop_r, POP<Solution_type>& Pop_t, int g) {
			const float alpha = 0.1;

			int PT_max_pop_r = std::max_element(Pop_r.population.begin(), Pop_r.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;
			int PT_min_pop_r = std::min_element(Pop_r.population.begin(), Pop_r.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;
			int PT_max_pop_t = std::max_element(Pop_t.population.begin(), Pop_t.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;
			int PT_min_pop_t = std::min_element(Pop_t.population.begin(), Pop_t.population.end(), [](auto& a, auto& b) {
				return a.SI.Penalty_of_TBS < b.SI.Penalty_of_TBS;
				})->SI.Penalty_of_TBS;


			// calculate score
			if (g == 0) {
				ASSERT_MSG(Pop_r.pop_size == Pop_t.pop_size, "Wrong Pop size setting");
				for (int i = 0; i < Pop_r.pop_size; i++) {
					auto& s_r = Pop_r.population[i];
					auto& s_t = Pop_t.population[i];

					float score_r = (float)(PT_max_pop_r - s_r.SI.Penalty_of_TBS) / (PT_max_pop_r - PT_min_pop_r);
					float score_t = (float)(PT_max_pop_t - s_t.SI.Penalty_of_TBS) / (PT_max_pop_t - PT_min_pop_t);


					s_r.score.push_back(score_r);
					s_t.score.push_back(score_t);
					s_r.PT.push_back(s_r.SI.Penalty_of_TBS);
					s_t.PT.push_back(s_t.SI.Penalty_of_TBS);
				}
			}
			else {
				for (int i = 0; i < Pop_r.pop_size; i++) {
					auto& s_r = Pop_r.population[i];
					auto& s_t = Pop_t.population[i];

					float weighted_hist_r = (*s_r.score.rbegin()) * (1 - alpha);
					float weighted_hist_t = (*s_t.score.rbegin()) * (1 - alpha);

					float self_improv_r = (float)(s_r.SI.Penalty_of_TBS - (*s_r.PT.rbegin())) / (*s_r.PT.rbegin());
					float self_improv_t = (float)(s_t.SI.Penalty_of_TBS - (*s_t.PT.rbegin())) / (*s_t.PT.rbegin());

					float relative_solution_quality_r = alpha * (PT_max_pop_r - s_r.SI.Penalty_of_TBS) / (PT_max_pop_r - PT_min_pop_r);
					float relative_solution_quality_t = alpha * (PT_max_pop_t - s_t.SI.Penalty_of_TBS) / (PT_max_pop_t - PT_min_pop_t);

					float score_r = weighted_hist_r + relative_solution_quality_r + self_improv_r;
					float score_t = weighted_hist_t + relative_solution_quality_t + self_improv_t;

					s_r.PT.push_back(s_r.SI.Penalty_of_TBS);
					s_t.PT.push_back(s_t.SI.Penalty_of_TBS);
				}
			}

			// merge 
			std::vector<Solution_type> merged;
			merged.reserve(Pop_r.population.size() + Pop_t.population.size());
			ASSERT_MSG(Pop_r.population.size() == Pop_t.population.size(), "Wrong Pop_size in each pop");

			merged.insert(merged.end(), Pop_r.population.begin(), Pop_r.population.end());
			merged.insert(merged.end(), Pop_t.population.begin(), Pop_t.population.end());

			std::sort(merged.begin(), merged.end(), [](auto& a, auto& b) {
				return *a.score.rbegin() > *b.score.rbegin();
				});

			Pop_r.population.assign(merged.begin(), merged.begin() + Pop_r.pop_size);
			Pop_t.population.assign(merged.begin() + Pop_r.pop_size, merged.end());
			ASSERT_MSG(Pop_r.population.size() == Pop_r.pop_size, "Wrong repartition");
			ASSERT_MSG(Pop_t.population.size() == Pop_t.pop_size, "Wrong repartition");


		}
	};
}