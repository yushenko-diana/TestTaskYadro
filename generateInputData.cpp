#include <iostream>
#include <fstream>

#include <limits>
#include <random>

namespace
{
	const uint8_t dataSize = sizeof(int32_t);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Number of elements must be specified\n";
		return -1;
	}

	const int64_t numberOfElements = std::atoi(argv[1]);

	if (numberOfElements < 0)
	{
		std::cerr << "Number of elements must be positive value\n";
		return -1;
	}

	std::ofstream inputFile("../inputData", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if (!inputFile.is_open())
	{
		std::cerr << "Unable to create file\n";
		return -1;
	}

	std::random_device rd;
	std::mt19937 gen{rd()};

	std::uniform_int_distribution<> dist{std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()};

	for (int i = 0; i < numberOfElements; ++i)
	{
		int32_t data = dist(gen);
		inputFile.write(reinterpret_cast<char*>(&data), dataSize);
	}

	inputFile.close();
}