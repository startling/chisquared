#include <stdio.h>
#include <stdlib.h>

/* https://en.wikipedia.org/wiki/Letter_frequency#Relative_frequencies_of_letters_in_the_English_language */
const double frequencies[26] = {
  0.08167,
  0.01492,	
  0.02782,	
  0.04253,	
  0.12702,	
  0.02228,	
  0.02015,	
  0.06094,	
  0.06966,	
  0.00153,	
  0.00772,	
  0.04025,	
  0.02406,	
  0.06749,	
  0.07507,	
  0.01929,	
  0.00095,	
  0.05987,	
  0.06327,	
  0.09056,	
  0.02758,	
  0.00978,	
  0.02361,	
  0.00150,	
  0.01974,	
  0.00074,
};



double chi_squared(int counts[26], int length) {
  double chi2 = 0.0;
  for (int i = 0; i < 26; i++) {
    double expected = (double) length * frequencies[i];
    double error = (double) (expected - counts[i]);
    chi2 += (error * error) / expected;
  }
  return chi2;
}


int process_char(int counts[26], char x) {
  /* lowercase ascii. */
  if (x >= 'a' && x <= 'z') {
    counts[x - 'a']++;
  } else if (x >= 'A' && x <= 'Z') {
    counts[x - 'A']++;
  } else {
    return 1;
  }
  return 0;
}

int main (int argc, char **argv) {
  if (argc > 1) {
    fprintf(stderr, "usage:\n$ %s <<< \"data\"\n16.883203\n$\n", argv[0]);
    exit(1);
  }

  /* initialize to zero */
  char here = 0;
  int counts[26] = {0};
  int letter_count = 0;

  /* read everything. */
  while (fread(&here, sizeof(char), 1, stdin))
    letter_count += 1 - process_char(counts, here);

  printf("%f\n", chi_squared(counts, letter_count));
}
