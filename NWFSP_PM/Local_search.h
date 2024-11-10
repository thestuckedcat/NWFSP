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
					if (epsilon > param.epsilon) {
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


		/*
			Use q_learning choose neighbor for this solution
		*/
		/*
		void LN_structure(	POP<Solution_type>& pop,
							const std::vector<int> chosen_solution,
							std::vector<std::vector<float>>& q_table,
							int generation,
							int max_gen) 
		{
			int scenario_num = pop.scenario_processing_time.size();
			ASSERT_MSG(scenario_num > 0, "scenario num wrongly setup");

			std::vector<std::vector<int>> neighbor4;

			if (generation == 0) {
				// randomly select a scenario
				for (int i = 0; i < chosen_solution.size(); i++) {
					int chosen_scenario = generate_random_int(0, scenario_num);
					neighbor4 = Find_Neighbor_by_CriticalPath(pop.population[chosen_solution[i]], pop.scenario_processing_time[chosen_scenario]);

					find_best_neighbor_and_update_q_table(neighbor4,
						pop,
						pop.population[chosen_solution[i]],
						q_table,
						generation,
						max_gen,
						pop.population[chosen_solution[i]].last_state,
						chosen_scenario);
				}
			}
			else {
				// chose scenario by q_table
				for (int i = 0; i < chosen_solution.size(); i++) {
					int epsilon = generate_random_float();
					Solution_type& target = pop.population[chosen_solution[i]];

					int state = target.last_state;
					int action_scen;

					// choose best scenario
					if (epsilon > 1 / std::sqrt(generation + 1)) {
						// pick largest
						auto it = std::max_element(q_table[state].begin(), q_table[state].end());
						action_scen = std::distance(q_table[state].begin(), it);
						ASSERT_MSG(action_scen >= 0 && action_scen < scenario_num, "Wrong action chosen");
					}
					// randomly choose
					else {
						//random pick
						action_scen = generate_random_int(0, scenario_num);
					}

					neighbor4 = Find_Neighbor_by_CriticalPath(target, pop.scenario_processing_time[action_scen]);
					find_best_neighbor_and_update_q_table(neighbor4,
						pop,
						target,
						q_table,
						generation,
						max_gen,
						state,
						action_scen);
				}
			}
		}
		*/
		/*
			generate neighbor in each scenario

			randomly choose solution do UN_structure, two possible implementations

				a. do UN search for all solution selected by SELECTION

				b. do UN search for part of solution selected by SELECTION

				c. do UN search for part of solutions in population
		
		void UN_structure(	POP<Solution_type>& pop,
							const std::vector<int> &chosen_solution)
		{
			
			// choose plan 1
			for (int i = 0; i < chosen_solution.size(); i++) {
				if (generate_random_float() < 0.1)
				{
					int solution_index = chosen_solution[i];
					std::unordered_set<std::vector<int>> neighbors;
					for (auto& a : pop.population[solution_index].SI.bad_scenario_set)
					{
						auto temp = Find_Neighbor_by_CriticalPath(pop.population[solution_index], pop.scenario_processing_time[a]);
						neighbors.insert(temp.begin(), temp.end());


					}
					find_best_neighbor(neighbors, pop, pop.population[solution_index]);
				}
			}
		}
		*/

	private:
		/*
			Set Operation Relation in a Graph
		*/
		std::vector<std::vector<graphNode>> Set_graph(	std::vector<int>& seq,
														std::vector<std::vector<node>>& seq_info) {

			int machine_num = seq_info.size();
			int job_num = seq_info[0].size();

			// Set Graph
			std::vector<std::vector<graphNode>> Graph(machine_num, std::vector<graphNode>(job_num));

			// Build Graph
			for (int m = 0; m < machine_num; m++) {
				for (int p = 0; p < job_num; p++) {
					int job_id = seq[p];
					Graph[m][p] = graphNode(m, p, job_id);

					// Judge if the operation of the following job on the same machine is available
					if (p + 1 < job_num && seq_info[m][p].end == seq_info[m][p + 1].begin) {
						Graph[m][p].add_neighbor(m, p + 1);
					}

					// Following no-wait constraint, the operation on the next machine of the same job is always available
					if (m + 1 < machine_num) {
						Graph[m][p].add_neighbor(m + 1, p);
					}
					if (m > 0) {
						Graph[m][p].add_neighbor(m - 1, p);
					}
				}


			}

			return Graph;
		}

		/*
			Find Critical Path in Graph

			Return these operations

		*/


		
		// recursive version
		bool dfs(const std::vector<std::vector<graphNode>>& Graph,
			int machine_id, int pos,
			std::stack<std::pair<int, int>>& path,
			std::vector<std::vector<bool>>& visited) {
			//std::cout << "当前在" << machine_id << "  " << pos << std::endl;
			// if found
			if (machine_id == Graph.size() - 1 && pos == Graph[0].size() - 1) {
				//std::cout << "找到了终点" << std::endl;
				path.push({ machine_id, pos });
				return true;
			}


			visited[machine_id][pos] = true;
			path.push({ machine_id, pos });

			// find all neighbors of this node
			for (const auto& neighbor : Graph[machine_id][pos].neighbor) {
				int next_machine_id = neighbor.first;
				int next_pos = neighbor.second;

				if (!visited[next_machine_id][next_pos]) {
					if (dfs(Graph, next_machine_id, next_pos, path, visited)) {
						return true;
					}
				}
			}
			// if no return occurs, means this node is unavailable
			path.pop();
			return false;
		}
		
		/*
		// 定义栈元素结构，包含当前节点及其下一个待访问的邻居索引
		struct StackElement {
			int machine_id;
			int pos;
			size_t next_neighbor_idx; // 下一个待访问的邻居索引
		};

		// 非递归DFS函数
		bool dfs(const std::vector<std::vector<graphNode>>& Graph,
						 int start_machine_id, int start_pos,
						 std::stack<std::pair<int, int>>& path,
						 std::vector<std::vector<bool>>& visited) {
			// 初始化遍历栈，并将起始节点压入栈中
			std::stack<StackElement> traversalStack;
			traversalStack.push(StackElement{start_machine_id, start_pos, 0});

			while (!traversalStack.empty()) {
				StackElement& current = traversalStack.top();
				int machine_id = current.machine_id;
				int pos = current.pos;

				// 如果当前节点尚未被访问
				if (!visited[machine_id][pos]) {
					visited[machine_id][pos] = true;
					path.push({machine_id, pos});
            
					// 检查是否到达终点
					if (machine_id == Graph.size() - 1 && pos == Graph[0].size() - 1) {
						return true;
					}
				}

				// 获取当前节点的邻居列表
				const auto& neighbors = Graph[machine_id][pos].neighbor;

				// 如果还有未访问的邻居，继续遍历
				if (current.next_neighbor_idx < neighbors.size()) {
					// 获取下一个邻居
					auto neighbor = neighbors[current.next_neighbor_idx];
					current.next_neighbor_idx++; // 更新下一个待访问的邻居索引

					int next_machine_id = neighbor.first;
					int next_pos = neighbor.second;

					// 如果邻居未被访问，压入遍历栈
					if (!visited[next_machine_id][next_pos]) {
						traversalStack.push(StackElement{next_machine_id, next_pos, 0});
					}
				} else {
					// 当前节点的所有邻居都已访问，回溯
					traversalStack.pop();
					path.pop();
				}
			}

			// 如果栈空且未找到终点，返回失败
			return false;
		}
		*/
		

		/*
			find blocks through operations in Critical path
		*/
		std::vector<std::vector<std::pair<int, int>>> find_blocks(std::stack<std::pair<int, int>> old_path) {
			std::vector<std::vector<std::pair<int, int>>> blocks;

			// get block
			int last_machine_id = old_path.top().first;
			std::vector<std::pair<int, int>> block;

			for (; !old_path.empty();)
			{
				std::pair<int, int> this_operation = old_path.top();

				if (this_operation.first == last_machine_id) {
					block.push_back(this_operation);

				}
				else {
					blocks.push_back(block);
					block.clear();
					block.push_back(this_operation);
					last_machine_id = this_operation.first;

				}
				old_path.pop();

			}
			if (!block.empty()) {
				blocks.push_back(block);
			}

			


			ASSERT_MSG(!blocks.empty(), "No block found");
			return blocks;
		}

		/*
			find Operations in Critical Path
		*/
		std::stack<std::pair<int, int>> find_Operations_in_Critical_Path(const std::vector<std::vector<graphNode>> &Graph) {
			// Find shortest path, it should return several blocks with (machine_id,job_id) pair in it
			int machine_num = Graph.size();
			int job_num = Graph[0].size();
			// machine,pos
			std::stack<std::pair<int, int>> path;
			std::vector<std::vector<bool>> visited(machine_num, std::vector<bool>(job_num, false));
			// dfs find a path
			bool found = dfs(Graph, 0, 0, path, visited);

			/*
			// check if new dfs is correct 
			
			

			{
				std::vector<std::pair<int, int>> newpath;
				bool foundnew = CP::dfs(Graph, newpath);
				if (path.size() != newpath.size())
					std::cout << "新的dfs不对" << std::endl;


				std::stack<std::pair<int, int>> tempStack = path;
				for (auto it = newpath.rbegin(); it != newpath.rend(); it++) {
					if (*it != tempStack.top()) {
						std::cout << "新的dfs内容不对" << std::endl;
					}
					tempStack.pop();
				}
			}
			*/


			ASSERT_MSG(found, "Critical Path not Found");

			return path;
		}

		
		/*
			find Neighbor for Solution target

			INPUT:  Solution, processing_time

			OUTPUT: The potential neighbor set
		*/
		std::vector<std::vector<int>> Find_Neighbor_by_CriticalPath(Solution_type& target, int scenario_idx, PARAMETERS::Params& param)
		{

			ASSERT_MSG(!target.trans_sequence.empty(), "Trans sequence is empty");
			

			std::vector<std::vector<int>>& pt = param.scenario[scenario_idx];
			

			std::vector<std::vector<graphNode>> Graph = Set_graph(	target.trans_sequence, 
																	target.scenario_sequence_info_trans[scenario_idx]);

			std::stack<std::pair<int, int>> path = find_Operations_in_Critical_Path(Graph);
			ASSERT_MSG(!path.empty(), "Path is empty");
			// find blocks with (machine_id, pos) in it
			std::vector<std::vector<std::pair<int, int>>> blocks = find_blocks(path);

			

			// calculate each pos's contribution, find largest block, contribution, contribution is defined as weighted sum of critical operation
			std::vector<std::pair<int, int>> path_(path.size());
			int i = path.size()-1;
			while (i >= 0 && !path.empty()) {
				path_[i--] = path.top();
				path.pop();
			}
			

			// contribution[i]: contribution of job on pos i
			std::vector<int> contribution(param.job_num, 0);
			// if current machine is less than past machine ,a negative weight is given 
			for (int i = 0; i < path_.size(); i++) {
				int weight = 1;

				// judge if this operation is reverse
				if (i > 0 && path_[i].first < path_[i - 1].first) {
					weight = -1;
				}

				// judge if this operation should be ignored
				if (i - 1 >= 0 && i + 1 < param.job_num) {
					if (path_[i].first == path_[i - 1].first && path_[i].first == path_[i + 1].first + 1) {
						weight = 0;
					}

					if (path_[i].first == path_[i - 1].first-1 && path_[i].first == path_[i + 1].first) {
						weight = 0;
					}
				}

				int job_id = target.trans_sequence[path_[i].second];
				contribution[path_[i].second] += weight * pt[job_id][path_[i].first];
			}




			
			// INSERT: least contribution job insert into largest block
			std::vector<int> neighbor1 = INSERT(blocks, contribution, target.trans_sequence);

			// SWAP: Largest block both side swap
			std::vector<int> neighbor2;
			std::vector<int> neighbor3;
			std::tie(neighbor2, neighbor3) = SWAP(blocks, target.trans_sequence);

			// INSERTBLOCK: a least contribution block is inserted into
			std::vector<int> neighbor4 = INSERTBLOCK(blocks, target.trans_sequence);

			return { neighbor1, neighbor2, neighbor3, neighbor4 };

		}
		

		/*
			THIS IS AN OPERATOR

			Find job with least contribution, inserting into largest block

			Output : neighbor sequence
		*/
		std::vector<int> INSERT(std::vector<std::vector<std::pair<int, int>>>& blocks, std::vector<int>& contribution, std::vector<int> target) {
			// find largest block
			// CAN BE OPTIMIZED TO LARGEST BLOCK SET INSTEAD OF THE FIRST LARGEST BLOCK
			auto blockit = std::max_element(blocks.begin(), blocks.end(), [](std::vector<std::pair<int, int>>& a, std::vector<std::pair<int, int>>& b) {
				return a.size() < b.size();
				});

			int random_bias = generate_random_int(0, blockit->size() + 1);

			// find least contribution job
			auto contribution_it = std::min_element(contribution.begin(), contribution.end());

			int min_contri_pos = std::distance(contribution.begin(), contribution_it);


			int insert_pos;
			// generate neighbor
			if (random_bias == blockit->size()) {
				insert_pos = blockit->rbegin()->second + 1;
			}
			else {
				insert_pos = (*blockit)[random_bias].second;
			}

			if (min_contri_pos <= insert_pos) {
				insert_pos--;
			}

			ASSERT_MSG(insert_pos >= 0 && insert_pos < target.size(), "Something wrong with INSERT Operator ");
			
			auto rmvit = target.begin();
			std::advance(rmvit, min_contri_pos);
			int rmv_job_id = *rmvit;
			target.erase(rmvit);


			auto insit = target.begin();
			std::advance(insit, insert_pos);
			target.insert(insit, rmv_job_id);

			return target;
		}

		/*
			THIS IS AN OPERATOR

			Find largest block, swap its head with adjacent, swap its end with adjacent
		*/
		std::tuple<std::vector<int>, std::vector<int>> SWAP(const std::vector<std::vector<std::pair<int, int>>>& blocks, std::vector<int> target)
		{
			// find largest block
			// CAN BE OPTIMIZED TO FIND LARGEST BLOCK SET (considering there should be multiple)

			auto it = std::max_element(blocks.begin(), blocks.end(), [](auto& a, auto& b) {
				return a.size() < b.size();
				});


			int swap1 = it->begin()->second;
			int swap2 = it->rbegin()->second;

			if (swap1 > swap2) {
				std::swap(swap1, swap2);
			}

			std::vector<int> neighbor1 = target;
			std::vector<int> neighbor2 = target;

			if (swap1 > 0) {
				std::swap(neighbor1[swap1], neighbor1[swap1 - 1]);
			}
			else if(swap1 + 1 < target.size()) {
				std::swap(neighbor1[swap1], neighbor1[swap1 +1]);
			}
			if (swap2 < target.size() - 1) {
				std::swap(neighbor2[swap2], neighbor2[swap2 + 1]);
			}
			else if (swap2 - 1 >= 0) {
				std::swap(neighbor2[swap2], neighbor2[swap2 - 1]);
			}

			return std::make_tuple(neighbor1, neighbor2);
		}

		/*
			THIS IS AN OPERATOR

			Find smallest block, insert small one to largest
		*/
		std::vector<int> INSERTBLOCK(std::vector<std::vector<std::pair<int, int>>>& blocks, std::vector<int> target)
		{
			// Find largest block size
			
			auto maxit = std::max_element(blocks.begin(), blocks.end(), [](auto& a, auto& b) {
				return a.size() < b.size();
				});
			int max_block_size = maxit->size();

			// Find smallest block size
			auto minit = std::min_element(blocks.begin(), blocks.end(), [](auto& a, auto& b) {
				if (a.size() > 1 && b.size() > 1) {
					return a.size() < b.size();
				}
				else if (a.size() > 1 && b.size() <= 1)
					return true;
				else
					return false;
				});
			int min_block_size = (minit->size() == max_block_size) ? 1 : minit->size();
			
			std::vector<std::vector<std::pair<int, int>>> max_block_set;
			std::vector<std::vector<std::pair<int, int>>> min_block_set;
			
			std::for_each(blocks.begin(), blocks.end(), [&](std::vector<std::pair<int,int>>& a) {
				if (a.size() == max_block_size) max_block_set.push_back(a);

				if (a.size() == min_block_size) min_block_set.push_back(a);
				});
				
			
			int max_block = generate_random_int(0, max_block_set.size());
			int insert_pos_begin = max_block_set[max_block].begin()->second;
			int insert_pos_end = max_block_set[max_block].rbegin()->second;
			if (insert_pos_begin > insert_pos_end) {
				std::swap(insert_pos_begin, insert_pos_end);
			}


			int min_block = generate_random_int(0, min_block_set.size());
			int erase_begin = min_block_set[min_block].begin()->second;
			int erase_end = min_block_set[min_block].rbegin()->second;
			if (erase_begin > erase_end) {
				std::swap(erase_begin, erase_end);
			}




			if (erase_begin == erase_end) {
				// randomly insert
				int bias = generate_random_int(0, target.size());
				int ele = target[erase_begin];
				target.erase(target.begin() + erase_begin);
				target.insert(target.begin() + bias, ele);

			}
			else {
				if (erase_end == insert_pos_begin) {
					erase_end--;
					ASSERT_MSG(erase_begin <= erase_end, "Wrong with INSERT_BLOCK");
				}
				else if (erase_begin == insert_pos_end) {
					erase_begin++;
					ASSERT_MSG(erase_begin <= erase_end, "Wrong with INSERT_BLOCK");
				}
				if (erase_end < insert_pos_begin) {
					insert_pos_begin -= (erase_end - erase_begin + 1);
					insert_pos_end -= (erase_end - erase_begin + 1);
				}


				//PRINT_VECTOR(target,"Origin sequence is");
				std::vector<int> to_insert(target.begin() + erase_begin, target.begin() + erase_end + 1);
				//PRINT_VECTOR(to_insert,"the erased subsequence is");
				target.erase(target.begin() + erase_begin, target.begin() + erase_end + 1);
				//PRINT_VECTOR(target, "the sequence after erase is");

				int bias = generate_random_int(0, insert_pos_end - insert_pos_begin + 2);
				int insert_pos = insert_pos_begin + bias;
				ASSERT_MSG(insert_pos <= target.size(), "Wrong Insert Pos");
				target.insert(target.begin() + insert_pos, to_insert.begin(), to_insert.end());
				//PRINT_VECTOR(target, "sequence after insert is");
			}
			
			return target;
		}


		/*
			THIS IS AN OPERATOR

			Find job with least contribution, swap with the largetst block's first job
		*/


		/*
			THIS IS AN OPERATOR

			Find job with least contribution, swap with the largest block's last job
		*/



		/*
			THIS IS AN OPERATOR

			Find job with most contribution, swap with the largest block's first job
		*/


		/*
			THIS IS AN OPERATOR

			Find job with most contribution, swap with the largest block's last job
		*/


		/*dd
			THIS IS AN OPERATOR

			Find largest block, swap its first and last job
		*/













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



