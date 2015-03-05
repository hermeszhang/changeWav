#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct{
	int riff; //id of the file
	int length; //chunk size of the file
	int middle[8]; //rest
	int block;	//block = total bytes of the data excluding the 44 byte of the header
} Header;

int main(int argc, char *argv[]) {
	//input is the sourcewav (source wav file), output is the destwav (destination wave file) and input_copy is the copy of sourcewav
	FILE *input, *output, *input_copy;
	int option, delay, volume_scale;
	int originalSize;	// The length of the data block
	short temp, tempZero = 0, echo;
	int i;
	Header header;
	
	// Checking whether three arguments are provided (i.e. remvocals sourcewav destwav)
	if ((argc < 3) || (argc > 7)) {
		fprintf(stderr, "Usage: %s [-d delay] [-v volume_scale_scale] <sourcewav> <destwav>\n", argv[0]);
		exit(1);
	}
	// Getting optional input delay and volume_scale_scale
	while ((option = getopt(argc, argv, "d:v:")) != -1) {
		switch (option) {
			case 'd':
				delay = atoi(optarg);
				break;
			case 'v':
				volume_scale = atoi(optarg);
				break;
		}
	}
	// Initializing the buff whose size is the number of the samples in delay
	char *buff = (char *) malloc(sizeof(short)*delay);
	
	// Checking whether the sourcewav exists
	if ((input = fopen(argv[argc - 2], "rb")) == NULL) {
		perror(argv[argc - 2]);
		exit(1);
	}
	// Making another another copy of the input
	if ((input_copy = fopen(argv[argc - 2], "rb")) == NULL) {
		perror(argv[argc - 2]);
		exit(1);
	}
	// Checking whether the destwav exists
	if ((output = fopen(argv[argc - 1], "wb")) == NULL) {
		perror(argv[argc - 1]);
		exit(1);
	}
	// Checking whether header reads the whole 44 bytes of the input (sourcewav)
	if (fread(&header, sizeof(Header), 1, input) == EOF) {
		perror(argv[argc-2]);
		exit(1);
	}
	
	// Increase the size of the fil by 2*delay
	header.length = header.length + (2 * delay);
	originalSize = header.block;
	// Increase the size of the data by 2*delay
	header.block = header.block + (2 * delay);
	
	// Writing the modified header to the output file
	fwrite(&header, sizeof(Header), 1, output);
	
	if (originalSize < delay *sizeof(short)) {
	// If the delay is longer than the original samples
	
		//step1: Put original samples to the output in the same location
		for (i = 0; i * sizeof(short) < originalSize; i++) {
			if (fread(&temp, sizeof(short), 1, input) == EOF){
				perror(argv[argc-2]);
				free(buff);
				exit(1);
			}
			// Writing the original data to the output
			fwrite(&temp, sizeof(short), 1, output);
		}
		//step2: Put all zeros for the duration of (delay - originalSize) to the output
		for (i = 0; i * sizeof(short) < (delay *sizeof(short) - originalSize); i++) {
			// tempZero is short whose value is zero.
			fwrite(&tempZero, sizeof(short), 1, output);
		}
		//step3: Add echo for the duration of delay to the end of the output
		rewind(input);	// Send the pointer to the front
		if (fread(&header, sizeof(Header), 1, input) == EOF){
			//skipping the first 44 bytes
			perror(argv[argc - 2]);
			exit(1);
		}
		for (i = 0; i * sizeof(short) < originalSize; i++){
			if (fread(&temp, sizeof(short), 1, input) == EOF) {
				perror(argv[argc - 2]);
				exit(1);
			}
			// Adding echo to the end of the output
			temp = (temp / volume_scale);
			fwrite(&temp, sizeof(short), 1, output);
		}
	} else {
		// The original samples > delay.
		//step1: put original samples to the output in the first segment for the duration of the 1st delay
		for (i = 0; i < delay; i++) {
			if (fread(&temp, sizeof(short), 1, input) == EOF) {
				perror(argv[argc - 2]);
				exit(1);
			}
			fwrite(&temp, sizeof(short), 1, output);
		}
		//step2: Mixing. original + echo in the middle of the output
		// Skipping the first 44 bytes of the header
		if (fread(&header, sizeof(Header), 1, input_copy) == EOF) {
			perror(argv[argc - 2]);
			exit(1);
		}
		// Reading the data for the duration of the echo
		for (i = 0; i * sizeof(short) < (originalSize - delay *sizeof(short)); i++) {
			if(fread(&temp, sizeof(short), 1, input) == EOF) {
				// Reading from the input
				perror(argv[argc - 2]);
				exit(1);
			}
			if (fread(&echo, sizeof(short), 1, input_copy) == EOF) {
				// Reading from the input_copy from the front of the data block to produce an echo
				perror(argv[argc - 2]);
				exit(1);
			}
			// Mixing the original with an echo
			short mixing = temp + (echo / volume_scale);
			fwrite(&mixing, sizeof(short), 1, output);
		}		
		//step3: add echo at the end of the output
		int echoPart = delay *sizeof(short);
		if (echoPart != 0) {
			for (i = 0; i * sizeof(short) < echoPart; i++) {
				if (fread(&temp, sizeof(short), 1, input_copy) == EOF) {
					perror(argv[argc - 2]);
					exit(1);
				}
				temp = (temp / volume_scale);
				fwrite(&temp, sizeof(short), 1, output);
			}	
		}
	}
	
	free(buff);	// Since buff used the malloc to avoid memory leak
	// Closing the input and output file
	fclose(input);
	fclose(output);
	return 0;
}