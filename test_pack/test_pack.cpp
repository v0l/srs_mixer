#include <assert.h>

#include <cmath>
#include <ostream>
#include <string>
#include <iostream>
#include <memory.h>

#include "../include/util.hpp"

static constexpr auto BM(const uint64_t val, const uint8_t shift, const uint8_t bits) {
	return (val >> shift) & (uint64_t)(std::pow(2, bits) - 1);
}

static constexpr auto BM(const char* val, const uint8_t rshift, const uint8_t bits) {
	auto v = *((uint64_t*)val);
	v = v >> rshift;
	v = __builtin_bswap64(v);
	return v & (uint64_t)(std::pow(2, bits) - 1);
}

class ModeSNew {
public:
	uint8_t capability; //3
    uint8_t downlink_format; //5
    uint32_t icao_address; //24
    uint64_t data; //56
    uint32_t parity; //24

private:
	

	ModeSNew(uint8_t cap, uint8_t df, uint32_t icao, uint64_t data, uint32_t parity)
		: capability(cap), downlink_format(df), icao_address(icao), data(data), parity(parity) {

		}

public:
	ModeSNew(const ModeSNew& other) = default; 

	static const ModeSNew Parse(const char* msg, const uint16_t len) {
		const ModeSNew ret(
			(msg[0] & 0x7),
			(msg[0] >> 3) & 0x1f,
			ssr::__u24(msg + 1),
			ssr::__u56(msg + 4),
			ssr::__u24(msg + 11)
		);
		return ret;
	}

	/**
	 * Downlink Register (B-DS, Comm-B Data Selector)
	 * Otherwise known as TypeCode
	 */
	constexpr auto Register() const -> const uint8_t {
		return data & 0x0F;
	}
};

namespace neolib
{
	template<class Elem, class Traits>
	inline void hex_dump(const void* aData, std::size_t aLength, std::basic_ostream<Elem, Traits>& aStream, std::size_t aWidth = 16)
	{
		const char* const start = static_cast<const char*>(aData);
		const char* const end = start + aLength;
		const char* line = start;
		while (line != end)
		{
			aStream.width(4);
			aStream.fill('0');
			aStream << std::hex << line - start << " : ";
			std::size_t lineLength = std::min(aWidth, static_cast<std::size_t>(end - line));
			for (std::size_t pass = 1; pass <= 2; ++pass)
			{	
				for (const char* next = line; next != end && next != line + aWidth; ++next)
				{
					char ch = *next;
					switch(pass)
					{
					case 1:
						aStream << (ch < 32 ? '.' : ch);
						break;
					case 2:
						if (next != line)
							aStream << " ";
						aStream.width(2);
						aStream.fill('0');
						aStream << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(ch));
						break;
					}
				}
				if (pass == 1 && lineLength != aWidth)
					aStream << std::string(aWidth - lineLength, ' ');
				aStream << " ";
			}
			aStream << std::endl;
			line = line + lineLength;
		}
	}
}

int main(int arc, char** argv) {
    auto msg_s = "\x8D\x48\x40\xD6\x20\x2C\xC3\x71\xC3\x2C\xE0\x57\x60\x98";
	auto msg_c = "\x5d\x3c\x4a\xb3\x4c\x1c\x8f";

    auto modeS = ModeSNew::Parse(msg_c, strlen(msg_c));

    //assert(modeS->downlink_format == 17);
	assert(modeS.capability == 0x05);
	assert(modeS.downlink_format = 0x10);
	assert(modeS.icao_address = 0x484020);
	assert(modeS.data == 0x202CC371C32CE0);
	assert(modeS.parity = 0x576098);
}