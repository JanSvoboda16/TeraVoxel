/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
class IView
{
public:
	virtual ~IView() {};
	virtual void Update() = 0;
};

