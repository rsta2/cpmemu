//
// z80memory.cpp
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
#include "z80memory.h"
#include "z80stub.h"
#include "config.h"
#include <assert.h>

#ifdef __circle__
	#include <circle/logger.h>
#else
	#include <stdio.h>
#endif

unsigned char memory[Z80_RAM_SIZE];

#ifdef __circle__
static const char FromMemory[] = "z80mem";
#endif

#ifdef __circle__
CZ80Memory::CZ80Memory (CFATFileSystem *pFileSystem)
:	m_pFileSystem (pFileSystem)
#else
CZ80Memory::CZ80Memory (void)
#endif
{
	// poke jump to BIOS entry on reset address
	memory[0] = 0xC3;			// opcode "JMP"
	memory[1] = MEM_BIOS & 0xFF;
	memory[2] = MEM_BIOS >> 8;
}

CZ80Memory::~CZ80Memory (void)
{
#ifdef __circle__
	m_pFileSystem = 0;
#endif
}

boolean CZ80Memory::Initialize (void)
{
#ifdef __circle__
	assert (m_pFileSystem != 0);
	unsigned hFile = m_pFileSystem->FileOpen (SYSTEM_FILENAME);
	if (hFile == 0)
	{
		CLogger::Get ()->Write (FromMemory, LogError, "File not found: %s", SYSTEM_FILENAME);

		return FALSE;
	}

	unsigned nResult = m_pFileSystem->FileRead (hFile, memory + MEM_CCP, SYSTEM_MAXSIZE);
	if (   nResult == FS_ERROR
	    || nResult < SYSTEM_MINSIZE)
	{
		CLogger::Get ()->Write (FromMemory, LogError, "Error loading system");

		m_pFileSystem->FileClose (hFile);

		return FALSE;
	}

	if (!m_pFileSystem->FileClose (hFile))
	{
		CLogger::Get ()->Write (FromMemory, LogError, "Cannot close file");

		return FALSE;
	}
#else
	FILE *pFile = fopen (SYSTEM_FILENAME, "r");
	if (pFile == 0)
	{
		fprintf (stderr, "File not found: %s\n", SYSTEM_FILENAME);

		return FALSE;
	}

	if (fread (memory + MEM_CCP, 1, SYSTEM_MAXSIZE, pFile) < SYSTEM_MINSIZE)
	{
		fprintf (stderr, "Error loading system\n");

		fclose (pFile);

		return FALSE;
	}

	fclose (pFile);
#endif

	return TRUE;
}

void *CZ80Memory::GetDMAPointer (u16 usAddress, u16 usLength)
{
	assert (usAddress < Z80_RAM_SIZE);
	if (usAddress + usLength < usAddress)		// address wraps
	{
		return 0;
	}

	return memory + usAddress;
}
