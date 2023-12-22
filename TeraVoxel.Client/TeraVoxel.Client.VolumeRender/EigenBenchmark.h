#pragma once
#include <Eigen/Dense>
#include <chrono>
#include <iostream>

using Eigen::Matrix4f;
using Eigen::Vector3f;

class EigenBenchmark
{
public:
	static long long RunEigen(size_t iterations)
	{
		auto timeStart = std::chrono::high_resolution_clock::now();
		
		Vector3f vector1(0, 0, 0);
		Vector3f vector2(1, 2, 3);
		float sum = 0;

		for (size_t i = 0; i < iterations; i++)
		{
			vector1[0] += 1.0f;
			vector1[1] += 1.0f;
			vector1[2] += 1.0f;

			vector1 += vector2;
			vector2 += vector1;
			sum += vector1.dot(vector2);
		}

		auto elapsed = std::chrono::high_resolution_clock::now() - timeStart;
		std::cout << sum << "\n";
		return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	}

	static long long RunNormal(size_t iterations)
	{
		auto timeStart = std::chrono::high_resolution_clock::now();

		float ax = 0, ay = 0, az = 0;
		float bx = 1, by = 2, bz = 3;

		float sum = 0;

		for (size_t i = 0; i < iterations; i++)
		{
			ax += 1.0f;
			ay += 1.0f;
			az += 1.0f;
			
			ax += bx;
			ay += by;
			az += bz;

			bx += ax;
			by += ay;
			bz += az;

			sum += ax * bx + ay * by + az * bz;		
		}

		auto elapsed = std::chrono::high_resolution_clock::now() - timeStart;
		std::cout << sum << "\n";
		return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	}
};

