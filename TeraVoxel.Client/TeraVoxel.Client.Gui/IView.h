#pragma once
class IView
{
public:
	virtual ~IView() {};
	virtual void Update() = 0;
};

