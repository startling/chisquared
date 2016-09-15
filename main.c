#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>


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
	int last_char;
} digram_state;

double chi_squared_digram(digram_state *state, size_t length)
{
	double chi2 = 0.0;
	for (size_t i = 0; i < 256; i++) {
		for (size_t j = 0; j < 256; j++) {
			/* so we don't divide by zero. */
			double freq = digram_frequencies[i][j] ? digram_frequencies[i][j] : FLT_MIN;
			double expected = ((double) length) * freq;
			double error = expected - state->counts[i][j];
			chi2 += (error * error) / expected;
		}
	}
	return chi2;
}

int process_char(char_state counts, int x)
{
	counts[x] += 1;
	return 0;
}

void process_digram(digram_state *state, int here)
{
	if (state->last_char)
		state->counts[state->last_char][here] += 1;
	state->last_char = here;
}

int main (int argc, char **argv)
{
	int is_digram_mode = 0;
	int option = 0;
	opterr = 0;
	while ((option = getopt(argc, argv, "n:")) != -1) {
		switch (option) {
		case 'n':
			if (!strcmp(optarg, "1")) {
				is_digram_mode = 0;
			} else if (!strcmp(optarg, "2")) {
				is_digram_mode = 1;
			} else {
				goto usage;
			}
			break;
		default:
			goto usage;
		}
	}

	int here = 0;
	if (is_digram_mode) {
		size_t length = 0;
		digram_state state = {0};
		while ((here = getchar()) > 0) {
			length += 1;
			process_digram(&state, here);
		}
		printf("%f\n", chi_squared_digram(&state, length));
	} else {
		size_t length = 0;
		char_state state = {0};
		while ((here = getchar()) > 0) {
			length += 1;
			process_char(state, here);
		}
		printf("%f\n", chi_squared_char(state, length));
	}
	return 0;
usage:
	fprintf(stderr,
		"usage: \n"
		"$ printf \"data\" | %s [-n 1 | -n 2)]\n"
		"16.883203\n"
		"$\n",
		argv[0]);
	return 1;
}
