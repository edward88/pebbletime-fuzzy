#pragma once

#include <pebble.h>
#include <stdlib.h>

int get_simple_random(int max_limit);

int get_random(int max_limit);

char *get_fuzzy(int index, int hh, int mm);
