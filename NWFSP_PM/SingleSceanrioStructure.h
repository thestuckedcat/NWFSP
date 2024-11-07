#pragma once
#include <vector>
#include "myutils.h"
#include <stack>
#include "solution.h"
#include "Parameter.h"
// input: A solution
// output: A new solution
struct graphNode {
	int machine_id;
	int job_id;
	int pos;
	// the accessable operations, including (machine_idx, pos_idx)
	std::vector<std::pair<int, int>> neighbor;
	graphNode() {};
	graphNode(int m, int pos, int j)
		: machine_id{ m }, job_id{ j }, pos{ pos } {

	}

	void add_neighbor(int m, int pos) {
		neighbor.push_back(std::make_pair(m, pos));
	}
};




/*
	find Neighbor for Solution target

	INPUT:  Solution, processing_time

	OUTPUT: The potential neighbor set
*/
std::vector<std::vector<int>> CriticalPath_based_neighbor(NWFSP_Solution& target, int scenario_idx, PARAMETERS::Params& param)
{

	ASSERT_MSG(!target.trans_sequence.empty(), "Trans sequence is empty");


	std::vector<std::vector<int>>& pt = param.scenario[scenario_idx];


	std::vector<std::vector<graphNode>> Graph = CP::Set_graph(target.trans_sequence, target.scenario_sequence_info_trans[scenario_idx]);

	std::vector<std::pair<int, int>> path = CP::get_critical_path(Graph);
	ASSERT_MSG(!path.empty(), "Path is empty");

	// find blocks with (machine_id, pos) in it
	std::vector<std::vector<std::pair<int, int>>> blocks = CP::find_blocks(path);


	// calculate each pos's contribution, find largest block, contribution, contribution is defined as weighted sum of critical operation
	// contribution[i]: contribution of job on pos i
	std::vector<int> contribution = CP::calculate_contribution();
	// if current machine is less than past machine ,a negative weight is given 
	for (int i = 0; i < path.size(); i++) {
		int weight = 1;

		// judge if this operation is reverse
		if (i > 0 && path[i].first < path[i - 1].first) {
			weight = -1;
		}

		// judge if this operation should be ignored
		if (i - 1 >= 0 && i + 1 < param.job_num) {
			if (path[i].first == path[i - 1].first && path[i].first == path[i + 1].first + 1) {
				weight = 0;
			}

			if (path[i].first == path[i - 1].first - 1 && path[i].first == path[i + 1].first) {
				weight = 0;
			}
		}

		int job_id = target.trans_sequence[path[i].second];
		contribution[path[i].second] += weight * pt[job_id][path[i].first];
	}





	// INSERT: least contribution job insert into largest block
	std::vector<int> neighbor1 = OPERATORS::INSERT(blocks, contribution, target.trans_sequence);

	// SWAP: Largest block both side swap
	std::vector<int> neighbor2;
	std::vector<int> neighbor3;
	std::tie(neighbor2, neighbor3) = OPERATORS::SWAP(blocks, target.trans_sequence);

	// INSERTBLOCK: a least contribution block is inserted into
	std::vector<int> neighbor4 = OPERATORS::INSERTBLOCK(blocks, target.trans_sequence);

	return { neighbor1, neighbor2, neighbor3, neighbor4 };

}


namespace CP {
	/*
			Set Operation Relation in a Graph
		*/
	std::vector<std::vector<graphNode>> Set_graph(std::vector<int>& seq,
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
		find blocks through operations in Critical path
	*/
	std::vector<std::vector<std::pair<int, int>>> find_blocks(std::vector<std::pair<int, int>> old_path) {
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
	std::vector<std::pair<int, int>> get_critical_path(const std::vector<std::vector<graphNode>>& Graph) {
		// Find shortest path, it should return several blocks with (machine_id,job_id) pair in it
		int machine_num = Graph.size();
		int job_num = Graph[0].size();
		// machine,pos
		std::stack<std::pair<int, int>> path;
		std::vector<std::vector<bool>> visited(machine_num, std::vector<bool>(job_num, false));
		// dfs find a path
		bool found = dfs(Graph, 0, 0, path, visited);
		ASSERT_MSG(found, "Critical Path not Found");


		std::vector<std::pair<int, int>> path_(path.size());
		int i = path.size() - 1;
		while (i >= 0 && !path.empty()) {
			path_[i--] = path.top();
			path.pop();
		}
		return path_;
	}


	std::vector<int> calculate_contribution() {

	}

}






namespace OPERATORS {
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
		else if (swap1 + 1 < target.size()) {
			std::swap(neighbor1[swap1], neighbor1[swap1 + 1]);
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

		std::for_each(blocks.begin(), blocks.end(), [&](std::vector<std::pair<int, int>>& a) {
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
}