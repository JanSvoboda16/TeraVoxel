/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>

class Logger
{
private:
	Logger(const std::string &filePath);
	inline static Logger* logger;
	std::ofstream fileStream;
	std::mutex mutex;
	std::chrono::steady_clock::time_point timeStart;
	std::string GetTimeStamp();
	~Logger();
public:
	static void Initialize(const std::string &filePath);
	static Logger* GetInstance();
	static void DestroyInstance();
	void LogEvent(const std::string& component, const std::string& action, const std::string& value = "", const std::string & context = "");
};

