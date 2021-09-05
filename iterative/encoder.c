#include <stdio.h>
#include <malloc.h>

void read_file();
void char_to_charbits(char* ch, char* arr, int* pos);
void encode();
void printArr(char* arr, long size);
void write_to_file();

long
	arraySize, // Amount of information bits
	encodedArraySize; // Encoded amount of bits

// This header is inserted at the beginning of the encoded data
// and points to what amount of bits to be dropped-off
int headerSize = 16;


char inputFileName[] = "input.txt";
char outputFileName[] = "encoded.txt";

// Amount of zeroes to cut when reading and decoding file
char cutOffBitSize;
char originalDataCutOffSize;

char* dataArray; // Will contain data bits
char* encodedArray; // Will contain encoded data bits

//------------------------------------------------------------------------------

int main()
{
	read_file();
	encode();
	write_to_file();

	getchar();
	return 0;
}

//------------------------------------------------------------------------------

void read_file()
{
	FILE* inputFile = (FILE*) fopen(inputFileName, "rb");
	int bitPos = 0; // Keeps the current bit position that is being written to array
	char buff; // Contains the read char from file

	// Get file size and alloc. memory for array
	fseek(inputFile, 0, SEEK_END);
	arraySize = ftell(inputFile) * 8;
	fseek(inputFile, 0, SEEK_SET);
	dataArray = (char*) malloc(arraySize);

	// printf("%d", arraySize);

	while(fread(&buff, 1, 1, inputFile) == 1)
	{
		char_to_charbits(&buff, dataArray, &bitPos);
	}

	// Print the bits
	printf("INPUT DATA: ");
	printArr(dataArray, arraySize);
}

//------------------------------------------------------------------------------

void char_to_charbits(char* ch, char* arr, int* pos)
{
	for (int i = 7; i > -1; i--)
	{
		// Reading char bit-by-bit and saving it into array
	 	arr[(*pos)] = (((*ch >> i) & 0x1) == 0x1) ? '1' : '0';
		(*pos)++;
	}
}

//------------------------------------------------------------------------------

void encode()
{
	int
		I = 6,
		J = 3,
		N = I * J,
		II = 7,
		JJ = 4,
		NN = II * JJ,
		blocksCount = 0,
		appendPos = 0,
		encPos = 0,
		appendSize = 0;
	char
		isRowPair = '0', // Pairity of row
		isColPair[7] = {'0','0','0','0','0','0','0'}, // Pairity of column
		isControlPair = '0'; // Pairity of control bits

	// Append zeroes to reach full combination length
	while (arraySize % N != 0) { arraySize++; appendSize++; }

	dataArray = (char*) realloc(dataArray, arraySize);
	appendPos = arraySize - appendSize - 1;

	for (int i = arraySize; i > appendPos; i--) { dataArray[i] = '0'; }
	blocksCount = arraySize / N;
	printf("COMPLETE DATA ARRAY LOOK (WITH APPENDED BITS): ");
	printArr(dataArray, arraySize);

	originalDataCutOffSize = appendSize;


	appendSize = 0;
	// Append zeroes to reach equality to 8-bit (byte)
	encodedArraySize = (arraySize / N * II * JJ);
	encodedArraySize += headerSize;
	while (encodedArraySize % 8 != 0) { encodedArraySize++; appendSize++; }
	cutOffBitSize = appendSize;

	appendPos = encodedArraySize - appendSize - 1;
	encodedArray = (char*) malloc(encodedArraySize);
	for (int i = encodedArraySize; i > appendPos; i--) { encodedArray[i] = '0'; }


	for (int i = 7; i >= 0; i--, encPos++)
	{
		encodedArray[encPos] = (((cutOffBitSize >> i) & 0xFF) == 0x01) ? '1' : '0';
	}
	// Inserting header sequence
	for (int i = 7; i >= 0; i--, encPos++)
	{
		encodedArray[encPos] = (((originalDataCutOffSize >> i) & 0xFF) == 0x01) ? '1' : '0';
	}

	int amount = NN * blocksCount;

	for (int i = 0, j = 0, counter = 0; counter < amount; i++, encPos++, j++, counter++)
	{
		// Write information bits to the encoded array
		encodedArray[encPos] = dataArray[i];

		// If info. bit equals to 1, change row and column pairity
		if (dataArray[i] == '1')
		{
			isRowPair = (isRowPair == '1') ? '0' : '1';
			isColPair[j] = (isColPair[j] == '1') ? '0' : '1';
		}
		// When the row is passed write the pairity bit to the encoded array
		if ((i + 1) % I == 0 && i != 0)
		{
			encodedArray[++encPos] = isRowPair;
			j++;
			// Change pairity of control column
			if (isRowPair == '1')
			{ isColPair[j] = (isColPair[j] == '1') ? '0' : '1'; }
			isRowPair = '0';
		}
		// Reset control row counter
		if (j == 6) { j = -1; }

		// Fill encoded array with control row
		if ((i + 1) % N == 0 && i != 0)
		{
			char isPair = '0';
			for (int a = 0; a < II; a++)
			{
				encodedArray[++encPos] = isColPair[a];
				if (isColPair[a] == '1')
				{ isPair = (isPair == '1') ? '0' : '1'; }
				isColPair[a] = '0';
			}
			encodedArray[encPos] = isPair;
		}
	}

	printf("ENCODED ARRAY LOOK: \n");
	printArr(encodedArray, encodedArraySize);
}

//------------------------------------------------------------------------------

void printArr(char* arr, long size)
{
	printf("\n");
	printf("|\n");
	printf("|\t");
	for (int i = 0; i < size; i++)
	{
		if (i % 8 == 0 && i != 0) { printf(" "); }
		printf("%c",  arr[i]);
	}
	printf("\n");
	printf("|\n");
	printf("| SIZE: %d\n", size);
	printf("-----------------------------------------------------------------");
	printf("\n");
}

//------------------------------------------------------------------------------

void write_to_file()
{
	FILE* encodedFile = fopen(outputFileName, "wb");
	char ch = 0x00;

	for (int c = 7, i = 0; i < encodedArraySize; i++, c--)
	{
		ch |= ((encodedArray[i] == '1') ? 0xFF & (0x01 << c) : 0x00);
		if (c == 0)
		{
			fwrite(&ch, 1, 1, encodedFile);
			c = 8;
			ch = 0x00;
		}
	}
	fclose(encodedFile);
}
