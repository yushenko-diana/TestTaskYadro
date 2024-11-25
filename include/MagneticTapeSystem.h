#ifndef MAGNETICSTORAGESYSTEM_H
#define MAGNETICSTORAGESYSTEM_H

#include <memory>

#include "ITape.h"

namespace TestTask
{

	class MagneticTapeSystem
	{
	private:
		uint32_t		_readWriteDelay;
		uint32_t		_rewindDelay;

	public:
		MagneticTapeSystem(uint32_t readWriteDelay, uint32_t rewindDelay);

		std::unique_ptr<ITape> LoadTape(const std::string& tapeName, bool newTape = false);
	};

}

#endif