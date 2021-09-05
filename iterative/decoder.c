#include <stdio.h>
#include <malloc.h>

void read_file(); // Read encoded file into an array
void char_to_charbits(char* ch, char* arr, int* counter);
void decode();
void write_to_file();

long
	encArraySize, // Size of encoded array of bits
	decArraySize; // Size of decoded array of bits

int headerSize = 16;

char inputFileName[] = "encoded.txt";
char outputFileName[] = "decoded.txt";

char* encodedArray; // Will contain encoded bits from file
char* decodedArray; // Will contain decoded data



//------------------------------------------------------------------------------

int main()
{
	read_file();
	decode();
	write_to_file();
	getchar();
	return 0;
}

//------------------------------------------------------------------------------

void read_file()
{
	FILE* encodedFile = fopen(inputFileName, "rb");
	char ch;
	int counter = 0;

	// Get encoded file size and allocating memory for bits arrray
	fseek(encodedFile, 0, SEEK_END);
	encArraySize = ftell(encodedFile) * 8;
	encodedArray = (char*) malloc(encArraySize);
	fseek(encodedFile, 0, SEEK_SET);

	// Reading file into an array
	while (fread(&ch, 1, 1, encodedFile) == 1)
	{
		char_to_charbits(&ch, encodedArray, &counter);
	}
}

//------------------------------------------------------------------------------

void char_to_charbits(char* ch, char* arr, int* counter)
{
	for (int i = 7; i > -1; i--)
	{
		arr[(*counter)] = ((*ch >> i) & 0x01) ? '1' : '0';
		(*counter)++;
	}
}

//------------------------------------------------------------------------------

void decode()
{
	int
		I = 6,
		J = 3,
		N = 18,
		II = 7,
		JJ = 4,
		NN = II * JJ,
		encPos = 0,
		decPos = 0,
		tmp_pos = 0,
		curBlock = 0;
	char
		isRowPair[3] = { '0', '0', '0'},
		isColPair[7] = { '0', '0', '0', '0', '0', '0', '0' },
		isRowPairCheck[3] = { '0', '0', '0' },
		isColPairCheck[7];

	int dropOffAmount = 0, originalDataCutOffSize = 0;


	for (int i = 7; i >= 0; i--, encPos++)
	{
		dropOffAmount |= ((encodedArray[encPos] == '1') ? 0x01 : 0x00) << i;
	}
	for (int i = 7; i >= 0; i--, encPos++)
	{
		originalDataCutOffSize |= ((encodedArray[encPos] == '1') ? 0x01 : 0x00) << i;
	}
	printf("-----------------------------------------------------------\n" );
	printf("| DROP-OFF AMOUNT: %d\n", dropOffAmount);
	printf("-----------------------------------------------------------\n" );
	printf("-----------------------------------------------------------\n" );
	printf("| ORIGINAL DATA CUT-OFF SIZE: %d\n", originalDataCutOffSize);
	printf("-----------------------------------------------------------\n" );


	printf("| ENCODED DATA: \n|\n|\t");
	for (int i = 0; i < encArraySize; i++)
	{
		if (i % 8 == 0) { printf(" "); }
		printf("%c", encodedArray[i]);
	}
	printf("\n|\n-----------------------------------------------------------\n");

	// Cut-off redundant values from encoded array
	if (dropOffAmount != 0)
	{
		encArraySize -= dropOffAmount;
		encodedArray = (char*) realloc(encodedArray, encArraySize);
	}

	int blocksCount = (encArraySize - headerSize) / NN;
	decArraySize = blocksCount * N;
	decodedArray = (char*) malloc(decArraySize);

	// Decoding
	for (int i = 1, r = 0, c = 0, counter = encPos
		; counter < encArraySize
		; i++, encPos++, decPos++, counter++, c++
	){
		if (i % 7 == 0)
		{
			isRowPairCheck[r] = encodedArray[encPos];

			if (i % 21 == 0)
			{	
				for (int a = 0; a < I; a++)
				{
					counter++;
					encPos++;
					if (encodedArray[encPos] != isColPair[a])
					{
						for (int b = 0; b < J; b++)
						{
							if (isRowPair[b] != isRowPairCheck[b])
							{
								tmp_pos = curBlock * N + (a + b * I);
								printf("\n| CHANGED BIT: %d\n", tmp_pos + 1);
								decodedArray[tmp_pos]
									= (decodedArray[tmp_pos] == '1') ? '0' : '1';
							}
						}
					}
					isColPair[a] = '0';
				}
				printf("\n");
				encPos++;
				counter++; 
				curBlock++;
				i=0;
				r = -1;
				isRowPair[0] = '0';
				isRowPair[1] = '0';
				isRowPair[2] = '0';
			}
			r++;
			c = -1;
			decPos--;
			continue;
		}
		decodedArray[decPos] = encodedArray[encPos];
		
		if (decodedArray[decPos] == '1')
		{
		 	isRowPair[r] = (isRowPair[r] == '1') ? '0' : '1';
			isColPair[c] = (isColPair[c] == '1') ? '0' : '1';
		}
	}
	
	decArraySize = decArraySize - originalDataCutOffSize;

	printf("| DECODED DATA: \n|\n|\t");

	for (int i = 0; i < decArraySize; i++)
	{
		if (i % 8 == 0) { printf(" "); }
		printf("%c", decodedArray[i]);
	}
	printf("\n|\n-----------------------------------------------------------\n");
}

//------------------------------------------------------------------------------

void write_to_file()
{
	FILE* decodedFile = fopen(outputFileName, "wb");
	char ch = 0x00;

	for (int c = 7, i = 0; i < decArraySize; i++, c--)
	{
		ch |= ((decodedArray[i] == '1') ? 0xFF & (0x01 << c) : 0x00);
		if (c == 0)
		{
			fwrite(&ch, 1, 1, decodedFile);
			c = 8;
			ch = 0x00;
		}
	}
	fclose(decodedFile);
}
