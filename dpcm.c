/*	CSCI366 Assignment 3
	Name: Aaron Colin Foote
	Date: 17th May 2014
	File: dpcm.c
*/

// Standard C libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL/SDL.h>

/* Bool not defined in C, defined here to identify false = 0 and true = 1
Could have included the line:
#include <stdbool.h>
But I am not certain what standard of C UOW systems operate on */

typedef enum {false, true} bool;

struct {
	int *data;
	int dataSize;
	bool encodeMode;
	int *resultSet;
} prog;

bool checkUsage(int argc, char *argv[]);	// Returns as 1 for any misuse of command-line arguments
void allocateMemory();				// Allocate storage buffers for passed and output data
void deallocateMemory();			// Deallocate storage buffers for passed and output data
void fillData(char *argv[]);			// Store command-line arguments in storage buffer
void encode();					// Encodes all command line values, inserting them into the final result set
int quantize(int e);				// Quantize the value e and return e_tilde
void decode();					// Decodes all command line values, inserting them into the final result set
void printResults();				// Output all encoded / decoded values

// Returns as 1 for any misuse of command-line arguments
bool checkUsage(int argc, char *argv[])
{
	// User passed wrong amount of values to program
	if (argc == 2 && !(strcmp(argv[1], "-h"))) {
		// Display help messages followed by usage messages
		printf("\nUsage:\tdpcm –[h|e|d] n1 n2 n3 ...\n\n-e encoding mode\n-d decoding mode\n-h this help\n\n");
		printf("Encoding example:\t$dpcm –e 130 150 140 200 230\nEncoding output: \t130 24 -8 56 56\n\n");
		printf("Decoding example:\t$dpcm –d 130 24 -8 56\nDecoding output: \t130 154 134 200\n\n");
		return false;
	}
	// Incorrect argument count for non-help functionality
	else if (argc < 3) {
		printf("Usage:\tdpcm –[h|e|d] n1 n2 n3 ...\n");
		return false;
	}
	// Determine program purpose (encoding or decoding) and perform such actions if first argument is -e or -d
	else if (!(strcmp(argv[1], "-e"))) {
		prog.encodeMode = true;
		prog.dataSize = argc - 2;
		return true;
	}
	else if (!(strcmp(argv[1], "-d"))) {
		prog.encodeMode = false;
		prog.dataSize = argc - 2;
		return true;
	}
	else {
		printf("Usage:\tdpcm –[h|e|d] n1 n2 n3 ...\n");
		return false;
	}
}

// Allocate storage buffers for passed and output data
void allocateMemory()
{	
	prog.data = (int *)malloc(sizeof(int) * (prog.dataSize));
	prog.resultSet = (int *)malloc(sizeof(int) * (prog.dataSize));
}

// Deallocate storage buffers for passed and output data
void deallocateMemory()
{
	free(prog.data);
	free(prog.resultSet);
}

// Store command-line arguments in storage buffer
void fillData(char *argv[])
{
	int i = 0, j = 2, convertedArgument = 0;	// j moves in accordance with arguments as passed to main (since option occupies first argument)
	for (i = 0; i < prog.dataSize; i++, j++)
	{
		char *argument = argv[j];
		if (argument[0] == '-') {		// Identify negative values passed to program. Ignore '-' and then multiply by -1
			convertedArgument = atoi(++argument);
			convertedArgument = convertedArgument * -1;
		}
		else {
			convertedArgument = atoi(argument);	// Convert command-line arguments to integer values
		}

		// Prevent encoding of illegitimate values (outside of 0 to 255 range)
		if (convertedArgument > 255 && prog.encodeMode == true)
			convertedArgument = 255;
		else if (convertedArgument < 0 && prog.encodeMode == true)
			convertedArgument = 0;

		prog.data[i] = convertedArgument;
	}
}

// Encodes all command line values, inserting them into the final result set
void encode()
{
	int encodeCount = 0;
	int f_tilde_OneAgo = prog.data[encodeCount];	// Initialize f_tilde values to initial seed value
	int f_tilde_TwoAgo = prog.data[encodeCount];

	prog.resultSet[encodeCount] = prog.data[encodeCount];	// Guaranteed to be at least 1 argument, already checked

	for (encodeCount = 1; encodeCount < prog.dataSize; encodeCount++)	// Iterate through converted command line arguments
	{	
		int f_hat = ((f_tilde_OneAgo + f_tilde_TwoAgo)/2);	// Calculate f_hat (f_hat = (f(n-1) + (f(n-2)) / 2)
		int e = prog.data[encodeCount] - f_hat;			// Calculate e for current f (e = f(n) - f_hat)
		int e_tilde = quantize(e);				// Calculate e_tilde

		prog.resultSet[encodeCount] = e_tilde;			// Append e_tilde to result set

		f_tilde_TwoAgo = f_tilde_OneAgo;			// Shift f_tilde(n-2) and f_tilde(n-1) forward
		f_tilde_OneAgo = f_hat + e_tilde;
	}
}

// Quantize the value e and return e_tilde
int quantize(int e)
{
	return 16 * ((255 + e)/16) - 256 + 8;
}

// Decodes all command line values, inserting them into the final result set
void decode()
{
	int decodeCount = 0;
	int f_tilde_OneAgo = prog.data[decodeCount];	// Initialize f_tilde values to initial seed value
	int f_tilde_TwoAgo = prog.data[decodeCount];

	prog.resultSet[decodeCount] = prog.data[decodeCount];	// Guaranteed to be at least 1 argument, already checked

	for (decodeCount = 1; decodeCount < prog.dataSize; decodeCount++)	// Iterate through converted command line arguments
	{
		int e_tilde = prog.data[decodeCount];	// Get next value in given sequence
	
		// 	f_tilde(n) = e_tilde(n) + (f_tilde(n-1) + f_tilde(n-2) / 2)
		prog.resultSet[decodeCount] = e_tilde + ((f_tilde_OneAgo + f_tilde_TwoAgo) / 2);

		// Prevent decoding of illegitimate values to any value outside of 0 to 255 range
		if (prog.resultSet[decodeCount] > 255)
			prog.resultSet[decodeCount] = 255;
		else if (prog.resultSet[decodeCount] < 0)
			prog.resultSet[decodeCount] = 0;

		/* 	f_tilde(n-2) = f_tilde(n-1)
			f_tilde(n-1) = f_tilde(n) */
		f_tilde_TwoAgo = f_tilde_OneAgo;
		f_tilde_OneAgo = prog.resultSet[decodeCount];
	}
}

// Output all encoded / decoded values
void printResults()
{
	int i = 0;

	if (prog.encodeMode == true)
		printf("Encoding output: ");
	else
		printf("Decoding output: ");

	for (i = 0; i < prog.dataSize; i++)
	{
		printf(" %d", prog.resultSet[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (!(checkUsage(argc, argv)))	// Returns as 1 for any misuse of command-line arguments
		return 1;

	allocateMemory();		// Allocate storage buffers for passed and output data

	if (prog.data == 0)
	{
		printf("ERROR: Out of memory\n");
		return 1;
	}
	fillData(argv);			// Store command-line arguments in storage buffer
	
	if (prog.encodeMode == true)	// Determine program purpose (encoding or decoding) and perform such actions
		encode();
	else
		decode();

	printResults();			// Output all encoded / decoded values

	deallocateMemory();		// Deallocate storage buffers for passed and output data

	return 0;
}
