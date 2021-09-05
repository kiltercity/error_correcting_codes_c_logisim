#include <stdio.h>
#include <malloc.h>

void read_file(); // Read encoded file into an array
void char_to_charbits(char* ch, char* arr, int* counter);
void decode();
void write_to_file();
uint16_t crc_check(uint16_t n);
void print_11(char txt[], uint16_t n);
uint16_t w_calc(uint16_t n);
uint16_t rem_check(uint16_t remainder);
 void print_15(char txt[], uint16_t n);
long
	encArraySize, // Size of encoded array of bits
	decArraySize; // Size of decoded array of bits

int headerSize = 16;

char inputFileName[] = "encoded.txt";
char outputFileName[] = "decoded.txt";

char* encodedArray; // Will contain encoded bits from file
char* decodedArray; // Will contain decoded data

int P = 0x19, // 11001 - Polynominal
	PP = 4, // The power of polynominal
	N = 11, // Information part sise
	NN = 15, // Encoded combination length
	R = 4; // Amount of control bits

//------------------------------------------------------------------------------

int main()
{
	read_file();
	decode();
	
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
uint16_t crc_check(uint16_t n)
{
	uint16_t remainder = (uint16_t) n; // Cyclic redundancy check (check bits)
	int pos_count = 0;
	remainder = rem_check(remainder);
	
	
	if (remainder != 0)
	{
		while (w_calc(remainder) > 1)
		{
			
			n = (n << 1) | (n >> (NN - 1));
			pos_count++;
			remainder = rem_check(n);
		}
		n ^= remainder;
		n = (n >> pos_count) | (n << (NN - pos_count));
	}
	n >>= 4;
	
	return n;
}
uint16_t rem_check(uint16_t remainder)
{
	uint16_t poly = (uint16_t) 0x6400;

	for (int i = NN; i > 0; i--)
	{	
		if (remainder & 0x4000)
		{
			remainder ^= poly;
		}
		remainder <<= 1;
	}
	return remainder;
}
uint16_t w_calc(uint16_t n)
{
	int w = 0;
	for (int i = 0; i < NN; i++)
	{
		if ((0x01 << i) & n)
		{
			w++;
		}
	}
	return w;
}
void print_11(char txt[], uint16_t n)
{

	printf("\t%s: \n\t", txt);

	for (int i = 0; i < N; i++)
	{
		printf("%c", (0x01 << i) & n ? '1' : '0');
	}
	printf("\n\n");
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

void decode()
{
	int
		encPos = 0,
		decPos = 0,
		tmp_pos = 0,
		curBlock = 0;

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
	uint16_t temp = 0;

	decArraySize = blocksCount * N;
	decodedArray = (char*) malloc(decArraySize);

	
	for (int i = headerSize, pos = 0; i < encArraySize; i++, pos++)
	{
		temp |= ((encodedArray[i] == '1') ? 0x01 : 0x00) << pos;
		
		if (pos == NN - 1)
		{
			printf("\n\n");
			temp = crc_check(temp);
			print_11("CC: ", temp);
			pos = -1;
			temp=0;
		}
	}
}