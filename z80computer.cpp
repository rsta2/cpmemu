//
// z80computer.cpp
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
#include "z80computer.h"

#define CYCLES_PER_STEP		100

#ifdef __circle__
CZ80Computer::CZ80Computer (CFATFileSystem *pFileSystem)
:	m_Memory (pFileSystem),
	m_RAMDisk (pFileSystem),
#else
CZ80Computer::CZ80Computer (void)
:
#endif
	m_Ports (this, &m_Memory, &m_Console, &m_RAMDisk),
	m_bContinue (TRUE)
{
}

CZ80Computer::~CZ80Computer (void)
{
}

boolean CZ80Computer::Initialize (void)
{
	if (!m_Memory.Initialize ())
	{
		return FALSE;
	}

	if (!m_Console.Initialize ())
	{
		return FALSE;
	}

	if (!m_RAMDisk.Initialize ())
	{
		return FALSE;
	}

	if (!m_Ports.Initialize ())
	{
		return FALSE;
	}

	return TRUE;
}

void CZ80Computer::Run (void)
{
	Z80Reset (&m_CPU);

	while (m_bContinue)
	{
		Z80Emulate (&m_CPU, CYCLES_PER_STEP);
	}
}

void CZ80Computer::Shutdown (void)
{
	m_bContinue = FALSE;
}
