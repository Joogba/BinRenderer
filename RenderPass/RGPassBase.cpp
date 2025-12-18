#include "RGPassBase.h"

namespace BinRenderer
{
	RGPassBase::RGPassBase(RHI* rhi, const std::string& name)
		: rhi_(rhi)
		, name_(name)
		, width_(0)
		, height_(0)
		, executionOrder_(0)
	{
	}

	RGPassBase::~RGPassBase()
	{
	}

} // namespace BinRenderer
