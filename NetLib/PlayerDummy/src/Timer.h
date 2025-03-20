#pragma once
#include <Windows.h>

namespace Common
{
	class Timer
	{
	public:
		void SetBaseTime(DWORD time);
		DWORD GetBaseTime() const;
		
		void SetTargetTime();
		DWORD GetTargetTime() const;

		void CheckElaspedTime();
	private:
		DWORD _BaseTime;
		DWORD _TargetTime;
	};
}