#include "Tape.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace TestTask
{

	namespace
	{
		const uint8_t IntSize = sizeof(int32_t);
	}


	Tape::Tape(const std::string& tapeName, bool newTape, size_t readWriteDelay, size_t rewindDelay, size_t capacity)
		:	_currentPos(1),
			_capacity(capacity),
			_readWriteDelay(readWriteDelay),
			_rewindDelay(_rewindDelay)
	{

		std::_Ios_Openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary;
		if (newTape)
			mode |= std::ios_base::trunc;

		_tapeBand.open(tapeName, mode);

		if (!_tapeBand.is_open())
			throw std::runtime_error("Can't load the tape " + tapeName);

		_tapeBand.seekp(0, std::ios_base::end);
		size_t endPose = _tapeBand.tellp();
		_tapeBand.seekp(0, std::ios_base::beg);
		const auto b = _tapeBand.tellp();

		_length  = endPose / IntSize;
		if (endPose > _capacity)
			_capacity = endPose;

		_tapeName = tapeName;
	}


	Tape::~Tape()
	{
		if (_tapeBand.is_open())
		_tapeBand.close();
	}


	int32_t Tape::Read(size_t cellNumber)
	{
		if (_currentPos == cellNumber)
			return DoRead();

		if (cellNumber > _currentPos)
			RewindForward(cellNumber - _currentPos);
		else
			RewindBackward(_currentPos - cellNumber);

		return DoRead();
	}


	void Tape::Write(size_t cellNumber, int32_t data)
	{
		if (_currentPos == cellNumber)
		{
			DoWrite(data);
			return;
		}

		if (cellNumber > _currentPos)
			RewindForward(cellNumber - _currentPos);
		else
			RewindBackward(_currentPos - cellNumber);

		DoWrite(data);
	}


	int32_t Tape::ReadFromCurrentCell()
	{ return DoRead(); }


	void Tape::WriteToCurrentCell(int32_t data)
	{ DoWrite(data, false); }


	void Tape::RewindTape(size_t numberOfPositions, Direction direction)
	{
		if (numberOfPositions == 0)
			return;

		if (direction == Direction::Forward)
			RewindForward(numberOfPositions);
		else
			RewindBackward(numberOfPositions);
	}


	void Tape::RewindTape(size_t cellNumber)
	{
		if (cellNumber == _currentPos)
			return;

		if (cellNumber > _currentPos)
			RewindForward(cellNumber - _currentPos);
		else
			RewindBackward(_currentPos - cellNumber);
	}


	void Tape::RewindTape(Position position)
	{
		switch (position)
		{
		case Position::Begin:
			RewindBackward(_currentPos - 1);
			break;
		case Position::End:
			RewindForward(_length - _currentPos);
			break;
		}
	}


	int32_t Tape::DoRead()
	{
		if (!_tapeBand.is_open() || _tapeBand.tellp() == -1)
			throw std::runtime_error("Bad tape " + _tapeName);

		int32_t result;
		_tapeBand.read(reinterpret_cast<char*>(&result), IntSize);
		_tapeBand.seekg(-IntSize, std::ios_base::cur);

		std::this_thread::sleep_for(std::chrono::microseconds(_readWriteDelay));

		return result;
	}


	void Tape::DoWrite(int32_t data, bool placeWrite)
	{
		if (!_tapeBand.is_open() || _tapeBand.tellp() == -1)
			throw std::runtime_error("Bad tape " + _tapeName);

		_tapeBand.write(reinterpret_cast<char*>(&data), IntSize);
		_tapeBand.seekp(-IntSize, std::ios_base::cur);

		if (!placeWrite)
			if (_currentPos  >= _length)
				++_length;

		std::this_thread::sleep_for(std::chrono::microseconds(_readWriteDelay));
	}


	size_t Tape::RewindForward(size_t steps)
	{
		size_t remainingStepsNumber = (_capacity - _tapeBand.tellp() / 4) - 1;
		if (steps > remainingStepsNumber)
		{
			std::cout << "Unable to rewind tape " + std::to_string(steps) + " steps. Rewind it to the end";
			steps = remainingStepsNumber;
		}

		DoRewind(steps, Direction::Forward);
		return _currentPos;
	}


	size_t Tape::RewindBackward(size_t steps)
	{
		if (steps > _tapeBand.tellp() / 4)
			throw std::out_of_range("Unable to rewind tape backward");

		DoRewind(steps, Direction::Backward);
		return _currentPos;
	}


	void Tape::DoRewind(size_t steps, Direction direction)
	{
		int8_t dataUnitSize;
		switch (direction)
		{
		case Direction::Forward:
			dataUnitSize = IntSize;
			break;

		case Direction::Backward:
			dataUnitSize = -IntSize;
			break;
		}

		for (size_t step = 0; step < steps; ++step)
			_tapeBand.seekp(dataUnitSize, std::ios_base::cur);

		_currentPos = (_tapeBand.tellp() / 4) + 1;

		std::this_thread::sleep_for(std::chrono::microseconds(_rewindDelay));
	}

}