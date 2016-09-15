#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

const double frequencies[256];
const double digram_frequencies[256][256];

double chi_squared_char(int counts[256], int length) {
  double chi2 = 0.0;
  for (int i = 0; i < 256; i++) {
    /* so we don't divide by zero. */
    double freq = frequencies[i] ? frequencies[i] : FLT_MIN;
    double expected = ((double) length) * freq;
    double error = (double) (expected - counts[i]);
    chi2 += (error * error) / expected;
  }
  return chi2;
}

double chi_squared_digram(int counts[256][256], int length) {
  double chi2 = 0.0;
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 256; j++) {
      /* so we don't divide by zero. */
      double freq = digram_frequencies[i][j] ? digram_frequencies[i][j] : FLT_MIN;
      double expected = ((double) length) * freq;
      double error = (double) (expected - counts[i][j]);
      chi2 += (error * error) / expected;
    }
  }
  return chi2;
}

int process_char(int counts[256], char x) {
  counts[(int) x] += 1;
  return 0;
}

void process_digram(char *digram_state, int counts[256][256], char here) {
  if (*digram_state)
    counts[(int) (*digram_state)][(int) here] += 1;
  *digram_state = here;
}

int main (int argc, char **argv) {
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

  /* initialize to zero */
  char here = 0;
  int length = 0;
  char digram_state = 0;
  if (is_digram_mode) {
    int counts[256][256] = {0};
    /* read everything, keep track of zeros vs ones */
    while (fread(&here, sizeof(char), 1, stdin) && ++length)
      process_digram(&digram_state, counts, here);
    printf("%f\n", chi_squared_digram(counts, length));
  } else {
    int counts[256] = {0};
    /* read everything, keep track of zeros vs ones */
    while (fread(&here, sizeof(char), 1, stdin) && ++length)
      process_char(counts, here);
    printf("%f\n", chi_squared_char(counts, length));

  }
  exit(0);

 usage:
  fprintf(stderr, "usage:\n$ printf \"data\" | %s [-n 1 | -n 2)]\"\n16.883203\n$\n", argv[0]);
  exit(1);
}
