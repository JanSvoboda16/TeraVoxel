/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <stdlib.h>
#include <iostream>
#include <fstream>

class Logger
{
private:
	Logger(const std::string &filePath);
	inline static Logger* logger;
	std::ofstream fileStream;
	std::string GetTimeStamp();
	~Logger();
public:
	static void Initialize(const std::string &filePath);
	static Logger* GetInstance();
	static void DestroyInstance();
	void LogEvent(const std::string& flag, const std::string& name, const std::string& message = "");
};

