#include <iostream>
#include <time.h>
#include <cassert>
#include "Logger.h"

using namespace Common;

void Logger::Logging(int errorCode, int line, const WCHAR* cause)
{
	struct tm t;
	time_t timer;
	_time64(&timer);

	localtime_s(&t, &timer);
	FILE* fp;

	WCHAR FILENAME[128] = { 0 , };
	WCHAR ErrorMessage[256] = { 0 , };

	swprintf_s(FILENAME, 128, L"Logger_%dy_%dm_%d day_%dh%dm.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_wday,
		t.tm_hour,
		t.tm_min);

	_wfopen_s(&fp, FILENAME, L"wb");
	if (fp == NULL)
	{
		return;
	}
	fputwc(0xFEFF, fp);

	swprintf_s(ErrorMessage, 256, L"Why : %s | ErrorCode : %d | Line : %d \n",
		cause,
		errorCode,
		line);
	fwprintf(fp, ErrorMessage);

	fclose(fp);
}

void Logger::LoggingWithIP(int errorCode, int line, const WCHAR* cause, const WCHAR* ip)
{
	//어느 세션인지도 기록을 해놓을까?
	struct tm t;
	time_t timer;
	_time64(&timer);

	localtime_s(&t, &timer);
	FILE* fp;

	WCHAR FILENAME[128] = { 0 , };
	WCHAR ErrorMessage[256] = { 0 , };

	swprintf_s(FILENAME, 128, L"Logger_%dy_%dm_%d day_%dh%dm.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_wday,
		t.tm_hour,
		t.tm_min);
	_wfopen_s(&fp, FILENAME, L"wb");
	if (fp == NULL)
	{
		return;
	}
	fputwc(0xFEFF, fp);

	swprintf_s(ErrorMessage, 256, L"ip : %s | Why : %s | ErrorCode : %d | Line : %d \n",
		ip,
		cause,
		errorCode,
		line);

	fwprintf(fp, ErrorMessage);

	fclose(fp);
}