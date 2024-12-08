#include "factory/TemporaryTapeFactory.h"

#include "Tape.h"

namespace TestTask
{

	TemporaryTapeFactory::TemporaryTapeFactory(uint32_t readWriteDelay, uint32_t rewindDelay, const std::string& pathToWorkDirectory)
		:	_readWriteDelay(readWriteDelay),
			_rewindDelay(rewindDelay),
			_pathToTempDirectory(pathToWorkDirectory + "/tmp/"),
			_tempTapeNumbers(0)
	{ }


	std::unique_ptr<ITape> TemporaryTapeFactory::Create(std::string tapeName)
	{
		const std::string fileName = _pathToTempDirectory + tapeName + std::to_string(_tempTapeNumbers);
		std::unique_ptr<Tape> tape(new Tape(fileName, _readWriteDelay, _rewindDelay));

		++_tempTapeNumbers;

		return tape;
	}
}