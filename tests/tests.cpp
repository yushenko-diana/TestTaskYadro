#include "gtest/gtest.h"
#include <filesystem>

#include <exception>
#include <limits>
#include <random>
#include <vector>

#include "json.hpp"
#include "Sort.h"

namespace
{
	const std::string RamSizeField = "ramSize";
	const std::string NumberOfTemporaryTapes = "numberOfTemporaryTapes";

	const std::string ReadWriteDelay = "readWriteDelay";
	const std::string RewindDelay = "rewindDelay";

	const std::string PathToWorkDirectory = "pathToWorkDirectory";

	void Write(std::fstream& file, int32_t data)
	{ file.write(reinterpret_cast<char*>(&data), sizeof(data)); }

	void ClearFolder(const std::filesystem::path& dir)
	{
		for (const auto& entry : std::filesystem::directory_iterator(dir))
			std::filesystem::remove_all(entry.path());
	}
}

class TestTaskCase : public ::testing::Test
{
protected:
	inline static std::shared_ptr<TestTask::TapeFactory> tapeFactory;
	inline static std::shared_ptr<TestTask::TemporaryTapeFactory> tempTapeFactory;

	inline static uint16_t numberOfTemporaryTapes;
	inline static size_t ramSize;
	inline static std::string pathToWorkDirectory;

	inline static std::string samplesDirectoryPath;
	inline static std::string temporaryDirectoryPath;
	inline static std::string temporarySortingDirectoryPath;

	inline static std::string configurationFilePath;

	inline static std::string samplesDirectory;
	inline static std::string inputSortSampleFilePath;


	inline static std::string samplePath;
	inline static std::string writeSamplePath;
	inline static std::string inputSortSamplePath;
	inline static std::string outputSortSamplePath;

	static void SetUpTestSuite()
	{
		std::ifstream configFile(configurationFilePath);

		if (!configFile.is_open())
			throw std::runtime_error("Unable to open configuration file");

		const nlohmann::json configData = nlohmann::json::parse(configFile);
		configFile.close();

		ramSize = configData.at(RamSizeField);

		numberOfTemporaryTapes = configData.at(NumberOfTemporaryTapes);

		const uint32_t readWriteDelay = configData.at(ReadWriteDelay);
		const uint32_t rewindDelay = configData.at(RewindDelay);

		pathToWorkDirectory = configData.at(PathToWorkDirectory);

		samplesDirectoryPath = pathToWorkDirectory + "/testSamples";
		tapeFactory = std::make_shared<TestTask::TapeFactory>(readWriteDelay, rewindDelay, samplesDirectoryPath);
		tempTapeFactory = std::make_shared<TestTask::TemporaryTapeFactory>(readWriteDelay, rewindDelay, samplesDirectoryPath);

		temporaryDirectoryPath = samplesDirectoryPath + "/tmp";
		temporarySortingDirectoryPath = pathToWorkDirectory + "/tmp";

		inputSortSampleFilePath = samplesDirectoryPath + "/input";

		samplePath = "/testSample";
		writeSamplePath = "/writeSample";
		inputSortSamplePath = "/input";
		outputSortSamplePath = "/output";

		if (!std::filesystem::exists(samplesDirectoryPath))
			std::filesystem::create_directories(samplesDirectoryPath);

		if (!std::filesystem::exists(temporaryDirectoryPath))
			std::filesystem::create_directories(temporaryDirectoryPath);

		if (!std::filesystem::exists(temporarySortingDirectoryPath))
			std::filesystem::create_directories(temporarySortingDirectoryPath);

		ClearFolder(temporaryDirectoryPath);
		ClearFolder(temporarySortingDirectoryPath);

		std::fstream sampleFile;
		sampleFile.open(samplesDirectoryPath + samplePath, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

		Write(sampleFile, 43);
		Write(sampleFile, 36);
		Write(sampleFile, 878);
		Write(sampleFile, -2431);
		Write(sampleFile, 4653);
		Write(sampleFile, 0);
		Write(sampleFile, -10);
		Write(sampleFile, 356430);
		Write(sampleFile, 9823);
		Write(sampleFile, 9);
		Write(sampleFile, 122354542);
		Write(sampleFile, 3);
		Write(sampleFile, 65);
		Write(sampleFile, -23);
		Write(sampleFile, 12121212);

		sampleFile.close();
	}

public:
	static void init(const std::string& configFilePath)
	{ configurationFilePath = configFilePath; }
};


TEST_F(TestTaskCase, RewindTapeTest)
{
	const auto tape = tapeFactory->Create(samplePath);
	ASSERT_EQ(tape->CurrentPosition(), 1);

	tape->RewindTape(5);
	ASSERT_EQ(tape->CurrentPosition(), 5);

	tape->RewindTape(10);
	ASSERT_EQ(tape->CurrentPosition(), 10);

	tape->RewindTape(2);
	ASSERT_EQ(tape->CurrentPosition(), 2);

	tape->RewindTape(8);
	ASSERT_EQ(tape->CurrentPosition(), 8);

	tape->RewindTape(8);
	ASSERT_EQ(tape->CurrentPosition(), 8);

	tape->RewindTape(TestTask::Position::End);
	ASSERT_EQ(tape->CurrentPosition(), tape->Length());
	tape->RewindTape(TestTask::Position::Begin);
	ASSERT_EQ(tape->CurrentPosition(), 1);
}


TEST_F(TestTaskCase, ReadTapeDataTest)
{
	auto tape = tapeFactory->Create(samplePath);

	EXPECT_EQ(43, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(36, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(878, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(-2431, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(4653, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(0, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(-10, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(356430, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(9823, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(9, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(122354542, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(3, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(65, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(-23, tape->ReadFromCurrentCell());
	tape->RewindTape(1,TestTask::Direction::Forward);
	EXPECT_EQ(12121212, tape->ReadFromCurrentCell());

	EXPECT_EQ(43, tape->Read(1));
	EXPECT_EQ(36, tape->Read(2));
	EXPECT_EQ(878, tape->Read(3));
	EXPECT_EQ(-2431, tape->Read(4));
	EXPECT_EQ(4653, tape->Read(5));
	EXPECT_EQ(0, tape->Read(6));
	EXPECT_EQ(-10, tape->Read(7));
	EXPECT_EQ(356430, tape->Read(8));
	EXPECT_EQ(9823, tape->Read(9));
	EXPECT_EQ(9, tape->Read(10));
	EXPECT_EQ(122354542, tape->Read(11));
	EXPECT_EQ(3, tape->Read(12));
	EXPECT_EQ(65, tape->Read(13));
	EXPECT_EQ(-23, tape->Read(14));
	EXPECT_EQ(12121212, tape->Read(15));

	EXPECT_EQ(43, tape->Read(1));
	EXPECT_EQ(43, tape->Read(1));
	EXPECT_EQ(43, tape->Read(1));

	EXPECT_EQ(878, tape->Read(3));
	EXPECT_EQ(878, tape->Read(3));
	EXPECT_EQ(878, tape->Read(3));

	EXPECT_EQ(43, tape->Read(1));
	EXPECT_EQ(-10, tape->Read(7));
	EXPECT_EQ(12121212, tape->Read(15));
	EXPECT_EQ(36, tape->Read(2));
	EXPECT_EQ(122354542, tape->Read(11));
	EXPECT_EQ(-2431, tape->Read(4));
}


TEST_F(TestTaskCase, WriteDataToTapeTest)
{
	auto tape = tapeFactory->Create(writeSamplePath);

	const int numberOfElements = 5;
	for(int i = 0; i < numberOfElements ; ++i)
	{
		tape->WriteToCurrentCell(i);

		if (i != 4)
			tape->RewindTape(1,TestTask::Direction::Forward);
	}
	EXPECT_EQ(tape->CurrentPosition(), tape->Length());

	for(int i = 1; i <= numberOfElements; ++i)
	{
		int d = i;
		EXPECT_EQ(tape->Read(i), --d);
	}

	EXPECT_EQ(tape->Length(), numberOfElements);

	for(int i = 1; i <= numberOfElements; ++i)
		tape->Write(i, i);
	EXPECT_EQ(tape->Length(), numberOfElements);
}


TEST_F(TestTaskCase, SortDataTest)
{
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::uniform_int_distribution<> sizeDistribution{std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()};
	std::uniform_int_distribution<> dataDistribution{std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()};

	std::ofstream sampleFile;

	std::vector<int32_t> dataSample;

	for (int i = 0; i < 20; ++i)
	{
		const uint8_t dataSize = sizeDistribution(gen);

		sampleFile.open(inputSortSampleFilePath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

		ASSERT_TRUE(sampleFile.is_open());

		for (int i = 0; i < dataSize; ++i)
		{
			int32_t data = dataDistribution(gen);
			sampleFile.write(reinterpret_cast<char*>(&data), sizeof(int32_t));
			dataSample.push_back(data);
		}
		sampleFile.close();

		std::sort(dataSample.begin(), dataSample.end());

		TestTask::Sort sort(tempTapeFactory, ramSize, numberOfTemporaryTapes);
		const auto inputTape = tapeFactory->Create(inputSortSamplePath);
		const auto outputTape = tapeFactory->Create(outputSortSamplePath);

		sort.SortData(inputTape, outputTape);

		for(int i = 0; i < dataSize; ++i)
			EXPECT_EQ(outputTape->Read(i + 1), dataSample.at(i));

		ClearFolder(temporaryDirectoryPath);
		ClearFolder(temporarySortingDirectoryPath);
		dataSample.clear();
	}
}


int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Path to configuration file must be specified";
		return -1;
	}

	testing::InitGoogleTest(&argc, argv);
	TestTaskCase::init(argv[1]);
	return RUN_ALL_TESTS();
}
