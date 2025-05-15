/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <utility>
#include <functional>
#include <string>
#include <map>

/// <summary>
/// Provide comunication between multiple windows
/// </summary>
class WindowNotification
{
public:
	/// <summary>
	/// Registers callback function. 
	/// </summary>
	/// <param name="key">Identifier</param>
	/// <param name="function">Callback function</param>
	void Register(const std::string& key, std::function<void()> function)
	{
		_namedFunctions.insert({ key, function });
	}

	/// <summary>
	/// Registers callback function.
	/// </summary>
	/// <param name="function">Callback function</param>
	void Register(std::function<void()> function)
	{
		_functions.push_back(function);
	}

	/// <summary>
	/// Registers callback function. 
	/// </summary>
	/// <param name="context">Pointer to registered object (use this)</param>
	/// <param name="key">Identifier</param>
	/// <param name="function">Callback function</param>
	/// <returns></returns>
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

	/// <summary>
	/// Notifies all registered windows by callbacks. 
	/// </summary>
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

	/// <summary>
	/// Unregisters the callback function by it's key from this notification.
	/// </summary>
	/// <param name="key">Identifier</param>
	void Unregister(const std::string& key)
	{
		_namedFunctions.erase(key);
	}

	/// <summary>
	/// Unregisters the callback function by it's key and registered object from this notification. 
	/// </summary>
	/// <param name="context">Registered object (use this)</param>
	/// <param name="key">Identifier</param>
	void Unregister(void* context, const std::string& key)
	{
		_namedFunctions.erase(std::to_string((int64_t)context) + key);
	}

private:
	std::map<std::string, std::function<void()>> _namedFunctions;
	std::list<std::function<void()>> _functions;
};

