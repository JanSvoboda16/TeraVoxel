/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "pch.h"
#include "Logger.h"

Logger::Logger(const std::string &filePath)
{
	timeStart = std::chrono::high_resolution_clock::now();
	fileStream.open(filePath);
	fileStream << "component" << ";" << "action" << ";" << "value" << ";" << "context" << ";" << "timestamp" << "\n";
}

std::string Logger::GetTimeStamp()
{
	auto elapsed = std::chrono::high_resolution_clock::now() - timeStart;

	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
		elapsed).count();

	return std::to_string(microseconds / 1000000.0);
}

Logger::~Logger()
{
	fileStream.close();
}

void Logger::Initialize(const std::string &filePath)
{
	logger = new Logger(filePath);
}

void Logger::DestroyInstance() {
	delete logger;
}

Logger* Logger::GetInstance()
{
	return logger;
}

void Logger::LogEvent(const std::string& component, const std::string& action,  const std::string& value, const std::string& context)
{
	/*
	mutex.lock();
	fileStream << component << ";" << action << ";"  << value << ";" << context << ";" << GetTimeStamp() << "\n";
	mutex.unlock();
	*/
}


