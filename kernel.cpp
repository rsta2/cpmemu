//
// kernel.cpp
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
#include "kernel.h"

#define PARTITION	"emmc1-1"

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (640, 480),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_DWHCI (&m_Interrupt, &m_Timer),
	m_Computer (&m_FileSystem)
{
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_EMMC.Initialize ();
	}

	if (bOK)
	{
		CDevice *pPartition = m_DeviceNameService.GetDevice (PARTITION, TRUE);
		if (pPartition != 0)
		{
			if (!m_FileSystem.Mount (pPartition))
			{
				m_Logger.Write (FromKernel, LogError, "Cannot mount partition: %s", PARTITION);

				bOK = FALSE;
			}
		}
		else
		{
			m_Logger.Write (FromKernel, LogError, "Partition not found: %s", PARTITION);

			bOK = FALSE;
		}
	}

	if (bOK)
	{
		bOK = m_DWHCI.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Computer.Initialize ();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Computer.Run ();

	m_FileSystem.UnMount ();

	return ShutdownHalt;
}
