#include "system.h"
#include "analytics.h"
#include "simulation.h"

#include <vector>

void System::calculateTheoreticalParameters()
{
	clearComputedParameters();

	for (const uint64_t& k : K)
	{
		calculateRates(k);
		buildTransitionMatrix(k);
	}

	Analytics analytics(*this);

	analytics.gordonNewell();

	for (const uint64_t& n : N) analytics.buzen(n);
}

void System::performSimulation(double simulationTime)
{
	if (rates.empty())
	{
		for (const uint64_t& k : K)
		{
			calculateRates(k);
			buildTransitionMatrix(k);
		}
	}

	Simulation simulation(*this);

	for (const uint64_t& n : N) simulation.simulate(n, simulationTime);
}

void System::calculateRates(uint64_t k)
{
	std::vector<double> u;

	for (uint64_t i = 0; i < NUMBER_OF_RESOURCES; i++)
		u.push_back(1000.0 / S[i]);

	for (uint64_t i = 0; i < k; i++)
		u.push_back(1000.0 / S[USER_DISK]);

	rates.push_back(u);
}

void System::buildTransitionMatrix(uint64_t k)
{
	matrix matrix;

	double userDiskRate = 0.6 / k;

	for (uint64_t i = 0; i < k + NUMBER_OF_RESOURCES; i++)
	{
		std::vector<double> row;

		for (uint64_t j = 0; j < k + NUMBER_OF_RESOURCES; j++)
		{
			if (i == 0)
			{
				if (j < 4) row.push_back(0.1);
				else row.push_back(userDiskRate);
			}
			else if (i < 4)
			{
				if (j == 0) row.push_back(0.4);
				else if (j < 4) row.push_back(0.0);
				else row.push_back(userDiskRate);
			}
			else
			{
				if (j == 0) row.push_back(1.0);
				else row.push_back(0.0);
			}
		}

		matrix.push_back(row);
	}

	transitionMatrices.push_back(matrix);
}

void System::clearComputedParameters()
{
	rates.clear();
	transitionMatrices.clear();
}