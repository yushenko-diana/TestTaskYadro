#ifndef ABSTRACTTAPEFACTORY_H
#define ABSTRACTTAPEFACTORY_H

#include <memory>
#include <optional>

#include "ITape.h"

namespace TestTask
{

	struct AbstractTapeFactory
	{
		virtual ~AbstractTapeFactory()
		{ }

		virtual std::unique_ptr<ITape> Create(std::string tapeName) = 0;
	};

}

#endif
