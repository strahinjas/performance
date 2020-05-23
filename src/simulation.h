#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "system.h"

#include <string>
#include <random>
#include <chrono>
#include <queue>
#include <vector>
#include <unordered_map>

class Simulation
{
public:
	Simulation(const System& system) : system(system) {}

	void simulate(uint64_t n, double simulationTime);
private:
	void initializeEnvironment(uint64_t n, uint64_t k);

	void insertEvent(uint64_t server, uint64_t task);

	void run(uint64_t index, double simulationTime);

	void calculateParameters(bool accumulate, uint64_t index, double simulationTime);

	void writeResultsFile(const std::string& fileName) const;

	static const std::string directory;
	static const std::string resultsFileName;
	static const std::string averageFileName;
	static const std::string extension;

	static const std::unordered_map<uint64_t, uint64_t> cycles;

	class Server
	{
	public:
		Server(double lambda)
		{
			exponential = std::exponential_distribution<double>(lambda);
		}

		double workTime = 0.0;
		uint64_t tasksProcessed = 0ULL;
		uint64_t runningTask = 0ULL;

		bool busy = false;

		std::queue<uint64_t> taskQueue;

		std::exponential_distribution<double> exponential;
	};

	class Event
	{
	public:
		Event(uint64_t server, uint64_t task, double endTime) : server(server), task(task), endTime(endTime) {}

		uint64_t server;
		uint64_t task;
		double endTime;
	};

	class EventCompare
	{
	public:
		bool operator()(const Event& left, const Event& right) const
		{
			return left.endTime > right.endTime;
		}
	};

	System system;

	std::vector<Server> servers;
	std::vector<uint64_t> serverTaskCount;

	uint64_t runCount = 0ULL;

	double time = 0.0;

	std::priority_queue<Event, std::vector<Event>, EventCompare> eventQueue;

	uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::default_random_engine generator = std::default_random_engine(seed);
	std::uniform_real_distribution<double> uniform = std::uniform_real_distribution<double>(0.0, 1.0);

	// average response time calculation

	uint64_t taskCount = 0ULL;
	double cumulativeResponseTime = 0.0;
	std::unordered_map<uint64_t, double> arrivalTimes;

	std::vector<std::vector<double>> U;
	std::vector<std::vector<double>> X;
	std::vector<std::vector<double>> Nsr;
	std::vector<double> T;
};

#endif