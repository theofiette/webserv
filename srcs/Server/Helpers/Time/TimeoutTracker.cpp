#include "TimeoutTracker.hpp"

TimeoutTracker::TimeoutTracker() : _timerStarted(false), _expired(false), _startTime(0), _timeout(5)
{
	processTime();
}

TimeoutTracker::~TimeoutTracker(){}

void TimeoutTracker::processTime()
{
	if (_timerStarted == false)
	{
		_timerStarted = true;
		_startTime = std::time(0);
	}
}

bool TimeoutTracker::isExpired()
{
	// std::cout << "TIME IS : " << std::time(0) - _startTime << "\n";	
	if (std::time(0) - _startTime >= _timeout)
	{
		_expired = true;
		// std::cout << "Connection timed out..." << std::endl; 
	}
	return (_expired);
}