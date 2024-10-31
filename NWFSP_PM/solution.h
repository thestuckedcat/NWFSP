#pragma once
# include<iostream>
# include<vector>
# include<cassert>
# include<algorithm>
# include<random>
# include<set>
# include"myutils.h"
# include "Parameter.h"

/*

	添加通过sequence 更新solution 的方法，包括Scenario INFO
	

*/

// for decode
struct node {
	int begin;
	int end;
	int job_id;
	node() {};
	node(int _b, int pt, int _id)
		: begin{ _b }, job_id(_id){
		end = begin + pt;
	}

	void update(int push) {
		begin += push;
		end += push;
	}
};


// for TBS model
struct SCENARIO_INFO {
	std::vector<int> scenario_makespan;
	int threshold;
	int bad_scenario_num;
	std::unordered_set<int> bad_scenario_set;
	int worst_scenario_id;
	int worst_scenario_makespan;
	int Penalty_of_TBS;

	//SCENARIO_INFO() { Penalty_of_TBS = -1; }
	SCENARIO_INFO(std::vector<int> &scen, int Threshold) {
		bad_scenario_num = -1;
		worst_scenario_makespan = -1;
		scenario_makespan = scen;
		threshold = Threshold;

		Update_INFO();
	}
	SCENARIO_INFO() {
		threshold = 0;
	}
	SCENARIO_INFO& operator=(const SCENARIO_INFO& other) noexcept {
		if (this != &other) {
			scenario_makespan = other.scenario_makespan;
			threshold = other.threshold;
			bad_scenario_num = other.bad_scenario_num;
			bad_scenario_set = other.bad_scenario_set;
			worst_scenario_id = other.worst_scenario_id;
			worst_scenario_makespan = other.worst_scenario_makespan;
			Penalty_of_TBS = other.Penalty_of_TBS;
		}
		return *this;
	}
private:
	void Update_INFO() {
		ASSERT_MSG(threshold > 0, "threshold setup wrongly");

		Penalty_of_TBS = 0;

		bad_scenario_num = 0;
		bad_scenario_set.clear();
		for (int i = 0; i < scenario_makespan.size(); i++) {
			if (scenario_makespan[i] > threshold) {
				bad_scenario_num++;
				bad_scenario_set.insert(i);
				Penalty_of_TBS += (scenario_makespan[i] - threshold) * (scenario_makespan[i] - threshold);
				
			}
		}


		worst_scenario_id = std::distance(	scenario_makespan.begin(),
											std::max_element(scenario_makespan.begin(),scenario_makespan.end()));
		worst_scenario_makespan = scenario_makespan[worst_scenario_id];

	}


};





// Single solution
class NWFSP_Solution {
	std::vector<std::vector<node>> Decode_sequence(std::vector<std::vector<int>>& processing_time, std::vector<int> &seq) {
		
		int num_jobs = seq.size();
		int num_machines = processing_time[0].size();


		std::vector<std::vector<node>> sequence_info_(num_machines, std::vector<node>(num_jobs));


		// Calculate the completion times for the first job in the sequence
		int this_job = seq[0];
		sequence_info_[0][0] = node(0, processing_time[this_job][0], this_job);
		for (int j = 1; j < num_machines; ++j) {
			sequence_info_[j][0] = node(sequence_info_[j - 1][0].end, processing_time[this_job][j], this_job);
			
		}
		//for (int j = 0; j < num_machines; j++) {
		//	std::cout << sequence_info_[j][0].begin << " " << sequence_info_[j][0].end << std::endl;;
		//}

		

		// Calculate the completion times for the rest of the jobs
		for (int j = 1; j < num_jobs; ++j) {
			int job = seq[j];
			sequence_info_[0][j] = node(sequence_info_[0][j - 1].end, processing_time[job][0], job);

			// 应该向后推的距离
			int max_push = 0;
			// Initial
			for (int m = 1; m < num_machines; ++m) {
				// no-wait constraint
				sequence_info_[m][j] = node(sequence_info_[m - 1][j].end, processing_time[job][m], job);


				// flowshop constraint
				int need_push = sequence_info_[m][j - 1].end - sequence_info_[m][j].begin;
				if (need_push > max_push) {
					max_push = need_push;
				}
			}
			// flowshop constraint
			if (max_push)
				for (int m = 0; m < num_machines; m++) {
					sequence_info_[m][j].update(max_push);
				}
		}

		return sequence_info_;
	}
public:
	/*
	int machine_num;
	int job_num;
	*/
	std::vector<int> scenario_makespan;
	std::vector<int> trans_scenario_makespan;
	/*
	int makespan;
	int makespan_trans;
	*/
	// job sequence
	std::vector<int> sequence;
	std::vector<int> trans_sequence;
	// store the (start_time, completion_time) pair, shape(num_machines, num_jobs) for particular scenario
	

	

	std::vector<std::vector<std::vector<node>>> scenario_sequence_info;
	std::vector<std::vector<std::vector<node>>> scenario_sequence_info_trans;

	// scenario information
	SCENARIO_INFO SI;
	SCENARIO_INFO SI_trans;

	// score history
	std::vector<float> score;
	std::vector<int> PT;

	int last_state;


	// A solution can only be constructed by sequence
	NWFSP_Solution(std::vector<int> _sequence)
		:sequence{ _sequence } {
		//makespan = -1;
		//makespan_trans = -1;
		last_state = 0;

	}

	bool operator==(const NWFSP_Solution& rhs) const{
		int job_num = rhs.sequence.size();
		if (this == &rhs) {
			return true;
		}

		for (int i = 0; i < job_num; i++) {
			if (sequence[i] != rhs.sequence[i]) {
				return false;
			}
		}
		return true; 
	}

	bool operator<(const NWFSP_Solution& rhs) const {
		return this->SI.Penalty_of_TBS < rhs.SI.Penalty_of_TBS;
	}

	NWFSP_Solution& operator=(const NWFSP_Solution& rhs) {
		if (this == &rhs) {
			return *this;
		}

		//this->machine_num					= rhs.machine_num;
		//this->job_num						= rhs.job_num;
		//this->makespan						= rhs.makespan;
		//this->makespan_trans				= rhs.makespan_trans;
		this->scenario_makespan				= rhs.scenario_makespan;
		this->trans_scenario_makespan		= rhs.trans_scenario_makespan;
		this->sequence						= rhs.sequence;
		this->trans_sequence				= rhs.trans_sequence;
		//this->sequence_info					= rhs.sequence_info;
		//this->sequence_info_trans			= rhs.sequence_info_trans;
		this->scenario_sequence_info		= rhs.scenario_sequence_info;
		this->scenario_sequence_info_trans	= rhs.scenario_sequence_info_trans;
		this->SI							= rhs.SI;
		this->SI_trans						= rhs.SI_trans;
		this->score							= rhs.score;
		this->PT							= rhs.PT;
		this->last_state					= rhs.last_state;

		return *this;
	}


	friend std::ostream& operator<<(std::ostream& os, const NWFSP_Solution& individual) {
		os << "***************************\n" ;
		os << "The individual Information is as follows\n";
		os << "Threshold is :\t\t" << individual.SI.threshold << std::endl;
		os << "Bad Scenario Num is: \t\t" << individual.SI.bad_scenario_num << std::endl;
		os << "Worst scenario makespan is: \t\t" << individual.SI.worst_scenario_id << std::endl;
		os << "Penalty of TBS is: \t\t" << individual.SI.Penalty_of_TBS << std::endl;

		os << "The current solution sequence is :\n";

		for (int i = 0; i < individual.sequence.size(); i++) {
			os << individual.sequence[i] << " ";
		}
		os << std::endl << std::endl;


		os << "The bad scenario set is :\n";
		for (auto& i : individual.SI.bad_scenario_set) {
			os << i << " ";
		}
		os << std::endl << std::endl;
		os << "***************************\n";

		return os;
	}

	/*
		Assume Processing Time is int 
		sequence: a solution 
		processing_time matrix: num_jobs * num_machines
		scenario_proessing_time_matrix: num_scenarios * num_jobs * num_machines
	*/

	
	int get_makespan(std::vector<std::vector<int>>& processing_time) {
		assert(!processing_time.empty());
		int machine_num = processing_time[0].size();
		int job_num = sequence.size();
		std::vector<std::vector<node>> sequence_info;
		std::vector<std::vector<node>> sequence_info_trans;
		sequence_info = Decode_sequence(processing_time, sequence);
		int makespan = sequence_info[machine_num - 1][job_num - 1].end;
		return makespan;
	}
	/*
	int get_transition_makespan(std::vector<std::vector<int>>& processing_time) {
		assert(!processing_time.empty());
		machine_num = processing_time[0].size();
		job_num = trans_sequence.size();

		sequence_info_trans = Decode_sequence(processing_time, trans_sequence);
		makespan_trans = sequence_info_trans[machine_num - 1][job_num - 1].end;
		return makespan;
	}
	*/




	// Calculate Scenario Info of this solution
	void calculate_scenario_makespan(PARAMETERS::Params& param) {
		// makespan under each scenario
		scenario_makespan = std::vector<int>(param.scenario_num);

		scenario_sequence_info.clear();


		// Calculate makespan under all sceanrios
		for (int i = 0; i < param.scenario_num; i++) {
			scenario_sequence_info.push_back(Decode_sequence(param.scenario[i],sequence));
			scenario_makespan[i] = scenario_sequence_info[i][param.machine_num - 1][param.job_num - 1].end;
		}

		// Calculate scenario information under these scenarios
		SI = SCENARIO_INFO(scenario_makespan, param.Threshold);
	}
	/*
	void calculate_scenario_makespan(std::vector<std::vector<std::vector<int>>> &scenario_processing_time, int Threshold) {
		int scenario_num = scenario_processing_time.size();
		job_num = scenario_processing_time[0].size();
		machine_num = scenario_processing_time[0][0].size();

		// makespan under each scenario
		std::vector<int> scenario_makespan(scenario_num);

		// Start-End time of each operation under every scenario
		scenario_sequence_info.clear();



		for (int i = 0; i < scenario_num; i++) {
			scenario_sequence_info.push_back(Decode_sequence(scenario_processing_time[i], sequence));
			scenario_makespan[i] = scenario_sequence_info[i][machine_num - 1][job_num - 1].end;
		}


		SI = SCENARIO_INFO(scenario_makespan, Threshold);
	}*/


	// Calculate Scenario Info of trans solution
	void calculate_transition_scenario_makespan(PARAMETERS::Params& param) {
		// makespan under each scenario
		trans_scenario_makespan = std::vector<int>(param.scenario_num);

		// Start-End time of each operation under every scenario
		scenario_sequence_info_trans.clear();
		for (int i = 0; i < param.scenario_num; i++) {
			scenario_sequence_info_trans.push_back(Decode_sequence(param.scenario[i], trans_sequence));
			scenario_makespan[i] = scenario_sequence_info_trans[i][param.machine_num - 1][param.job_num - 1].end;
		}
		SI_trans = SCENARIO_INFO(scenario_makespan, param.Threshold);
	}
	/*
	void calculate_transition_scenario_makespan(std::vector<std::vector<std::vector<int>>>& scenario_processing_time, int Threshold) {
		int scenario_num = scenario_processing_time.size();
		job_num = scenario_processing_time[0].size();
		machine_num = scenario_processing_time[0][0].size();
		// makespan under each scenario
		std::vector<int> scenario_makespan(scenario_num);

		// Start-End time of each operation under every scenario
		scenario_sequence_info_trans.clear();



		for (int i = 0; i < scenario_num; i++) {
			scenario_sequence_info_trans.push_back(Decode_sequence(scenario_processing_time[i], trans_sequence));
			scenario_makespan[i] = scenario_sequence_info_trans[i][machine_num - 1][job_num - 1].end;
		}


		SI_trans = SCENARIO_INFO(scenario_makespan, Threshold);
	}

	*/

	// Elite solution
	// Trans tran_seq to current_seq
	bool improve_solution() {
		if (SI_trans.Penalty_of_TBS >= 0 && SI.Penalty_of_TBS > SI_trans.Penalty_of_TBS) {
			ASSERT_MSG(SI_trans.Penalty_of_TBS >= 0, "Trans penalty is not calculated");
			ASSERT_MSG(SI.Penalty_of_TBS >= 0, "Origin penalty is not calculated");
			scenario_makespan = std::move(trans_scenario_makespan);
			sequence = std::move(trans_sequence);
			//sequence_info = std::move(sequence_info_trans);
			scenario_sequence_info = std::move(scenario_sequence_info_trans);
			SI = std::move(SI_trans);
			return true;
		}
		return false;
	}

};



// T for solution type, current for NWFSP_Solution
template<typename T>
class Population{
public:
	// Solutions
	std::vector<T> population;
	int pop_size;
	int job_num;
	int machine_num;
	int Threshold;

	// shape(num_jobs, num_machines)
	std::vector<std::vector<std::vector<int>>> scenario_processing_time;


	// Constructor
	Population() = default;

	Population(int _pop_size, int num_jobs,int _num_machines, std::vector<std::vector<std::vector<int>>> scen,int _Threshold) 
		:job_num{ num_jobs }, machine_num{ _num_machines }, pop_size{ _pop_size }, scenario_processing_time{ scen },Threshold{_Threshold}
	{
		
	}

	Population(const Population& rhs) {
		if (this != &rhs) {
			this->pop_size = rhs.pop_size;
			this->job_num = rhs.job_num;
			this->machine_num = rhs.machine_num;
			this->population = rhs.population;
			this->scenario_processing_time = rhs.scenario_processing_time;
		} 
		
	}

	// copy assignment
	Population& operator=(const Population& rhs) {
		if (this == &rhs) {
			return *this;
		}


		population = rhs.population;
		pop_size = rhs.pop_size;
		job_num = rhs.job_num;
		machine_num = rhs.machine_num;
		scenario_processing_time = rhs.scenario_processing_time;

		return *this;
	}


	// void Improve solution
	void Improve_pop(std::vector<int>& chosen_solution) {
		int count = 0;
		for (int i = 0; i < chosen_solution.size(); i++) {
			if (population[chosen_solution[i]].improve_solution()) {
				count++;
			}
		}
		//std::cout << count << " of " << population.size() << " has been improved" << std::endl;
	}



	friend std::ostream& operator<<(std::ostream& os, const Population& pop) {
		os << "--------------------------------------------------------------------------------------\n";
		for (auto i : pop.population) {
			os << i << std::endl;
		}
		os << "----------------------------------------------------------------------------------------\n";

		return os;
	}
};











