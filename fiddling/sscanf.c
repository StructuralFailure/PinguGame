#include <stdio.h>

int main(int argc, char** argv) 
{
	char* input = "1 23 456 7890";
	char* input_start = input;

	int bytes_read;

	int number1, number2, number3, number4;


	sscanf(input, "%d%n", &number1, &bytes_read);
	input += bytes_read;
	sscanf(input, "%d %d%n", &number2, &number3, &bytes_read);
	input += bytes_read;
	sscanf(input, "%d%n", &number4, &bytes_read);
	input += bytes_read;

	printf("%d %d %d %d (%ld bytes read.)\n", number1, number2, number3, number4, input - input_start);
}