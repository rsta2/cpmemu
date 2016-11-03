//
// config.h
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
#ifndef _config_h
#define _config_h

// Files
#define SYSTEM_FILENAME		"system.bin"		// CP/M binary (system)
#define DISK_FILENAME		"cpmdisk.bin"		// CP/M disk image

// Disk image
#define SECTOR_SIZE		128			// CP/M sector size
#define TRACK_COUNT		80			// Number of tracks
#define SECTORS_PER_TRACK	80			// Total sectors per track

#define TRACK_SIZE		(SECTOR_SIZE * SECTORS_PER_TRACK)
#define SECTOR_COUNT		(TRACK_COUNT * SECTORS_PER_TRACK)
#define DISK_SIZE		(SECTOR_SIZE * SECTOR_COUNT)

// System load addresses
#define MEM_CCP			0xDC00
#define MEM_BDOS		0xE400
#define MEM_BIOS		0xF200

// System size
#define SYSTEM_MINSIZE		(MEM_BIOS - MEM_CCP + SECTOR_SIZE)
#define SYSTEM_MAXSIZE		(0x10000 - MEM_CCP)

#endif
