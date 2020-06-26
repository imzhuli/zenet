#include <zenet/ZERpc.h>
#include <ze/raw.h>
#include <cassert>

namespace ze
{

	ZE_API void ZERpc::writeHeader(ze::ubyte * output) const
	{
		ze::StreamWriter sw(output);
		sw.w2(MAGIC);
		sw.w2(_xBodySize);
		sw.w2(_xMajorCommandId);
		sw.w2(_xMinorCommandId);
		sw.w8(_xRequestId);
		sw.w(_xReserved, sizeof(_xReserved));
		assert(sw.offset() == HEADER_SIZE);
	}

	ZE_API bool ZERpc::readHeader(const ze::ubyte * input)
	{
		ze::StreamReader sr(input);
		if(MAGIC != sr.r2()) {
			return false;
		}
		_xBodySize = sr.r2();
		_xMajorCommandId = sr.r2();
		_xMinorCommandId = sr.r2();
		_xRequestId = sr.r8();
		sr.r(_xReserved, sizeof(_xReserved));
		assert(sr.offset() == HEADER_SIZE);
		return true;
	}

}
