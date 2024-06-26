/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "pch.h"
#include "ProjectManager.h"
#include "httplib/httplib.h"

 // fix byte colision in ZLIB
typedef unsigned char byteRedefiner;
#define byte byteRedefiner
#define ZLIB_WINAPI
#include "zlib.h"
#include "zconf.h"
using nlohmann::json;

std::vector<ProjectInfo> ProjectManager::GetAllProjectsInfo()
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	httplib::Result result = cli.Get("/ProjectManagement/GetAllProjectsInfo");

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}

	json data = json::parse(result->body);

	return data.get<std::vector<ProjectInfo>>();
}

void ProjectManager::CreateProject(const std::string& projectName)
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	auto result = cli.Get("/ProjectManagement/CreateProject?projectName=" + projectName);

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}
}

void ProjectManager::DeleteProject(const std::string& projectName)
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	auto result = cli.Get("/ProjectManagement/DeleteProject?projectName=" + projectName);

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}
}

void ProjectManager::ConvertProject(const std::string& projectName)
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	auto result = cli.Get("/ProjectManagement/ConvertProject?projectName=" + projectName);

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}
}

std::vector<unsigned char> ProjectManager::GetSegment(const std::string& projectName, int x, int y, int z, int downscale, int bytesToRead, bool compressed)
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);

	Logger::GetInstance()->LogEvent("ProjectManager", "SegmentLoading:ServerLoading:Started", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

	httplib::Result result = cli.Get("/Data/GetBlock?projectName=" + projectName + "&xIndex=" + std::to_string(x) + "&yIndex=" + std::to_string(y) + "&zIndex=" + std::to_string(z) + "&downscale=" + std::to_string(downscale));
	
	Logger::GetInstance()->LogEvent("ProjectManager", "SegmentLoading:ServerLoading:Ended", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}
	if (result->body.empty())
	{
		throw ServerException("Body is empty");
	}	
	
	if (compressed)
	{	// Decompression
		auto data = std::vector<unsigned char>();
		data.resize(bytesToRead);		
		Logger::GetInstance()->LogEvent("ProjectManager", "SegmentLoading:Decompression:Started", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));
		DecompressData((const unsigned char*)(result->body.c_str()), data.data(), result->body.length(), bytesToRead);
		Logger::GetInstance()->LogEvent("ProjectManager", "SegmentLoading:Decompression:Ended", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

		return data;
	}
	else
	{
		return std::vector<unsigned char>(result->body.begin(), result->body.end());
	}
}

void ProjectManager::DecompressData(const unsigned char* abSrc, unsigned char* abDst, int inputLength, int outputLength)
{
	// This funciton is based on article:
	// URL: https://www.experts-exchange.com/articles/3189/In-Memory-Compression-and-Decompression-Using-ZLIB.html
	// Author: DanRollins

	z_stream zInfo = { 0 };
	zInfo.total_in = inputLength;
	zInfo.avail_in = inputLength;
	zInfo.total_out = outputLength;
	zInfo.avail_out = outputLength;
	zInfo.next_in = (unsigned char*)abSrc;
	zInfo.next_out = abDst;

	int nErr, nRet = -1;
	nErr = inflateInit2(&zInfo, 16 + MAX_WBITS);
	if (nErr == Z_OK)
	{
		nErr = inflate(&zInfo, Z_FINISH);
		if (nErr == Z_STREAM_END)
		{
			nRet = zInfo.total_out;
		}
	}
	inflateEnd(&zInfo);

	if (nRet == -1)
	{
		throw ServerException("Unable to uncompress data");
	}
}

void ProjectManager::UploadFile(const std::string& projectName, const std::string& filePath)
{

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		throw exception("Bad file");
	}

	std::filesystem::path p(filePath);
	std::string filename = p.filename().string();
	
	file.seekg(0, std::ios::end);
	long long fileSize = file.tellg();
	long maxLength = 2000000;

	httplib::Client cli = httplib::Client(Url);
	cli.set_write_timeout(300);
	std::vector<char>  buffer(maxLength);
	auto result = cli.Post("/ProjectManagement/UploadFile?projectName=" + projectName + "&fileName=" + filename + "",
		[&file, maxLength, fileSize, &buffer](size_t offset, httplib::DataSink& sink)
		{
			file.seekg(offset, std::ios::beg);
			long long bytesToEnd = fileSize - file.tellg();

			long length = bytesToEnd > maxLength ? maxLength : (long long)bytesToEnd;
			

			if (length == 0)
			{
				sink.done();
			}
			else
			{
				file.read(&buffer[0], length);
				sink.write(&buffer[0], length);
			}

			return true;
		}
	, "application/octet-stream");

	file.close();

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}
}