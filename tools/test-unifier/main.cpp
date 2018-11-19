#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/unifier.hpp"

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	Unifier<function_t> u;

	u.insert(1, "/home/brendel/trace1", 1, "a");
	u.insert(1, "/home/brendel/trace1", 2, "b");
	u.insert(1, "/home/brendel/trace1", 3, "a");
	u.insert(2, "/home/brendel/trace2", 1, "a");
	u.insert(2, "/home/brendel/trace2", 2, "c");

	assert(u.map(1, 1) == 1);
	assert(u.map(1, 2) == 2);
	assert(u.map(1, 3) == 1); // two diff tokens for same function name shall be mapped onto the same token
	assert(u.map(2, 1) == 1); // normal mapping
	assert(u.map(2, 2) == 3); // 2 is already reserved and shall be mapped onto 3

	return 0;
}

