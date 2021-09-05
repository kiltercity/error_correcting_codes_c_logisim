/* The task data is: R = 4, K = 11, n = 15
 * K - amount of information bits | Кількість інформаційних бітів (розрядів)
 * R - amount of control bits | Кількість перевірочних (контролюючих) бітів
 * N - common size of the code combination | Розмір кодової комбінації (КК)
 *
 * This program is more memory-favour
 * as it hold the bitwise data in an array of chars but not integers
 * for comparsion - integer has the sizee of 32-bits, while char only 8-bits
 */
#include <stdio.h>
#include <malloc.h>
#include <math.h>

//--DECLARATION BLOCK-----------------------------------------------------------
// Read some intput data (such as input filename)
void get_initial_data();
void read_and_fill(); // Read data from file and fill array with it
// Convert chars to bits and write it into char array
void char_to_charbits(char* ch, char* chbits, unsigned int* counter);
void insert_control_bits();
// Write the array of bits to the file
void charbits_to_file();

// Hamming coder parameters
const unsigned int
    N = 15, // End block size
    K = 11, // Amount of information bits
    R = 4; // Amount of control bits

FILE* originalFile; // original file pointer (input file)
FILE* encodedFile; // encoded file pointer

long fsize; // Original file size
long encSize; // Array size for encoded data array

/* Header contains information about
 * what amount of bits we need to drop-off when decoding
 * It contains two 8-bit integers,
 * so cannot be clearly represented as 32-bit integer */
unsigned int headerSize = 16;
unsigned int dropOffSize; // Amount of bits we need to drop off when decoding
unsigned int dropOffCharSize; // Chars to drop-off when reading encoded file

char inputFilename[51]; // Name of a file with input data
char* encFilename = "encoded.txt"; // End file name with encoded data
char* bitset = NULL; // Array for information bits
char* encBitset = NULL; // Array for encoded bits

//--MAIN-----------------------------------------------------------------------
int main(int argc, char** argv)
{
    get_initial_data();
    read_and_fill();
    insert_control_bits();
	return 0;
}

//-----------------------------------------------------------------------------
// Obtain needed initial data from user
void get_initial_data()
{
    printf("Please enter input data file name (50 symbols max): ");
    scanf("%s", inputFilename); // Get input filename from user
}

//-----------------------------------------------------------------------------
// Reading the input file and pushing into array of bits
void read_and_fill()
{
    unsigned int counter = 0; // Counts the char position in file
    char chBuff; // Char buffer

    if (!(originalFile = fopen(inputFilename, "rb")))
    {
        printf("\n!!! We encouneted a problem while reading file: no file with such name or we have no rights to read. Sorry :(\n\n");
    } else
    {
      // Getting file size and allocating memory
        fseek(originalFile, 0, SEEK_END);
        fsize = ftell(originalFile) * 8; // Global var, filesize in bits
        bitset = (char*) malloc(fsize); // Contains info bits
        fseek(originalFile, 0, SEEK_SET); // Rewind

        printf("\n\n| * Character data representation: ");
        while(fread(&chBuff, 1, 1, originalFile) == 1)
        {
            // Converting char into a char array of bits
            char_to_charbits(&chBuff, bitset, &counter);
            printf("%c", chBuff);
        }

        // this is not neccessary
        printf("\n\n| * Binary data representation: ");
        for (int i = 0, breakCount = 0; i < fsize; i++, breakCount++)
        {
            if (breakCount == 8) { printf(" "); breakCount = 0; }
            printf("%c", bitset[i]);
        }
    }

    fclose(originalFile);
}

//-----------------------------------------------------------------------------
/* CONVERT CHAR read from file to array of chars that contains 1-s and 0-s
 * @ch - char to convert
 * @chbits - end array which will contain converted bits
 * @counter - needed to set right position in the end array
 */
void char_to_charbits(char* ch, char* chbits, unsigned int* counter)
{
    for (int i = 7; i >= 0; i--)
    {
      // Reading the char bit by bit using mask
        chbits[*counter] = ((*ch >> i) & 0x01) ? '1' : '0';
        (*counter)++;
    }
}

//-----------------------------------------------------------------------------
void insert_control_bits()
{
    unsigned int
        counter = 0,
        blocksCount, // Amount of N-size blocks
        startZeroPos, // Position to start append zeroes from (see below)
        curBitPos = 0, // Bit pos. for appended array of bits
        targetBitPos = headerSize; // Bit pos. for encoded array to start write from

    // Number of bits that need to be dropped-off when decoding
    char dropOffSizeChar, dropOffCharSizeChar;

    // Counting append bits (zeroes) to match the complete block size
    while (fsize % K != 0) { fsize++; dropOffSize++; }

    // Saving number of drop-off bits into char header
    dropOffSizeChar = dropOffSize & 0xFF;
    blocksCount = fsize / K;
    startZeroPos = fsize - dropOffSize;

    encSize = fsize + (blocksCount * 4) + headerSize;

    while (encSize % 8 != 0) { encSize++; dropOffCharSize++; }

    dropOffCharSizeChar = dropOffCharSize & 0xFF;
    encBitset = malloc(encSize);
    bitset = (char*) realloc(bitset, fsize + dropOffSize);

    // Inserting header to the encoded array
    char_to_charbits(&dropOffCharSizeChar, encBitset, &counter);
    char_to_charbits(&dropOffSizeChar, encBitset, &counter);

    // Completing array with zeroes to match needed amount of blocks and bits
    for (int i = startZeroPos; i < fsize; i++) { bitset[i] = '0'; }

    // Appending zeroes to be able to write by aliquot to char amount of bits
    startZeroPos = encSize - dropOffCharSize;
    for (int i = startZeroPos; i < encSize; i++) { encBitset[i] = '0'; }

    // Writing info. bits to the encoded array
    for (int i = 1, counter = 1; counter <= fsize; counter++, i++)
    {
        if (i == 1 || i == 2 || i == 4 || i == 8)
        {
            encBitset[targetBitPos] = 'x';
            targetBitPos++;
            counter--;
        }
        else
        {
            encBitset[targetBitPos] = bitset[curBitPos];
            curBitPos++;
            targetBitPos++;
        }
        if (i == 15) { i = 0; }
    }


    unsigned int
        curBlock = 0,
        controlPos = 0, // Position of control bit
        blockPos = 0, // Points to the first bit's position in the current block
        curPow = 0, // Cycles through all available powers of two (0,1,2,3..)
        maxPow = R - 1, // The power of 2 begins from 0, so: 0 (1), 1 (2), 2 (4), 3 (8)
        curPowPos = 0, // Contains value of current powers of 2 (1,2,4,8..)
        bitPos = 0, // Points to the current bit position
        evenCount = 0; // Count amount of ones in controled block

    // Calculating control bits
    while (curBlock < blocksCount)
    {
        /* We append (-1) in the [blockPos]
         * because the minimal value of power of (2) equals to 1,
         * but the first element of the array is refferenced by 0
         */
        blockPos = curBlock * N;
        curPowPos = pow(2, curPow); // Control bit position for current power
        controlPos = blockPos + curPowPos + headerSize - 1;
        evenCount = 0;
        bitPos = curBlock * K + curPowPos - 1;

        for (int i = 0, counter = 1; counter < K; counter++, i++)
        {
            if (i == curPowPos) // Jump through control positions
            {
                i = 0;
                bitPos += curPowPos;
                counter += curPowPos;
            }
            if (bitset[bitPos] == '1') { evenCount++; }
            printf("\nBPOS: %d\n%c\n", bitPos, bitset[bitPos]);

            bitPos++;
        }
        // Setting control bit depend on pairity amount of info bits
        if (evenCount % 2 == 0) { encBitset[controlPos] = '0'; } // Even
        else { encBitset[controlPos] = '1'; } // Odd

        printf("\nCONTROL: %c\nCONPOS: %d\n", encBitset[controlPos], controlPos);
        // Reset power count and go to the next block
        if (curPow == maxPow) { curPow = -1; curBlock++; }

        curPow++;
    }


    printf("\n\n| * Encod. data representation: ");
    for (int i = headerSize, breakCount = 0; i < encSize; i++, breakCount++)
    {
        if (breakCount == N) { printf(" "); breakCount = 0; }
        printf("%c", encBitset[i]);
    }
    printf("\n\n| * Header size: %d", headerSize);
    printf("\n\n| * Header binary representation: ");
    for (int i = 0; i < headerSize; i++) { printf("%c", encBitset[i]); }

    printf("\n\n| * Drop-off size for: \n| * --> Block: %d\n| * --> Chars: %d", dropOffSize, dropOffCharSize);

    printf("\n\n| * Encoded file name: %s", encFilename);
    printf("\n\n");

    // Writing file
    charbits_to_file();

}

//-----------------------------------------------------------------------------
void charbits_to_file()
{
    char ch = 0x00;
    encodedFile = fopen(encFilename, "wb");

    for (int i = 0, bitPos = 7; i < encSize; i++, bitPos--)
    {
      // Fillig char bit-by-bit using masks
        ch |= ((encBitset[i] == '1') ? 0xFF & (0x01 << bitPos) : 0x00);

        if (bitPos == 0)
        {
            fwrite(&ch, 1, 1, encodedFile);
            bitPos = 8;
            ch = 0x00;
        }
    }

    fclose(encodedFile);
}
