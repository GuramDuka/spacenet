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
//------------------------------------------------------------------------------
#include "cdc512.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
void cdc512_data::shuffle()
{
	a -= e; f ^= h >>  9; h += a;
	b -= f; g ^= a <<  9; a += b;
	c -= g; h ^= b >> 23; b += c;
	d -= h; a ^= c << 15; c += d;
	e -= a; b ^= d >> 14; d += e;
	f -= b; c ^= e << 20; e += f;
	g -= c; d ^= f >> 17; f += g;
	h -= d; e ^= g << 14; g += h;
}
//---------------------------------------------------------------------------
void cdc512_data::shuffle(const cdc512_data & v)
{
	//constexpr const uint64_t prime_a = UINT64_C(0x992E367BE6F0EA1E);
	//constexpr const uint64_t prime_b = UINT64_C(0x71DCF41FFACC283F);
	//constexpr const uint64_t prime_c = UINT64_C(0xC9581F48D85ABD75);
	//constexpr const uint64_t prime_d = UINT64_C(0xE4B93335FF1CE990);
	//constexpr const uint64_t prime_e = UINT64_C(0xE51D6424EFEC1E01);
	//constexpr const uint64_t prime_f = UINT64_C(0x353867A0E66C2A39);
	//constexpr const uint64_t prime_g = UINT64_C(0xA8DBF7B782226B67);
	//constexpr const uint64_t prime_h = UINT64_C(0x9F8B7F0DC254488E);
	
	//a -= v.e ^ prime_e; f ^= v.h >>  9; h += v.a ^ prime_a;
	//b -= v.f ^ prime_f; g ^= v.a <<  9; a += v.b ^ prime_b;
	//c -= v.g ^ prime_g; h ^= v.b >> 23; b += v.c ^ prime_c;
	//d -= v.h ^ prime_h; a ^= v.c << 15; c += v.d ^ prime_d;
	//e -= v.a ^ prime_a; b ^= v.d >> 14; d += v.e ^ prime_e;
	//f -= v.b ^ prime_b; c ^= v.e << 20; e += v.f ^ prime_f;
	//g -= v.c ^ prime_c; d ^= v.f >> 17; f += v.g ^ prime_g;
	//h -= v.d ^ prime_d; e ^= v.g << 14; g += v.h ^ prime_h;

	a -= v.e; f ^= v.h >>  9; h += v.a;
	b -= v.f; g ^= v.a <<  9; a += v.b;
	c -= v.g; h ^= v.b >> 23; b += v.c;
	d -= v.h; a ^= v.c << 15; c += v.d;
	e -= v.a; b ^= v.d >> 14; d += v.e;
	f -= v.b; c ^= v.e << 20; e += v.f;
	g -= v.c; d ^= v.f >> 17; f += v.g;
	h -= v.d; e ^= v.g << 14; g += v.h;
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

	while( size >= sizeof(cdc512_data) ){
		shuffle(*reinterpret_cast<const cdc512_data *>(data));
		shuffle();
		data = (const uint8_t *) data + sizeof(cdc512_data);
		size -= sizeof(cdc512_data);
	}

	if( size > 0 ) {
		cdc512_data pad;
		
		std::memcpy(&pad, data, size);
		std::memset((uint8_t *) &pad + size, 0, sizeof(cdc512_data) - size);

		shuffle(pad);
		shuffle();
	}
}
//---------------------------------------------------------------------------
void cdc512::finish()
{
    if( p ) {
        cdc512_data pad = { p, p, p, p, p, p, p, p };
	
        shuffle(pad);
        shuffle();
    }
	
	ba = vhtobe64(a);
	bb = vhtobe64(b);
	bc = vhtobe64(c);
	bd = vhtobe64(d);
	be = vhtobe64(e);
	bf = vhtobe64(f);
	bg = vhtobe64(g);
	bh = vhtobe64(h);
}
//---------------------------------------------------------------------------
std::string cdc512::to_string() const
{
    std::stringstream s;
	
	s.fill('0');
	s.width(2);
	s.unsetf(std::ios::dec);
	s.setf(std::ios::hex | std::ios::uppercase);

	size_t i;
	
	for( i = 0; i < sizeof(digest) - 1; i++ ) {
		s << std::setw(2) << uint16_t(digest[i]);
		if( (i & 1) != 0 )
			s << '-';
	}

	s << std::setw(2) << uint16_t(digest[i]);

	return s.str();
}
//---------------------------------------------------------------------------
std::string cdc512::to_short_string() const
{
    std::string s;
    //constexpr const char * abc = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    constexpr const char abc[] = "._,=~!@#$%^&-+0123456789abcdefghijklmnopqrstuvwxyz";

    for( intptr_t i = sizeof(digest) / sizeof(uint64_t) - 1; i >= 0; i-- ) {
        uint64_t a = digest64[i];

        while( a ) {
            s.push_back(abc[a % (sizeof(abc) - 1)]);
            a /= sizeof(abc) - 1;
        }
    }

    return s;
}
//---------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
