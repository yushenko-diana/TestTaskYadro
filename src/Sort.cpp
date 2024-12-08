#include "Sort.h"

#include <queue>

namespace TestTask
{

	namespace
	{
		const std::string TemporaryTapeName = "tmp";
	}

	Sort::Sort(const TapeFactoryPtr& tapeFactory, size_t ramSize, uint16_t numberOfTemporaryTapes)
		:	_tapeFactory(tapeFactory),
			_ramDataCapacity(ramSize / sizeof(int32_t)),
			_numberOfTemporaryTapes(numberOfTemporaryTapes)
	{
		if (_ramDataCapacity == 0)
			throw std::runtime_error("Zero RAM size");

		if (_numberOfTemporaryTapes < 2)
			_numberOfTemporaryTapes = 2;
	}


	void Sort::SortData(const ITapeUniquePtr& inputTape, const ITapeUniquePtr& outputTape)
	{
		const size_t tapeSize = inputTape->Length();
		if (tapeSize == 0)
			return;

		if (tapeSize == 1)
		{
			outputTape->WriteToCurrentCell(inputTape->ReadFromCurrentCell());
			return;
		}

		if (tapeSize <= _ramDataCapacity)
		{
			std::vector<int32_t> dataChunk;
			dataChunk.reserve(_ramDataCapacity);
			for (int pos = 0; pos < tapeSize; ++pos)
				dataChunk.push_back(inputTape->Read(pos + 1));

			std::sort(dataChunk.begin(), dataChunk.end());

			for (size_t i = 0; i < tapeSize; ++i)
			{
				outputTape->WriteToCurrentCell(dataChunk.at(i));
				outputTape->RewindTape(1, Direction::Forward);
			}
			return;
		}

		Configure(tapeSize);

		SplitData(inputTape);
		MergeSeries(outputTape);
		MergeLastSeries(outputTape);
	}


	void Sort::Configure(size_t tapeLength)
	{
		size_t totalNumberOfChunks = tapeLength / _ramDataCapacity;
		if (tapeLength % _ramDataCapacity != 0)
			totalNumberOfChunks += 1;

		if (totalNumberOfChunks < _numberOfTemporaryTapes)
			_numberOfTemporaryTapes = totalNumberOfChunks;

		const uint16_t totalNumberOfChunksOfOneSeries = _ramDataCapacity * _numberOfTemporaryTapes;
		if (totalNumberOfChunksOfOneSeries > tapeLength)
			_seriesCount = 1;

		else
		{
			_seriesCount = tapeLength / totalNumberOfChunksOfOneSeries;
			if (tapeLength % totalNumberOfChunksOfOneSeries != 0)
				_seriesCount += 1;
		}
	}


	void Sort::SplitData(const ITapeUniquePtr& inputTape)
	{
		for (uint16_t tempTapeIndex = 0; tempTapeIndex < _numberOfTemporaryTapes; tempTapeIndex++)
			_tempTapes.push_back(_tapeFactory->Create(TemporaryTapeName));

		std::vector<int32_t> dataChunk;
		dataChunk.reserve(_ramDataCapacity);

		uint16_t tempTapeIndex = 0;
		uint64_t elementPos = 0;

		for(size_t pos = 1; pos <= inputTape->Length(); ++pos)
		{
			dataChunk.push_back(inputTape->Read(pos));

			++elementPos;
			if (elementPos >= _ramDataCapacity || inputTape->EndOfTape())
			{
				const size_t chunkSize = dataChunk.size();
				elementPos = 0;

				if (chunkSize != 1)
					std::sort(dataChunk.begin(), dataChunk.end());

				for (size_t i = 0; i < chunkSize; ++i)
				{
					_tempTapes[tempTapeIndex]->WriteToCurrentCell(dataChunk.at(i));
					_tempTapes[tempTapeIndex]->RewindTape(1, Direction::Forward);
				}

				++tempTapeIndex;
				if (tempTapeIndex >= _numberOfTemporaryTapes)
					tempTapeIndex = 0;

				dataChunk.clear();
			}
		}

		for(size_t tapeIndex = 0; tapeIndex < _numberOfTemporaryTapes; ++tapeIndex)
			_tempTapes[tapeIndex]->RewindTape(Position::Begin);
	}


	void Sort::MergeOneSeries(const ITapeUniquePtr& tape, uint32_t seriesNumber)
	{
		std::vector<uint32_t> seriesElementsNumber(_numberOfTemporaryTapes, _ramDataCapacity);

		std::priority_queue<std::pair<int32_t, uint16_t>, std::vector<std::pair<int32_t, uint16_t>>, std::greater<std::pair<int32_t, uint16_t>>> chunksRuns;

		const size_t tempTapePosition = seriesNumber * _ramDataCapacity + 1;
		for(uint16_t tempTapeIdx = 0; tempTapeIdx < _numberOfTemporaryTapes; ++tempTapeIdx)
		{
			const size_t tapeLength = _tempTapes[tempTapeIdx]->Length();

			if (tapeLength == 0)
				continue;

			else if (tapeLength == 1)
				chunksRuns.push({_tempTapes[tempTapeIdx]->ReadFromCurrentCell(), tempTapeIdx});

			if (!_tempTapes[tempTapeIdx]->EndOfTape())
			{
				_tempTapes[tempTapeIdx]->RewindTape(tempTapePosition);
				chunksRuns.push({_tempTapes[tempTapeIdx]->ReadFromCurrentCell(), tempTapeIdx});

				seriesElementsNumber.at(tempTapeIdx) -= 1;
			}
		}

		while (!chunksRuns.empty())
		{
			const auto minRun = chunksRuns.top();
			chunksRuns.pop();

			tape->WriteToCurrentCell(minRun.first);
			tape->RewindTape(1, Direction::Forward);
			const uint16_t minElemTapeIdx = minRun.second;

			if (!_tempTapes.at(minElemTapeIdx)->EndOfTape() && seriesElementsNumber.at(minElemTapeIdx) > 0)
			{
				_tempTapes.at(minElemTapeIdx)->RewindTape(1, Direction::Forward);
				chunksRuns.push({_tempTapes.at(minElemTapeIdx)->ReadFromCurrentCell(), minElemTapeIdx});

				seriesElementsNumber.at(minElemTapeIdx) -= 1;
			}
		}

	}


	void Sort::MergeSeries(const ITapeUniquePtr& outputTape)
	{
		uint32_t seriesNumber = 0;

		if (_seriesCount == 1)
		{
			MergeOneSeries(outputTape, seriesNumber);
			return;
		}

		while (_seriesCount)
		{
			_lastPhaseTapes.push_back(_tapeFactory->Create(TemporaryTapeName));
			MergeOneSeries(_lastPhaseTapes.at(seriesNumber), seriesNumber);

			--_seriesCount;
			++seriesNumber;
		}
	}


	void Sort::MergeLastSeries(const ITapeUniquePtr& outputTape)
	{
		std::priority_queue<std::pair<int32_t, uint16_t>, std::vector<std::pair<int32_t, uint16_t>>, std::greater<std::pair<int32_t, uint16_t>>> chunksRuns;

		for(uint16_t idx = 0; idx < _lastPhaseTapes.size(); ++idx)
		{
			if (!_lastPhaseTapes[idx]->EndOfTape())
			{
				_lastPhaseTapes[idx]->RewindTape(Position::Begin);
				chunksRuns.push({_lastPhaseTapes[idx]->ReadFromCurrentCell(), idx});
			}
		}

		while (!chunksRuns.empty())
		{
			const auto minRun = chunksRuns.top();
			chunksRuns.pop();

			outputTape->WriteToCurrentCell(minRun.first);
			outputTape->RewindTape(1, Direction::Forward);
			const uint16_t minElemTapeIdx = minRun.second;

			if (!_lastPhaseTapes.at(minElemTapeIdx)->EndOfTape())
			{
				_lastPhaseTapes.at(minElemTapeIdx)->RewindTape(1, Direction::Forward);
				chunksRuns.push({_lastPhaseTapes.at(minElemTapeIdx)->ReadFromCurrentCell(), minElemTapeIdx});
			}
		}
	}

}