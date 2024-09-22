#pragma once
#include <chrono>
#include <string>

namespace RGS {

	class Timer {
	public:
		Timer();
		~Timer();

		void Stop();

		double GetDuration();

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	};
}