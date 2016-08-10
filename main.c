#include <stdio.h>
#include <stdlib.h>
#include <float.h>


/* ratios from http://www.daviddlewis.com/resources/testcollections/reuters21578/,
   with xmly bits stripped, non-SPACE whitespace elided */
const double frequencies [256] = {
  [0] = 0.000000,
  [1] = 0.000000,
  [2] = 0.000000,
  [3] = 0.000000,
  [4] = 0.000000,
  [5] = 0.000000,
  [6] = 0.000000,
  [7] = 0.000000,
  [8] = 0.000000,
  [9] = 0.000000,
  [10] = 0.000000,
  [11] = 0.000000,
  [12] = 0.000000,
  [13] = 0.000000,
  [14] = 0.000000,
  [15] = 0.000000,
  [16] = 0.000000,
  [17] = 0.000000,
  [18] = 0.000000,
  [19] = 0.000000,
  [20] = 0.000000,
  [21] = 0.000000,
  [22] = 0.000000,
  [23] = 0.000000,
  [24] = 0.000000,
  [25] = 0.000000,
  [26] = 0.000000,
  [27] = 0.000000,
  [28] = 0.000000,
  [29] = 0.000000,
  [30] = 0.000000,
  [31] = 0.000000,
  [32] = 0.148508,
  [33] = 0.000001,
  [34] = 0.001343,
  [35] = 0.000000,
  [36] = 0.000000,
  [37] = 0.000000,
  [38] = 0.000000,
  [39] = 0.001375,
  [40] = 0.000315,
  [41] = 0.000314,
  [42] = 0.000868,
  [43] = 0.000002,
  [44] = 0.008819,
  [45] = 0.010686,
  [46] = 0.011759,
  [47] = 0.000697,
  [48] = 0.011954,
  [49] = 0.010653,
  [50] = 0.005664,
  [51] = 0.005007,
  [52] = 0.003868,
  [53] = 0.004226,
  [54] = 0.003447,
  [55] = 0.004526,
  [56] = 0.005010,
  [57] = 0.005147,
  [58] = 0.002414,
  [59] = 0.000007,
  [60] = 0.000000,
  [61] = 0.000001,
  [62] = 0.001152,
  [63] = 0.000004,
  [64] = 0.000000,
  [65] = 0.009111,
  [66] = 0.004024,
  [67] = 0.007070,
  [68] = 0.003263,
  [69] = 0.008469,
  [70] = 0.003088,
  [71] = 0.002453,
  [72] = 0.002258,
  [73] = 0.006417,
  [74] = 0.001292,
  [75] = 0.001309,
  [76] = 0.004148,
  [77] = 0.005018,
  [78] = 0.006729,
  [79] = 0.005988,
  [80] = 0.003488,
  [81] = 0.000356,
  [82] = 0.008558,
  [83] = 0.008176,
  [84] = 0.007935,
  [85] = 0.003438,
  [86] = 0.000867,
  [87] = 0.001642,
  [88] = 0.000351,
  [89] = 0.001507,
  [90] = 0.000195,
  [91] = 0.000001,
  [92] = 0.000000,
  [93] = 0.000001,
  [94] = 0.000002,
  [95] = 0.000000,
  [96] = 0.000000,
  [97] = 0.055060,
  [98] = 0.009233,
  [99] = 0.022342,
  [100] = 0.027523,
  [101] = 0.078158,
  [102] = 0.015913,
  [103] = 0.011106,
  [104] = 0.023209,
  [105] = 0.047300,
  [106] = 0.000606,
  [107] = 0.004384,
  [108] = 0.028282,
  [109] = 0.015610,
  [110] = 0.047811,
  [111] = 0.046438,
  [112] = 0.015246,
  [113] = 0.001001,
  [114] = 0.047664,
  [115] = 0.045274,
  [116] = 0.056108,
  [117] = 0.019808,
  [118] = 0.006685,
  [119] = 0.008233,
  [120] = 0.002036,
  [121] = 0.009498,
  [122] = 0.000552,
  [123] = 0.000000,
  [124] = 0.000000,
  [125] = 0.000000,
  [126] = 0.000000,
  [127] = 0.000000,
};

double chi_squared(int counts[256], int length) {
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


int process_char(int counts[256], char x) {
  counts[(int) x] += 1;
  return 0;
}

int main (int argc, char **argv) {
  if (argc > 1) {
    fprintf(stderr, "usage:\n$ printf \"data\" | %s\"\n16.883203\n$\n", argv[0]);
    exit(1);
  }

  /* initialize to zero */
  char here = 0;
  int counts[256] = {0};
  int length = 0;

  /* read everything, keep track of zeros vs ones */
  while (fread(&here, sizeof(char), 1, stdin) && ++length)
    process_char(counts, here);
  printf("%f\n", chi_squared(counts, length));
}
