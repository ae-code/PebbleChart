# Charting library for Pebble

## Overview

This library can be included in Pebble apps/watchfaces to draw charts.  Charting functionality is provided through a new ChartLayer, which supports a variety of options.  Line, scatter, and bar charts are supported.

This repository includes a tester app, pebble_chart_test.c, which contains several chart examples and showcases the functionality.

## Usage

Copy pebble_chart.h and pebble_chart.c into your src directory.  Use the provided API to create and configure charts.

### Example

```C
#include "pebble_chart.h"
...
ChartLayer* chart_layer;
...
chart_layer = chart_layer_create((GRect) { 
  .origin = { 0, 40},
  .size = { bounds.size.w, 80 } });
chart_layer_set_plot_color(chart_layer, GColorBlack);
chart_layer_set_canvas_color(chart_layer, GColorWhite);
chart_layer_show_points_on_line(chart_layer, true);
layer_add_child(window_layer, chart_layer_get_layer(chart_layer));

const int x[] = { 50, 60, 80, 90, 100, 110 };
const float y[] = { 0.5, -2.0, -1.3, 1.0, 0.4, 0.1 };
chart_layer_set_data(chart_layer, x, eINT, y, eFLOAT, 6);

```

## API

See comments within pebble_chart.h.