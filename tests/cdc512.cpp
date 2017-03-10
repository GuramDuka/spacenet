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
#include <iostream>
#include <cstring>
//------------------------------------------------------------------------------
#include "cdc512.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void cdc512_test()
{
	bool fail = false;

	try {
		uint8_t t[240] = { 0 };

		for( size_t i = 1; i < sizeof(t); i++ )
			t[i] = t[i - 1] + 1;

		cdc512 ctx1;
		ctx1.init();
		ctx1.update(t, sizeof(t));
		ctx1.finish();

		//std::cerr << ctx1.to_string() << std::endl;

		t[3] ^= 0x40;

		cdc512 ctx2;
		ctx2.init();
		ctx2.update(t, sizeof(t));
		ctx2.finish();

		//std::cerr << ctx2.to_string() << std::endl;
		
		if( ctx1.to_string() != "F427-CCD4-183C-79B9-731E-1E79-2796-7C54-560A-DD7F-6AA4-D302-354C-5F15-02B2-3D6B-1B46-F16C-AEA6-7A55-2A3A-F4D2-F388-5916-7769-8A3A-160A-3DBD-79B4-150B-026D-CEA0" )
			throw std::runtime_error("bad cdc512 implementation");

		if( ctx2.to_string() != "A0AA-3C5A-2B41-1585-53F4-17E4-F0F1-FE9D-7E68-9734-3B6F-42AB-B641-D3A9-D44E-C426-FC61-C99C-B47B-795A-913B-2A91-8E40-6733-19E0-AF37-4781-B5E0-3BFD-D83F-69DB-3460" )
			throw std::runtime_error("bad cdc512 implementation");
	}
	catch (...) {
		fail = true;
	}

    std::cerr << "cdc512 test " << (fail ? "failed" : "passed") << std::endl;
}
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
