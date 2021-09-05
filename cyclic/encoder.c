#include <stdio.h>
#include <malloc.h>

void read_file();
void char_to_charbits(char* ch, char* arr, int* pos);
uint16_t crc_append(uint16_t n);
void encode();
void printArr(char* arr, long size);
void write_to_file();
void print_15(char txt[], uint16_t n);

long 
	arraySize, // Amount of information bits
	encodedArraySize; // Encoded amount of bits

// This header is inserted at the beginning of the encoded data
// and points to what amount of bits to be dropped-off
int headerSize = 16;
int P = 0x19, // 11001 - Polynominal
	PP = 4, // The power of polynominal
	N = 11, // Information part sise
	NN = 15, // Encoded combination length
	R = 4; // Amount of control bits

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
/*
 * Calculates the code Cyclic Redundancy Check bits
 * @n {uint16_t} - Data that is aimed to be encoded
 * @pp {uint16_t} - the polynominal generator (5 leftmost bits)
 * */
uint16_t crc_append(uint16_t n)
{
	uint16_t poly = (uint16_t) 0x6400;
	uint16_t data = (uint16_t) n << 4;
	uint16_t remainder = (uint16_t) data; // Cyclic redundancy check (check bits)

	int amount = 0;
	
	for (int i = N; i > 0; i--)
	{	
		if (remainder & 0x4000)
		{
			remainder ^= poly;
		}
		remainder <<= 1;
	}
	remainder >>= 11;
	data += remainder;


	print_15("BEFORE", n);
	print_15("AFTER", data);

	return data;
}
void print_15(char txt[], uint16_t n)
{

	printf("\t%s: \n\t", txt);

	for (int i = 0; i < NN; i++)
	{
		printf("%c", (0x01 << i) & n ? '1' : '0');
	}
	printf("\n\n");
}
//------------------------------------------------------------------------------

void encode()
{
	int
		blocksCount = 0,
		appendPos = 0,
		encPos = 0,
		appendSize = 0;

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
	encodedArraySize = (blocksCount * NN);
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

	int amount = N * blocksCount;
	int encCounter = headerSize;
	uint16_t temp = 0;
	int curBlock = 1;


	// temp = 	0x1F; //0x5C8;
	// temp = crc_append(temp);
	
	char tmpval = '0';
	printf("\n");
	for (int i = 0, counter = 0, cc = 1; counter < amount; i++, encPos++, counter++)
	{
		temp |= ((dataArray[counter] == '1') ? 0x01 : 0x00) << i;

		if ( i == N - 1)
		{
			printf("\nCODE COMBINATION %d\n", cc);
			temp = crc_append(temp);
			for (int a = 0; a < NN; a++)
			{
				tmpval = temp & (0x01 << a) ? '1' : '0';
				encodedArray[encCounter++] = tmpval;
			}
			i = -1;
			temp = 0;
			cc++;
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
