#ifndef TAPE_H
#define TAPE_H

#include <fstream>
#include <memory>
#include <string>

#include "ITape.h"

#include "factory/TapeFactory.h"
#include "factory/TemporaryTapeFactory.h"


namespace TestTask
{

	class Tape : public ITape
	{
	public:
		friend class TemporaryTapeFactory;
		friend class TapeFactory;

	private:
		const static size_t TeraByte = 1099511627776;

	private:
		std::fstream	_tapeBand;
		size_t			_length;
		size_t			_currentPos;
		std::string		_tapeName;
		size_t			_capacity;

		size_t			_readWriteDelay;
		size_t			_rewindDelay;

	public:
		~Tape() override;

		int32_t Read(size_t cellNumber) override;
		void Write(size_t cellNumber, int32_t data) override;

		int32_t ReadFromCurrentCell() override;
		void WriteToCurrentCell(int32_t data) override;

		void RewindTape(size_t numberOfPositions, Direction direction) override;
		void RewindTape(size_t cellNumber) override;
		void RewindTape(Position position) override;

		size_t Length() const override
		{ return _length; }

		size_t CurrentPosition() const override
		{ return _currentPos; }

		bool EndOfTape() const override
		{ return _currentPos == _length; }

	private:
		Tape(const std::string& tapeName, size_t readWriteDelay, size_t rewindDelay, size_t capacity = TeraByte);

		int32_t DoRead();
		void DoWrite(int32_t data, bool placeWrite = true);

		size_t RewindForward(size_t steps);
		size_t RewindBackward(size_t steps);
		void DoRewind(size_t steps, Direction direction);
	};

}

#endif