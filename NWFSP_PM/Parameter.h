#pragma once
#include <stdexcept>
#include <vector>
#include <iostream>
#include "myutils.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <algorithm>
#include <climits>
#include <unordered_map>
#include <cmath>
#undef max
namespace PARAMETERS {
	// 使用scenario初始化，获得各项参数
	struct Params {

		// for Q-table
		float alpha;
		float gamma;
		float epsilon;
		// selection rate
		float selection_rate;
		

		/*
			Population partition rate based on BSN

			BSN < beta * scenario_num的交给UN
			BSN >= beta * scenario_num的交给LN
		*/
		float beta;

		int max_gen;
		int job_num;
		int machine_num;
		int pop_size;
		int Threshold;
		int scenario_num;
		std::vector<std::vector<std::vector<int>>> scenario;
		bool is_cpu_terminated;
		int cpu_terminate_time;
		int P;

		// sbm[i][j][k]:scenario i the delay of job k insert after job j
		std::vector<std::vector<std::vector<int>>> scenario_based_matrix;
		Params(float _selection_rate, float _alpha, float _gamma, float _epsilon, float _beta,
			int _max_gen, int _job_num, int _machine_num, int _pop_size, int _scenario_num, int _P)
			:alpha{ _alpha }, gamma{ _gamma }, epsilon{ _epsilon }, beta{ _beta }, P{ _P } {
			selection_rate = _selection_rate;
			max_gen = _max_gen;
			job_num = _job_num;
			machine_num = _machine_num;
			pop_size = _pop_size;
			scenario_num = _scenario_num;
			Threshold = 0;
			is_cpu_terminated = false;
		}
		Params() {
			alpha = 0.1;
			gamma = 0.9;
			beta = 0.5;

			selection_rate = 0.5;
			epsilon = 0.8;
			max_gen = 10000;
			job_num = 20;
			machine_num = 5;
			pop_size = 10;
			scenario_num = 20;
			Threshold = 0;
			is_cpu_terminated = false;
			P = 10;
		}
		
		void generate_jobTojobInfo() {
			/*
				sceanrio_based_matrxi[s][i][j] 代表[工件i,工件j]的组合，工件j的makespan比工件i的makespan推迟多少
				
				其中scenario_based_matrix[s][0][j]代表dummy job在前，因此也就是工件j在第一个位置时的推迟时间，也就是所有operation的和
			*/
			scenario_based_matrix = std::vector<std::vector<std::vector<int>>>(scenario_num,
				std::vector<std::vector<int>>(job_num + 1, std::vector<int>(job_num+1,INT_MIN)));


			for (int s = 0; s < scenario_num; s++) {

				for (int j = 1; j <= job_num; j++) {
					scenario_based_matrix[s][0][j] = std::accumulate(scenario[s][j - 1].begin(), scenario[s][j - 1].end(), 0);
				}

				for (int jbefore = 1; jbefore <= job_num; jbefore++) {
					for (int jafter = 1; jafter <= job_num; jafter++) {
						if (jbefore == jafter) continue;

						const std::vector<int>& frontJob = scenario[s][jbefore - 1];
						const std::vector<int>& backJob = scenario[s][jafter - 1];

						int max_delay = 0;
						int frontfinish = frontJob[0];
						int backfinish = frontfinish + backJob[0];
						for (int m = 1; m < machine_num; m++) {
							frontfinish += frontJob[m];
							backfinish = std::max(backfinish + backJob[m], frontfinish + backJob[m]);
						}
						scenario_based_matrix[s][jbefore][jafter] = backfinish - frontfinish;
					}
				}
			}
			//std::cout << "Info generated successfully\n";
		}
	

		void load_or_generate() {
			std::string filepath = "../scenario/";
			std::string str = filepath + "J_" + std::to_string(job_num) + "_M_" + std::to_string(machine_num) + "-paramInfo.txt";
			std::ifstream file(str);

			if(file.good())
				load_scenario();
			else
				generate_scenario();
		}

		void set_to_cpu_terminate() {
			is_cpu_terminated = true;
			max_gen = 10000000;
			cpu_terminate_time = machine_num * job_num * scenario_num * 40;

			/*if (job_num == 10) {
				if (machine_num == 5) cpu_terminate_time = 1131;
				if (machine_num == 10) cpu_terminate_time = 1686;
				if (machine_num == 15) cpu_terminate_time = 2134;
				if (machine_num == 20) cpu_terminate_time = 2625;
			}*/
			if (job_num == 20) {
				if (machine_num == 5) cpu_terminate_time = 5663;
				if (machine_num == 10) cpu_terminate_time = 8004;
				if (machine_num == 15) cpu_terminate_time = 9607;
				if (machine_num == 20) cpu_terminate_time = 12102;
			}
			else if (job_num == 30) {
				if (machine_num == 5) cpu_terminate_time = 10660;
				if (machine_num == 10) cpu_terminate_time = 13946;
				if (machine_num == 15) cpu_terminate_time = 23289;
				if (machine_num == 20) cpu_terminate_time = 32314;
			}
			else if (job_num == 40) {	

				if (machine_num == 5) cpu_terminate_time = 15696;
				if (machine_num == 10) cpu_terminate_time = 17820;
				if (machine_num == 15) cpu_terminate_time = 22194;
				if (machine_num == 20) cpu_terminate_time = 24977;
			}else if (job_num == 50) {
							

				if (machine_num == 5) cpu_terminate_time = 15108;
				if (machine_num == 10) cpu_terminate_time = 25736;
				if (machine_num == 15) cpu_terminate_time = 31737;
				if (machine_num == 20) cpu_terminate_time = 40802;
			}else if (job_num == 60) {
							

				if (machine_num == 5) cpu_terminate_time = 31622;
				if (machine_num == 10) cpu_terminate_time = 27000;
				if (machine_num == 15) cpu_terminate_time = 37114;
				if (machine_num == 20) cpu_terminate_time = 65220;
			}
			else if (job_num == 100) {
						

				if (machine_num == 20) cpu_terminate_time = 98537;
				if (machine_num == 40) cpu_terminate_time = 149940;
				if (machine_num == 60) cpu_terminate_time = 279286;
			}
			else if (job_num == 200) {
						

				if (machine_num == 20) cpu_terminate_time = 196543;
				if (machine_num == 40) cpu_terminate_time = 355601;
				if (machine_num == 60) cpu_terminate_time = 632565;
			}
			else if (job_num == 300) {
						

				if (machine_num == 20) cpu_terminate_time = 399175;
				if (machine_num == 40) cpu_terminate_time = 933224;
				if (machine_num == 60) cpu_terminate_time = 1429708;
			}
			else if (job_num == 400) {
						

				if (machine_num == 20) cpu_terminate_time = 256096;
				if (machine_num == 40) cpu_terminate_time = 398498;
				if (machine_num == 60) cpu_terminate_time = 1854891;
			}
			else if (job_num == 500) {
						

				if (machine_num == 20) cpu_terminate_time = 267823;
				if (machine_num == 40) cpu_terminate_time = 499655;
				if (machine_num == 60) cpu_terminate_time = 876451;
			}
			else if (job_num == 600) {
						

				if (machine_num == 20) cpu_terminate_time = 585930;
				if (machine_num == 40) cpu_terminate_time = 940732;
				if (machine_num == 60) cpu_terminate_time = 1136724;
			}

			cpu_terminate_time *= 1.5;
		}

		// scenario related

		void load_scenario() {
			std::string filepath = "../scenario/";
			std::string str = filepath+"J_" + std::to_string(job_num) + "_M_" + std::to_string(machine_num) + "-paramInfo.txt";
			scenario = ArrayReader()(str);
			
			{
				if (job_num == 10) {// 已修改
					if (machine_num == 5) {
						Threshold = 1131;
					}
					if (machine_num == 10) {
						Threshold = 1686;
					}
					if (machine_num == 15) {
						Threshold = 2134;
					}
					if (machine_num == 20) {
						Threshold = 2625;
					}
				}

				else if (job_num == 20 && machine_num == 5) Threshold = 2014;
				else if (job_num == 20 && machine_num == 10) Threshold = 2803;
				else if (job_num == 20 && machine_num == 15) Threshold = 3421;
				else if (job_num == 20 && machine_num == 20) Threshold = 4045;

				else if (job_num == 30) {
					if (machine_num == 5) {
						Threshold = 2887;
					}
					if (machine_num == 10) {
						Threshold = 3994;
					}
					if (machine_num == 15) {
						Threshold = 4801;
					}
					if (machine_num == 20) {
						Threshold = 5519;
					}
				}
				else if (job_num == 40) {
					if (machine_num == 5) {
						Threshold = 3805;
					}
					if (machine_num == 10) {
						Threshold = 5141;
					}
					if (machine_num == 15) {
						Threshold = 6076;
					}
					if (machine_num == 20) {
						Threshold = 6911;// 6991/10367(5)
					}
				}
				else if (job_num == 50) {
					if (machine_num == 5) {
						Threshold = 4692;
					}
					if (machine_num == 10) {
						Threshold = 6248;
					}
					if (machine_num == 15) {
						Threshold = 7399;//7466/6980(8)
					}
					if (machine_num == 20) {
						Threshold = 8489;// 8512/27896(9)
					}
				}
				else if (job_num == 60) {
					if (machine_num == 5) {
						Threshold = 5499;//5562/7307(10)
					}
					if (machine_num == 10) {
						Threshold = 7311;//7354/10533(10)
					}
					if (machine_num == 15) {
						Threshold = 8782;//8808/4056(8)
					}
					if (machine_num == 20) {
						Threshold = 10121;//10166/4467(5)
					}
				}

				else if (job_num == 80 && machine_num == 5) Threshold = 7402;
				else if (job_num == 80 && machine_num == 10) Threshold = 9825;
				else if (job_num == 80 && machine_num == 15) Threshold = 11537;
				else if (job_num == 80 && machine_num == 20) Threshold = 13246;

				
				else if (job_num == 100 && machine_num == 20) Threshold = 16204;//16254(0)-100,16254(0)-50
				else if (job_num == 100 && machine_num == 40) Threshold = 22252;//22292(0)-50//OK
				else if (job_num == 100 && machine_num == 60) Threshold = 27439;//27539(0)-100//OK

				
				else if (job_num == 200 && machine_num == 20) Threshold = 31579;//31599(0)-20//OK
				else if (job_num == 200 && machine_num == 40) Threshold = 43022;//43082(0)-100,43082(0)-80
				else if (job_num == 200 && machine_num == 60) Threshold = 51624;//51714(0)-100//OK


				else if (job_num == 300) {
					if (machine_num == 20) {
						Threshold =48562;//48682(0)-100,48582(0)-20
					}
					if (machine_num == 40) {
						Threshold = 66060;//66170(0)-100//OK
					}

					if (machine_num == 60) {
						Threshold = 80188;//80208(0)-20
					}
				
				}

				else if (job_num == 400) {
					if (machine_num == 20) {
						Threshold = 64429;//64579(0)-100,64479(0)-50//OK
					}
					if (machine_num == 40) {
						Threshold = 87915;//88015(0)-100//OK
					}

					if (machine_num == 60) {
						Threshold = 105797;//105847(0)-100
					}
				}
				else if (job_num == 500) {
					if (machine_num == 20) {
						Threshold = 80409;//80569(0)-100,80469(0)-100,80369(8)+30-10//OK
					}
					if (machine_num == 40) {
						Threshold = 110349;//110929(0)-300,110629(0)-300-------------//OK
					}

					if (machine_num == 60) {
						Threshold = 132240;//132440(0)-100,132340(0)-100//OK
					}
				}
				else if (job_num == 600) {
					if (machine_num == 20) {
						Threshold = 96777;//97097(0)-100,96997(0)-100//96897(0)-100//96797(0)-20//OK
					}
					if (machine_num == 40) {
						Threshold = 131400;//131600(0)-100//OK-10//16500(0)-100//OK
					}

					if (machine_num == 60) {
						Threshold = 158155;//158755(0)-100,158655(0)-300,158355(0)-200//OK
					}
					}
				else Threshold = calculateThreshold();
			}
			std::cout << job_num << "x" << machine_num << "\t" << "Current threshold is " << Threshold << std::endl;
			generate_jobTojobInfo();
		}

		void generate_scenario() {
			if (!scenario.empty()) {
				std::cout << "Scenario Already " << std::endl;
				//PRINT_CURRENT_LOCATION();
				return;
			}
			scenario = std::vector<std::vector<std::vector<int>>>(scenario_num,
				std::vector<std::vector<int>>(job_num,
					std::vector<int>(machine_num)));

			for (int i = 0; i < scenario_num; i++) {
				for (int j = 0; j < job_num; j++) {
					for (int m = 0; m < machine_num; m++) {
						scenario[i][j][m] = generate_random_int(1,100);
					}
				}
			}
			//if (job_num <= 20) generate_threshold();
			//else
			{
				Threshold = calculateThreshold();
				std::cout << "Current threshold is " << Threshold << std::endl;
			}

			generate_jobTojobInfo();
			saveParamToFile();
			return;
		}


	private:
		// threshold related
		void generate_threshold() {
			if (scenario.empty()) {
				std::cout << "Scenario not exist" << std::endl;
				//PRINT_CURRENT_LOCATION();
				return;
			}

			if (Threshold) {
				std::cout << "Threshold already exist" << std::endl;
				//PRINT_CURRENT_LOCATION();
				return;
			}

			int totalSum = 0;

			for (const auto& i : scenario) {
				for (const auto& job : i) {
					totalSum += std::accumulate(job.begin(), job.end(), 0);
				}
			}
			std::cout << "total sum is " << totalSum << std::endl;
			Threshold = (int)(0.37 *  totalSum / scenario_num);
			std::cout << "Current threshold is " <<  Threshold << std::endl;
			return;

		}

		// 生成随机序列的辅助函数
		std::vector<int> generateRandomSequence(int job_num) {
			std::vector<int> sequence(job_num);
			std::iota(sequence.begin(), sequence.end(), 0);  // 生成 [0, 1, 2, ..., job_num-1]
			std::shuffle(sequence.begin(), sequence.end(), std::mt19937{ std::random_device{}() });
			return sequence;
		}

		int calculateMakespan(std::vector<std::vector<int>>& processing_time, std::vector<int>& seq) {
			std::vector<std::vector<node>> sequence_info_(machine_num, std::vector<node>(job_num));


			// Calculate the completion times for the first job in the sequence
			int this_job = seq[0];
			sequence_info_[0][0] = node(0, processing_time[this_job][0], this_job);
			for (int j = 1; j < machine_num; ++j) {
				sequence_info_[j][0] = node(sequence_info_[j - 1][0].end, processing_time[this_job][j], this_job);

			}
			//for (int j = 0; j < num_machines; j++) {
			//	std::cout << sequence_info_[j][0].begin << " " << sequence_info_[j][0].end << std::endl;;
			//}



			// Calculate the completion times for the rest of the jobs
			for (int j = 1; j < job_num; ++j) {
				int job = seq[j];
				sequence_info_[0][j] = node(sequence_info_[0][j - 1].end, processing_time[job][0], job);

				// 应该向后推的距离
				int max_push = 0;
				// Initial
				for (int m = 1; m < machine_num; ++m) {
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
					for (int m = 0; m < machine_num; m++) {
						sequence_info_[m][j].update(max_push);
					}
			}

			return sequence_info_[machine_num-1][job_num-1].end;
		}

		// 计算 threshold 的主函数
		int calculateThreshold() {

			long long makespan_sum = 0.0;
			int sequence_count = 10;

			// 生成5个随机序列并计算每个序列在每个场景的makespan
			for (int i = 0; i < sequence_count; ++i) {
				std::vector<int> sequence = generateRandomSequence(job_num);

				for (int j = 0; j < scenario_num; j++) {
					int makespan = calculateMakespan(scenario[j], sequence);
					makespan_sum += makespan;
				}
			}

			// 计算 makespan 的平均值，并乘以 90% 得到 threshold
			double average_makespan = makespan_sum / (scenario_num * sequence_count);

			float factor = 0.75;

			if (job_num == 20) {
				if (machine_num == 5) {
					factor = 0.955;
				}
				if (machine_num == 10) {
					factor = 0.965;
				}
				if (machine_num == 15) {
					factor = 0.965;
				}
				if (machine_num == 20) {
					factor = 0.97;
				}
			}
			if (job_num == 200) {
				if (machine_num == 5) {
					factor = 0.951;
				}

				if (machine_num == 20) {
					factor = 0.945;
				}
				if (machine_num == 15) {
					factor = 0.943;
				}
				if (machine_num == 10) {
					factor = 0.947;
				}
				
			}
			if (job_num == 80) {
				if (machine_num == 20) {
					factor = 0.9429;
				}
				if (machine_num == 15) {
					factor = 0.945;
				}
				if (machine_num == 5) {
					factor = 0.95;
				}
				if (machine_num == 10) {
					factor = 0.945;
				}
			}

			if (job_num == 50) {
				if (machine_num == 5) {
					factor = 0.947;
				}
				if (machine_num == 10) {
					factor = 0.947;
				}
				if (machine_num == 15) {
					factor = 0.9415;
				}
				if (machine_num == 20) {
					factor = 0.94;
				}
			}

			if (job_num == 100) {
				if (machine_num == 20) {
					factor = 0.94;
				}
			}
			

			return average_makespan * factor;
		}





		/*DEBUGGING TOOL*/
		template<typename T>
		void writeToFile(std::ofstream& file, const std::string& name, const T& value) {
			file << name << "=" << value << std::endl;
		}

		void writeToFile(std::ofstream& file) {
			file << "{\n";
			for (int s = 0; s < scenario.size(); s++) {
				file << "\t{\n";
				for (int j = 0; j < scenario[0].size(); j++) {
					file << "\t\t{";
					for (int m = 0; m < scenario[0][0].size(); m++) {
						file << scenario[s][j][m];

						if (m != scenario[0][0].size() - 1) {
							file << ", ";
						}
					}
					file << "}";
					if (j != scenario[0].size() - 1) {
						file << ",";
					}
					file << "\n";
				}
				file << "\t}";
				if (s != scenario.size() - 1) {
					file << ",";
				}
				file << "\n";
			}
			file << "}";

		}


		void saveParamToFile() {
			std::string file_name = "J_" + std::to_string(job_num) + "_M_" + std::to_string(machine_num) + "-paramInfo.txt";
			std::string filePath = "../data/" + file_name;
			std::ofstream file(filePath);

			if (!file.is_open()) {
				std::cerr << "Error opening file: " << filePath << std::endl;
				return;
			}

			// 将每个成员变量写入文件
			writeToFile(file, "alpha", alpha);
			writeToFile(file, "gamma", gamma);
			writeToFile(file, "selection_rate", selection_rate);
			writeToFile(file, "max_gen", max_gen);
			writeToFile(file, "job_num", job_num);
			writeToFile(file, "machine_num", machine_num);
			writeToFile(file, "pop_size", pop_size);
			writeToFile(file, "Threshold", Threshold);
			writeToFile(file, "scenario_num", scenario_num);
			writeToFile(file);

			file.close();
			std::cout << "Param information saved to " << filePath << std::endl;
		}

	};
}