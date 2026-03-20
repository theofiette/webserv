#pragma once
#include <ctime>
#include <iostream>

class TimeoutTracker
{
	private:
		bool	_timerStarted;
		bool	_expired;
		
		std::time_t _startTime;
		std::time_t	_timeout;

		void processTime();
	public:

		bool isExpired();

		TimeoutTracker();
		~TimeoutTracker();
};