#ifndef SORT_H
#define SORT_H

#include <algorithm>
#include <vector>

#include "MagneticTapeSystem.h"
#include "Tape.h"

namespace TestTask
{

	class Sort
	{
	private:
		using MagneticTapeSystemPtr = std::shared_ptr<MagneticTapeSystem>;
		using ITapeUniquePtr = std::unique_ptr<ITape>;

		MagneticTapeSystemPtr				_tapeSystem;

		uint16_t							_numberOfTemporaryTapes;
		uint64_t							_ramDataCapacity;

		std::vector<ITapeUniquePtr>			_tempTapes;
		std::vector<ITapeUniquePtr>			_lastPhaseTapes;

	public:
		Sort(const MagneticTapeSystemPtr& tapeSystem, size_t ramSize, uint16_t numberOfTemporaryTapes);

		void SortData(const ITapeUniquePtr& inputTape, const ITapeUniquePtr& outputTape);

    private:
		void SplitData(const ITapeUniquePtr& inputTape);

		void MergeOneSeries(const ITapeUniquePtr& tape, uint32_t seriesNumber);
		void MergeSeries(size_t tapeLength, const ITapeUniquePtr& outputTape);
		void MergeLastSeries(const ITapeUniquePtr& outputTape);
    };

}

#endif