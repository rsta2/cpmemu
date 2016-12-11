//
// console.h
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
#ifndef _console_h
#define _console_h

#include "types.h"

#ifdef __circle__
	#include <circle/usb/usbkeyboard.h>
	#include <circle/screen.h>
#else
	#include <sys/ioctl.h>
	#include <termios.h>
#endif

class CConsole
{
public:
	CConsole (void);
	~CConsole (void);

	boolean Initialize (void);

	void PutChar (u8 ucChar);

	boolean GetStatus (void);	// returns TRUE if character is available
	u8 GetChar (void);

#ifdef __circle__
	void SetLEDs (void);

private:
	static void KeyPressedHandler (const char *pString);
#endif

private:
#ifdef __circle__
	CUSBKeyboardDevice *m_pKeyboard;
	CScreenDevice *m_pScreen;

	u8 m_ucLEDStatus;
#else
	struct termio m_SaveTTY;
#endif
	boolean m_bInited;
	volatile u8 m_ucCharBuf;

	static CConsole *s_pThis;
};

#endif
