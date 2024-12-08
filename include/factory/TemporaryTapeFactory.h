#ifndef TEMPORARYTAPEFACTORY_H
#define TEMPORARYTAPEFACTORY_H

#include <memory>
#include <string>

#include "AbstractTapeFactory.h"

namespace TestTask
{

	class TemporaryTapeFactory: public AbstractTapeFactory
	{
	private:
		uint32_t		_readWriteDelay;
		uint32_t		_rewindDelay;

		std::string		_pathToTempDirectory;

		uint32_t		_tempTapeNumbers;

	public:
		TemporaryTapeFactory(uint32_t readWriteDelay, uint32_t rewindDelay, const std::string& pathToWorkDirectory);

		std::unique_ptr<ITape> Create(std::string tapeName) override;
	};

}

#endif