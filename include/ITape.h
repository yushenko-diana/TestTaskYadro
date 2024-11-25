#ifndef ITAPE_H
#define ITAPE_H

#include <cstddef>

namespace TestTask
{

	enum Direction
	{
		Forward,
		Backward
	};

	enum Position
	{
		Begin,
		End
	};

	struct ITape
	{
		virtual ~ITape() { }

		virtual int32_t Read(size_t cellNumber) = 0;
		virtual void Write(size_t cellNumber, int32_t data) = 0;

		virtual int32_t ReadFromCurrentCell() = 0;
		virtual void WriteToCurrentCell(int32_t data) = 0;

		virtual void RewindTape(size_t numberOfPositions, Direction direction) = 0;
		virtual void RewindTape(size_t cellNumber) = 0;
		virtual void RewindTape(Position position) = 0;

		virtual size_t Length() const = 0;

		virtual size_t CurrentPosition() const = 0;

		virtual bool EndOfTape() const = 0;
	};

}

#endif