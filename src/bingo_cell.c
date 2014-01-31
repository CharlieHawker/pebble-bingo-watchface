#include <pebble.h>
#include "bingo_cell.h"

void bingo_cell_init(BingoCell *bingo_cell, GPoint pos) {
  app_log(1, "bingo_cell.c", 9, "%s", "Creating bitmap layer for cell");
  bingo_cell->layer = bitmap_layer_create((GRect){ .origin = {pos.x, pos.y}, .size = {46, 46} });
  
  app_log(1, "bingo_cell.c", 12, "%s", "Creating text layer for cell");
  bingo_cell->text_layer = text_layer_create((GRect){ .origin = {0, 5}, .size = {46, 41} });
  text_layer_set_text_alignment(bingo_cell->text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(bingo_cell->text_layer, GColorClear);

  layer_add_child(bingo_cell_get_layer(bingo_cell), text_layer_get_layer(bingo_cell->text_layer));
  app_log(1, "bingo_cell.c", 18, "%s", "Text layer added as child of cell bitmap layer");
}

Layer* bingo_cell_get_layer(BingoCell* bingo_cell) {
  return bitmap_layer_get_layer(bingo_cell->layer);
}

void bingo_cell_update(BingoCell* bingo_cell) {
  app_log(1, "bingo_cell.c", 33, "%s %s", "Bingo Cell will be assigned new value:", bingo_cell->value_string);
  text_layer_set_text(bingo_cell->text_layer, bingo_cell->value_string);
  bingo_cell_rehighlight(bingo_cell);
}

void bingo_cell_set_value(BingoCell* bingo_cell, int value) {
  app_log(1, "bingo_cell.c", 36, "%s %i %s %i", "Comparing current bingo cell value of", bingo_cell->value, "with new value of", value);
  if (bingo_cell->value != value) {
    bingo_cell->value = value;
    app_log(1, "bingo_cell.c", 39, "%s", "Cell value mismatch...");    
    snprintf(bingo_cell->value_string, 16, "%d", bingo_cell->value);
    app_log(1, "bingo_cell.c", 39, "%s", "test");   
    bingo_cell_update(bingo_cell);
  } else {
    // All we need to do is re-highlight a layer if it's already got the right value
    app_log(1, "bingo_cell.c", 45, "%s", "Cell already had correct value, rehighlighting...");
    bingo_cell_rehighlight(bingo_cell);
  }
  app_log(1, "bingo_cell.c", 48, "%s", "Bingo cell value updated successfully");
}

void bingo_cell_rehighlight(BingoCell* bingo_cell) {
  // Bold if hour, everything else normal
  if (bingo_cell->hr_layer) {
    text_layer_set_font(bingo_cell->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  } else {
    text_layer_set_font(bingo_cell->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  }

  // White text on black bg for time cells, black on clear for everything else
  if (bingo_cell->hr_layer || bingo_cell->min_layer) {
    bingo_cell_highlight(bingo_cell);
  } else {
    bingo_cell_unhighlight(bingo_cell);
  }
}

void bingo_cell_highlight(BingoCell* bingo_cell) {
  bitmap_layer_set_background_color(bingo_cell->layer, GColorBlack);  
  text_layer_set_text_color(bingo_cell->text_layer, GColorWhite);
}


void bingo_cell_unhighlight(BingoCell* bingo_cell) {
  bitmap_layer_set_background_color(bingo_cell->layer, GColorClear);  
  text_layer_set_text_color(bingo_cell->text_layer, GColorBlack);
}

void bingo_cell_destroy(BingoCell* bingo_cell) {
  bitmap_layer_destroy(bingo_cell->layer);
  text_layer_destroy(bingo_cell->text_layer);
}