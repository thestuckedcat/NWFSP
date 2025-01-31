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
		neighbor.reserve(4);
	}

	void add_neighbor(int m, int pos) {
		neighbor.push_back(std::make_pair(m, pos));
	}
};


namespace CP {
	std::vector<std::vector<graphNode>> Set_graph(std::vector<int>&, std::vector<std::vector<node>>&);
	bool get_critical_path(const std::vector<std::vector<graphNode>>&, std::vector<std::pair<int, int>>&);
	std::vector<std::vector<int>> find_blocks(const std::vector<std::pair<int, int>>&);
	bool get_critical_path_fast(const std::vector<std::vector<graphNode>>&, std::vector<std::pair<int, int>>&);
	void calculate_contribution(std::vector<int>&, const std::vector<std::pair<int, int>>&, std::vector<std::vector<int>>&, NWFSP_Solution&);
}
namespace OPERATORS{
	std::vector<int> SWAP(std::vector<int>& seq, int a, int b);
}



// SINGLE SCENARIO NEIGHBORHOOD
namespace NEIGHBOUR {

	/*
		find Neighbor for Solution target under selected scenario

		INPUT:  Solution, processing_time

		OUTPUT: The potential neighbor set
	*/
	std::vector<std::vector<int>> CriticalPath_based_neighbor(NWFSP_Solution& target, int scenario_idx, PARAMETERS::Params& param)
	{

		ASSERT_MSG(!target.trans_sequence.empty(), "Trans sequence is empty");

		// set alias
		std::vector<std::vector<int>>& pt = param.scenario[scenario_idx];

		/*
			set the accessible relation between operations
			J_i[j]: Graph[i][j] stands for the accessible relation of operation of ith job in sequence on machine i
		*/

		
		std::vector<std::vector<node>> scenario_sequence_info;
		target.calculate_scenario_info(param, scenario_idx, target.trans_sequence, scenario_sequence_info);
		std::vector<std::vector<graphNode>> Graph = CP::Set_graph(target.trans_sequence, scenario_sequence_info);

		/*
			The combination of operations on critical path
			path[i].first stands for machine_id , path[i].second stands for pos
			Guarantee the beginning time of path[i]
		*/
	
		//auto start = std::chrono::high_resolution_clock::now();
		std::vector<std::pair<int, int>> path;
		//bool pathexisted = CP::get_critical_path(Graph, path);
		bool pathexisted = CP::get_critical_path_fast(Graph, path);
		//auto end = std::chrono::high_resolution_clock::now();
		//std::chrono::duration<double, std::milli> elapsed = (end - start);
		//std::cout << elapsed.count() << std::endl;
		/*
		{
			auto start = std::chrono::high_resolution_clock::now();
			std::vector<std::pair<int, int>> path2;
			bool pathexisted2 = CP::get_critical_path_fast(Graph, path2);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed2 = (end - start);
			std::cout << elapsed2.count() << std::endl;

			bool is_same = true;
			for (int i = 0; i < path.size(); i++) {
				if (path[i] != path2[i]) {
					std::cout << "wrong on " << i << "-th job" << std::endl;
					is_same = false;
				}
			}
			if(is_same)
				std::cout << "is_same" << std::endl;
			else
				std::cout << "is not same" << std::endl;

		}*/
		
		ASSERT_MSG(pathexisted, "Path is not found");

		// find blocks with job_pos in it
		// block[i][j] the ith block's jth job, which is represented by its job_pos
		std::vector<std::vector<int>> blocks = CP::find_blocks(path);

		if (blocks.size() == 0) {
			std::cout << " Current generation do not have block, turn to random pick" << std::endl;
			const int mx = param.job_num;

			std::vector<int> b(mx);
			for (int i = 0; i < mx; i++) {
				b[i] = i;
			}

			blocks.push_back(b);
		}





		// calculate each pos's contribution, find largest block, contribution, contribution is defined as weighted sum of critical operation
		// contribution[i]: contribution of job on pos i
		std::vector<int> contribution;
		CP::calculate_contribution(contribution, path, param.scenario[scenario_idx], target);


		


		/*
			Related Info for Operators
		*/
		std::vector<int> blockcontribution(blocks.size(), 0);
		for (int i = 0; i < blocks.size(); i++) {
			for (int j = 0; j < blocks[i].size(); j++) {
				blockcontribution[i] += contribution[blocks[i][j]];
			}
		}
		int max_block_id = std::distance(blockcontribution.begin(), std::max_element(blockcontribution.begin(), blockcontribution.end()));
		int l = *blocks[max_block_id].begin();// max block's leftmost
		int r = *blocks[max_block_id].rbegin();// max block's rightmost
		int b = std::distance(	contribution.begin(),
								std::max_element(contribution.begin(), contribution.end()));// most contribution job
		int w = std::distance(	contribution.begin(), 
								std::min_element(contribution.begin(), contribution.end()));// least contribution job



		/*
			OPERATORS:
		*/
		std::vector<std::vector<int>> neighbors;
		std::vector<int>& currentseq = target.trans_sequence;

		std::swap(currentseq[l], currentseq[r]);
		neighbors.push_back(currentseq);
		std::swap(currentseq[l], currentseq[r]);

		std::swap(currentseq[l], currentseq[b]);
		neighbors.push_back(currentseq);
		std::swap(currentseq[l], currentseq[b]);

		std::swap(currentseq[b], currentseq[r]);
		neighbors.push_back(currentseq);
		std::swap(currentseq[b], currentseq[r]);


		//neighbors.push_back(OPERATORS::SWAP(currentseq, l, r));
		//neighbors.push_back(OPERATORS::SWAP(currentseq, l, b));
		//neighbors.push_back(OPERATORS::SWAP(currentseq, l, w));
		//neighbors.push_back(OPERATORS::SWAP(currentseq, r, b));
		//neighbors.push_back(OPERATORS::SWAP(currentseq, r, w));
		//neighbors.push_back(OPERATORS::SWAP(currentseq, b, w));

		//std::cout << std::endl;
		return neighbors;

	}





	std::vector<std::vector<int>> VNS_neighbor(NWFSP_Solution& target, PARAMETERS::Params& param)
	{

		
		std::vector<std::vector<int>> neighbors;
		std::vector<int>& currentseq = target.trans_sequence;


		std::vector<int> mark(currentseq.size(), -1);
		int blockcount = 0;
		std::vector<int> blocksize;
		// define blocks in sequence
		int start = -1;
		
		while (start < (int)currentseq.size() - 1) {
			int jump = generate_random_int(1, currentseq.size()-start);
			assert(start+jump < currentseq.size());
			for (int i = start+1; i <= start+jump; i++) {
				mark[i] = blockcount;
			}
			blocksize.push_back(jump);
			blockcount++;
			start += jump;
		}
		

		{
			for (int i = 0; i < param.machine_num/2; i++) {

				std::srand(std::time(nullptr));


				std::vector<int> newseq = currentseq;

				// 随机选择一个位置
				int rand_pos1 = std::rand() % newseq.size();
				int rand_pos2;
				do {
					rand_pos2 = std::rand() % newseq.size();
				} while (rand_pos1 == rand_pos2);

				int value_to_move = newseq[rand_pos1];

				newseq.erase(newseq.begin() + rand_pos1);

				newseq.insert(newseq.begin() + rand_pos2, value_to_move);


				neighbors.push_back(newseq);
			}
		}
		

		// swap two job of a block, which is its begin and end
		{
			int blockIdx = generate_random_int(0, blockcount);
			int beginIdx = 0;
			for (int i = 0; i < currentseq.size(); i++) {
				if (mark[i] == blockIdx) {
					beginIdx = i;
					break;
				}
			}
			int a = generate_random_int(0, blocksize[blockIdx]);
			int b = generate_random_int(0, blocksize[blockIdx]);

			neighbors.push_back(OPERATORS::SWAP(currentseq, beginIdx + a, beginIdx + b));
		}

		// swap begin of first block and end of second block
		{
			int blockIdx = generate_random_int(0, blockcount);
			int blockIdx2 = generate_random_int(0, blockcount);
			int beginIdx = 0;
			int endIdx = currentseq.size()-1;
			
			for (int i = 0; i < currentseq.size(); i++) {
				if (mark[i] == blockIdx) {
					beginIdx = i;
					break;
				}
			}
			for (int i = 0; i < currentseq.size(); i++) {
				if (mark[i] == blockIdx2 + 1) {
					beginIdx = i-1;
					break;
				}
			}

			neighbors.push_back(OPERATORS::SWAP(currentseq, beginIdx, endIdx));
			
		}


		// swap whole first block with random job of second block
		{
			int blockIdx = generate_random_int(0, blockcount);


			std::vector<int> newseq = currentseq;


			int firstBlockBegin = 0;
			int firstBlockend = currentseq.size();
			for (int i = 0; i < currentseq.size(); i++) {
				if (mark[i] == blockIdx) {
					firstBlockBegin = i;
					break;
				}
			}

			for (int i = 0; i < currentseq.size(); i++) {
				if (mark[i] == blockIdx + 1) {
					firstBlockend = i;
					break;
				}
			}
			std::vector<int> firstBlock;
			for (int i = firstBlockBegin; i < firstBlockend; i++) {
				firstBlock.push_back(newseq[i]);
			}
			newseq.erase(newseq.begin() + firstBlockBegin, newseq.begin() + firstBlockend);

			
			int a;
			if (newseq.size() == 0) {
				a = 0;
			}
			else
			{
				a = generate_random_int(0, newseq.size());
			} 



			newseq.insert(newseq.begin() + a, firstBlock.begin(), firstBlock.end());

			neighbors.push_back(newseq);

		

		}

		{
			for (int i = 0; i < 3; i++) {
				int a = generate_random_int(0, currentseq.size());
				int b = generate_random_int(0, currentseq.size());

				neighbors.push_back(OPERATORS::SWAP(currentseq, a, b));
			}
		}

		

		return neighbors;

	}

}



// for critical path calculation tools
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

	bool get_critical_path(const std::vector<std::vector<graphNode>>& Graph, std::vector<std::pair<int,int>>& path) {
		const int target_machine = Graph.size() - 1;
		const int target_job_pos = Graph[0].size() - 1;

		path.clear();
		path.reserve(Graph.size() * Graph[0].size());

		std::vector<int> neighborIdx;// Where has the neighbor been accessed to
		neighborIdx.reserve(Graph.size() * Graph[0].size());

		// visited[i][j]: Is operation J_i[j] been visited
		std::vector<std::vector<bool>> visited(Graph.size(), std::vector<bool>(Graph[0].size(), false));



		// Initialize
		path.push_back(std::make_pair<int,int>(0, 0));
		neighborIdx.push_back(0);
		visited[0][0] = true;
		int count = 0;
		while (!path.empty()) {
			count++;
			int current_machine = path.rbegin()->first;
			int current_pos = path.rbegin()->second;
			// search the last

			// if it is target
			if (current_machine == target_machine && current_pos == target_job_pos) {
				return true;
			}

			
			// it is not target, push its neighbor into path
			int current_idx = path.size()-1;
			

			
			auto& nei = Graph[current_machine][current_pos].neighbor;
			if (neighborIdx[current_idx] < nei.size()) {
				
				int nxtmachine = nei[neighborIdx[current_idx]].first;
				int nxtpos = nei[neighborIdx[current_idx]].second;
				
				if (!visited[nxtmachine][nxtpos])
				{
					path.push_back(nei[neighborIdx[current_idx]]);
					neighborIdx.push_back(0);
					visited[nxtmachine][nxtpos] = true;
				}
				neighborIdx[current_idx]++;
				
			}
			else {
				// the neighbor of current operation has been all visited, and path not found

				visited[current_machine][current_pos] = false;
				path.pop_back();
				neighborIdx.pop_back();


			}
		}
		std::cout << count << std::endl;
		return false;
	}


	bool get_critical_path_fast(const std::vector<std::vector<graphNode>>& Graph, std::vector<std::pair<int, int>>& path) {
		const int target_machine = Graph.size() - 1;
		const int target_job_pos = Graph[0].size() - 1;

		path.clear();
		path.reserve(Graph.size() * Graph[0].size());

		std::vector<int> neighborIdx;// Where has the neighbor been accessed to
		neighborIdx.reserve(Graph.size() * Graph[0].size());

		// visited[i][j]: Is operation J_i[j] been visited
		std::vector<std::vector<bool>> visited(Graph.size(), std::vector<bool>(Graph[0].size(), false));



		// Initialize
		path.push_back(std::make_pair<int, int>(0, 0));
		neighborIdx.push_back(0);
		visited[0][0] = true;
		int count = 0;
		while (!path.empty()) {
			count++;
			int current_machine = path.rbegin()->first;
			int current_pos = path.rbegin()->second;
			// search the last

			// if it is target
			if (current_machine == target_machine && current_pos == target_job_pos) {
				return true;
			}


			// it is not target, push its neighbor into path
			int current_idx = path.size() - 1;



			auto& nei = Graph[current_machine][current_pos].neighbor;
			if (neighborIdx[current_idx] < nei.size()) {

				int nxtmachine = nei[neighborIdx[current_idx]].first;
				int nxtpos = nei[neighborIdx[current_idx]].second;

				if (!visited[nxtmachine][nxtpos])
				{
					path.push_back(nei[neighborIdx[current_idx]]);
					neighborIdx.push_back(0);
					visited[nxtmachine][nxtpos] = true;
				}
				neighborIdx[current_idx]++;

			}
			else {
				// the neighbor of current operation has been all visited, and path not found

				//visited[current_machine][current_pos] = false;
				path.pop_back();
				neighborIdx.pop_back();


			}
		}
		std::cout << count << std::endl;

		return false;
	}




	/*
		find blocks through operations in Critical path

		return vector<vector<int>> where stands for the unit of blocks

		a block contains a set of continuous index of job_pos 
	*/
	std::vector<std::vector<int>> find_blocks(const std::vector<std::pair<int, int>>& path) {
		std::vector<std::vector<int>> blocks;

		// get block
		std::vector<int> curblock;
		curblock.push_back(0);

		for (int i = 1;i < path.size(); i++)
		{
			// 在同一个机器上
			if (path[i].first == path[i-1].first) {
				curblock.push_back(i);
			}
			else {
				blocks.push_back(curblock);
				curblock.clear();

				// 不在同一个机器上
				while (i < path.size() && path[i].first != path[i - 1].first) {
					i++;
				}
				// 此时i指向的是下一个block的开头, 此时分为两种情况，即为反向路径上去的，和正向路径下来的，
				// 区别在于反向路径上去的block不应算上i-1，而正向路径下去的应该算上i-1
				if (i - 2 >= 0 && path[i - 1].first > path[i - 2].first) {
					curblock.push_back(i-1);
				}
				curblock.push_back(i);
			}

		}

		ASSERT_MSG(!blocks.empty(), "No block found");


		blocks.erase(
			std::remove_if(blocks.begin(), blocks.end(), [](const std::vector<int>& innerVec) {
				return innerVec.size() < 2;
				}),
			blocks.end()
		);


		for (int i = 0; i < blocks.size(); i++) {
			for (int j = 0; j < blocks[i].size(); j++) {
				blocks[i][j] = path[blocks[i][j]].second;
			}
		}

		
		return blocks;
	}

	/*
		calculate contribution of each operation

		and transfer to contribution of each job

		contribution[i] stands for the contribution of job on path i
	*/
	void calculate_contribution(std::vector<int>& ans,
								const std::vector<std::pair<int,int>>& path, 
								std::vector<std::vector<int>> &scenario, 
								NWFSP_Solution& target) 
	{
		ASSERT_MSG(!target.trans_sequence.empty(), "The solution do not have trans sequence");

		std::vector<int> contribution(path.size(), 0);

		for (int i = 0; i < path.size(); i++) {
			int current_job = target.trans_sequence[path[i].second];
			int current_machine = path[i].first;


			contribution[i] = scenario[current_job][current_machine];
			if (i - 1 >= 0 && path[i - 1].first > path[i].first) {
				// 上一个机器的机器号大于当前，意味着当前为reverse
				contribution[i] *= -1;
			}
			if ((i-1 >= 0 && path[i-1].first == path[i].first) && (i + 1 < path.size() && path[i + 1].first < path[i].first)) {
				// corner 1
				// 上一个operation的机器现在的一样，但是下一个operation的机器号小于当前，这意味着这是一个转折点
				contribution[i] = 0;
			}
			if ((i-1 >= 0 && path[i-1].first > path[i].first) && (i+1 < path.size() && path[i+1].first == path[i].first)) {
				// corner 2
				// 上一个operation的机器大于当前，但是下一个operation的机器等于当前，这意味着这是一个转折点
				contribution[i] = 0;
			}
		}
		ans = std::vector<int>(target.sequence.size(),0);


		for (int i = 0; i < contribution.size(); i++) {
			int currentjobPos = path[i].second;
			ans[currentjobPos] += contribution[i];
		}

	}

	


}






namespace OPERATORS {
	/*
		THIS IS AN OPERATOR

		INPUT: sequence, swap target(a,b)

		OUTPUT: swapped sequence
	
	*/
	std::vector<int> SWAP(std::vector<int>& seq, int a, int b) {
		std::vector<int> newseq = seq;
		std::swap(newseq[a], newseq[b]);
		return newseq;
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
}