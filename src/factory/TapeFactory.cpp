#include "factory/TapeFactory.h"

#include "Tape.h"

namespace TestTask
{

	TapeFactory::TapeFactory(uint32_t readWriteDelay, uint32_t rewindDelay, const std::string& pathToWorkDirectory)
		:	_readWriteDelay(readWriteDelay),
			_rewindDelay(rewindDelay),
			_pathToWorkDirectory(pathToWorkDirectory + "/")
	{ }

	std::unique_ptr<ITape> TapeFactory::Create(std::string tapeName)
	{ return std::unique_ptr<Tape>(new Tape(_pathToWorkDirectory + tapeName, _readWriteDelay, _rewindDelay)); }

}