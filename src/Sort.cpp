#include "Sort.h"

#include <queue>

namespace TestTask
{

	namespace
	{
		const std::string TemporaryFolderPath = "../tmp/";
		const std::string TempFilePath = TemporaryFolderPath + "tmp";
		const std::string LastPhaseFilePath = TemporaryFolderPath + "lp";
	}


	Sort::Sort(const MagneticTapeSystemPtr& tapeSystem, size_t ramSize, uint16_t numberOfTemporaryTapes)
		:	_tapeSystem(tapeSystem),
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

		if (tapeSize == _ramDataCapacity)
		{
			std::vector<int32_t> dataChunk;
			dataChunk.reserve(_ramDataCapacity);
			for (int pos = 0; pos < _ramDataCapacity; ++pos)
				dataChunk.push_back(inputTape->Read(pos + 1));

			std::sort(dataChunk.begin(), dataChunk.end());

			for (size_t i = 0; i < _ramDataCapacity; ++i)
			{
				outputTape->WriteToCurrentCell(dataChunk.at(i));
				outputTape->RewindTape(1, Direction::Forward);
			}
			return;
		}

		size_t totalNumberOfSeries = tapeSize / _ramDataCapacity;
		if (tapeSize % _ramDataCapacity != 0)
			totalNumberOfSeries += 1;

		if (totalNumberOfSeries < _numberOfTemporaryTapes)
			_numberOfTemporaryTapes = totalNumberOfSeries;

		SplitData(inputTape);
		MergeSeries(inputTape->Length(), outputTape);
		MergeLastSeries(outputTape);
	}


	void Sort::SplitData(const ITapeUniquePtr& inputTape)
	{
		for (uint16_t i = 0; i < _numberOfTemporaryTapes; i++)
		{
			const std::string fileName = TempFilePath + std::to_string(i);
			_tempTapes.push_back(_tapeSystem->LoadTape(fileName, true));
		}

		std::vector<int32_t> dataChunk;
		dataChunk.reserve(_ramDataCapacity);

		uint16_t tempFileIndex = 0;
		uint64_t elementPos = 0;

		for(size_t pos = 1; pos <= inputTape->Length(); ++pos)
		{
			const auto d = inputTape->Read(pos);
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
					_tempTapes[tempFileIndex]->WriteToCurrentCell(dataChunk.at(i));
					_tempTapes[tempFileIndex]->RewindTape(1, Direction::Forward);
				}

				++tempFileIndex;
				if (tempFileIndex >= _numberOfTemporaryTapes)
					tempFileIndex = 0;

				dataChunk.clear();
			}
		}

		for(size_t fileIndex = 0; fileIndex < _numberOfTemporaryTapes; ++fileIndex)
			_tempTapes[fileIndex]->RewindTape(Position::Begin);
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
			const uint16_t minElemFileIdx = minRun.second;

			if (!_tempTapes.at(minElemFileIdx)->EndOfTape() && seriesElementsNumber.at(minElemFileIdx) > 0)
			{
				_tempTapes.at(minElemFileIdx)->RewindTape(1, Direction::Forward);
				chunksRuns.push({_tempTapes.at(minElemFileIdx)->ReadFromCurrentCell(), minElemFileIdx});

				seriesElementsNumber.at(minElemFileIdx) -= 1;
			}
		}

	}


	void Sort::MergeSeries(size_t tapeLength, const ITapeUniquePtr& outputTape)
	{
		uint16_t tempChunksSize = _ramDataCapacity * _numberOfTemporaryTapes;

		uint32_t seriesCount;
		if (tempChunksSize > tapeLength)
			seriesCount = 1;

		else
		{
			seriesCount = tapeLength / tempChunksSize;
			if (tapeLength % tempChunksSize != 0)
				seriesCount += 1;
		}

		uint32_t seriesNumber = 0;

		if (seriesCount == 1)
		{
			MergeOneSeries(outputTape, seriesNumber);
			return;
		}

		while (seriesCount)
		{
			std::string tapeName = LastPhaseFilePath + std::to_string(seriesNumber);
			_lastPhaseTapes.push_back(_tapeSystem->LoadTape(tapeName, true));
			MergeOneSeries(_lastPhaseTapes.at(seriesNumber), seriesNumber);

			--seriesCount;
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
			const uint16_t minElemFileIdx = minRun.second;

			if (!_lastPhaseTapes.at(minElemFileIdx)->EndOfTape())
			{
				_lastPhaseTapes.at(minElemFileIdx)->RewindTape(1, Direction::Forward);
				chunksRuns.push({_lastPhaseTapes.at(minElemFileIdx)->ReadFromCurrentCell(), minElemFileIdx});
			}
		}
	}

}