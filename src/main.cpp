#include "system.h"

#include <iostream>

int main()
{
	System system;

	const double simulationTime = 10000.0;

	system.calculateTheoreticalParameters();
	std::cout << "Analytical calculation finished!" << std::endl;

	system.performSimulation(simulationTime);
	std::cout << "Simulation finished!" << std::endl;

	std::cout << "Results are stored in appropriate files." << std::endl;

	return 0;
}