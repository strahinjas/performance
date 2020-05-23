#include "simulation.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iomanip>

const std::string Simulation::directory = "../results/";
const std::string Simulation::resultsFileName = "simulation_results";
const std::string Simulation::averageFileName = "simulation_results_averaged";
const std::string Simulation::extension = ".txt";

const std::unordered_map<uint64_t, uint64_t> Simulation::cycles = { { 10, 10 }, { 15, 25 }, { 20, 100 } };

void Simulation::simulate(uint64_t n, double simulationTime)
{
	U.clear();   X.clear();
	Nsr.clear(); T.clear();

	for (uint64_t i = 0; i < system.K.size(); i++)
	{
		initializeEnvironment(n, system.K[i]);

		run(i, simulationTime);

		calculateParameters(false, 0ULL, simulationTime);
	}

	writeResultsFile(directory + resultsFileName + "_" + std::to_string(n) + extension);

	uint64_t iterations = cycles.at(n);

	for (uint64_t i = 0; i < system.K.size(); i++)
	{
		for (uint64_t j = 0; j < iterations - 1; j++)
		{
			initializeEnvironment(n, system.K[i]);

			run(i, simulationTime);

			calculateParameters(true, i, simulationTime);
		}

		for (double& u : U[i]) u /= iterations;
		for (double& x : X[i]) x /= iterations;
		for (double& nsr : Nsr[i]) nsr /= iterations;

		T[i] /= iterations;
	}

	writeResultsFile(directory + averageFileName + "_" + std::to_string(n) + extension);
}

void Simulation::initializeEnvironment(uint64_t n, uint64_t k)
{
	servers.clear();

	time = 0.0;
	eventQueue = std::priority_queue<Event, std::vector<Event>, EventCompare>();

	taskCount = 0ULL;
	cumulativeResponseTime = 0.0;
	arrivalTimes.clear();

	for (uint64_t i = 0; i < system.NUMBER_OF_RESOURCES; i++)
		servers.push_back(Server(1.0 / system.S[i]));

	for (uint64_t i = 0; i < k; i++)
		servers.push_back(Server(1.0 / system.S[System::USER_DISK]));

	serverTaskCount = std::vector<uint64_t>(servers.size(), 0ULL);

	runCount = 0ULL;

	for (uint64_t i = 0; i < n; i++)
	{
		if (i == 0) insertEvent(System::PROCESSOR, i);
		else servers[System::PROCESSOR].taskQueue.push(i);

		arrivalTimes.insert({ i, 0.0 });
	}
}

void Simulation::insertEvent(uint64_t server, uint64_t task)
{
	Server& target = servers[server];

	if (target.busy)
	{
		target.taskQueue.push(task);
	}
	else
	{
		double processTime = target.exponential(generator);

		target.workTime += processTime;
		target.runningTask = task;
		target.busy = true;

		eventQueue.push(Event(server, task, time + processTime));
	}
}

void Simulation::run(uint64_t index, double simulationTime)
{
	while (time < simulationTime)
	{
		for (uint64_t i = 0; i < servers.size(); i++)
		{
			serverTaskCount[i] += servers[i].taskQueue.size() + (servers[i].busy ? 1ULL : 0ULL);
		}

		runCount++;

		if (eventQueue.empty())
		{
			break;
		}
		else
		{
			Event next = eventQueue.top();
			eventQueue.pop();
			
			time = next.endTime;

			Server& server = servers[next.server];

			server.tasksProcessed++;
			server.busy = false;

			if (!server.taskQueue.empty())
			{
				insertEvent(next.server, server.taskQueue.front());
				server.taskQueue.pop();
			}

			double number = uniform(generator);
			double probability = 0.0;

			matrix& transitions = system.transitionMatrices[index];

			uint64_t target = transitions[next.server].size() - 1;

			for (uint64_t i = 0; i < transitions[next.server].size(); i++)
			{
				probability += transitions[next.server][i];

				if (number <= probability)
				{
					target = i;
					break;
				}
			}

			insertEvent(target, next.task);

			if (target == next.server && target == System::PROCESSOR)
			{
				taskCount++;
				cumulativeResponseTime += next.endTime - arrivalTimes[next.task];
				arrivalTimes[next.task] = next.endTime;
			}
		}
	}
}

void Simulation::calculateParameters(bool accumulate, uint64_t index, double simulationTime)
{
	if (accumulate)
	{
		for (uint64_t i = 0; i < servers.size(); i++)
		{
			U[index][i] += servers[i].workTime / simulationTime;
			X[index][i] += servers[i].tasksProcessed * 1000.0 / simulationTime;
			Nsr[index][i] += ((double) serverTaskCount[i]) / runCount;
		}

		T[index] += cumulativeResponseTime / (taskCount * 1000.0);
	}
	else
	{
		std::vector<double> u;
		std::vector<double> x;
		std::vector<double> nsr;

		for (uint64_t i = 0; i < servers.size(); i++)
		{
			u.push_back(servers[i].workTime / simulationTime);
			x.push_back(servers[i].tasksProcessed * 1000.0 / simulationTime);
			nsr.push_back(((double) serverTaskCount[i]) / runCount);
		}

		U.push_back(u);
		X.push_back(x);
		Nsr.push_back(nsr);

		T.push_back(cumulativeResponseTime / (taskCount * 1000.0));
	}
}

void Simulation::writeResultsFile(const std::string& fileName) const
{
	std::ofstream output(fileName, std::ofstream::out | std::ofstream::trunc);

	for (uint64_t i = 0; i < U.size(); i++)
	{
		output << "###############################" << std::endl;
		output << "############ K = " << system.K[i] << " ############" << std::endl;
		output << "###############################" << std::endl << std::endl;

		output << std::left
			   << std::setw(10) << "Server"
			   << std::setw(10) << "U"
			   << std::setw(10) << "X"
			   << std::setw(10) << "Nsr";

		output << std::endl;

		for (uint64_t j = 0; j < U[i].size(); j++)
		{
			output << std::left
				   << std::setw(10) << j
				   << std::setw(10) << U[i][j]
				   << std::setw(10) << X[i][j]
				   << std::setw(10) << Nsr[i][j];

			output << std::endl;
		}

		output << std::endl;

		output << "T = " << T[i] << std::endl;

		output << std::endl;
	}

	output << "###############################";

	output.close();
}