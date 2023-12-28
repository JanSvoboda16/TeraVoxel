#include "CPURayCastingVolumeVisualizer.h"


template<typename T>
bool CPURayCastingVolumeVisualizer<T>::DataChanged()
{
	return this->_memory.MemoryChanged();
}

template<typename T>
inline void CPURayCastingVolumeVisualizer<T>::ComputeFrameInternal(int downscale)
{
	this->_memory.Prepare();
	_settingsCopy = *_settings;
	_settingsCopy.mappingTable.RecomputeDeltas();
	_reneringPosition.store(0, std::memory_order_release);

	Vector2i screenSize = this->_camera->GetScreenSize();
	this->_camera->ChangeScreenSize(std::ceil(screenSize[1] / (float)downscale), std::ceil(screenSize[0] / (float)downscale));
	_meshVisualizer.ComputeFrame();
	_meshFramebuffer = _meshVisualizer.GetFrameBuffer();

	auto renderingThreadCount = SettingsContext::GetInstance().renderingThreadCount.load(std::memory_order::acquire);
	std::vector<std::future<void>> threads;
	for (size_t i = 0; i < renderingThreadCount; i++)
	{
		threads.push_back(std::async(std::launch::async, &CPURayCastingVolumeVisualizer<T>::ComputePartOfFrame, this, renderingThreadCount, i, screenSize[1], screenSize[0], downscale));
	}
	for (size_t i = 0; i < renderingThreadCount; i++)
	{
		threads[i].get();
	}
	this->_camera->ChangeScreenSize(screenSize[1], screenSize[0]);

	if (downscale != 1)
	{
		unsigned char* framebuffer = this->_framebuffer.get();
		auto screenSizes = this->_camera->GetScreenSize();
		int width = screenSizes[1];
		int height = screenSizes[0];

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				for (size_t c = 0; c < 4; c++)
				{
					framebuffer[y * width * 4 + x * 4 + c] = framebuffer[y / 2 * 2 * width * 4 + x / 2 * 2 * 4 + c];
				}
			}
		}
	}
	
	this->_memory.Revalidate();
}


template <typename T>
void CPURayCastingVolumeVisualizer<T>::ComputePartOfFrame(int threads, int threadIndex, int framebufferWidth, int framebufferHeight, int downscale)
{
	unsigned char* framebuffer = this->_framebuffer.get();

	if (downscale == 1)
	{
		while (true)
		{
			int i = _reneringPosition.fetch_add(1, std::memory_order_acq_rel);// operace provádí load, store naráz, nelze tedy vložit mezi tyto dva příkazy jiný příkaz. 
			// Compiler tedy zaručí cache coherenci, protože v opačném případě by operace neodpovídaly žádnému existujícímu pořadí. Operace XCHG říká nahraď co za co. Store říká
			// nahraj sem tohle bez ohledu na to co tam bylo předtím
			if (i >= framebufferWidth * framebufferHeight)
			{
				break;
			}
			auto val = ComputeRay(i % framebufferWidth, i / framebufferWidth);
			framebuffer[i * 4] = val.r;
			framebuffer[i * 4 + 1] = val.g;
			framebuffer[i * 4 + 2] = val.b;
			framebuffer[i * 4 + 3] = val.a;
		}
	}
	else
	{		
		while (true)
		{
			int i = _reneringPosition.fetch_add(1, std::memory_order_acq_rel);
			if (i >= framebufferWidth * framebufferHeight)
			{
				break;
			}
			int x = i % framebufferWidth;
			int y = i / framebufferWidth;
			if (y % downscale == 0 && x % downscale == 0)
			{
				auto val = ComputeRay(x/downscale, y/downscale);

				framebuffer[i * 4] = val.r;
				framebuffer[i * 4 + 1] = val.g;
				framebuffer[i * 4 + 2] = val.b;
				framebuffer[i * 4 + 3] = val.a;
			}
		}
	}
}


template <typename T>
__forceinline void CPURayCastingVolumeVisualizer<T>::MixColors(float& r, float& g, float& b, float& a, const float ra, const float ga, const float ba, const float aa)
{
	r = r + ra * aa * (1 - a);
	g = g + ga * aa * (1 - a);
	b = b + ba * aa * (1 - a);
	a = a + aa * (1 - a);
}


template <typename T>
color CPURayCastingVolumeVisualizer<T>::ComputeRay(int x, int y)
{
	ColorMappingTable mappingTable = _settingsCopy.mappingTable;
	Vector3f stepVector = this->_camera->GetShrankRayDirection(x, y).normalized();

	auto fragments = _meshFramebuffer->GetFragmentsOrdered(x, y);
	float nextFragmentPahtLength = FLT_MAX;
	int fragmentIndex = 0;

	Vector3f start, stop;
	float stepMultiplyer = 1;
	float r = 0, g = 0, b = 0, a = 0; // Color

	if (this->ComputeRayIntersection(stepVector, start, stop))
	{
		auto alphaCoeficient = this->_camera->DeshrinkVector(stepVector).norm();

		Vector3f pathLength = (stop - start).array().abs(); // Lenght of the ray path
		Vector3f position = start; // Position of the ray casting

		int maxPathLengthIndex = static_cast<int>(std::distance(pathLength.data(), std::max_element(pathLength.data(), pathLength.data() + pathLength.size())));
		double maxPathLength = pathLength[maxPathLengthIndex];

		int qualityStepCount = 100 - (position - start).norm();
		bool lighting = _settingsCopy.shading;
		auto dataSizes = this->_memory.GetDataSizes();

		Vector3f cameraPos = this->_camera->GetShrankPosition();

		// use fragments that are closer than start of RC
		for (auto fragment : fragments)
		{
			Vector3f fragmentVector = this->_camera->ShrinkVector(this->_camera->GedDistanceFromProjected(fragment.depth, x, y));
			
			if (fragmentVector.norm() < (cameraPos - start).norm())
			{
				MixColors(r, g, b, a, fragment.r / 255.f, fragment.g / 255.f, fragment.b / 255.f, fragment.a / 255.f);
				
				fragmentIndex++;
				
				if (a > 0.97) { a = 1; break; }
			}
			else
			{
				break;
			}
		}

		// set nextFragmentPathLength to distance between RC position and fragment position
		if (fragments.size() > fragmentIndex)
		{
			Vector3f fragmentVector = this->_camera->ShrinkVector(this->_camera->GedDistanceFromProjected(fragments[fragmentIndex].depth, x, y));
			nextFragmentPahtLength = (fragmentVector + cameraPos - position).norm();
		}		
		

		if (lighting)
		{
			while (maxPathLength >= fabs(position[maxPathLengthIndex] - start[maxPathLengthIndex]))
			{
				// Use fragment if RC is on correct position and prepare for next
				if (nextFragmentPahtLength <= 0)
				{
					MixColors(r, g, b, a, fragments[fragmentIndex].r / 255.f, fragments[fragmentIndex].g / 255.f, fragments[fragmentIndex].b / 255.f, fragments[fragmentIndex].a / 255.f);

					fragmentIndex++;

					if (fragments.size() > fragmentIndex)
					{
						nextFragmentPahtLength = (this->_camera->ShrinkVector(this->_camera->GedDistanceFromProjected(fragments[fragmentIndex].depth, x, y)) + cameraPos - position).norm();
					}
					else
					{
						nextFragmentPahtLength = FLT_MAX;
					}

					if (a > 0.97) { a = 1; break; }
				}

				int x0 = static_cast<int>(position[0]);
				int y0 = static_cast<int>(position[1]);
				int z0 = static_cast<int>(position[2]);

				// Hodnoty ve všech osmi nejbližších voxelů
				int downscale;
				double f000 = this->_memory.GetValue(x0, y0, z0, downscale);

				int gridSize = (1 << downscale);
				x0 -= x0 % gridSize;
				y0 -= y0 % gridSize;
				z0 -= z0 % gridSize;

				int x1 = x0 + gridSize;
				int y1 = y0 + gridSize;
				int z1 = z0 + gridSize;

				if (!(x1 >= dataSizes[0] || y1 >= dataSizes[1] || z1 >= dataSizes[2]))
				{
					double f100 = this->_memory.GetValue(x1, y0, z0, downscale);
					double f010 = this->_memory.GetValue(x0, y1, z0, downscale);
					double f110 = this->_memory.GetValue(x1, y1, z0, downscale);
					double f001 = this->_memory.GetValue(x0, y0, z1, downscale);
					double f101 = this->_memory.GetValue(x1, y0, z1, downscale);
					double f011 = this->_memory.GetValue(x0, y1, z1, downscale);
					double f111 = this->_memory.GetValue(x1, y1, z1, downscale);

					// Váhy pro interpolaci
					double dx = position[0] - x0;
					double dy = position[1] - y0;
					double dz = position[2] - z0;
					double invGridSizeCube = 1.0 / (gridSize * gridSize * gridSize);

					double gridSizeMinDx = (gridSize - dx);
					double gridSizeMinDy = (gridSize - dy);
					double gridSizeMinDz = (gridSize - dz);

					double w000 = gridSizeMinDx * gridSizeMinDy * gridSizeMinDz * invGridSizeCube;
					double w100 = dx * gridSizeMinDy * gridSizeMinDz * invGridSizeCube;
					double w010 = gridSizeMinDx * dy * gridSizeMinDz * invGridSizeCube;
					double w110 = dx * dy * gridSizeMinDz * invGridSizeCube;
					double w001 = gridSizeMinDx * gridSizeMinDy * dz * invGridSizeCube;
					double w101 = dx * gridSizeMinDy * dz * invGridSizeCube;
					double w011 = gridSizeMinDx * dy * dz * invGridSizeCube;
					double w111 = dx * dy * dz * invGridSizeCube;

					double value = f000 * w000
						+ f001 * w001
						+ f010 * w010
						+ f011 * w011
						+ f100 * w100
						+ f101 * w101
						+ f110 * w110
						+ f111 * w111;

					stepMultiplyer = gridSize * 0.5; // equals 2^downscale

					bool itemFound = false; size_t index;
					size_t tableSize = mappingTable.Table.size();
					for (size_t i = 0; i < tableSize; i++)
					{
						const ColorMappingItem& item2 = mappingTable.Table.at(i);
						if (item2.Range[0] <= value && item2.Range[1] >= value)
						{
							itemFound = true;
							index = i;
							break;
						}
					}

					if (itemFound)
					{
						// Výpočet gradientu
						double gx = (f100 - f000) * w100 + (f110 - f010) * w110 + (f101 - f001) * w101 + (f111 - f011) * w111;
						double gy = (f010 - f000) * w010 + (f110 - f100) * w110 + (f011 - f001) * w011 + (f111 - f101) * w111;
						double gz = (f001 - f000) * w001 + (f101 - f100) * w101 + (f011 - f010) * w011 + (f111 - f110) * w111;

						double divider = 1 / sqrtf(powf(gx, 2) + powf(gy, 2) + powf(gz, 2));
						gx *= divider;
						gy *= divider;
						gz *= divider;

						double lightx = 8000 - position[0];
						double lighty = 0 - position[1];
						double lightz = 0 - position[2];

						divider = 1 / sqrtf(powf(lightx, 2) + powf(lighty, 2) + powf(lightz, 2));
						lightx *= divider;
						lighty *= divider;
						lightz *= divider;

						double lightDotGrad = (lightx * gx + lighty * gy + lightz * gz);
						double reflectionx = 2 * lightDotGrad * gx - lightx;
						double reflectiony = 2 * lightDotGrad * gy - lighty;
						double reflectionz = 2 * lightDotGrad * gz - lightz;

						double ambientInt = _settingsCopy.ampbientIntensity;
						double difusionInt = fmax(0.0, gx * lightx + gy * lighty + gz * lightz) * _settingsCopy.difustionIntensity;
						double reflectionInt = powf(fmax(0.0, reflectionx * stepVector[0] + reflectiony * stepVector[1] + reflectionz * stepVector[2]), _settingsCopy.reflectionSharpness) * _settingsCopy.reflectionIntensity;

						double lightInt = (ambientInt + difusionInt);

						// Color computing
						const ColorMappingItem& item = mappingTable.Table.at(index);

						double valminran0 = value - item.Range[0];
						double red = item.DreDivDra() * valminran0 + item.ColorFrom[0];
						double green = item.DgrDivDra() * valminran0 + item.ColorFrom[1];
						double blue = item.DblDivDra() * valminran0 + item.ColorFrom[2];
						double alpha = item.DalDivDra() * valminran0 + item.ColorFrom[3];

						alpha = 1.0 - pow(1 - alpha, alphaCoeficient * stepMultiplyer);//changes projection: (fabsf(red - rl) + fabsf(green - gl) + fabsf(blue - bl)) * alpha /3;

						r = r + (red * lightInt + reflectionInt) * alpha * (1 - a);
						g = g + (green * lightInt + reflectionInt) * alpha * (1 - a);
						b = b + (blue * lightInt + reflectionInt) * alpha * (1 - a);
						a = a + alpha * (1 - a);

						if (a > 0.97) { a = 1; break; }
					}
				}

				position += stepVector * stepMultiplyer;	
				nextFragmentPahtLength -= stepMultiplyer;
			}
		}
		else
		{
			// While the position is inside of the volume
			while (maxPathLength >= fabs(position[maxPathLengthIndex] - start[maxPathLengthIndex]))
			{
				// Use fragment if RC is on correct position and prepare for next
				if (nextFragmentPahtLength <= 0)
				{
					MixColors(r, g, b, a, fragments[fragmentIndex].r / 255.f, fragments[fragmentIndex].g / 255.f, fragments[fragmentIndex].b / 255.f, fragments[fragmentIndex].a / 255.f);

					fragmentIndex++;

					if (fragments.size() > fragmentIndex)
					{
						nextFragmentPahtLength = (this->_camera->ShrinkVector(this->_camera->GedDistanceFromProjected(fragments[fragmentIndex].depth, x, y)) + cameraPos - position).norm();
					}
					else
					{
						nextFragmentPahtLength = FLT_MAX;
					}

					if (a > 0.97) { a = 1; break; }
				}

				int downscale;
				float value = this->_memory.GetValue(position[0], position[1], position[2], downscale);

				stepMultiplyer = 1 << downscale; // equals 2^downscale

				if (qualityStepCount-- > 0)
				{
					stepMultiplyer *= 0.5;
				}

				bool itemFound = false; size_t index;
				size_t tableSize = mappingTable.Table.size();
				for (size_t i = 0; i < tableSize; i++)
				{
					const ColorMappingItem& item2 = mappingTable.Table.at(i);
					if (item2.Range[0] <= value && item2.Range[1] >= value)
					{
						itemFound = true;
						index = i;
						break;
					}
				}

				if (itemFound)
				{
					// Color computing
					const ColorMappingItem& item = mappingTable.Table.at(index);

					float valminran0 = value - item.Range[0];
					float red = item.DreDivDra() * valminran0 + item.ColorFrom[0];
					float green = item.DgrDivDra() * valminran0 + item.ColorFrom[1];
					float blue = item.DblDivDra() * valminran0 + item.ColorFrom[2];
					float alpha = item.DalDivDra() * valminran0 + item.ColorFrom[3];

					alpha = 1.0 - pow(1 - alpha, alphaCoeficient * stepMultiplyer);//changes projection: (fabsf(red - rl) + fabsf(green - gl) + fabsf(blue - bl)) * alpha /3;

					MixColors(r, g, b, a, red, green, blue, alpha);

					if (a > 0.97) { a = 1; break; }
				}

				position += stepVector * stepMultiplyer;
				nextFragmentPahtLength -= stepMultiplyer;				
			}
		}		
	}

	// Use unused fragments
	for (size_t i = fragmentIndex; i < fragments.size(); i++)
	{
		MixColors(r, g, b, a, fragments[i].r / 255.f, fragments[i].g / 255.f, fragments[i].b / 255.f, fragments[i].a / 255.f);
		if (a > 0.97) { a = 1; break; }
	}

	return color{ (uint8_t)(fmin(255,r * 255)), (uint8_t)(fmin(255,g * 255)), (uint8_t)(fmin(255,b * 255)), (uint8_t)(a * 255) };
}


template CPURayCastingVolumeVisualizer<uint8_t>;
template CPURayCastingVolumeVisualizer<uint16_t>;
template CPURayCastingVolumeVisualizer<uint32_t>;
template CPURayCastingVolumeVisualizer<uint64_t>;
template CPURayCastingVolumeVisualizer<float>;
template CPURayCastingVolumeVisualizer<double>;
template CPURayCastingVolumeVisualizer<int8_t>;
template CPURayCastingVolumeVisualizer<int16_t>;
template CPURayCastingVolumeVisualizer<int32_t>;
template CPURayCastingVolumeVisualizer<int64_t>;