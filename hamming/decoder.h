#include <stdio.h>
#include <malloc.h>
#include <math.h>


//--DECLARATION BLOCK-----------------------------------------------------------
void read_and_fill();
void char_to_charbits(char* ch, char* chbits, unsigned int* counter);
void calculate_and_fix();
void charbits_to_file();

const unsigned int
    N = 15,
    K = 11,
    R = 4;
const unsigned int headerSize = 16; // Integer size, points to the amount of drop-off bits

FILE* encodedFile;
FILE* decodedFile;

long encSize = 0;
long decSize = 0;

unsigned int dropOffCharSize; // Amount of bits we need to drop off when reading file
unsigned int dropOff = 0; // Amount of bits we need to drop off when reading file

char* encBitset;
char* decBitset;
//char* encSize; // Control bits that are brought with the message
char* calcControlSet; // Calculated control bits from the message

//----MAIN---------------------------------------------------------------------
int main()
{
    read_and_fill();
    calculate_and_fix();
    charbits_to_file();
    return 0;
}

//-----------------------------------------------------------------------------
void read_and_fill()
{
    unsigned int counter = 0;
    char chBuff;

    if (!(encodedFile = fopen("encoded.txt", "rb")))
    {
        printf("\n!!! We encouneted a problem while reading file: no file with ");
        printf("such name or we have no rights to read. Sorry");
    } else
    {
        fseek(encodedFile, 0, SEEK_END);
        encSize = ftell(encodedFile) * 8;
        fseek(encodedFile, 0, SEEK_SET);

        // Drop-off useless chars
        fread(&chBuff, 1, 1, encodedFile);
        dropOff = chBuff;
        encSize = encSize - 8 - dropOff;

        encBitset = malloc(encSize);

        printf("\n\n| * Encoded data character representation: \n");
        while(fread(&chBuff, 1, 1, encodedFile) == 1 && counter < encSize)
        {
            // Converting into a char array of bits
            char_to_charbits(&chBuff, encBitset, &counter);
            printf("%c", chBuff);
        }

        printf("\n\n| * Encoded data binary representation: ");
        for (int i = 0, breakCount = 0; i < encSize; i++, breakCount++)
        {
            if (breakCount == 11) { printf(" "); breakCount = 0; }
            printf("%c", encBitset[i]);
        }

        fseek(encodedFile, 1, SEEK_SET);
        fread(&dropOffCharSize, 1, 1, encodedFile);

        fclose(encodedFile);
    }
}

//-----------------------------------------------------------------------------
/* ___ CONVERT CHAR to 8bit chars of zeroes and ones ___
 * @ch - char to convert
 * @chbits - end array which will contain converted bits
 * @counter - needed to set right position in the end array
 */
void char_to_charbits(char* ch, char* chbits, unsigned int* counter)
{
    for (int i = 7; i >= 0; i--)
    {
        chbits[*counter] = ((*ch >> i) & 0x01) ? '1' : '0';
        (*counter)++;
    }
}

//-----------------------------------------------------------------------------
void calculate_and_fix()
{
    decSize = encSize - 8 - dropOffCharSize;
    decBitset = malloc(decSize);

    char isPair;
    unsigned int
        countDown = 0,
        hasErr = 0,
        errPos = 0,
        blocksCount = decSize / N,
        curBlock = 0,
        controlPos = 0, // Position of control bit
        blockPos = 0, // Points to the first bit's position in the current block
        curPow = 0, // Cycles through all available powers of two (0,1,2,3..)
        maxPow = R - 1, // The power of 2 begins from 0, so: 0 (1), 1 (2), 2 (4), 3 (8)
        curPowPos = 0, // Contains value of current powers of 2 (1,2,4,8..)
        bitPos = 0, // Points to the current bit position
        evenCount = 0; // Count amount of ones in controled block

    for (int i = 1, counter = 1, encBitPos = 8, decBitPos = 0;
         counter <= decSize; counter++, i++)
    {
        if (i != 1
            && i != 2
            && i != 4
            && i != 8)
        {
            decBitset[decBitPos] = encBitset[encBitPos];
            decBitPos++;
        }
        encBitPos++;
        if (i == 15) { i = 0; }
    }

    while (curBlock < blocksCount)
    {
        blockPos = curBlock * N + 8;
        curPowPos = pow(2, curPow); // Control bit position for current power
        controlPos = blockPos + curPowPos - 1;
        evenCount = 0;
        bitPos = curBlock * K + curPowPos - 1;


        for (int i = 0, counter = 1; counter < K; i++, counter++)
        {
            //printf("\nBITCO: %c\n", decBitset[i]);
            if (i == curPowPos) // Jump through control positions
            {
                i = 0;
                bitPos += curPowPos;
                counter += curPowPos;
            }

            if (decBitset[bitPos] == '1') { evenCount++; }
            bitPos++;
        }
        isPair = (evenCount % 2 == 0) ? '0' : '1';

        if (isPair != encBitset[controlPos]) { hasErr = 1; errPos += curPowPos; }

        // Reset power count and go to the next block
        if (curPow == maxPow)
        {
            // errPos -= 1;
            curPow = -1;
            curBlock++;

            if (hasErr)
            { decBitset[errPos] = (decBitset[errPos] == '1') ? '0' : '1'; }
            hasErr = 0;
            errPos = 0;
        }

        curPow++;
    }


    printf("\n\n\n\n");
    printf("Decoded data: \n");
    for (int i = 0, breakCount = 1; i < decSize -1; i++, breakCount++)
    {
        printf("%c", decBitset[i]);
        if(breakCount == 11) { printf(" "); breakCount = 0; }
    }
}
void charbits_to_file()
{
    char ch = 0x00;
    decodedFile = fopen("decoded.txt", "wb");

    printf("\n\nDecoded data char. representation: \n");
    for (int i = 0, bitPos = 7; i < decSize; i++, bitPos--)
    {
        ch |= ((decBitset[i] == '1') ? 0xFF & (0x01 << bitPos) : 0x00);

        if (bitPos == 0)
        {
            printf("%c", ch);
            fwrite(&ch, 1, 1, encodedFile);
            bitPos = 8;
            ch = 0x00;
        }
    }

    fclose(decodedFile);
}
