//
// z80computer.h
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
#ifndef _z80computer_h
#define _z80computer_h

#include "z80emu.h"
#include "z80memory.h"
#include "console.h"
#include "ramdisk.h"
#include "z80ports.h"
#include "types.h"

#ifdef __circle__
	#include <circle/fs/fat/fatfs.h>
#endif

class CZ80Computer
{
public:
#ifdef __circle__
	CZ80Computer (CFATFileSystem *pFileSystem);
#else
	CZ80Computer (void);
#endif
	~CZ80Computer (void);

	boolean Initialize (void);

	void Run (void);

	void Shutdown (void);

private:
	Z80_STATE  m_CPU;
	CZ80Memory m_Memory;
	CConsole   m_Console;
	CRAMDisk   m_RAMDisk;
	CZ80Ports  m_Ports;

	boolean m_bContinue;
};

#endif
