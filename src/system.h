#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <vector>

typedef std::vector<std::vector<double>> matrix;

constexpr double DEFAULT_SIMULATION_TIME = 24.0 * 60.0 * 60.0 * 1000.0;

class System
{
public:
	void calculateTheoreticalParameters();
	void performSimulation(double simulationTime = DEFAULT_SIMULATION_TIME);

	friend class Analytics;
	friend class Simulation;
private:
	void calculateRates(uint64_t k);
	void buildTransitionMatrix(uint64_t k);

	void clearComputedParameters();

	std::vector<std::vector<double>> rates;
	std::vector<matrix> transitionMatrices;

	const uint64_t NUMBER_OF_RESOURCES = 4ULL;
	const uint64_t NUMBER_OF_SYSTEM_DISKS = 3ULL;

	enum Resources
	{
		PROCESSOR,
		SYSTEM_DISK_1,
		SYSTEM_DISK_2,
		SYSTEM_DISK_3,
		USER_DISK
	};

	const std::vector<uint64_t> K { 2, 3, 4, 5, 6, 7, 8 };

	const std::vector<uint64_t> N { 10, 15, 20 };

	const std::vector<double> S { 5.0, 20.0, 15.0, 15.0, 20.0 };
};

#endif