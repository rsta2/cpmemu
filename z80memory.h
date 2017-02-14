//
// z80memory.h
//
// Copyright (C) 2016-2017  R. Stange <rsta2@o2online.de>
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

#define Z80_RAM_SIZE	0x10000

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

	u8 ReadByte (u16 usAddress)
	{
		return memory[usAddress];
	}

	u16 ReadWord (u16 usAddress)
	{
		return         memory[usAddress]
		       | (u16) memory[usAddress+1] << 8;
	}

	void WriteByte (u16 usAddress, u8 ucValue)
	{
		memory[usAddress] = ucValue;
	}

	void WriteWord (u16 usAddress, u16 usValue)
	{
		memory[usAddress]   = usValue & 0xFF;
		memory[usAddress+1] = usValue >> 8;
	}

private:
#ifdef __circle__
	CFATFileSystem *m_pFileSystem;
#endif

	u8 memory[Z80_RAM_SIZE];
};

#endif
