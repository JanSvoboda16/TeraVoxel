#pragma once
#include "Camera.h"
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"
#include <chrono>
#include <queue>

using nlohmann::json;

struct CameraPosition
{
	long long milisFromStart;
	float x;
	float y;
	float z;

	std::vector<float> rotation;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CameraPosition, milisFromStart, x, y, z, rotation)
};

class TrackableCamera : public Camera
{
	
public:
	TrackableCamera(const std::string &fileName, const Vector3f& observerCenter, int observerDistance, const Vector3f& voxelDimensions, int width, int height, float viewAngle, float nearPlaneDistance = 100, float farPlaneDiscance = 10000) : Camera(observerCenter, observerDistance, voxelDimensions, width, height, viewAngle, nearPlaneDistance, farPlaneDiscance), _fileName(fileName)
	{
		_fileName = fileName;
		_timeStart = std::chrono::high_resolution_clock::now();	
	}

	void ReadFile()
	{
		_readedPositions = std::queue<CameraPosition>();
		_fileReader.open(_fileName);
		std::string line;
		std::getline(_fileReader, line);

		while (line != "")
		{
			json data = json::parse(line);
			_readedPositions.push(data.get<CameraPosition>());
			std::getline(_fileReader, line);
		}

		_fileReader.close();
	}

	void ResetTimer()
	{
		_timeStart = std::chrono::high_resolution_clock::now();
	}

	bool ReadPosition()
	{
		auto elapsed = std::chrono::high_resolution_clock::now() - _timeStart;
		long long miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			elapsed).count();

		CameraPosition pos;
		bool any = false;
		int size = _readedPositions.size();
		for (size_t i = 0; i < size; i++)
		{
			if (_readedPositions.front().milisFromStart < miliseconds)
			{
				pos = _readedPositions.front();
				_readedPositions.pop();
				any = true;
			}
			else
			{
				break;
			}
		}

		if (any)
		{
			Matrix4f rotation;
			std::memcpy(rotation.data(), pos.rotation.data(), sizeof(float) * 16);
			this->ChangePosition(Vector3f(pos.x, pos.y,pos.z));
			this->SetRotationMatrix(rotation);
		}
		return any;
	}

	void CleanRecord()
	{
		_removePrevious = true;
	}

	void RecordPostition()
	{
		auto elapsed = std::chrono::high_resolution_clock::now() - _timeStart;
		long long miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			elapsed).count();

		auto position = this->GetPosition();
		auto rotation = this->GetRotationMatrix().data();
		auto rotationSerialized = std::vector<float>(&rotation[0], &rotation[16]);


		CameraPosition positionObject = { miliseconds, position[0], position[1], position[2], std::move(rotationSerialized) };
		_file = std::ofstream(_fileName, _removePrevious ? std::ios::trunc :std::ios::app);
		_removePrevious = false;
		json data = positionObject;
		_file << data << "\n";
		_file.close();
	}

private:
	std::queue<CameraPosition> _readedPositions;
	std::string _fileName;
	std::ofstream _file;
	std::ifstream _fileReader;
	bool _removePrevious = true;
	std::chrono::steady_clock::time_point _timeStart;
};

