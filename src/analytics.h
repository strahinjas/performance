#ifndef _ANALYTICS_H_
#define _ANALYTICS_H_

#include "system.h"

#include <string>
#include <vector>
#include <armadillo>

class Analytics
{
public:
	Analytics(const System& system) : system(system) {}

	void gordonNewell();
	void buzen(uint64_t n);
private:
	arma::mat convertToArmaMatrix(const matrix& matrix) const;

	void writeDemandsFile() const;
	void writeResultsFile(uint64_t n) const;

	static const std::string directory;
	static const std::string demandsFileName;
	static const std::string resultsFileName;
	static const std::string extension;

	System system;

	std::vector<std::vector<double>> demands;

	std::vector<std::vector<double>> G;
	std::vector<std::vector<double>> U;
	std::vector<std::vector<double>> X;
	std::vector<std::vector<double>> Nsr;
	std::vector<std::vector<double>> R;
	std::vector<double> T;
};

#endif