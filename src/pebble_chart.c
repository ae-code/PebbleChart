#include "pebble_chart.h"

#define NOT_SET -777 // magic number to represent not value not set

typedef struct {
  // original data
  float* pXOrigData;
  float* pYOrigData;
  unsigned int iNumOrigPoints;

  // cached data
  int* pXData;
  int* pYData;
  unsigned int iNumPoints;
  int iXAxisIntercept;
  int iYAxisIntercept;
  int iYTicks;
  int iBarWidth;

  // other attributes
  ChartPlotType typePlot;
  GColor clrPlot;
  GColor clrCanvas;
  bool bShowPoints;
  int iMargin;
  float fXMin;
  float fXMax;
  float fYMin;
  float fYMax;
  bool bShowFrame;
  bool bAnimate;
  uint32_t iAnimationDuration;

  // state
  bool bLayoutDirty;
  Animation* pAnimation;
  AnimationImplementation* pAnimationImpl;
  unsigned int iPointsToDraw;
} ChartLayerData;

// function prototypes
static int closest_log10(float);
static float exponential10(int);
static void chart_layer_update_func(Layer*, GContext*);
static void chart_layer_update_layout(ChartLayer* layer);
static void animation_started(Animation*, void*);
static void animation_stopped(Animation*, bool, void*);
static void animation_update(Animation*, const uint32_t);

// helper to extract ChartLayerData from ChartLayer
static ChartLayerData* get_chart_data(ChartLayer* layer) {
  return (ChartLayerData*)(layer_get_data(chart_layer_get_layer(layer)));
}

// extracts "root" Layer
Layer* chart_layer_get_layer(ChartLayer* layer) {
  return (Layer*)layer;
}

// creator
ChartLayer* chart_layer_create(GRect frame) {
  // create "root" Layer
  ChartLayer* layer = (ChartLayer*)layer_create_with_data(frame, sizeof(ChartLayerData));
  if (!layer)
    return layer;

  // set defaults
  ChartLayerData* data = get_chart_data(layer);
  data->pXOrigData = NULL;
  data->pYOrigData = NULL;
  data->iNumOrigPoints = 0;
  data->pXData = NULL;
  data->pYData = NULL;
  data->iNumPoints = 0;
  data->bLayoutDirty = false;
  data->typePlot = eLINE;
  data->clrPlot = GColorWhite;
  data->clrCanvas = GColorBlack;
  data->bShowPoints = false;
  data->iMargin = 5;
  data->fXMin = NOT_SET;
  data->fXMax = NOT_SET;
  data->fYMin = NOT_SET;
  data->fYMax = NOT_SET;
  data->bShowFrame = false;
  data->bAnimate = true;
  data->iAnimationDuration = 1500;
  data->pAnimation = animation_create();
  data->pAnimationImpl = (struct AnimationImplementation*) malloc(sizeof(struct AnimationImplementation));
  data->iPointsToDraw = 0;

  // extra animation setup
  animation_set_curve(data->pAnimation, AnimationCurveLinear);
  animation_set_handlers(data->pAnimation, 
			 ((AnimationHandlers) {
			   .started = (AnimationStartedHandler)animation_started,
			     .stopped = (AnimationStoppedHandler)animation_stopped
			     }), layer);

  data->pAnimationImpl->setup = NULL;
  data->pAnimationImpl->teardown = NULL;
  data->pAnimationImpl->update = &animation_update;
  animation_set_implementation(data->pAnimation, data->pAnimationImpl);

  // sets function to draw
  layer_set_update_proc(chart_layer_get_layer(layer), chart_layer_update_func);

  return layer;
}

// destroys the ChartLayer
void chart_layer_destroy(ChartLayer* layer) {
  if (layer) {
    // clean-up
    ChartLayerData* pData = get_chart_data(layer);
    free(pData->pXOrigData);
    free(pData->pYOrigData);
    free(pData->pXData);
    free(pData->pYData);
    animation_destroy(pData->pAnimation);
    free(pData->pAnimationImpl);

    // destroy "root" Layer
    layer_destroy(chart_layer_get_layer(layer));
  }
}

//////////////////////////////////////
// set attributes

void chart_layer_set_plot_type(ChartLayer* layer, const ChartPlotType type) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->typePlot = type;
    pData->bLayoutDirty = true;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_set_plot_color(ChartLayer* layer, GColor color) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->clrPlot = color;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_set_canvas_color(ChartLayer* layer, GColor color) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->clrCanvas = color;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_show_points_on_line(ChartLayer* layer, bool bShow) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->bShowPoints = bShow;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_set_margin(ChartLayer* layer, int margin) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->iMargin = margin;
    // add extra pixel for frame
    if (pData->bShowFrame)
      ++pData->iMargin;
    pData->bLayoutDirty = true;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_set_xmin(ChartLayer* layer, float xmin) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->fXMin = xmin;
    pData->bLayoutDirty = true;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_clear_xmin(ChartLayer* layer) {
  chart_layer_set_xmin(layer, NOT_SET);
}

void chart_layer_set_xmax(ChartLayer* layer, float xmax) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->fXMax = xmax;
    pData->bLayoutDirty = true;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_clear_xmax(ChartLayer* layer) {
  chart_layer_set_xmax(layer, NOT_SET);
}

void chart_layer_set_ymin(ChartLayer* layer, float ymin) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->fYMin = ymin;
    pData->bLayoutDirty = true;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_clear_ymin(ChartLayer* layer) {
  chart_layer_set_ymin(layer, NOT_SET);
}

void chart_layer_set_ymax(ChartLayer* layer, float ymax) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->fYMax = ymax;
    pData->bLayoutDirty = true;

    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

void chart_layer_clear_ymax(ChartLayer* layer) {
  chart_layer_set_ymax(layer, NOT_SET);
}

void chart_layer_show_frame(ChartLayer* layer, bool bShow) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    if (pData->bShowFrame != bShow) {
      pData->bShowFrame = bShow;
      if (bShow)
	--pData->iMargin;
      else
	++pData->iMargin;

      layer_mark_dirty(chart_layer_get_layer(layer));
    }
  }
}

void chart_layer_animate(ChartLayer* layer, bool bAnimate) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->bAnimate = bAnimate;
  }
}

void chart_layer_set_animation_duration(ChartLayer* layer, const uint32_t ms) {
  if (layer) {
    ChartLayerData* pData = get_chart_data(layer);
    pData->iAnimationDuration = ms;
  }
}

////////////////////////////////////

// sets data into chart
void chart_layer_set_data(ChartLayer* layer, 
			  const void* pX, 
			  const ChartDataType typeX,
			  const void* pY, 
			  const ChartDataType typeY,
			  const unsigned int iNumPoints) {
  if (layer) {
    
    ChartLayerData* pData = get_chart_data(layer);

    // clean up previous data
    if (pData->iNumOrigPoints) {
      free(pData->pXOrigData);
      free(pData->pYOrigData);
    }
    
    // make space to copy data
    pData->iNumOrigPoints = iNumPoints;
    pData->pXOrigData = (float*) malloc(pData->iNumOrigPoints * sizeof(float));
    pData->pYOrigData = (float*) malloc(pData->iNumOrigPoints * sizeof(float));

    if (typeX == eINT) {
      // cast
      for (unsigned int i = 0; i < iNumPoints; ++i)
	pData->pXOrigData[i] = (float)(((int*)pX)[i]);
    }
    else if (typeX == eFLOAT) {
      memcpy(pData->pXOrigData, pX, iNumPoints * sizeof(float));
    }

    if (typeY == eINT) {
      // cast
      for (unsigned int i = 0; i < iNumPoints; ++i)
	pData->pYOrigData[i] = (float)(((int*)pY)[i]);
    }
    else if (typeY == eFLOAT) {
      memcpy(pData->pYOrigData, pY, iNumPoints * sizeof(float));
    }

    pData->bLayoutDirty = true;
    layer_mark_dirty(chart_layer_get_layer(layer));
  }
}

// heler struct for sorting x-axis values
typedef struct {
  float x_value;
  int index;
} ChartSortHelper;

// helper comparator for sorting x-axis values
static int cmpChartSortHelper(const void* a, const void* b) {
  return ((const ChartSortHelper*)a)->x_value - ((const ChartSortHelper*)b)->x_value;
}

// if needed, prepares data for drawing
// this is where the heavy lifting is done
static void chart_layer_update_layout(ChartLayer* layer) {
  if (layer) {
    
    // if nothing to do, return
    ChartLayerData* pData = get_chart_data(layer);
    if (!pData->bLayoutDirty)
      return;
    pData->bLayoutDirty = false;

    // clear out previously cached values
    if (pData->iNumPoints) {
      free(pData->pXData);
      free(pData->pYData);
    }
    pData->iNumPoints = 0;

    if (pData->pXOrigData && pData->pYOrigData && pData->iNumOrigPoints) {
      // figure out sort order
      ChartSortHelper* sort_order = (ChartSortHelper*) malloc(pData->iNumOrigPoints * sizeof(ChartSortHelper));
      for (unsigned int i = 0; i < pData->iNumOrigPoints; ++i)
	sort_order[i] = ((ChartSortHelper) { .x_value = pData->pXOrigData[i], .index= i });
      if (pData->typePlot != eSCATTER) {
	qsort(sort_order, pData->iNumOrigPoints, sizeof(ChartSortHelper), &cmpChartSortHelper);
      }

      // figure out sampling rate
      GRect bounds = layer_get_bounds(chart_layer_get_layer(layer));
      const unsigned int iSampling = ((pData->typePlot == eSCATTER) || (((unsigned int)bounds.size.w - (2 * pData->iMargin)) > pData->iNumOrigPoints)) ? 1 : pData->iNumOrigPoints / ((unsigned int)bounds.size.w - (2 * pData->iMargin));

      // init for cached data
      pData->iNumPoints = pData->iNumOrigPoints / iSampling;
      pData->pXData = (int*)malloc(pData->iNumPoints * sizeof(int));
      pData->pYData = (int*)malloc(pData->iNumPoints * sizeof(int));
      
      // figure out Y-scale
      float fMaxY = pData->pYOrigData[0];
      float fMinY = pData->pYOrigData[0];
      for (unsigned int i = 0; i < pData->iNumOrigPoints; i += iSampling) {
	if (pData->pYOrigData[i] > fMaxY)
	  fMaxY = pData->pYOrigData[i];
	if (pData->pYOrigData[i] < fMinY)
	  fMinY = pData->pYOrigData[i];
      }
      if (pData->fYMin != NOT_SET)
	fMinY = pData->fYMin;
      if (pData->fYMax != NOT_SET)
	fMaxY = pData->fYMax;
      const float fYScale = (float)(bounds.size.h - (2 * pData->iMargin)) / (fMaxY - fMinY); 

      // calc Y values
      for (unsigned int i = 0, j = 0; i < pData->iNumOrigPoints; i += iSampling, ++j) {
	pData->pYData[j] = bounds.size.h - ((int)(fYScale * (pData->pYOrigData[sort_order[i].index] - fMinY)) + pData->iMargin);
      }

      // x-axis position
      pData->iYAxisIntercept = bounds.size.h - ((int)(fYScale * -fMinY) + pData->iMargin);

      // calc y tick spacing
      pData->iYTicks = (int)(fYScale * exponential10(closest_log10(fMaxY - fMinY)));

      // figure out X-scale
      float fMaxX = pData->pXOrigData[0];
      float fMinX = pData->pXOrigData[0];
      float fMinXSep = (pData->iNumOrigPoints > 1) ? pData->pXOrigData[sort_order[1].index] - pData->pXOrigData[sort_order[0].index] : 0;
      for (unsigned int i = 0; i < pData->iNumOrigPoints; i += iSampling) {
	if (pData->pXOrigData[i] > fMaxX)
	  fMaxX = pData->pXOrigData[i];
	if (pData->pXOrigData[i] < fMinX)
	  fMinX = pData->pXOrigData[i];
	if (i != 0) {
	  if ((pData->pXOrigData[sort_order[i].index] - pData->pXOrigData[sort_order[i-1].index]) < fMinXSep)
	    fMinXSep = pData->pXOrigData[sort_order[i].index] - pData->pXOrigData[sort_order[i-1].index];
	}
      }
      if (pData->fXMin != NOT_SET)
	fMinX = pData->fXMin;
      if (pData->fXMax != NOT_SET)
	fMaxX = pData->fXMax;
      if (pData->typePlot != eBAR)
	fMinXSep = 0;
      const float fXScale = (float)(bounds.size.w - (2 * pData->iMargin)) / (fMaxX - fMinX + fMinXSep); 

      // calc x values
      for (unsigned int i = 0, j = 0; i < pData->iNumOrigPoints; i += iSampling, ++j) {
	pData->pXData[j] = (int)(fXScale * (pData->pXOrigData[sort_order[i].index] - fMinX + fMinXSep/2)) + pData->iMargin;
      }

      // bar width
      if (pData->typePlot == eBAR) {
	pData->iBarWidth = (int)(fXScale * fMinXSep);
	if (pData->iBarWidth > 2)
	  pData->iBarWidth -= 2;
      }

      // y-axis position
      pData->iXAxisIntercept = (int)(fXScale * -fMinX) + pData->iMargin;

      // clean-up
      pData->iPointsToDraw = 0;
      free(sort_order);
    }
  }
}

static void animation_started(Animation *animation, void *data) {
}

static void animation_stopped(Animation *animation, bool finished, void *l) {
}

// called for each frame of the animation
static void animation_update(Animation* animation, const uint32_t time_normalized) {
  ChartLayer* layer = (ChartLayer*) animation_get_context(animation);
  ChartLayerData* data = get_chart_data(layer);

  // calculate num points to draw proportionally to percent of duration elapsed
  if (time_normalized == ANIMATION_NORMALIZED_MAX)
    data->iPointsToDraw = data->iNumPoints;
  else
    data->iPointsToDraw = (int) (data->iNumPoints * ((float)time_normalized/(float)ANIMATION_NORMALIZED_MAX));
  
  // trigger re-draw
  layer_mark_dirty(chart_layer_get_layer(layer));
}

// function to draw chart
static void chart_layer_update_func(Layer* l, GContext* ctx) {
  ChartLayer* layer = (ChartLayer*)l;
  chart_layer_update_layout(layer);

  ChartLayerData* data = get_chart_data(layer);

  // handle animations
  if ((data->iNumPoints != data->iPointsToDraw) && !animation_is_scheduled(data->pAnimation)) {
    if (data->bAnimate) {
      // do this here since duration is configurable
      animation_set_duration(data->pAnimation, data->iAnimationDuration);
      // kick off animation
      animation_schedule(data->pAnimation);
    }
    else {
      // cause entire chart to be drawn
      data->iPointsToDraw = data->iNumPoints;
    }
  }
  
  GRect bounds = layer_get_bounds(l);

  // draw background
  GRect canvas = (GRect) { .origin = { 0, 0 },
			   .size = { bounds.size.w-1, bounds.size.h-1 } };
  graphics_context_set_fill_color(ctx, data->clrCanvas);
  graphics_fill_rect(ctx, canvas, 0, 0);

  // set color for rest of draw cycle
  graphics_context_set_fill_color(ctx, data->clrPlot);
  graphics_context_set_stroke_color(ctx, data->clrPlot);
  graphics_context_set_text_color(ctx, data->clrPlot);
  
  // draw frame
  if (data->bShowFrame)
    graphics_draw_rect(ctx, canvas);

  if (data->iNumPoints) {
    // x-axis
    graphics_draw_line(ctx,
		       ((GPoint) {
			 .x = data->iMargin,
			   .y = data->iYAxisIntercept }),
		       ((GPoint) { 
			 .x = bounds.size.w - data->iMargin,
			   .y = data->iYAxisIntercept }));
    
    // y-axis major ticks
    for (int i = data->iYAxisIntercept; i <= (bounds.size.h - data->iMargin); i += data->iYTicks)
      graphics_draw_line(ctx,
			 ((GPoint) {
			   .x = data->iMargin,
			     .y = i }),
			 ((GPoint) {
			   .x = data->iMargin + 4,
			     .y = i }));
    for (int i = data->iYAxisIntercept; i > data->iMargin; i -= data->iYTicks)
      graphics_draw_line(ctx,
			 ((GPoint) {
			   .x = data->iMargin,
			     .y = i }),
			 ((GPoint) {
			   .x = data->iMargin + 4,
			     .y = i }));

    // y-axis minor ticks
    for (int i = data->iYAxisIntercept + (data->iYTicks / 2); i <= (bounds.size.h - data->iMargin); i += data->iYTicks)
      graphics_draw_line(ctx,
			 ((GPoint) {
			   .x = data->iMargin,
			     .y = i }),
			 ((GPoint) {
			   .x = data->iMargin + 2,
			     .y = i }));
    for (int i = data->iYAxisIntercept - (data->iYTicks / 2); i > data->iMargin; i -= data->iYTicks)
      graphics_draw_line(ctx,
			 ((GPoint) {
			   .x = data->iMargin,
			     .y = i }),
			 ((GPoint) {
			   .x = data->iMargin + 2,
			     .y = i }));

    // y-axis
    graphics_draw_line(ctx,
		       ((GPoint) {
			 .x = data->iXAxisIntercept,
			   .y = data->iMargin }),
		       ((GPoint) { 
			 .x = data->iXAxisIntercept,
			   .y = bounds.size.h - data->iMargin }));

    // main plot
    const bool bShowPoints = (data->typePlot != eBAR) && ((data->typePlot == eSCATTER) || (data->bShowPoints && (data->iNumOrigPoints < ((unsigned int)bounds.size.w / 3))));
    const uint16_t iPointRadius = ((data->typePlot == eLINE) || (data->iNumOrigPoints < ((unsigned int)bounds.size.w / 3))) ? 3 : 2;
    for (unsigned int i = 0; i < data->iPointsToDraw; ++i) {
      if ((data->typePlot == eLINE) && (i != data->iNumPoints-1)) {
	graphics_draw_line(ctx, 
			   ((GPoint) { 
			     .x = data->pXData[i],
			       .y = data->pYData[i] }),
			   ((GPoint) {
			     .x = data->pXData[i+1],
			       .y = data->pYData[i+1] }));
      }
      else if (data->typePlot == eBAR) {
	graphics_fill_rect(ctx,
			   ((GRect) {
			     .origin = { data->pXData[i] - (data->iBarWidth / 2), data->pYData[i] },
			       .size = { data->iBarWidth, (((data->iYAxisIntercept > (bounds.size.h - data->iMargin)) ? (bounds.size.h - data->iMargin) : data->iYAxisIntercept) - data->pYData[i]) } }),
			   0,
			   GCornersAll);
      }

      if (bShowPoints) {
	graphics_fill_circle(ctx, 
			     ((GPoint) {
			       .x = data->pXData[i],
				 .y = data->pYData[i] } ), iPointRadius);
      }
    }
  }
}

///////////////////////////////////
// math helpers

static int closest_log10(float num) {
  int log = 0;
  if (num < 0)
    log = 0;
  else if (num > 1.0) {
    while (num > 10) {
      num = num / 10;
      ++log;
    }
  }
  else {
    while (num < 10) {
      num = num * 10;
      --log;
    }
  }
  
  return log;
}

static float exponential10(int exp) {
  float f = 1;
  if (exp > 0) {
    for (int i = 0; i < exp; ++i)
      f = f * 10.0;
  }
  else if (exp < 0) {
    for (int i = 0; i < -exp; ++i)
      f = f / 10.0;
  }
  return f;
}

///////////////////////////////////
