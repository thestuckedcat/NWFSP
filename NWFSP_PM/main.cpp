#include"Swarm_Intelligence.h"
#include"solution.h"

int main() {
	const int max_gen = 2000;
	const int job_num = 20;
	const int machine_num = 5;
	const int pop_size = 100;
	const int Threshold = 2020;
	const int scenario_num = 20;


	Memetic<NWFSP_Solution, Population> test1(	max_gen,
												job_num,
												machine_num,
												pop_size,
												Threshold,
												scenario_num);
	test1.LN_UN_comparison();



}