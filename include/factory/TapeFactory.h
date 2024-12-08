#ifndef TAPEFACTORY_H
#define TAPEFACTORY_H

#include <memory>
#include <string>


#include "AbstractTapeFactory.h"

namespace TestTask
{

	class TapeFactory: public AbstractTapeFactory
	{
	private:
		uint32_t		_readWriteDelay;
		uint32_t		_rewindDelay;

		std::string		_pathToWorkDirectory;

	public:
		TapeFactory(uint32_t readWriteDelay, uint32_t rewindDelay, const std::string& pathToWorkDirectory);

		std::unique_ptr<ITape> Create(std::string tapeName) override;
	};

}

#endif