//
// multicore.cpp
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
#include <circle/sysconfig.h>

#ifdef ARM_ALLOW_MULTI_CORE

#include "multicore.h"
#include <assert.h>

CMultiCoreEmulation::CMultiCoreEmulation (CZ80Computer   *pComputer,
					  CMemorySystem  *pMemorySystem,
					  CFATFileSystem *pFileSystem)
:	CMultiCoreSupport (pMemorySystem),
	m_pComputer (pComputer),
	m_pFileSystem (pFileSystem)
{
}

CMultiCoreEmulation::~CMultiCoreEmulation (void)
{
}

void CMultiCoreEmulation::Run (unsigned nCore)
{
	// Core 0 handles the IRQs by default and does not need a special handling here.
	// 	It is halted automatically when core 1 terminates.

	// Core 2 and 3 are not used and can halt immediately.

	if (nCore == 1)
	{
		assert (m_pComputer != 0);
		m_pComputer->Run ();

		assert (m_pFileSystem != 0);
		m_pFileSystem->UnMount ();
	}
}

#endif
