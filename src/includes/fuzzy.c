#include "fuzzy.h"

static char *hour[] = {"MNIGHT;TWELVE;12",
                      "ONE;ONE;1",
                      "TWO;TWO;2",
                      "THREE;THREE;3",
                      "FOUR;FOUR;4",
                      "FIVE;FIVE;5",
                      "SIX;SIX;6",
                      "SEVEN;SEVEN;7",
                      "EIGHT;EIGHT;8",
                      "NINE;NINE;9",
                      "TEN;TEN;10",
                      "ELEVEN;ELEVEN;11",
                      "NOON;TWELVE;12"};

static char *pre_fuzzy[] = {"it's\n;exactly\n;about\n",  //0
                            "past\n;;around\n",
                            "ten past\n;;just past\n",
                            "quarter past\n;;about\n", //15
                            "quarter past\n;;around\n",
                            "almost half\n;;just past\n",
                            "half\n;;about\n", //30
                            "past half\n;;around\n",
                            "quarter to\n;;just past\n",
                            "quarter to\n;;about\n", //45
                            "ten to\n;;around\n",
                            "almost\n;;just past\n"};

static char *post_fuzzy[] = {"!;\no'clock;:00", //0
                             ";\nfive;:05\nish",
                             ";\nten;:10",
                             ";\nfifteen;:15\nor so", //15
                             "\nish;\ntwenty;:20\nish",
                             ";\ntwenty five;:25",
                             ";\nthirty;:30\nor so", //30
                             ";\nthirty five;:35\nish",
                             "\nor so;\nforty;:40",
                             ";\nforty five;:45\nor so", //45
                             ";\nfifty;:50\nish",
                             ";\nfifty five;:55"};

int get_simple_random(int max_limit) {

  if(max_limit < RAND_MAX)
  {
    return rand() % (max_limit);
  }
  return 0;
}

int get_random(int max_limit)
{
  unsigned int
  // max <= RAND_MAX < ULONG_MAX, so this is okay.
  num_bins = (unsigned int) max_limit,
  num_rand = (unsigned int) RAND_MAX + 1,
  bin_size = num_rand / num_bins,
  defect   = num_rand % num_bins;

  int x;
  do {
    x = rand();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned int)x);

  // Truncated division is intentional
  return x/bin_size;
}

char *get_fuzzy_text(char *fuzzy, int index)
{
  static char fuzzy_string[20];

  //clear fuzzy string from any previous values
  memset(fuzzy_string, 0, sizeof(fuzzy_string));
  fuzzy_string[0] = '\0';

  int len = strlen(fuzzy);
  int index_counter = 0;
  int fuzzy_string_index = 0;

  for (int i = 0; i < len; i++)
  {
    if(fuzzy[i] == ';')
    {
      index_counter++;
    }
    else if(index_counter == index)
    {
      fuzzy_string[fuzzy_string_index] = fuzzy[i];
      fuzzy_string_index++;
    }
  }

  return fuzzy_string;
}

char *get_fuzzy(int index, int hh, int mm)
{
    // write the fuzzy time string into a buffer
    static char s_buffer[30];

    strcpy(s_buffer, get_fuzzy_text(pre_fuzzy[mm], index));
    strcat(s_buffer, get_fuzzy_text(hour[hh], index));
    strcat(s_buffer, get_fuzzy_text(post_fuzzy[mm], index));

    return s_buffer;
}
