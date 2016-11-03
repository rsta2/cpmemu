//
// cpmdisk.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#define SECTOR_SIZE	128		// logical sector size of CP/M

#define FORMAT_BYTE	0xE5

#define PACKED		__attribute__ ((packed))

// parameters

static struct
{
	const char    *pDiskFileName;
	unsigned char  nUserNumber;
	unsigned       nFilesPerLine;	// for directory listing
}
ToolParam = {"cpmdisk.bin", 0, 5};

static struct
{
	// configured
	unsigned nTracks;
	unsigned nReservedTracks;
	unsigned nSectorsPerTrack;	// 128 byte sectors
	unsigned nBlockSize;		// byte size of an allocation block
	unsigned nExtendSize;		// byte size of an extend
	unsigned nDirectoryEntries;	// number of entries

	// calculated
	unsigned nReservedBytes;
	unsigned nTotalSectors;		// including reserved sectors
	unsigned nTotalBlocks;		// without reserved sectors
	unsigned nSectorsPerBlock;
	unsigned nSectorsPerExtend;
	unsigned nBlocksPerExtend;
	unsigned nDirectoryBlocks;
}
DiskParam = {80, 2, 80, 2*1024, 16*1024, 128};

// directory entry

#define FILENAME_LEN	8
#define EXTENSION_LEN	3

#define BLOCKCOUNT_BYTE	16		// per extend
#define BLOCKCOUNT_WORD	8		// per extend

typedef struct
{
	unsigned char	UserNumber;			// == 0xE5 for free entry
	unsigned char	FileName[FILENAME_LEN];		// padded with ' '
	unsigned char	Extension[EXTENSION_LEN];	// padded with ' '
	unsigned char	Extend;
	unsigned char	Reserved1;			// set to 0
	unsigned char	Reserved2;			// set to 0, TODO: may be high byte of sector count
	unsigned char	SectorCount;			// sectors in this extend

	union
	{
		// the field used depends on the total block size of the disk
		unsigned char	Byte[BLOCKCOUNT_BYTE];
		unsigned short	Word[BLOCKCOUNT_WORD];
	}
	BlockNumber;
}
PACKED TDirectoryEntry;

static const char Usage[] =
{
	"cpmdisk init [ options ]\t\tPrepare a file to use it as CP/M disk image\n"
	"cpmdisk dir [ options ]\t\t\tDisplay directory listing of CP/M disk image\n"
	"cpmdisk read [ options ] filename ...\tRead file(s) from CP/M disk image and save it\n"
	"cpmdisk write [ options ] filename ...\tWrite existing file(s) to CP/M disk image\n"
	"cpmdisk delete [ options ] filename ...\tDelete existing file(s) from CP/M disk image\n"
	"\n"
	"Options\t\t\t\t\t\t\t\tDefault\n"
	"\n"
	"-f filename\t\tFilename of the CP/M disk image\t\tcpmdisk.bin\n"
	"-t tracks\t\tNumber of tracks\t\t\t80\n"
	"-r reserved_tracks\tNumber of reserved tracks\t\t2\n"
	"-s sectors_per_track\tNumber of 128-byte sectors per track\t80\n"
	"-b block_size\t\tSize of an allocation block in Kbyte\t2\n"
	"-e extend_size\t\tSize of an extend in Kbyte\t\t16\n"
	"-d directory_entries\tMaximum number of directory entries\t128\n"
	"-u user\t\t\tCP/M user number\t\t\t0\n"
};

static unsigned GetOptionNumber (int nArgC, char **ppArgV,
				 unsigned nMin, unsigned nMax,
				 const char *pArg0, const char *pOption)
{
	if (nArgC == 0 || **ppArgV == '-')
	{
		fprintf (stderr, "%s: Option requires parameter: %s\n", pArg0, pOption);

		exit (1);
	}

	char *pEnd = NULL;
	unsigned long ulResult = strtoul (*ppArgV, &pEnd, 10);
	if (   (pEnd != NULL && *pEnd != '\0')
	    || !(nMin <= ulResult && ulResult <= nMax))
	{
		fprintf (stderr, "%s: Invalid option parameter: %s %s\n", pArg0, pOption, *ppArgV);

		exit (1);
	}

	return (unsigned) ulResult;
}

static int ConvertFileName (const char *pName, char *pCPMName)		// returns 0 on error
{
	assert (pName != 0);
	assert (pCPMName != 0);

	memset (pCPMName, ' ', FILENAME_LEN+EXTENSION_LEN);

	const char *pFrom;
	char *pTo;
	unsigned nLength;
	for (pFrom = pName, pTo = pCPMName, nLength = FILENAME_LEN; *pFrom != '\0'; pFrom++)
	{
		char c = *pFrom;

		if (c <= ' ')
		{
			return 0;
		}

		static const char *pBadChars = "\"*+,/:;<=>?[\\]|";
		const char *pBad;
		for (pBad = pBadChars; *pBad; pBad++)
		{
			if (c == *pBad)
			{
				return 0;
			}
		}

		if ('a' <= c && c <= 'z')
		{
			c -= 'a'-'A';
		}

		if (c == '.')
		{
			if (pTo > pCPMName+FILENAME_LEN)
			{
				return 0;
			}

			pTo = pCPMName+FILENAME_LEN;
			nLength = EXTENSION_LEN;
			continue;
		}

		if (nLength > 0)
		{
			*pTo++ = c;
			nLength--;
		}
	}

	if (pCPMName[0] == ' ')
	{
		return 0;
	}

	return 1;
}

static int DoInit (int nArgC, char **ppArgV, const char *pArg0)
{
	if (nArgC > 0)
	{
		fprintf (stderr, "%s: Unexpected argument: %s\n", pArg0, *ppArgV);

		return 1;
	}

	// do not overwrite existing file

	struct stat Stat;
	if (stat (ToolParam.pDiskFileName, &Stat) == 0)
	{
		fprintf (stderr, "%s: File exists: %s\n", pArg0, ToolParam.pDiskFileName);

		return 1;
	}

	// create disk file

	FILE *pOutFile = fopen (ToolParam.pDiskFileName, "w");
	if (pOutFile == NULL)
	{
		fprintf (stderr, "%s: Cannot create: %s\n", pArg0, ToolParam.pDiskFileName);

		return 1;
	}

	// write entire disk file with 0xE5

	unsigned char SectorBuffer[SECTOR_SIZE];
	memset (SectorBuffer, FORMAT_BYTE, sizeof SectorBuffer);

	unsigned nSector;
	for (nSector = 0; nSector < DiskParam.nTotalSectors; nSector++)
	{
		if (fwrite (SectorBuffer, sizeof SectorBuffer, 1, pOutFile) != 1)
		{
			fprintf (stderr, "%s: Write error: %s\n", pArg0, ToolParam.pDiskFileName);

			fclose (pOutFile);

			return 1;
		}
	}

	fclose (pOutFile);

	return 0;
}

static int DoDir (int nArgC, char **ppArgV, const char *pArg0)
{
	if (nArgC > 0)
	{
		fprintf (stderr, "%s: Unexpected argument: %s\n", pArg0, *ppArgV);

		return 1;
	}

	// open disk file

	FILE *pInFile = fopen (ToolParam.pDiskFileName, "r");
	if (pInFile == NULL)
	{
		fprintf (stderr, "%s: Cannot open: %s\n", pArg0, ToolParam.pDiskFileName);

		return 1;
	}

	// seek to directory (starts after reserved tracks)

	if (fseek (pInFile, DiskParam.nReservedBytes, SEEK_SET) != 0)
	{
		fprintf (stderr, "%s: Seeking directory failed\n", pArg0);

		fclose (pInFile);

		return 1;
	}

	unsigned nFile = 0;	// counts file names on a line

	unsigned nEntry;
	for (nEntry = 0; nEntry < DiskParam.nDirectoryEntries; nEntry++)	// for all directory entries
	{
		TDirectoryEntry Entry;
		if (fread (&Entry, sizeof Entry, 1, pInFile) != 1)
		{
			fprintf (stderr, "%s: Reading directory failed\n", pArg0);

			fclose (pInFile);

			return 1;
		}

		if (   Entry.UserNumber != ToolParam.nUserNumber
		    || Entry.Extend != 0)
		{
			continue;
		}

		// convert file name for display

		char FileName[FILENAME_LEN+EXTENSION_LEN+1+1];		// +1 for dot, +1 for '\0'
		memcpy (FileName, Entry.FileName, FILENAME_LEN);
		FileName[FILENAME_LEN] = '\0';

		char *p = strchr (FileName, ' ');
		if (p == 0)
		{
			p = FileName + FILENAME_LEN;
		}

		*p++ = '.';

		memcpy (p, Entry.Extension, EXTENSION_LEN);
		p[EXTENSION_LEN] = '\0';

		p = strchr (p, ' ');
		if (p != 0)
		{
			*p = '\0';
		}

		// display file name

		printf ("%-12s", FileName);

		if (++nFile < ToolParam.nFilesPerLine)
		{
			printf ("  ");
		}
		else
		{
			printf ("\n");

			nFile = 0;
		}
	}

	if (nFile != 0)
	{
		printf ("\n");
	}

	fclose (pInFile);

	return 0;
}

static int DoRead (int nArgC, char **ppArgV, const char *pArg0)
{
	if (nArgC == 0)
	{
		fprintf (stderr, "%s: Missing filename\n", pArg0);

		return 1;
	}

	// open disk file

	FILE *pInFile = fopen (ToolParam.pDiskFileName, "r");
	if (pInFile == NULL)
	{
		fprintf (stderr, "%s: Cannot open: %s\n", pArg0, ToolParam.pDiskFileName);

		return 1;
	}

	// seek to directory (starts after reserved tracks)

	if (fseek (pInFile, DiskParam.nReservedBytes, SEEK_SET) != 0)
	{
		fprintf (stderr, "%s: Seeking directory failed\n", pArg0);

		fclose (pInFile);

		return 1;
	}

	// read entire directory into pDirectory buffer

	TDirectoryEntry *pDirectory = (TDirectoryEntry *) malloc (DiskParam.nDirectoryEntries * sizeof (TDirectoryEntry));
	assert (pDirectory != 0);

	if (fread (pDirectory, sizeof (TDirectoryEntry), DiskParam.nDirectoryEntries, pInFile) != DiskParam.nDirectoryEntries)
	{
		fprintf (stderr, "%s: Reading directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInFile);

		return 1;
	}

	while (nArgC-- > 0)		// for all file names on the command line
	{
		const char *pFileName = *ppArgV++;
		assert (pFileName != 0);

		char CPMFileName[FILENAME_LEN+EXTENSION_LEN];
		if (!ConvertFileName (pFileName, CPMFileName))
		{
			fprintf (stderr, "%s: Invalid filename: %s\n", pArg0, pFileName);

			free (pDirectory);
			fclose (pInFile);

			return 1;
		}

		TDirectoryEntry *pEntry = NULL;		// current extend entry in directory
		unsigned char nExtend = 0;		// current extend number
		FILE *pOutFile = NULL;			// != NULL if open

		// allocate block buffer

		void *pBlockBuffer = malloc (DiskParam.nBlockSize);
		assert (pBlockBuffer != 0);

		do		// while another extend may follow
		{
			// does this extend exists for this user and file name?

			unsigned nEntry;
			for (nEntry = 0; nEntry < DiskParam.nDirectoryEntries; nEntry++)
			{
				pEntry = pDirectory + nEntry;

				if (   pEntry->UserNumber == ToolParam.nUserNumber
				    && pEntry->Extend == nExtend
				    && memcmp (pEntry->FileName, CPMFileName, sizeof CPMFileName) == 0)
				{
					break;
				}
			}

			if (nEntry >= DiskParam.nDirectoryEntries)
			{
				// extend not found: leave loop
				break;
			}

			// open the output file if this is the first found extend

			if (nExtend == 0)
			{
				assert (pOutFile == NULL);
				pOutFile = fopen (pFileName, "w");
				if (pOutFile == NULL)
				{
					fprintf (stderr, "%s: Cannot create: %s\n", pArg0, pFileName);

					free (pBlockBuffer);
					free (pDirectory);
					fclose (pInFile);

					return 1;
				}
			}

			// calculate sector and block count in this extend

			assert (pEntry->SectorCount > 0);
			unsigned nSectorsLeft = pEntry->SectorCount;

			unsigned nBlockCount =   (pEntry->SectorCount+DiskParam.nSectorsPerBlock-1)
					       / DiskParam.nSectorsPerBlock;

			// loop over all data blocks in this extend

			unsigned nBlock;
			for (nBlock = 0; nBlock < nBlockCount; nBlock++, nSectorsLeft -= DiskParam.nSectorsPerBlock)
			{
				// get block number for current block from directory entry for this extend

				unsigned nBlockNumber;
				if (DiskParam.nTotalBlocks <= 255)	// TODO: 256 (?)
				{
					// TODO: has not been tested
					assert (nBlock < BLOCKCOUNT_BYTE);
					nBlockNumber = pEntry->BlockNumber.Byte[nBlock];
				}
				else
				{
					assert (nBlock < BLOCKCOUNT_WORD);
					nBlockNumber = pEntry->BlockNumber.Word[nBlock];
				}

				// calculate byte offset of this block from block number

				assert (nBlockNumber > 0);
				assert (nBlockNumber < DiskParam.nTotalBlocks);
				unsigned nBlockOffset =   nBlockNumber * DiskParam.nBlockSize
							+ DiskParam.nReservedBytes;

				// seek this block

				if (fseek (pInFile, nBlockOffset, SEEK_SET) != 0)
				{
					fprintf (stderr, "%s: Seeking to block %u failed\n", pArg0, nBlockNumber);

					free (pBlockBuffer);
					free (pDirectory);
					fclose (pInFile);

					if (pOutFile != NULL)
					{
						fclose (pOutFile);
					}

					return 1;
				}

				// if this is the last block of the file:
				//	read the remaining sectors of the block only

				unsigned nSectors =   nSectorsLeft >= DiskParam.nSectorsPerBlock
						    ? DiskParam.nSectorsPerBlock
						    : nSectorsLeft;

				if (fread (pBlockBuffer, SECTOR_SIZE, nSectors, pInFile) != nSectors)
				{
					fprintf (stderr, "%s: Reading block %u failed\n", pArg0, nBlockNumber);

					free (pBlockBuffer);
					free (pDirectory);
					fclose (pInFile);

					if (pOutFile != NULL)
					{
						fclose (pOutFile);
					}

					return 1;
				}

				// write block to output file

				if (fwrite (pBlockBuffer, SECTOR_SIZE, nSectors, pOutFile) != nSectors)
				{
					fprintf (stderr, "%s: Write error\n", pArg0);

					free (pBlockBuffer);
					free (pDirectory);
					fclose (pInFile);

					if (pOutFile != NULL)
					{
						fclose (pOutFile);
					}

					return 1;
				}
			}

			nExtend++;
		}
		while (pEntry->SectorCount == DiskParam.nSectorsPerExtend);

		free (pBlockBuffer);

		if (nExtend > 0)	// at least one extend was written?
		{
			assert (pOutFile != NULL);
			fclose (pOutFile);
		}
		else
		{
			fprintf (stderr, "%s: File not found: %s\n", pArg0, pFileName);

			free (pDirectory);
			fclose (pInFile);

			return 1;
		}
	}

	free (pDirectory);

	fclose (pInFile);

	return 0;
}

static int DoWrite (int nArgC, char **ppArgV, const char *pArg0)
{
	if (nArgC == 0)
	{
		fprintf (stderr, "%s: Missing filename\n", pArg0);

		return 1;
	}

	// open disk file

	FILE *pInOutFile = fopen (ToolParam.pDiskFileName, "r+");
	if (pInOutFile == NULL)
	{
		fprintf (stderr, "%s: Cannot open: %s\n", pArg0, ToolParam.pDiskFileName);

		return 1;
	}

	// seek to directory (starts after reserved tracks)

	if (fseek (pInOutFile, DiskParam.nReservedBytes, SEEK_SET) != 0)
	{
		fprintf (stderr, "%s: Seeking directory failed\n", pArg0);

		fclose (pInOutFile);

		return 1;
	}

	// read entire directory into pDirectory buffer

	TDirectoryEntry *pDirectory = (TDirectoryEntry *) malloc (DiskParam.nDirectoryEntries * sizeof (TDirectoryEntry));
	assert (pDirectory != 0);

	if (fread (pDirectory, sizeof (TDirectoryEntry), DiskParam.nDirectoryEntries, pInOutFile) != DiskParam.nDirectoryEntries)
	{
		fprintf (stderr, "%s: Reading directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInOutFile);

		return 1;
	}

	// create CP/M block map (one allocation flag per block)

	unsigned char *pBlockMap = (unsigned char *) malloc (DiskParam.nTotalBlocks);
	assert (pBlockMap != 0);
	memset (pBlockMap, 0, DiskParam.nTotalBlocks);

	// mark directory blocks allocated

	unsigned i;
	for (i = 0; i < DiskParam.nDirectoryBlocks; i++)
	{
		pBlockMap[i] = 1;
	}

	// for all directory entries

	unsigned nEntry;
	for (nEntry = 0; nEntry < DiskParam.nDirectoryEntries; nEntry++)
	{
		TDirectoryEntry *pEntry = pDirectory + nEntry;

		if (pEntry->UserNumber != FORMAT_BYTE)		// TODO: check for valid user number
		{
			// entry is valid extend: mark blocks in this extend as allocated

			if (DiskParam.nTotalBlocks <= 255)	// TODO: 256 (?)
			{
				// TODO: has not been tested
				unsigned i;
				for (i = 0; i < BLOCKCOUNT_BYTE; i++)
				{
					assert (pEntry->BlockNumber.Byte[i] < DiskParam.nTotalBlocks);
					pBlockMap[pEntry->BlockNumber.Byte[i]] = 1;
				}
			}
			else
			{
				unsigned i;
				for (i = 0; i < BLOCKCOUNT_WORD; i++)
				{
					assert (pEntry->BlockNumber.Word[i] < DiskParam.nTotalBlocks);
					pBlockMap[pEntry->BlockNumber.Word[i]] = 1;
				}
			}
		}
	}

	// allocate data block buffer

	unsigned char *pBlockBuffer = (unsigned char *) malloc (DiskParam.nBlockSize);
	assert (pBlockBuffer != 0);

	// for all file names on the command line

	while (nArgC-- > 0)
	{
		const char *pFileName = *ppArgV++;
		assert (pFileName != 0);

		// convert file name into CP/M representation (without path)

		const char *pBaseName = strrchr (pFileName, '/');
		if (pBaseName != 0)
		{
			pBaseName = pBaseName + 1;
		}
		else
		{
			pBaseName = pFileName;
		}

		char CPMFileName[FILENAME_LEN+EXTENSION_LEN];
		if (!ConvertFileName (pBaseName, CPMFileName))
		{
			fprintf (stderr, "%s: Invalid filename: %s\n", pArg0, pBaseName);

			free (pBlockBuffer);
			free (pBlockMap);
			free (pDirectory);
			fclose (pInOutFile);

			return 1;
		}

		// do not overwrite an existing file

		unsigned nEntry;
		for (nEntry = 0; nEntry < DiskParam.nDirectoryEntries; nEntry++)
		{
			TDirectoryEntry *pEntry = pDirectory + nEntry;

			// does this file exist for this user?

			if (   pEntry->UserNumber == ToolParam.nUserNumber
			    && memcmp (pEntry->FileName, CPMFileName, sizeof CPMFileName) == 0)
			{
				fprintf (stderr, "%s: File exists: %s\n", pArg0, pBaseName);

				free (pBlockBuffer);
				free (pBlockMap);
				free (pDirectory);
				fclose (pInOutFile);

				return 1;
			}
		}

		// open input file

		FILE *pInFile = fopen (pFileName, "r");
		if (pInFile == NULL)
		{
			fprintf (stderr, "%s: File not found: %s\n", pArg0, pFileName);

			free (pBlockBuffer);
			free (pBlockMap);
			free (pDirectory);
			fclose (pInOutFile);

			return 1;
		}

		// get size of input file

		struct stat Stat;
		if (fstat (fileno (pInFile), &Stat) != 0)
		{
			fprintf (stderr, "%s: Cannot stat: %s\n", pArg0, pFileName);

			free (pBlockBuffer);
			free (pBlockMap);
			free (pDirectory);
			fclose (pInFile);
			fclose (pInOutFile);

			return 1;
		}

		// calculate number of extends to be created

		unsigned nExtends = (Stat.st_size + DiskParam.nExtendSize-1) / DiskParam.nExtendSize;
		if (nExtends == 0)
		{
			fprintf (stderr, "%s: File is empty: %s\n", pArg0, pFileName);

			free (pBlockBuffer);
			free (pBlockMap);
			free (pDirectory);
			fclose (pInFile);
			fclose (pInOutFile);

			return 1;
		}

		if (nExtends > 256)
		{
			fprintf (stderr, "%s: File too big: %s\n", pArg0, pFileName);

			free (pBlockBuffer);
			free (pBlockMap);
			free (pDirectory);
			fclose (pInFile);
			fclose (pInOutFile);

			return 1;
		}

		// calculate total bytes and blocks to be copied

		unsigned nBytesLeft  = Stat.st_size;
		unsigned nBlocksLeft = (nBytesLeft + DiskParam.nBlockSize-1) / DiskParam.nBlockSize;

		// for all extends to be created

		unsigned nExtend;
		for (nExtend = 0; nExtend < nExtends; nExtend++, nBlocksLeft -= DiskParam.nBlocksPerExtend)
		{
			// find a free directory entry to be used for this extend

			unsigned nEntry;
			for (nEntry = 0; nEntry < DiskParam.nDirectoryEntries; nEntry++)
			{
				if (pDirectory[nEntry].UserNumber == FORMAT_BYTE)
				{
					break;
				}
			}

			if (nEntry >= DiskParam.nDirectoryEntries)
			{
				fprintf (stderr, "%s: Directory full\n", pArg0);

				free (pBlockBuffer);
				free (pBlockMap);
				free (pDirectory);
				fclose (pInFile);
				fclose (pInOutFile);

				return 1;
			}

			// prepare directory entry for this extend

			TDirectoryEntry *pEntry = pDirectory + nEntry;
			memset (pEntry, 0, sizeof (TDirectoryEntry));

			pEntry->UserNumber = ToolParam.nUserNumber;
			memcpy (pEntry->FileName, CPMFileName, sizeof CPMFileName);
			pEntry->Extend = (unsigned char) nExtend;

			unsigned nSectorsLeft = (nBytesLeft + SECTOR_SIZE-1) / SECTOR_SIZE;
			pEntry->SectorCount =   nSectorsLeft >= DiskParam.nSectorsPerExtend
					      ? DiskParam.nSectorsPerExtend
					      : nSectorsLeft;

			// calculate number of data blocks in this extend

			unsigned nBlocks =   nBlocksLeft >= DiskParam.nBlocksPerExtend
					   ? DiskParam.nBlocksPerExtend
					   : nBlocksLeft;

			// for all data blocks in this extend

			unsigned nBlock;
			for (nBlock = 0; nBlock < nBlocks; nBlock++, nBytesLeft -= DiskParam.nBlockSize)
			{
				// find a free block to be allocated

				unsigned nFreeBlock;
				for (nFreeBlock = 0; nFreeBlock < DiskParam.nTotalBlocks; nFreeBlock++)
				{
					if (!pBlockMap[nFreeBlock])
					{
						break;
					}
				}

				if (nFreeBlock >= DiskParam.nTotalBlocks)
				{
					fprintf (stderr, "%s: Disk full\n", pArg0);

					free (pBlockBuffer);
					free (pBlockMap);
					free (pDirectory);
					fclose (pInFile);
					fclose (pInOutFile);

					return 1;
				}

				pBlockMap[nFreeBlock] = 1;		// allocate block

				// put block number of this new block into directory entry

				if (DiskParam.nTotalBlocks <= 255)	// TODO: 256 (?)
				{
					assert (nFreeBlock <= 255);
					assert (nBlock < BLOCKCOUNT_BYTE);
					pEntry->BlockNumber.Byte[nBlock] = (unsigned char) nFreeBlock;
				}
				else
				{
					assert (nFreeBlock <= 65535);
					assert (nBlock < BLOCKCOUNT_WORD);
					pEntry->BlockNumber.Word[nBlock] = (unsigned short) nFreeBlock;
				}

				// if this is the last block of the file: write the remaining bytes only

				unsigned nBytes =   nBytesLeft >= DiskParam.nBlockSize
						  ? DiskParam.nBlockSize
						  : nBytesLeft;

				if (fread (pBlockBuffer, 1, nBytes, pInFile) != nBytes)
				{
					fprintf (stderr, "%s: Read failed: %s\n", pArg0, pFileName);

					free (pBlockBuffer);
					free (pBlockMap);
					free (pDirectory);
					fclose (pInFile);
					fclose (pInOutFile);

					return 1;
				}

				// pad the last block with ^Z (EOF sign in CP/M text files)

				unsigned nPadBytes = DiskParam.nBlockSize - nBytes;
				if (nPadBytes > 0)
				{
					memset (pBlockBuffer + nBytes, 0x1A, nPadBytes);
				}

				// seek to this new data block in disk file

				assert (nFreeBlock > 0);
				assert (nFreeBlock < DiskParam.nTotalBlocks);
				unsigned nBlockOffset =   nFreeBlock * DiskParam.nBlockSize
							+ DiskParam.nReservedBytes;

				if (fseek (pInOutFile, nBlockOffset, SEEK_SET) != 0)
				{
					fprintf (stderr, "%s: Seeking to block %u failed\n", pArg0, nFreeBlock);

					free (pBlockBuffer);
					free (pBlockMap);
					free (pDirectory);
					fclose (pInFile);
					fclose (pInOutFile);

					return 1;
				}

				// write the data sectors of maximum one data block
				// 	(or less if this is the last block)

				unsigned nSectors = (nBytesLeft + SECTOR_SIZE-1) / SECTOR_SIZE;
				nSectors =   nSectors >= DiskParam.nSectorsPerBlock
					   ? DiskParam.nSectorsPerBlock
					   : nSectors;

				assert (pBlockBuffer != 0);
				if (fwrite (pBlockBuffer, SECTOR_SIZE, nSectors, pInOutFile) != nSectors)
				{
					fprintf (stderr, "%s: Writing block %u failed\n", pArg0, nFreeBlock);

					free (pBlockBuffer);
					free (pBlockMap);
					free (pDirectory);
					fclose (pInFile);
					fclose (pInOutFile);

					return 1;
				}
			}
		}

		fclose (pInFile);
	}

	free (pBlockBuffer);

	free (pBlockMap);

	// seek to directory (starts after reserved tracks)

	if (fseek (pInOutFile, DiskParam.nReservedBytes, SEEK_SET) != 0)
	{
		fprintf (stderr, "%s: Seeking directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInOutFile);

		return 1;
	}

	// write directory buffer back to disk file

	if (fwrite (pDirectory, sizeof (TDirectoryEntry), DiskParam.nDirectoryEntries, pInOutFile) != DiskParam.nDirectoryEntries)
	{
		fprintf (stderr, "%s: Writing directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInOutFile);

		return 1;
	}

	free (pDirectory);

	fclose (pInOutFile);

	return 0;
}

static int DoDelete (int nArgC, char **ppArgV, const char *pArg0)
{
	if (nArgC == 0)
	{
		fprintf (stderr, "%s: Missing filename\n", pArg0);

		return 1;
	}

	// open disk file

	FILE *pInOutFile = fopen (ToolParam.pDiskFileName, "r+");
	if (pInOutFile == NULL)
	{
		fprintf (stderr, "%s: Cannot open: %s\n", pArg0, ToolParam.pDiskFileName);

		return 1;
	}

	// seek to directory (starts after reserved tracks)

	if (fseek (pInOutFile, DiskParam.nReservedBytes, SEEK_SET) != 0)
	{
		fprintf (stderr, "%s: Seeking directory failed\n", pArg0);

		fclose (pInOutFile);

		return 1;
	}

	// read entire directory into pDirectory buffer

	TDirectoryEntry *pDirectory = (TDirectoryEntry *) malloc (DiskParam.nDirectoryEntries * sizeof (TDirectoryEntry));
	assert (pDirectory != 0);

	if (fread (pDirectory, sizeof (TDirectoryEntry), DiskParam.nDirectoryEntries, pInOutFile) != DiskParam.nDirectoryEntries)
	{
		fprintf (stderr, "%s: Reading directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInOutFile);

		return 1;
	}

	while (nArgC-- > 0)		// for all file names on the command line
	{
		const char *pFileName = *ppArgV++;
		assert (pFileName != 0);

		char CPMFileName[FILENAME_LEN+EXTENSION_LEN];
		if (!ConvertFileName (pFileName, CPMFileName))
		{
			fprintf (stderr, "%s: Invalid filename: %s\n", pArg0, pFileName);

			free (pDirectory);
			fclose (pInOutFile);

			return 1;
		}

		// for all directory entries

		int bFound = 0;

		unsigned nEntry;
		for (nEntry = 0; nEntry < DiskParam.nDirectoryEntries; nEntry++)
		{
			TDirectoryEntry *pEntry = pDirectory + nEntry;

			// if this entry belongs to this user and file name: mark it as free

			if (   pEntry->UserNumber == ToolParam.nUserNumber
			    && memcmp (pEntry->FileName, CPMFileName, sizeof CPMFileName) == 0)
			{
				pEntry->UserNumber = FORMAT_BYTE;

				bFound = 1;
			}
		}

		if (!bFound)
		{
			fprintf (stderr, "%s: File not found: %s\n", pArg0, pFileName);

			free (pDirectory);
			fclose (pInOutFile);

			return 1;
		}
	}

	// seek to directory (starts after reserved tracks)

	if (fseek (pInOutFile, DiskParam.nReservedBytes, SEEK_SET) != 0)
	{
		fprintf (stderr, "%s: Seeking directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInOutFile);

		return 1;
	}

	// write directory buffer back to disk file

	if (fwrite (pDirectory, sizeof (TDirectoryEntry), DiskParam.nDirectoryEntries, pInOutFile) != DiskParam.nDirectoryEntries)
	{
		fprintf (stderr, "%s: Writing directory failed\n", pArg0);

		free (pDirectory);
		fclose (pInOutFile);

		return 1;
	}

	free (pDirectory);

	fclose (pInOutFile);

	return 0;
}

int main (int nArgC, char **ppArgV)
{
	assert (nArgC > 0);
	const char *pArg0 = *ppArgV++;
	nArgC--;

	if (nArgC == 0)
	{
		fprintf (stderr, Usage);

		return 1;
	}

	// parse command line

	const char *pCmd = *ppArgV++;
	nArgC--;

	if (*pCmd == '-')
	{
		fprintf (stderr, "%s: Command expected: %s\n", pArg0, pCmd);

		return 1;
	}

	while (nArgC > 0 && **ppArgV == '-')
	{
		const char *pOption = *ppArgV++;
		nArgC--;

		if (pOption[1] == '\0' || pOption[2] != '\0')
		{
			fprintf (stderr, "%s: Invalid option: %s\n", pArg0, pOption);

			return 1;
		}

		switch (pOption[1])
		{
		case 'f':
			if (nArgC == 0 || **ppArgV == '-')
			{
				fprintf (stderr, "%s: Option requires parameter: %s\n", pArg0, pOption);

				return 1;
			}

			ToolParam.pDiskFileName = *ppArgV++;
			nArgC--;
			break;

		case 't':
			DiskParam.nTracks = GetOptionNumber (nArgC--, ppArgV++, 20, 160, pArg0, pOption);
			break;

		case 'r':
			DiskParam.nReservedTracks = GetOptionNumber (nArgC--, ppArgV++, 0, 20, pArg0, pOption);
			break;

		case 's':
			DiskParam.nSectorsPerTrack = GetOptionNumber (nArgC--, ppArgV++, 8, 160, pArg0, pOption);
			break;

		case 'b':
			DiskParam.nBlockSize = 1024 * GetOptionNumber (nArgC--, ppArgV++, 1, 16, pArg0, pOption);
			break;

		case 'e':
			DiskParam.nExtendSize = 1024 * GetOptionNumber (nArgC--, ppArgV++, 8, 256, pArg0, pOption);
			break;

		case 'd':
			DiskParam.nDirectoryEntries = GetOptionNumber (nArgC--, ppArgV++, 32, 2048, pArg0, pOption);
			break;

		case 'u':
			ToolParam.nUserNumber = GetOptionNumber (nArgC--, ppArgV++, 0, 15, pArg0, pOption);
			break;

		default:
			fprintf (stderr, "%s: Invalid option: %s\n", pArg0, pOption);
			return 1;
		}
	}

	// calculate disk parameters

	DiskParam.nReservedBytes    = DiskParam.nReservedTracks * DiskParam.nSectorsPerTrack * SECTOR_SIZE;

	DiskParam.nTotalSectors     = DiskParam.nTracks * DiskParam.nSectorsPerTrack;

	DiskParam.nTotalBlocks      =   (DiskParam.nTracks - DiskParam.nReservedTracks)
				      * DiskParam.nSectorsPerTrack * SECTOR_SIZE
				      / DiskParam.nBlockSize;

	DiskParam.nSectorsPerBlock  = DiskParam.nBlockSize / SECTOR_SIZE;

	DiskParam.nSectorsPerExtend = DiskParam.nExtendSize / SECTOR_SIZE;

	DiskParam.nBlocksPerExtend  = DiskParam.nExtendSize / DiskParam.nBlockSize;

	DiskParam.nDirectoryBlocks  =   (  DiskParam.nDirectoryEntries * sizeof (TDirectoryEntry)
					 + DiskParam.nBlockSize-1)
				      / DiskParam.nBlockSize;

	// run commands

	int nResult = 0;

	if (strcmp (pCmd, "init") == 0)
	{
		nResult = DoInit (nArgC, ppArgV, pArg0);
	}
	else if (strcmp (pCmd, "dir") == 0)
	{
		nResult = DoDir (nArgC, ppArgV, pArg0);
	}
	else if (strcmp (pCmd, "read") == 0)
	{
		nResult = DoRead (nArgC, ppArgV, pArg0);
	}
	else if (strcmp (pCmd, "write") == 0)
	{
		nResult = DoWrite (nArgC, ppArgV, pArg0);
	}
	else if (strcmp (pCmd, "delete") == 0)
	{
		nResult = DoDelete (nArgC, ppArgV, pArg0);
	}
	else
	{
		fprintf (stderr, Usage);

		nResult = 1;
	}

	return nResult;
}
