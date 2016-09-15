#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

const double frequencies[256];
const double digram_frequencies[256][256];

typedef int char_state[256];

double chi_squared_char(char_state counts, size_t length)
{
	double chi2 = 0.0;
	for (size_t i = 0; i < 256; i++) {
		/* so we don't divide by zero. */
		double freq = frequencies[i] ? frequencies[i] : FLT_MIN;
		double expected = ((double) length) * freq;
		double error = expected - counts[i];
		chi2 += (error * error) / expected;
	}
	return chi2;
}

typedef struct {
	int counts[256][256];
	char last_char;
} digram_state;

double chi_squared_digram(digram_state *state, size_t length)
{
	double chi2 = 0.0;
	for (size_t i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			/* so we don't divide by zero. */
			double freq = digram_frequencies[i][j] ? digram_frequencies[i][j] : FLT_MIN;
			double expected = ((double) length) * freq;
			double error = expected - state->counts[i][j];
			chi2 += (error * error) / expected;
		}
	}
	return chi2;
}

int process_char(char_state counts, char x)
{
	counts[(int) x] += 1;
	return 0;
}

void process_digram(digram_state *state, char here)
{
	if (state->last_char)
		state->counts[(int) (state->last_char)][(int) here] += 1;
	state->last_char = here;
}

int main (int argc, char **argv)
{
	int is_digram_mode = 0;
	if (argc > 3 || argc == 2)  goto usage;
	if (argc == 3) {
		if (!strcmp(argv[1], "-n")) {
			if (!strcmp(argv[2], "1")) {
				is_digram_mode = 0;
			} else if (!strcmp(argv[2], "2")) {
				is_digram_mode = 1;
			} else {
				goto usage;
			}
		} else {
			goto usage;
		}
	}

	size_t length = 0;
	char here = 0;
	if (is_digram_mode) {
		digram_state state = {0};
		errno = 0;
		while (read(STDIN_FILENO, &here, sizeof(char)) > 0 && ++length)
			process_digram(&state, here);
		if (errno) {
			fprintf(stderr, "(in `read`): %m\n");
			return -1;
		}
		printf("%f\n", chi_squared_digram(&state, length));
	} else {
		char_state state = {0};
		while (read(STDIN_FILENO, &here, sizeof(char)) > 0 && ++length)
			process_char(state, here);
		if (errno) {
			fprintf(stderr, "(in `read`): %m\n");
			return -1;
		}
		printf("%f\n", chi_squared_char(state, length));

	}
	return 0;
usage:
	fprintf(stderr, "usage:\n$ printf \"data\" | %s [-n 1 | -n 2)]\"\n16.883203\n$\n", argv[0]);
	return 1;
}
