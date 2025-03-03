/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <iostream>
#include <exception>
using namespace std;

class ServerException : public exception
{

private:
    std::string _message;

public:
    ServerException(const std::string &message) {
        _message = message;
    }

    const char* what() const throw () {
        return _message.c_str();
    }
};

