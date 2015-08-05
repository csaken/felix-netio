#ifndef TEST_UTILITY_HPP
#define TEST_UTILITY_HPP

#include <chrono>
#include <thread>

template <typename T>
bool wait_for_condition_timeout(T& condition, double seconds)
{
	using namespace std::chrono;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
	
	while( !condition )
	{
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		if(time_span.count() > seconds)
			return false;

		std::this_thread::yield();
	}

	return true;
}

#endif
