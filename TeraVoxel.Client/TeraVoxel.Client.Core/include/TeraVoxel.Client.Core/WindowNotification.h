#pragma once
#include <utility>
#include <functional>
#include <string>
#include <map>

class WindowNotification
{
public:
	void Register(const std::string& key, std::function<void()> function)
	{
		_namedFunctions.insert({ key, function });
	}

	void Register(std::function<void()> function)
	{
		_functions.push_back(function);
	}

	bool Register(void* context, const std::string& key, std::function<void()> function)
	{
		std::string functionKey = std::to_string((int64_t)context) + key;

		if (_namedFunctions.contains(functionKey))
		{
			return false;
		}

		_namedFunctions.insert({ functionKey, function });

		return true;
	}

	void Notify()
	{
		for (const auto& f : _functions)
		{
			f();
		}
		for (const auto& [key, f] : _namedFunctions)
		{
			f();
		}
	}

	void Unregister(const std::string& key)
	{
		_namedFunctions.erase(key);
	}

	void Unregister(void* context, const std::string& key)
	{
		_namedFunctions.erase(std::to_string((int64_t)context) + key);
	}
private:
	std::map<std::string, std::function<void()>> _namedFunctions;
	std::list<std::function<void()>> _functions;
};

