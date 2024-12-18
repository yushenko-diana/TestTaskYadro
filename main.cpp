#include <iostream>
#include <filesystem>

#include "Sort.h"
#include "json.hpp"


namespace
{
	const std::string ConfigurationFilePath = "../configureFile.json";
	const std::string TempDirectoryPath = "../tmp/";
	const std::string ResultDirectoryPath = "../result/";

	const std::string RamSizeField = "ramSize";
	const std::string NumberOfTemporaryTapes = "numberOfTemporaryTapes";

	const std::string ReadWriteDelay = "readWriteDelay";
	const std::string RewindDelay = "rewindDelay";

	const std::string PathToWorkDirectory = "pathToWorkDirectory";
}


int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cerr << "Path to the input and output tapes must be specified" << std::endl;
		return -1;
	}

	if (!std::filesystem::exists(TempDirectoryPath))
		std::filesystem::create_directories(TempDirectoryPath);

	if (!std::filesystem::exists(ResultDirectoryPath))
		std::filesystem::create_directories(ResultDirectoryPath);

	std::ifstream configFile(ConfigurationFilePath);

	if (!configFile.is_open())
	{
		std::cerr << "Unable to open configuration file";
		return -1;
	}

	try
	{
		const nlohmann::json configData = nlohmann::json::parse(configFile);
		configFile.close();

		const size_t ramSize = configData.at(RamSizeField);

		const uint16_t numberOfTemporaryTapes = configData.at(NumberOfTemporaryTapes);

		const uint32_t readWriteDelay = configData.at(ReadWriteDelay);
		const uint32_t rewindDelay = configData.at(RewindDelay);

		const std::string pathToWorkDirectory = configData.at(PathToWorkDirectory);

		std::shared_ptr<TestTask::AbstractTapeFactory> temporaryTapeFactory = std::make_shared<TestTask::TemporaryTapeFactory>(readWriteDelay, rewindDelay, pathToWorkDirectory);
		std::shared_ptr<TestTask::AbstractTapeFactory> tapeFactory = std::make_shared<TestTask::TapeFactory>(readWriteDelay, rewindDelay, pathToWorkDirectory);

		TestTask::Sort s(temporaryTapeFactory, ramSize, numberOfTemporaryTapes);
		const auto inputTape = tapeFactory->Create(std::string(argv[1]));
		const auto outputTape = tapeFactory->Create(std::string(argv[2]));

		s.SortData(inputTape, outputTape);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}

	return 0;
}