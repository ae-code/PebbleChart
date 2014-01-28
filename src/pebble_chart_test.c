#include <pebble.h>
#include "pebble_chart.h"

static Window *window;
static TextLayer *text_layer;
static ChartLayer* chart_layer;
static TextLayer * title_text_layer;
static bool bToggleColors = true;

static void load_chart_1() {
  const int x[] = { 50, 60, 80, 90, 100, 110 };
  const int y[] = { 20, 22, 21, 20, 19, 17 };
  chart_layer_set_data(chart_layer, x, eINT, y, eINT, 6);
  chart_layer_set_ymin(chart_layer, 0);
}

static void unload_chart_1() {
  chart_layer_clear_ymin(chart_layer);
}

static void load_chart_2() {
  const int x[] = { 50, 60, 80, 90, 100, 110 };
  const float y[] = { 0.5, -2.0, -1.3, 1.0, 0.4, 0.1 };
  chart_layer_set_data(chart_layer, x, eINT, y, eFLOAT, 6);
}

static void load_chart_3() {
  int x[200];
  int y[200];
  for (int i = 0; i < 200; ++i) {
    x[i] = i;
    y[i] = i ? y[i-1]+i : i;
  }
  chart_layer_set_data(chart_layer, x, eINT, y, eINT, 200);
}

static void load_chart_4() {
  chart_layer_set_plot_type(chart_layer, eSCATTER);
  int x[50];
  int y[50];
  for (int i = 0; i < 50; ++i) {
    x[i] = i;
    y[i] = i + ((i%10) - 5);
  }
  chart_layer_set_data(chart_layer, x, eINT, y, eINT, 50);
}

static void unload_chart_4() {
  chart_layer_set_plot_type(chart_layer, eLINE);
}

static void load_chart_5() {
  chart_layer_set_plot_type(chart_layer, eBAR);
  const int x[] = { 0, 1, 2, 3, 4, 5, 6 };
  const int y[] = { 10, 22, 20, 13, 15, 12 };
  chart_layer_set_data(chart_layer, x, eINT, y, eINT, 6);
  chart_layer_set_ymin(chart_layer, 0);
}

static void unload_chart_5() {
  chart_layer_clear_ymin(chart_layer);
  chart_layer_set_plot_type(chart_layer, eLINE);
}

static void load_chart_6() {
  chart_layer_set_plot_type(chart_layer, eBAR);
  const int x[] = { 50, 60, 80, 90, 100, 110 };
  const float y[] = { 0.5, -2.0, -1.3, 1.0, 0.4, 0.1 };
  chart_layer_set_data(chart_layer, x, eINT, y, eFLOAT, 6);
}

static void unload_chart_6() {
  chart_layer_set_plot_type(chart_layer, eLINE);
}

static void load_chart_7() {
  const float x[] = { 4.0, 2.0, 5.0, 0.0, 3.0, 1.0 };
  const float y[] = { 4.0, 2.0, 5.0, 0.0, 3.0, 1.0 };
  chart_layer_set_data(chart_layer, x, eFLOAT, y, eFLOAT, 6);
}

#define NUM_CHARTS 7
typedef void (*funcLoad)();
static funcLoad loadCallbacks[NUM_CHARTS] = { 
  &load_chart_1, 
  &load_chart_2,
  &load_chart_3,
  &load_chart_4,
  &load_chart_5,
  &load_chart_6,
  &load_chart_7
};
typedef void (*funcUnload)();
static funcUnload unloadCallbacks[NUM_CHARTS] = { 
  &unload_chart_1, 
  NULL,
  NULL,
  &unload_chart_4,
  &unload_chart_5,
  &unload_chart_6,
  NULL
};
static const char* chartTitles[NUM_CHARTS] = { 
  "Pinned X to 0",
  "Natural +/- with x-axis",
  "200 pt Sampling",
  "Scatter chart",
  "Bar chart",
  "Bar chart w/gap",
  "Unsorted X"
};
static int curr_chart = 0;

static void unload_curr_chart() {
  if (unloadCallbacks[curr_chart])
    unloadCallbacks[curr_chart]();
}

static void load_curr_chart() {
  loadCallbacks[curr_chart]();
  text_layer_set_text(title_text_layer, chartTitles[curr_chart]);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (bToggleColors) {
    chart_layer_set_plot_color(chart_layer, GColorWhite);
    chart_layer_set_canvas_color(chart_layer, GColorBlack);
    chart_layer_show_frame(chart_layer, true);
  }
  else {
    chart_layer_set_plot_color(chart_layer, GColorBlack);
    chart_layer_set_canvas_color(chart_layer, GColorWhite);
    chart_layer_show_frame(chart_layer, false);
  }

  bToggleColors = !bToggleColors;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  unload_curr_chart();
  
  --curr_chart;
  if (curr_chart < 0)
    curr_chart = NUM_CHARTS-1;

  load_curr_chart();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  unload_curr_chart();
  
  ++curr_chart;
  if (curr_chart >= NUM_CHARTS)
    curr_chart = 0;

  load_curr_chart();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press Up/Down");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  chart_layer = chart_layer_create((GRect) { 
      .origin = { 0, 40},
	.size = { bounds.size.w, 80 } });
  chart_layer_set_plot_color(chart_layer, GColorBlack);
  chart_layer_set_canvas_color(chart_layer, GColorWhite);
  chart_layer_show_points_on_line(chart_layer, true);
  //chart_layer_animate(chart_layer, false);
  layer_add_child(window_layer, chart_layer_get_layer(chart_layer));

  title_text_layer = text_layer_create((GRect) { .origin = { 0, 140 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_alignment(title_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(title_text_layer));

  load_curr_chart();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  chart_layer_destroy(chart_layer);
  text_layer_destroy(title_text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();

  return 0;
}
