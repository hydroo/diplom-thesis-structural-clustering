#include "profile/profile.hpp"

/* *** misc *** */

bool operator<(const FunctionTuple& a, const FunctionTuple& b) {
	for (int i = 0; i < std::min(a.size(), b.size()); i += 1) {
		if      (a[i] < b[i]) { return true; }
		else if (a[i] > b[i]) { return false; }
	}

	if (a.size() < b.size()) {
		return true;
	} else {
		return false;
	}
}
