/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <string>
class HttpServerService {
public:
	std::string Url;
	HttpServerService(const std::string& url) {
		Url = url;
	}
};