//
// z80computer.h
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
#ifndef _z80computer_h
#define _z80computer_h

#include "z80operations.h"
#include "z80memory.h"
#include "console.h"
#include "ramdisk.h"
#include "z80ports.h"
#include "z80.h"
#include "types.h"

#ifdef __circle__
	#include <circle/fs/fat/fatfs.h>
#endif

class CZ80Computer : public Z80operations
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

	// Z80operations:
	uint8_t fetchOpcode (uint16_t address);
	uint8_t peek8 (uint16_t address);
	void poke8 (uint16_t address, uint8_t value);
	uint16_t peek16 (uint16_t address);
	void poke16 (uint16_t address, uint16_t word);
	uint8_t inPort (uint16_t port);
	void outPort (uint16_t port, uint8_t value);
	void addressOnBus (uint16_t address, uint32_t wstates);
	void interruptHandlingTime (uint32_t wstates);
	uint8_t breakpoint (uint16_t address, uint8_t opcode);
	void execDone (void);

private:
	CZ80Memory m_Memory;
	CConsole   m_Console;
	CRAMDisk   m_RAMDisk;
	CZ80Ports  m_Ports;
	Z80        m_CPU;

	boolean m_bContinue;
};

#endif
