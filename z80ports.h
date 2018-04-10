//
// z80ports.h
//
// Copyright (C) 2016-2018  R. Stange <rsta2@o2online.de>
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
#ifndef _z80ports_h
#define _z80ports_h

#include "z80memory.h"
#include "console.h"
#include "ramdisk.h"
#include "types.h"

class CZ80Computer;

class CZ80Ports
{
public:
	CZ80Ports (CZ80Computer *pComputer, CZ80Memory *pMemory, CConsole *pConsole,
		   CRAMDisk *pRAMDisk0, CRAMDisk *pRAMDisk1);
	~CZ80Ports (void);

	boolean Initialize (void);

	u8 PortInput (u16 usPort);
	void PortOutput (u16 usPort, u8 ucValue);

public:
	static CZ80Ports *s_pThis;

private:
	CZ80Computer *m_pComputer;
	CZ80Memory   *m_pMemory;
	CConsole     *m_pConsole;
	CRAMDisk     *m_pRAMDisk0;
	CRAMDisk     *m_pRAMDisk1;

	u8	m_ucDiskDriveCount;
	u8	m_ucDiskDrive;
	u8      m_ucDiskTrack;
	u8      m_ucDiskSector;
	u16     m_usDMAAddress;
	boolean m_bDiskStatus;
};

#endif
