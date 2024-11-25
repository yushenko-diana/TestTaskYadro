#include "MagneticTapeSystem.h"

#include "Tape.h"

namespace TestTask
{

	MagneticTapeSystem::MagneticTapeSystem(uint32_t readWriteDelay, uint32_t rewindDelay)
		:	_readWriteDelay(readWriteDelay),
			_rewindDelay(rewindDelay)
	{ }

	std::unique_ptr<ITape> MagneticTapeSystem::LoadTape(const std::string& tapeName, bool newTape)
	{
		return std::unique_ptr<Tape>(new Tape(tapeName, newTape, _readWriteDelay, _rewindDelay));
	}

}