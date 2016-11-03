//
// z80memory.h
//
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
#ifndef _z80memory_h
#define _z80memory_h

#include "types.h"

#ifdef __circle__
	#include <circle/fs/fat/fatfs.h>
#endif

class CZ80Memory
{
public:
#ifdef __circle__
	CZ80Memory (CFATFileSystem *pFileSystem);
#else
	CZ80Memory (void);
#endif
	~CZ80Memory (void);

	boolean Initialize (void);

	void *GetDMAPointer (u16 usAddress, u16 usLength);

#ifdef __circle__
private:
	CFATFileSystem *m_pFileSystem;
#endif
};

#endif
