//
// console.cpp
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
#include "console.h"
#include <assert.h>

#ifdef __circle__
	#include <circle/devicenameservice.h>
	#include <circle/logger.h>
	#include <circle/string.h>
	#include <circle/util.h>
#else
	#include <stdio.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/types.h>
#endif

#ifdef __circle__
static const char FromConsole[] = "console";
#endif

CConsole *CConsole::s_pThis = 0;

CConsole::CConsole (void)
:
#ifdef __circle__
	m_pKeyboard (0),
	m_pScreen (0),
#endif
	m_bInited (FALSE),
	m_ucCharBuf (0)
{
	s_pThis = this;
}

CConsole::~CConsole (void)
{
#ifndef __circle__
	if (m_bInited)
	{
		ioctl (0, TCSETAF, &m_SaveTTY);
		m_bInited = FALSE;
	}
#endif

	s_pThis = 0;
}

boolean CConsole::Initialize (void)
{
	assert (!m_bInited);

#ifdef __circle__
	assert (m_pKeyboard == 0);
	m_pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get ()->GetDevice ("ukbd1", FALSE);
	if (m_pKeyboard == 0)
	{
		CLogger::Get ()->Write (FromConsole, LogError, "Keyboard not found");

		return FALSE;
	}

	m_pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);

	assert (m_pScreen == 0);
	m_pScreen = (CScreenDevice *) CDeviceNameService::Get ()->GetDevice ("tty1", FALSE);
	if (m_pScreen == 0)
	{
		CLogger::Get ()->Write (FromConsole, LogError, "Screen not found");

		return FALSE;
	}

	// set 80x24 characters window
	const char InitString[] = "\x1b[H\x1b[J\x1b[1;24r";
	m_pScreen->Write (InitString, sizeof InitString-1);
#else
	if (ioctl (0, TCGETA, &m_SaveTTY) < 0)
	{
		fprintf (stderr, "Not a tty\n");

		return FALSE;
	}

	struct termio NewTTY;
	NewTTY = m_SaveTTY;
	NewTTY.c_lflag &= ~ICANON;
	NewTTY.c_lflag &= ~ECHO;
	NewTTY.c_lflag &= ~ISIG;
	NewTTY.c_iflag &= ~IXON;
	NewTTY.c_cc[VMIN] = 1;
	NewTTY.c_cc[VTIME] = 0;

	if (ioctl (0, TCSETAF, &NewTTY) < 0)
	{
		fprintf (stderr, "Cannot put tty into raw mode\n");

		return FALSE;
	}
#endif

	m_bInited = TRUE;

	return TRUE;
}

void CConsole::PutChar (u8 ucChar)
{
	assert (m_bInited);

#ifdef __circle__
	assert (m_pScreen != 0);
	m_pScreen->Write (&ucChar, sizeof ucChar);
#else
	write (1, &ucChar, sizeof ucChar);
#endif
}

boolean CConsole::GetStatus (void)
{
	assert (m_bInited);

#ifdef __circle__
	return m_ucCharBuf == 0 ? FALSE : TRUE;
#else
	if (m_ucCharBuf == 0)
	{
		fd_set Fds;
		FD_ZERO (&Fds);
		FD_SET (0, &Fds);

		struct timeval Timeout;
		Timeout.tv_sec = 0;
		Timeout.tv_usec = 100;

		int nResult = select (1, &Fds, 0, 0, &Timeout);
		if (nResult < 0)
		{
			fprintf (stderr, "select returned %d\n", nResult);

			return FALSE;
		}

		if (nResult == 0)		// timeout
		{
			return FALSE;
		}

		u8 ucCharBuf;
		ssize_t nBytesRead = read (0, &ucCharBuf, sizeof ucCharBuf);
		if (nBytesRead <= 0)
		{
			fprintf (stderr, "read returned %d\n", nBytesRead);

			m_ucCharBuf = 0;

			return FALSE;
		}

		m_ucCharBuf = ucCharBuf;
	}

	return TRUE;
#endif
}

u8 CConsole::GetChar (void)
{
	assert (m_bInited);

#ifdef __circle__
	while (m_ucCharBuf == 0)
	{
		// just wait for key
	}
#else
	if (m_ucCharBuf == 0)
	{
		ssize_t nBytesRead;
		u8 ucCharBuf;
		while ((nBytesRead = read (0, &ucCharBuf, sizeof ucCharBuf)) <= 0)
		{
			fprintf (stderr, "read returned %d\n", nBytesRead);
		}

		m_ucCharBuf = ucCharBuf;
	}

#endif

	u8 ucResult = m_ucCharBuf;
	m_ucCharBuf = 0;

	if (ucResult == '\n')
	{
		ucResult = '\r';
	}

	if (ucResult == '\x7f')
	{
		ucResult = '\b';
	}

	return ucResult;
}

#ifdef __circle__

void CConsole::KeyPressedHandler (const char *pString)
{
	assert (s_pThis != 0);

	if (s_pThis->m_ucCharBuf == 0)
	{
		s_pThis->m_ucCharBuf = pString[0];
	}
}

#endif
