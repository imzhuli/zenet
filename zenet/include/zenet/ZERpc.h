#pragma once
#include <ze/convention.h>
#include <type_traits>

namespace ze
{
	/***
	 * ZERpc Header Protocol Layout:
	 *
	 * -- magic: 2
	 * -- body size: 2
	 * -- major command id: 2
	 * -- minor command id: 2
	 * -- request id: 8
	 * -- reserved: 16
	 *
	 * */

	/***
	 * ZERpc header initilization param order:
	 *
	 * -- major command id: 2
	 * -- minor command id: 2
	 * -- request id: 8
	 * -- client hash: 8
	 * -- body size: 2
	 *
	 * */

	struct ZERpc
	{
	public:
		using MagicNumber        = uint16_t;
		using BodySize           = uint16_t;
		using CommandId          = uint32_t;
		using MajorCommandId     = uint16_t;
		using MinorCommandId     = uint16_t;
		using RequestId          = uint64_t;
		using Reserved           = unsigned char [16];

		static constexpr const size_t HEADER_SIZE =
			+ sizeof(MagicNumber)      // magic
			+ sizeof(BodySize)         // body size
			+ sizeof(CommandId)        // command Id :  (major/minor)
			+ sizeof(RequestId)        // request id
			+ sizeof(Reserved)         // reserved
			+ 0; // End of header

		static_assert(HEADER_SIZE == 32);

	private:
		/* header contents */
		static constexpr const MagicNumber MAGIC = 0x01FF;

		// router use this to dispatch request
		MajorCommandId  _xMajorCommandId;
		MinorCommandId  _xMinorCommandId;
		RequestId       _xRequestId;
		BodySize        _xBodySize;
		Reserved        _xReserved;

		static ZE_FORCE_INLINE constexpr MajorCommandId majorId(CommandId id) { return static_cast<MajorCommandId>(id >> 16); }
		static ZE_FORCE_INLINE constexpr MinorCommandId minorId(CommandId id) { return static_cast<MinorCommandId>(id); }

	public:
		ZE_FORCE_INLINE size_t totalSize() const { return HEADER_SIZE + _xBodySize; }
		void writeHeader(ubyte * output) const;
		bool readHeader(const ubyte * input);

		ZE_FORCE_INLINE CommandId       commandId() const                     { return makeCommandId(_xMajorCommandId, _xMinorCommandId); }
		ZE_FORCE_INLINE MajorCommandId  majorId()   const                     { return _xMajorCommandId; }
		ZE_FORCE_INLINE MinorCommandId  minorId()   const                     { return _xMinorCommandId; }
		ZE_FORCE_INLINE BodySize        bodySize()  const                     { return _xBodySize; }
		ZE_FORCE_INLINE RequestId       requestId() const                     { return _xRequestId; }

		ZE_FORCE_INLINE void            setCommandId(CommandId cmdId)         { _xMajorCommandId = majorId(cmdId); _xMinorCommandId = minorId(cmdId); }
		ZE_FORCE_INLINE void            setBodySize(size_t bodySize)          { assert(bodySize <= std::numeric_limits<BodySize>::max()); _xBodySize = static_cast<BodySize>(bodySize); }
		ZE_FORCE_INLINE void            setRequestId(RequestId requestId)     { _xRequestId = requestId; }

		ZE_FORCE_INLINE void set(CommandId cmdId, uint64_t requestId, size_t bodySize) {
			setCommandId(cmdId);
			setRequestId(requestId);
			setBodySize(bodySize);
		}

		static ZE_FORCE_INLINE constexpr CommandId makeCommandId(MajorCommandId major, MinorCommandId minor) { return (static_cast<CommandId>(major) << 16) + static_cast<CommandId>(minor); }
	};

	static_assert(std::is_trivially_default_constructible_v<ZERpc>);

}
