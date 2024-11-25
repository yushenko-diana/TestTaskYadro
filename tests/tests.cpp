#include "gtest/gtest.h"
#include <filesystem>

#include <exception>
#include <limits>
#include <vector>
#include <random>

#include "Sort.h"

namespace
{
	const size_t RamSize = 120;

	const uint16_t NumberOfTemporaryTapes = 5;

	const uint32_t ReadWriteDelay = 10;
	const uint32_t RewindDelay = 20;

	const std::string SamplesDirectoryPath = "../testSamples/";
	const std::string TemporaryDirectoryPath = SamplesDirectoryPath + "tmp/";
	const std::string TemporarySortingDirectoryPath = "../tmp/";

	const std::string SampleFilePath = SamplesDirectoryPath + "testSample";
	const std::string InputSortSamplePath = SamplesDirectoryPath + "input";
	const std::string OutputSortSamplePath = SamplesDirectoryPath + "output";
}


void Write(std::fstream& file, int32_t data)
{ file.write(reinterpret_cast<char*>(&data), sizeof(data)); }

void ClearFolder(const std::filesystem::path& dir)
{
	for (const auto& entry : std::filesystem::directory_iterator(dir))
		std::filesystem::remove_all(entry.path());
}


TEST(TestTask, LoadTapeTest)
{
	TestTask::MagneticTapeSystem tapeSystem(ReadWriteDelay, RewindDelay);

	ASSERT_THROW(
			{
				try
				{
					tapeSystem.LoadTape("config");
				}
				catch(const std::exception& e)
				{
					EXPECT_STREQ(e.what(), "Can't load the tape config");
					throw;
				}
			} , std::runtime_error);


	ASSERT_NO_THROW(tapeSystem.LoadTape("sample", true));

	ASSERT_NO_THROW(tapeSystem.LoadTape(SampleFilePath));
}


TEST(TestTask, RewindTapeTest)
{

	TestTask::MagneticTapeSystem tapeSystem(ReadWriteDelay, RewindDelay);

	const auto tape = tapeSystem.LoadTape(SampleFilePath);
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


TEST(TestTask, ReadTapeDataTest)
{
	TestTask::MagneticTapeSystem tapeSystem(ReadWriteDelay, RewindDelay);

	auto tape = tapeSystem.LoadTape(SampleFilePath);

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


TEST(TestTask, WriteDataToTapeTest)
{
	TestTask::MagneticTapeSystem tapeSystem(ReadWriteDelay, RewindDelay);

	auto tape = tapeSystem.LoadTape("../testSamples/writeSample", true);

	int numberOfElements = 5;
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

	tape->RewindTape(1, TestTask::Direction::Forward);
	tape->WriteToCurrentCell(11111);
	EXPECT_EQ(tape->ReadFromCurrentCell(), 11111);
}


TEST(TestTask, SortDataTest)
{
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::uniform_int_distribution<> sizeDistribution{std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()};
	std::uniform_int_distribution<> dataDistribution{std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()};

	std::ofstream sampleFile;

	std::vector<int32_t> dataSample;
	std::shared_ptr<TestTask::MagneticTapeSystem> tapeSystem = std::make_shared<TestTask::MagneticTapeSystem>(ReadWriteDelay, RewindDelay);

	for (int i = 0; i < 20; ++i)
	{
		const uint8_t dataSize = sizeDistribution(gen);

		sampleFile.open(InputSortSamplePath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

		if (!sampleFile.is_open())
		return;

		for (int i = 0; i < dataSize; ++i)
		{
			int32_t data = dataDistribution(gen);
			sampleFile.write(reinterpret_cast<char*>(&data), sizeof(int32_t));
			dataSample.push_back(data);
		}
		sampleFile.close();

		std::sort(dataSample.begin(), dataSample.end());

		TestTask::Sort sort(tapeSystem, RamSize, NumberOfTemporaryTapes);
		const auto inputTape = tapeSystem->LoadTape(InputSortSamplePath);
		const auto outputTape = tapeSystem->LoadTape(OutputSortSamplePath, true);

		sort.SortData(inputTape, outputTape);

		for(int i = 0; i < dataSize; ++i)
			EXPECT_EQ(outputTape->Read(i + 1), dataSample.at(i));

		ClearFolder(TemporarySortingDirectoryPath);
		dataSample.clear();
	}
}


int main(int argc, char **argv)
{
	if (!std::filesystem::exists(SamplesDirectoryPath))
		std::filesystem::create_directories(SamplesDirectoryPath);

	if (!std::filesystem::exists(TemporaryDirectoryPath))
		std::filesystem::create_directories(TemporaryDirectoryPath);

	ClearFolder(TemporaryDirectoryPath);
	ClearFolder(TemporarySortingDirectoryPath);

	std::fstream sampleFile;
	sampleFile.open(SampleFilePath, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

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

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}