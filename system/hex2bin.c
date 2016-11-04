//
// hex2bin.c
//
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_ADDRESS		0xFFFF

#define MAX_LINE		1000	// max length of input line

#define OUTPUT_RECORD_SIZE	128	// output will be padded to be a multiple of this size

int main (int nArgC, char **ppArgV)
{
	assert (nArgC > 0);
	const char *pArg0 = *ppArgV++;
	nArgC--;

	if (nArgC != 2)
	{
		fprintf (stderr, "Usage: %s hexfile binfile\n", pArg0);

		return 1;
	}

	const char *pHexFileName = *ppArgV++;
	const char *pBinFileName = *ppArgV;

	FILE *pInFile = fopen (pHexFileName, "r");
	if (pInFile == NULL)
	{
		fprintf (stderr, "%s: File not found: %s\n", pArg0, pHexFileName);

		return 1;
	}

	unsigned char Memory[MAX_ADDRESS+1];
	memset (Memory, 0, sizeof Memory);

	unsigned nLowestAddress = MAX_ADDRESS;
	unsigned nHighestAddress = 0;

	unsigned nLine = 0;

	char Buffer[MAX_LINE+1];
	while (fgets (Buffer, sizeof Buffer, pInFile) != NULL)
	{
		nLine++;

		if (Buffer[0] == '\x1a')	// ^Z is EOF
		{
			break;
		}

		unsigned nLength;
		unsigned nAddress;
		unsigned nType;
		if (sscanf (Buffer, ":%2X%4X%2X", &nLength, &nAddress, &nType) != 3)
		{
			fprintf (stderr, "%s: Invalid record header in line %u\n", pArg0, nLine);

			fclose (pInFile);

			return 1;
		}

		if (nType == 1)			// EOF record
		{
			break;
		}

		if (nType != 0)			// Data record
		{
			fprintf (stderr, "%s: Invalid record type in line %u\n", pArg0, nLine);

			fclose (pInFile);

			return 1;
		}

		if (   nLength > 0
		    && nAddress + nLength-1 < nAddress)
		{
			fprintf (stderr, "%s: Address overflow in line %u\n", pArg0, nLine);

			fclose (pInFile);

			return 1;
		}

		unsigned char nCheckSumAccu = nLength + (nAddress & 0xFF) + (nAddress >> 8) + nType;

		const char *pData = Buffer + 9;
		unsigned i;
		for (i = 0; i < nLength; i++, pData += 2)
		{
			unsigned nByte;
			sscanf (pData, "%2X", &nByte);

			if (nAddress < nLowestAddress)
			{
				nLowestAddress = nAddress;
			}

			if (nAddress > nHighestAddress)
			{
				nHighestAddress = nAddress;
			}

			Memory[nAddress++] = (unsigned char) nByte;

			nCheckSumAccu += nByte;
		}

		nCheckSumAccu ^= 0xFF;		// two's complement
		nCheckSumAccu++;

		unsigned nCheckSum;
		sscanf (pData, "%2X", &nCheckSum);
		pData += 2;

		if (nCheckSumAccu != nCheckSum)
		{
			fprintf (stderr, "%s: Invalid checksum in line %u\n", pArg0, nLine);

			fclose (pInFile);

			return 1;
		}

		if (   !isspace (*pData)
		    && *pData != '\0')
		{
			fprintf (stderr, "%s: Unexpected character on line %u: %02X\n", pArg0, nLine, *pData);

			fclose (pInFile);

			return 1;
		}
	}

	unsigned nPadding = OUTPUT_RECORD_SIZE - nHighestAddress % OUTPUT_RECORD_SIZE - 1;
	nHighestAddress += nPadding;

	printf ("%04X-%04X\n", nLowestAddress, nHighestAddress);

	FILE *pOutFile = fopen (pBinFileName, "w");
	if (pOutFile == NULL)
	{
		fprintf (stderr, "%s: Cannot create: %s\n", pArg0, pBinFileName);

		fclose (pInFile);

		return 1;
	}

	assert (nLowestAddress < nHighestAddress);
	unsigned nAddress;
	for (nAddress = nLowestAddress; nAddress <= nHighestAddress; nAddress++)
	{
		if (fputc (Memory[nAddress], pOutFile) == EOF)
		{
			fprintf (stderr, "%s: Write error\n", pArg0);

			fclose (pOutFile);
			fclose (pInFile);

			return 1;
		}
	}

	fclose (pOutFile);

	fclose (pInFile);

	return 0;
}
