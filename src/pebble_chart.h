#pragma once

#include <pebble.h>


struct ChartLayer;
typedef struct ChartLayer ChartLayer;

//! Creates a new ChartLayer on the heap and initializes it with the default values.
//!
//! * Plot type: Line
//! * Plot color: GColorWhite
//! * Canvas color: GColorBlack
//! * Show Points: false
//! * Margin: 5 (px)
//! * X Minimum: None
//! * X Maximum: None
//! * Y Minimum: None
//! * Y Maximum: None
//! * Show Frame: false
//! * Animate: true
//! * Animation Duration: 1500 (ms)
//!
//! @param frame The frame with which to initialize the ChartLayer
//! @return A pointer to the ChartLayer. `NULL` if the ChartLayer could not
//! be created
ChartLayer* chart_layer_create(GRect frame);

//! Destroys a ChartLayer previously created by chart_layer_create.
//!
//! @param layer The ChartLayer to destroy
void chart_layer_destroy(ChartLayer* layer);

//! Gets the "root" Layer of the chart layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param layer Pointer to the ChartLayer for which to get the "root" Layer
//! @return The "root" Layer of the chart layer.
Layer* chart_layer_get_layer(ChartLayer* layer);

//! Enum representing data-type (`int` or `float`) of chart data
//! set int through chart_layer_set_data()
typedef enum {
  eINT,
  eFLOAT
} ChartDataType;

//! Sets the actual chart data into the chart.
//! Chart will immediately update with new data set.
//! X and Y values can be stack allocated, as they will
//! be copied internal to the ChartLayer.
//! If there are too many points to display given the
//! width of the ChartLayer, the data points displayed will
//! be a sampling of the original data points.
//! @param layer The ChartLayer to display the chart
//! @param pX The array containing the x-values
//! @param typeX The data type of `pX`'s values
//! @param pY The array containing the y-values
//! @param typeY The data type of `pY`'s values
//! @param iNumPoints The number of data points in `pX` and `pY`
void chart_layer_set_data(ChartLayer* layer, 
			  const void* pX,
			  const ChartDataType typeX,
			  const void* pY, 
			  const ChartDataType typeY,
			  const unsigned int iNumPoints);

//! Enum of supported plot types
typedef enum {
  eLINE,
  eSCATTER,
  eBAR
} ChartPlotType;

//! Sets the plot type (i.e. line, scatter, or bar)
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the plot type
//! @param type The new plot type
void chart_layer_set_plot_type(ChartLayer* layer, const ChartPlotType type);

//! Sets the color of the drawn items on the chart
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the plot color
//! @param color The new `GColor` for the drawn items
void chart_layer_set_plot_color(ChartLayer* layer, GColor color);

//! Set the background color of the chart
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the canvas color
//! @param color The new `GColor` for the background
void chart_layer_set_canvas_color(ChartLayer* layer, GColor color);

//! Sets whether or not the individual data points
//! will be displayed (as circles) on the chart.
//! Only applies to line charts, as points are always
//! displayed for scatter charts and never for
//! bar charts.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to show or hide points
//! @param bShow `true` if points should be shown, `false` otherwise
void chart_layer_show_points_on_line(ChartLayer* layer, bool bShow);

//! Sets the margin around the plot for the chart.
//! For example, if the Layer width is 100 pixels and the
//! margin is set to 5, then the plot will take up the
//! middle 90 pixels.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the margin
//! @param margin The new margin amount in pixels to set on the chart
void chart_layer_set_margin(ChartLayer* layer, int margin);

//! Sets the minimum value of the x-axis.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the minimum x-axis value
//! @param xmin The new minimum value for the x-axis
void chart_layer_set_xmin(ChartLayer* layer, float xmin);

//! Clears a previously set minimum x-axis value
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to clear the minimum x-axis value
void chart_layer_clear_xmin(ChartLayer* layer);

//! Sets the maximum value of the x-axis.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the maximum x-axis value
//! @param xmax The new maximum value for the x-axis
void chart_layer_set_xmax(ChartLayer* layer, float xmax);

//! Clears a previously set maximum x-axis value
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to clear the maximum x-axis value
void chart_layer_clear_xmax(ChartLayer* layer);

//! Sets the minimum value of the y-axis.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the minimum y-axis value
//! @param ymin The new minimum value for the y-axis
void chart_layer_set_ymin(ChartLayer* layer, float ymin);

//! Clears a previously set minimum y-axis value
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to clear the minimum y-axis value
void chart_layer_clear_ymin(ChartLayer* layer);

//! Sets the maximum value of the y-axis.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to set the maximum y-axis value
//! @param ymax The new maximum value for the y-axis
void chart_layer_set_ymax(ChartLayer* layer, float ymax);

//! Clears a previously set maximum y-axis value
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to clear the maximum y-axis value
void chart_layer_clear_ymax(ChartLayer* layer);

//! Sets whether or not a frame is drawn around
//! the chart canvas.
//! Will redraw chart if chart data is set.
//! @param layer The ChartLayer to which to toggle the drawing of the frame
//! @param bShow `true` if the frame should be drawn, `false` otherwise
void chart_layer_show_frame(ChartLayer* layer, bool bShow);

//! Sets whether or not the initial drawing of the chart
//! should be animated or not.
//! When animated, the points/bars will be drawn
//! on the chart over the duration of the animation
//! @param layer The ChartLayer to which to toggle the animation
//! @param bAnimate `true` if the drawing should be animated, `false` otherwise
void chart_layer_animate(ChartLayer* layer, bool bAnimate);

//! Sets the length of the drawing animation.
//! Has no impact if animation is turned off.
//! @param layer The ChartLayer to which to apply the duration
//! @param ms The duration of the animation in milliseconds
void chart_layer_set_animation_duration(ChartLayer* layer, const uint32_t ms);
