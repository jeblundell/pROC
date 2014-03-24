/* pROC: Tools Receiver operating characteristic (ROC curves) with
   (partial) area under the curve, confidence intervals and comparison. 
   Copyright (C) 2014 Xavier Robin

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Rcpp.h>
#include <vector>
#include <string>
#include <pair> 

#include "ROC.h"

using namespace Rcpp;
using std::vector;
using std::string;


/** Computes the se and sp member vectors 
 * Check that it is consistent with the number of thresholds
 */

template <typename PredictorType>
void ROC::computeSeSp() {

	// Vector sizes
	const int ncontrols {predictor.nControls};
	const int ncases {predictor.ncases};
	const int npredictors {ncontrols + ncases};
	
	// Compute an index vector for controls and cases as R's order() function
	vector<int> index = predictor.getOrder(direction);
	
	// Cummulative sum
	// no need for tp/fp, compute se/sp directly during the cummulative sum
	// NumericVector se, sp; // Now se, sp are member variables
	// And store the cummulative sums (tp, fp) in two variables
	int currentTpSum = 0, currentFpSum = 0;
	
	for (int i = 0; i < npredictors; ++i) {
		// Compute Se/Sp
		if (predictor.isControl(index[i])) { // we have one control
			++currentFpSum;
		}
		else { // we have one case
			++currentTpSum;
		}
		
		// Determine if if is a duplicate
		bool currentDupPred = false;
		// Is predictor[i] the same as predictor[i+1]?
		if (i < npredictors - 1) {
			currentDupPred = predictor[index[i + 1]] == predictor[index[i]];
		}
		// If different, add the Se/Sp as a valid position
		if (!currentDupPred) {
			se.push_back(static_cast<double>(currentTpSum) / static_cast<double>(ncases));
			sp.push_back((static_cast<double>(ncontrols) - static_cast<double>(currentFpSum)) / static_cast<double>(ncontrols));
		}
	}
	
	// Anchor the last position to 1/0
	se.push_back(0);
	sp.push_back(1);
	
	// Reverse - can we find a way to do it right from the start?
	std::reverse(se.begin(), se.end() - 1);
	std::reverse(sp.begin(), sp.end() - 1);
	
	// Ensure the # of thresholds is the same as the lenght of se
	// Todo: maybe remove this in a future version of pROC...
	if (se.size() != thresholds.size()) {
		//stop(string("Bug in pROC: fast algorithm (C++ version) computed an incorrect number of sensitivities and specificities: ") + toString(se.size()) + " and " + toString(sp.size()) + " vs " + toString(nthresholds));
		std::runtime_error("Bug in pROC: fast algorithm (C++ version) computed an incorrect number of sensitivities and specificities.");
	}

}