#ifndef SORT_H
#define SORT_H

#include <algorithm>
#include <vector>

#include "Tape.h"

namespace TestTask
{

	class Sort
	{
	private:
		using TapeFactoryPtr = std::shared_ptr<AbstractTapeFactory>;
		using ITapeUniquePtr = std::unique_ptr<ITape>;

		TapeFactoryPtr						_tapeFactory;

		uint16_t							_numberOfTemporaryTapes;
		uint64_t							_ramDataCapacity;
		uint32_t							_seriesCount;

		std::vector<ITapeUniquePtr>			_tempTapes;
		std::vector<ITapeUniquePtr>			_lastPhaseTapes;

	public:
		Sort(const TapeFactoryPtr& tapeFactory, size_t ramSize, uint16_t numberOfTemporaryTapes);

		void SortData(const ITapeUniquePtr& inputTape, const ITapeUniquePtr& outputTape);

    private:
		void SplitData(const ITapeUniquePtr& inputTape);

		void Configure(size_t tapeLength);

		void MergeOneSeries(const ITapeUniquePtr& tape, uint32_t seriesNumber);
		void MergeSeries(const ITapeUniquePtr& outputTape);
		void MergeLastSeries(const ITapeUniquePtr& outputTape);
    };

}

#endif