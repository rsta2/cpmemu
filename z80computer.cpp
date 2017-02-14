//
// z80computer.cpp
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
#include "z80computer.h"

#ifdef __circle__
	#include <circle/timer.h>
	#include <circle/cputhrottle.h>
#endif

#define INSTRUCTIONS_PER_STEP		1000

#ifdef __circle__
CZ80Computer::CZ80Computer (CFATFileSystem *pFileSystem)
:	m_Memory (pFileSystem),
	m_RAMDisk (pFileSystem),
#else
CZ80Computer::CZ80Computer (void)
:
#endif
	m_Ports (this, &m_Memory, &m_Console, &m_RAMDisk),
	m_CPU (this),
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
	m_CPU.reset ();

#ifdef __circle__
	unsigned nLastTicks = CTimer::Get ()->GetClockTicks ();
#endif

	while (m_bContinue)
	{
		for (unsigned i = 1; i <= INSTRUCTIONS_PER_STEP; i++)
		{
			m_CPU.execute ();
		}

#ifdef __circle__
		unsigned nTicks = CTimer::Get ()->GetClockTicks ();
		if (nTicks - nLastTicks >= 4*CLOCKHZ)			// call this every 4 seconds
		{
			CCPUThrottle::Get ()->SetOnTemperature ();

			nLastTicks = nTicks;
		}

		m_Console.SetLEDs ();
#endif
	}
}

void CZ80Computer::Shutdown (void)
{
	m_bContinue = FALSE;
}

uint8_t CZ80Computer::fetchOpcode (uint16_t address)
{
	return m_Memory.ReadByte (address);
}

uint8_t CZ80Computer::peek8 (uint16_t address)
{
	return m_Memory.ReadByte (address);
}

void CZ80Computer::poke8 (uint16_t address, uint8_t value)
{
	m_Memory.WriteByte (address, value);
}

uint16_t CZ80Computer::peek16 (uint16_t address)
{
	return m_Memory.ReadWord (address);
}

void CZ80Computer::poke16 (uint16_t address, uint16_t word)
{
	m_Memory.WriteWord (address, word);
}

uint8_t CZ80Computer::inPort (uint16_t port)
{
	return m_Ports.PortInput (port);
}

void CZ80Computer::outPort (uint16_t port, uint8_t value)
{
	m_Ports.PortOutput (port, value);
}

void CZ80Computer::addressOnBus (uint16_t address, uint32_t wstates)
{
}

void CZ80Computer::interruptHandlingTime (uint32_t wstates)
{
}

uint8_t CZ80Computer::breakpoint (uint16_t address, uint8_t opcode)
{
	return opcode;
}

void CZ80Computer::execDone (void)
{
}
