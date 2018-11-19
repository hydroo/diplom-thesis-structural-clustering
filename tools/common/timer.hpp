#ifndef TIMER_HPP
#define TIMER_HPP

#include "common/prereqs.hpp"

class Timer {
public:
	Timer();
	void start();
	int64_t elapsed() const;
	int64_t restart();
	int64_t resolution() const; // nanoseconds
private:
	int64_t last;
};

#endif // TIMER_HPP
