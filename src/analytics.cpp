#include "system.h"
#include "analytics.h"

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <armadillo>

const std::string Analytics::directory = "../results/";
const std::string Analytics::demandsFileName = "analytical_demands";
const std::string Analytics::resultsFileName = "analytical_results";
const std::string Analytics::extension = ".txt";

void Analytics::gordonNewell()
{
	for (uint64_t i = 0; i < system.K.size(); i++)
	{
		std::vector<double>	results;

		arma::mat P = convertToArmaMatrix(system.transitionMatrices[i]);

		// M = P^T - I
		
		arma::mat M = P.t() - arma::eye(P.n_rows, P.n_cols);

		// solve M * X = 0

		arma::mat R = null(M);

		// divide solutions by servers' service rates

		for (uint64_t j = 0; j < R.n_rows; j++)
			R(j, 0) /= system.rates[i][j];

		// normalize with processor's demand

		R = R / R(0, 0);

		for (uint64_t i = 0; i < R.n_rows; i++)
			results.push_back(R(i, 0));

		demands.push_back(results);
	}

	writeDemandsFile();
}

void Analytics::buzen(uint64_t n)
{
	// reset system data

	G.clear();   U.clear(); X.clear();
	Nsr.clear(); R.clear(); T.clear();

	for (uint64_t i = 0; i < demands.size(); i++)
	{
		// calculate G(n) function using optimized Buzen's algorithm

		std::vector<double> g(n + 1, 0);

		g[0] = 1;

		for (uint64_t j = 0; j < demands[i].size(); j++)
			for (uint64_t k = 1; k <= n; k++)
				g[k] += demands[i][j] * g[k - 1];

		G.push_back(g);

		// calculate U and X

		std::vector<double> u;
		std::vector<double> x;

		double factor = g[n - 1] / g[n];

		for (uint64_t j = 0; j < demands[i].size(); j++)
		{
			u.push_back(demands[i][j] * factor);
			x.push_back(demands[i][j] * factor * system.rates[i][j]);
		}

		U.push_back(u);
		X.push_back(x);

		// calculate average task count per server and
		// server's response time

		std::vector<double> averages;
		std::vector<double> responses;

		for (uint64_t j = 0; j < demands[i].size(); j++)
		{
			double cumulativeDemand = demands[i][j];
			double count = 0.0;

			for (uint64_t k = 1; k <= n; k++)
			{
				count += cumulativeDemand * g[n - k] / g[n];
				cumulativeDemand *= demands[i][j];
			}

			averages.push_back(count);
			responses.push_back(count * 1000.0 / x[j]);
		}

		Nsr.push_back(averages);
		R.push_back(responses);

		// calculate system response time
		// using Little's formula

		T.push_back(n / (0.1 * X[i][System::PROCESSOR]));
	}

	writeResultsFile(n);
}

arma::mat Analytics::convertToArmaMatrix(const matrix& matrix) const
{
	size_t size = matrix.size();

	arma::mat P(size, size);

	for (arma::uword i = 0; i < size; i++)
		for (arma::uword j = 0; j < size; j++)
			P(i, j) = matrix[i][j];

	return P;
}

void Analytics::writeDemandsFile() const
{
	std::ofstream output(directory + demandsFileName + extension,
						 std::ofstream::out | std::ofstream::trunc);

	for (uint64_t i = 0; i < demands.size(); i++)
	{
		output << "###############################################" << std::endl;
		output << "#################### K = " << system.K[i] << " ####################" << std::endl;
		output << "###############################################" << std::endl << std::endl;

		for (uint64_t j = 0; j < demands[i].size(); j++)
		{
			std::string demand = "x[" + std::to_string(j) + "]";

			output << std::left << std::setw(6) << demand;
			output << std::left << std::setw(2) << "=" << demands[i][j] << std::endl;
		}

		output << std::endl;
	}

	output << "###############################################";

	output.close();
}

void Analytics::writeResultsFile(uint64_t n) const
{
	std::ofstream output(directory + resultsFileName + "_" + std::to_string(n) + extension,
						 std::ofstream::out | std::ofstream::trunc);

	for (uint64_t i = 0; i < demands.size(); i++)
	{
		output << "###############################################" << std::endl;
		output << "#################### K = " << system.K[i] << " ####################" << std::endl;
		output << "###############################################" << std::endl << std::endl;

		output << std::left
			   << std::setw(10) << "Server"
			   << std::setw(10) << "U"
			   << std::setw(10) << "X [1/s]"
			   << std::setw(10) << "Nsr"
			   << std::setw(10) << "R [ms]";

		output << std::endl;

		for (uint64_t j = 0; j < demands[i].size(); j++)
		{
			output << std::left
				   << std::setw(10) << j
				   << std::setw(10) << U[i][j]
				   << std::setw(10) << X[i][j]
				   << std::setw(10) << Nsr[i][j]
				   << std::setw(10) << R[i][j];

			output << std::endl;
		}

		output << std::endl;
		
		output << "T = " << T[i] << "s" << std::endl;
		
		output << std::endl;
	}

	output << "###############################################";

	output.close();
}