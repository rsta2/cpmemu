//
// main.cpp
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
#ifdef __circle__
	#include "kernel.h"
	#include <circle/startup.h>
#else
	#include "z80computer.h"
#endif

#ifdef __circle__

int main (void)
{
	// cannot return here because some destructors used in CKernel are not implemented

	CKernel Kernel;
	if (!Kernel.Initialize ())
	{
		halt ();
		return EXIT_HALT;
	}

	TShutdownMode ShutdownMode = Kernel.Run ();

	switch (ShutdownMode)
	{
	case ShutdownReboot:
		reboot ();
		return EXIT_REBOOT;

	case ShutdownHalt:
	default:
		halt ();
		return EXIT_HALT;
	}
}

#else

int main (void)
{
	CZ80Computer Computer;

	if (!Computer.Initialize ())
	{
		return 1;
	}

	Computer.Run ();

	return 0;
}

#endif
