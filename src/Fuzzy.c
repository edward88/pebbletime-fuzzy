/*
author  : edward
date    : 2016 June 04
comment : somewhat fuzzy time pebble watchface
*/

#include <pebble.h>
#include <stdlib.h>

#define TEXT_LAYER_BORDER 3
#define BATTERY_LEVEL_LAYER_WIDTH 2
#define FUZZY_VARIATIONS 3

// global static variables
static Window *s_main_window;
static TextLayer *s_fuzzy_time_layer;
static GFont s_fuzzy_time_font;

static int s_battery_level;
static Layer *s_battery_layer;

static char *hour[] = {"NOON;TWELVE;12",
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
                      "MIDNIGHT;TWELVE;12"};
                      
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
                      
static char *post_fuzzy[] = {"!!!;\no'clock;:00\nor so", //0
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

int random_max_limit(int max) {

  if(max < RAND_MAX)
  {
    return rand() % (max);
  }
  return 0;
}

char *get_fuzzy(char *fuzzy, int index)
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

static void battery_update_proc(Layer *layer, GContext *ctx) {
  
  GRect bounds = layer_get_bounds(layer);

  // Find the height of the bar
  int height = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.h);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, GRect(0, bounds.size.h - height, bounds.size.w, height), 0, GCornerNone);
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void main_window_load(Window *window) {

  // get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // create the textlayer with specific bounds
  s_fuzzy_time_layer = text_layer_create(GRect(TEXT_LAYER_BORDER, TEXT_LAYER_BORDER, (bounds.size.w-TEXT_LAYER_BORDER*2), (bounds.size.h-TEXT_LAYER_BORDER*2)));

  // create gfont
  //s_fuzzy_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANONYMOUS_30));
  //s_fuzzy_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_APPLEBERRY_36));

  // improve the layout to be more like a watchface
  text_layer_set_background_color(s_fuzzy_time_layer, GColorClear);
  text_layer_set_text_color(s_fuzzy_time_layer, GColorRed);
  //text_layer_set_font(s_fuzzy_time_layer, s_fuzzy_time_font);
  text_layer_set_font(s_fuzzy_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_fuzzy_time_layer, GTextAlignmentRight);

  // add it as a child layer to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_fuzzy_time_layer));

  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(0, 0, BATTERY_LEVEL_LAYER_WIDTH, bounds.size.h));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
}

static void main_window_unload(Window *window) {
  
  // destroy textlayer
  text_layer_destroy(s_fuzzy_time_layer);

  // unload gfont
  fonts_unload_custom_font(s_fuzzy_time_font);
  
  layer_destroy(s_battery_layer);
}

static void update_time(bool force_update) {

  // get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  if(tick_time->tm_min%5==0 || force_update)
  {
    // convert to 12 hour format
    int hour_24_format = tick_time->tm_hour;
    int hour_12_format = (hour_24_format >= 12 ? hour_24_format - 12 : hour_24_format);
    int minute_5_gap = tick_time->tm_min/5;

    int fuzzy_index = random_max_limit(FUZZY_VARIATIONS);

    if(fuzzy_index == 0 && minute_5_gap > 7)
    {
      if(hour_12_format == 12)
      {
        hour_12_format = 1;
      }
      else if (hour_24_format == 11)
      {
        hour_12_format = 0;
      }
      else
      {
        hour_12_format = hour_12_format + 1;
      }
    }
    
    // write the fuzzy time string into a buffer
    static char s_buffer[30];

    strcpy(s_buffer, get_fuzzy(pre_fuzzy[minute_5_gap], fuzzy_index));
    strcat(s_buffer, get_fuzzy(hour[hour_12_format], fuzzy_index));
    strcat(s_buffer, get_fuzzy(post_fuzzy[minute_5_gap], fuzzy_index));

    // display this time on the textlayer
    text_layer_set_text(s_fuzzy_time_layer, s_buffer);
  }
}

static void  tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(false);
}

static void init() {
  // register with ticktimerservice
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);

  // create main window element and assign to pointer
  s_main_window = window_create();

  // set handlers to manage the elements inside the window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // show the window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // set main window background color
  window_set_background_color(s_main_window, GColorBlack);
  
  // make sure the time is displayed from the start
  update_time(true);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit() {
  // destroy window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
