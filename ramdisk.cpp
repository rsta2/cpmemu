//
// ramdisk.cpp
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
#include "ramdisk.h"
#include "config.h"
#include <assert.h>

#ifdef __circle__
	#include <circle/logger.h>
	#include <circle/util.h>
#else
	#include <stdio.h>
	#include <string.h>
#endif

#ifdef __circle__
static const char FromRAMDisk[] = "ramdisk";
#endif

#ifdef __circle__
CRAMDisk::CRAMDisk (CFATFileSystem *pFileSystem, unsigned nDrive)
:	m_pFileSystem (pFileSystem),
#else
CRAMDisk::CRAMDisk (unsigned nDrive)
:
#endif
	m_nDrive (nDrive),
	m_bAvailable (FALSE),
	m_bWritten (FALSE),
	m_pBuffer (0)
{
}

CRAMDisk::~CRAMDisk (void)
{
	delete (m_pBuffer);
	m_pBuffer = 0;
}

boolean CRAMDisk::Initialize (void)
{
	assert (m_pBuffer == 0);
	m_pBuffer = new u8[DISK_SIZE];
	assert (m_pBuffer != 0);

	assert (m_nDrive <= 1);
	const char *pFilename = m_nDrive == 0 ? DISK_A_FILENAME : DISK_B_FILENAME;

#ifdef __circle__
	assert (m_pFileSystem != 0);
	unsigned hFile = m_pFileSystem->FileOpen (pFilename);
	if (hFile == 0)
	{
		if (m_nDrive == 0)
		{
			CLogger::Get ()->Write (FromRAMDisk, LogError, "File not found: %s", pFilename);
		}

		return FALSE;
	}

	u8 *pReadPtr = m_pBuffer;
	for (unsigned nTrack = 0; nTrack < TRACK_COUNT; nTrack++)
	{
		unsigned nResult = m_pFileSystem->FileRead (hFile, pReadPtr, TRACK_SIZE);
		if (nResult != TRACK_SIZE)
		{
			CLogger::Get ()->Write (FromRAMDisk, LogError, "Error loading RAM disk");

			m_pFileSystem->FileClose (hFile);

			return FALSE;
		}

		pReadPtr += TRACK_SIZE;
	}

	if (!m_pFileSystem->FileClose (hFile))
	{
		CLogger::Get ()->Write (FromRAMDisk, LogError, "Cannot close file");

		return FALSE;
	}
#else
	FILE *pFile = fopen (pFilename, "r");
	if (pFile == 0)
	{
		if (m_nDrive == 0)
		{
			fprintf (stderr, "File not found: %s\n", pFilename);
		}

		return FALSE;
	}

	if (fread (m_pBuffer, SECTOR_SIZE, SECTOR_COUNT, pFile) != SECTOR_COUNT)
	{
		fprintf (stderr, "Error loading RAM disk\n");

		fclose (pFile);

		return FALSE;
	}

	fclose (pFile);
#endif

	m_bAvailable = TRUE;

	return TRUE;
}

boolean CRAMDisk::IsAvailable (void) const
{
	return m_bAvailable;
}

boolean CRAMDisk::Read (unsigned nSector, void *pBuffer)
{
	assert (m_bAvailable);

	unsigned nOffset = nSector * SECTOR_SIZE;
	if (nOffset + SECTOR_SIZE > DISK_SIZE)
	{
		return FALSE;
	}

	assert (pBuffer != 0);
	assert (m_pBuffer != 0);
	memcpy (pBuffer, m_pBuffer + nOffset, SECTOR_SIZE);

	return TRUE;
}

boolean CRAMDisk::Write (unsigned nSector, const void *pBuffer)
{
	assert (m_bAvailable);

	unsigned nOffset = nSector * SECTOR_SIZE;
	if (nOffset + SECTOR_SIZE > DISK_SIZE)
	{
		return FALSE;
	}

	assert (m_pBuffer != 0);
	assert (pBuffer != 0);
	memcpy (m_pBuffer + nOffset, pBuffer, SECTOR_SIZE);

	m_bWritten = TRUE;

	return TRUE;
}

boolean CRAMDisk::Save (void)
{
	assert (m_bAvailable);

	if (!m_bWritten)
	{
		return TRUE;
	}

	assert (m_nDrive <= 1);
	const char *pFilename = m_nDrive == 0 ? DISK_A_FILENAME : DISK_B_FILENAME;

#ifdef __circle__
	assert (m_pFileSystem != 0);
	unsigned hFile = m_pFileSystem->FileCreate (pFilename);
	if (hFile == 0)
	{
		CLogger::Get ()->Write (FromRAMDisk, LogError, "Cannot create file: %s", pFilename);

		return FALSE;
	}

	assert (m_pBuffer != 0);
	u8 *pWritePtr = m_pBuffer;
	for (unsigned nTrack = 0; nTrack < TRACK_COUNT; nTrack++)
	{
		unsigned nResult = m_pFileSystem->FileWrite (hFile, pWritePtr, TRACK_SIZE);
		if (nResult != TRACK_SIZE)
		{
			CLogger::Get ()->Write (FromRAMDisk, LogError, "Error saving RAM disk");

			m_pFileSystem->FileClose (hFile);

			return FALSE;
		}

		pWritePtr += TRACK_SIZE;
	}

	if (!m_pFileSystem->FileClose (hFile))
	{
		CLogger::Get ()->Write (FromRAMDisk, LogError, "Cannot close file");

		return FALSE;
	}
#else
	FILE *pFile = fopen (pFilename, "w");
	if (pFile == 0)
	{
		fprintf (stderr, "Cannot create: %s\n", pFilename);

		return FALSE;
	}

	assert (m_pBuffer != 0);
	if (fwrite (m_pBuffer, SECTOR_SIZE, SECTOR_COUNT, pFile) != SECTOR_COUNT)
	{
		fprintf (stderr, "Error saving RAM disk\n");

		fclose (pFile);

		return FALSE;
	}

	fclose (pFile);
#endif

	return TRUE;
}
