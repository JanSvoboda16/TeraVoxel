/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "pch.h"
#include "Logger.h"

Logger::Logger(const std::string &filePath)
{
	fileStream.open(filePath);
}

std::string Logger::GetTimeStamp()
{
	std::stringstream time;
	time  << clock();
	return time.str();
}

Logger::~Logger() {
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

void Logger::LogEvent(const std::string& flag, const std::string& name, const std::string& message)
{
	fileStream << flag << ";" << name << ";" << message << ";" << GetTimeStamp() << "\n";
}


