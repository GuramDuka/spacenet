/*-
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Guram Duka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
//------------------------------------------------------------------------------
#include <cstring>
#include <iomanip>
#include <sstream>
//------------------------------------------------------------------------------
#include "cdc512.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
void cdc512_data::shuffle(const cdc512_data & v)
{
	constexpr const uint64_t prime_a = chtobe64(0x992E367BE6F0EA1E);
	constexpr const uint64_t prime_b = chtobe64(0x71DCF41FFACC283F);
	constexpr const uint64_t prime_c = chtobe64(0xC9581F48D85ABD75);
	constexpr const uint64_t prime_d = chtobe64(0xE4B93335FF1CE990);
	constexpr const uint64_t prime_e = chtobe64(0xE51D6424EFEC1E01);
	constexpr const uint64_t prime_f = chtobe64(0x353867A0E66C2A39);
	constexpr const uint64_t prime_g = chtobe64(0xA8DBF7B782226B67);
	constexpr const uint64_t prime_h = chtobe64(0x9F8B7F0DC254488E);
	
	a -= v.e ^ prime_e; f ^= v.h >>  9; h += v.a ^ prime_a;
	b -= v.f ^ prime_f; g ^= v.a <<  9; a += v.b ^ prime_b;
	c -= v.g ^ prime_g; h ^= v.b >> 23; b += v.c ^ prime_c;
	d -= v.h ^ prime_h; a ^= v.c << 15; c += v.d ^ prime_d;
	e -= v.a ^ prime_a; b ^= v.d >> 14; d += v.e ^ prime_e;
	f -= v.b ^ prime_b; c ^= v.e << 20; e += v.f ^ prime_f;
	g -= v.c ^ prime_c; d ^= v.f >> 17; f += v.g ^ prime_g;
	h -= v.d ^ prime_d; e ^= v.g << 14; g += v.h ^ prime_h;
}
//---------------------------------------------------------------------------
void cdc512::init()
{
	a = chtobe64(0xA640524A5B44F1FC);
	b =	chtobe64(0xC535059705F0BB7E);
	c =	chtobe64(0xC8ED76CF6B6EA626);
	d =	chtobe64(0x531D1E8E254EA59E);
	e = chtobe64(0x8C0FE7F3E46E2A80);
	f = chtobe64(0x1C53F41FD1E3A7F8);
	g = chtobe64(0x08D4DEAAA1C33335);
	h = chtobe64(0x4C592980FBE9B011);
	
	p = 0;
}
//---------------------------------------------------------------------------
void cdc512::update(const void * data, uintptr_t size)
{
	p += size;

	while( size >= sizeof(data) ){
		shuffle(*reinterpret_cast<const cdc512_data *>(data));
		data = (const uint8_t *) data + sizeof(data);
		size -= sizeof(data);
	}

	cdc512_data pad;
	
	if( size > 0 ) {
		std::memcpy(&pad, data, size);
		std::memset((uint8_t *) &pad + size, 0, sizeof(data) - size);
		shuffle(pad);
	}
	
	pad.a = pad.b = pad.c = pad.d = pad.e = pad.f = pad.g = pad.h = p;

	shuffle(pad);
}
//---------------------------------------------------------------------------
std::string cdc512::to_string()
{
	std::stringstream s;
	
	s.setf(std::ios::hex);
	s.fill('0');
	s.width(2);
	//s << std::setfill('0') << std::setw(2) << std::hex();
	
	s
	<< data[ 0] << ' ' << data[ 1] << ' ' << data[ 2] << ' ' << data[ 3] << ' '
	<< data[ 4] << ' ' << data[ 5] << ' ' << data[ 6] << ' ' << data[ 7] << ' '
	<< data[ 8] << ' ' << data[ 9] << ' ' << data[10] << ' ' << data[11] << ' '
	<< data[12] << ' ' << data[13] << ' ' << data[14] << ' ' << data[15] << ' '
	<< data[16] << ' ' << data[17] << ' ' << data[18] << ' ' << data[19] << ' '
	<< data[20] << ' ' << data[21] << ' ' << data[22] << ' ' << data[23] << ' '
	<< data[24] << ' ' << data[25] << ' ' << data[26] << ' ' << data[27] << ' '
	<< data[28] << ' ' << data[29] << ' ' << data[30] << ' ' << data[31] << ' '
	<< data[32] << ' ' << data[33] << ' ' << data[34] << ' ' << data[35] << ' '
	<< data[36] << ' ' << data[37] << ' ' << data[38] << ' ' << data[39] << ' '
	<< data[40] << ' ' << data[41] << ' ' << data[42] << ' ' << data[43] << ' '
	<< data[44] << ' ' << data[45] << ' ' << data[46] << ' ' << data[47] << ' '
	<< data[48] << ' ' << data[49] << ' ' << data[50] << ' ' << data[51] << ' '
	<< data[52] << ' ' << data[53] << ' ' << data[54] << ' ' << data[55] << ' '
	<< data[56] << ' ' << data[57] << ' ' << data[58] << ' ' << data[59] << ' '
	<< data[60] << ' ' << data[61] << ' ' << data[62] << ' ' << data[63]
	;
	
	return s.str();
}
//---------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
