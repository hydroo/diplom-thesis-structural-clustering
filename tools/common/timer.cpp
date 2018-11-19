#include "common/timer.hpp"

#include <time.h>

const clockid_t usedClock = CLOCK_REALTIME;

Timer::Timer() {
	last = 0;
	restart();
}

void Timer::start() {
	timespec t;
	auto ret = clock_gettime(usedClock, &t);
	if (ret == 0) {
		last = (int64_t) t.tv_nsec + 1000 * 1000 * 1000 * (int64_t) t.tv_sec;
	} else {
		// errno is set. maybe look at it.
		last = std::numeric_limits<int64_t>::max();
	}
}

int64_t Timer::elapsed() const {
	timespec t;
	auto ret = clock_gettime(usedClock, &t);
	if (ret == 0) {
		int64_t now = (int64_t) t.tv_nsec + 1000 * 1000 * 1000 * (int64_t) t.tv_sec;
		return now - last;
	} else {
		// errno is set. maybe look at it.
		return -1;
	}
}

int64_t Timer::restart() {
	auto ret = elapsed();
	start();
	return ret;
}

int64_t Timer::resolution() const {
	timespec t;
	auto ret = clock_getres(usedClock, &t);

	if (ret == 0) {
		return (int64_t) t.tv_nsec + 1000 * 1000 * 1000 * (int64_t) t.tv_sec;
	} else {
		// errno is set. maybe look at it.
		return -1;
	}
}
