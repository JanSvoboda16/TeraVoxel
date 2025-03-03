/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <string>
class HttpManagerBase {
public:
	std::string Url;
	HttpManagerBase(const std::string& url) {
		Url = url;
	}
};