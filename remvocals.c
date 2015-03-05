#include <stdio.h>
#include <stdlib.h>

typedef struct{
	short left;
	short right;
} Music;

int main(int argc, char *argv[]) {
	FILE *input, *output; 	//input is the sourcewav and output is the destwav.
	char buff[44];
	
	// Checking whether three arguments are provided (i.e. remvocals sourcewav destwav)
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <sourcewav> <destwav>\n", argv[0]);
		exit(1);
	}
	
	// Checking whether the sourcewav exists
	if ((input = fopen(argv[1], "rb")) == NULL) {
		perror(argv[1]);
		exit(1);
	}
	// Checking whether the destwav exists
	if ((output = fopen(argv[2], "wb")) == NULL) {
		perror(argv[2]);
		exit(1);
	}
	
	if (fread(buff, sizeof(char), 44, input) != 44) {
		// Reading first 44 bytes of sourcewav to buff
		perror(argv[1]);
		exit(1);
	} else {	
		//sizeof(char) is 1 byte
		if (fwrite(buff, sizeof(char), 44, output) != 44) {
			perror(argv[2]);
			exit(1);	
		}
	}
	
	Music music;
	
	// Checking whether fread operates to the end of the file
	while (0 < (fread(&music, sizeof(Music), 1, input))) {
		// Removing vocals by subtracting left from right and dividing it by 2.
		short combined = (music.left - music.right)/2;
		fwrite(&combined, sizeof(short), 1, output);
		fwrite(&combined, sizeof(short), 1, output);
	}
	// Close both input and output.	
	fclose(input);
	fclose(output);
	
	return 0;
}