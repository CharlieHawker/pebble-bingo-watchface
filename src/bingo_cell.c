#include <pebble.h>
#include "bingo_cell.h"

BingoCell bingo_cell;

void bingo_cell_create(GPoint pos) {
  bingo_cell->hr_layer = false;
  bingo_cell->min_layer = false;

  bingo_cell->layer = bitmap_layer_create(GRect(pos.x, pos.y, 46, 46));
  bingo_cell->text_layer = text_layer_create(GRect(0, 5, 46, 41));
  text_layer_set_text_alignment(&bingo_cell->text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(&bingo_cell->text_layer, GColorClear);

  layer_add_child(&bingo_cell->layer.layer, &bingo_cell->text_layer.layer);
}

void bingo_cell_destroy(BingoCell* bingo_cell) {
  bitmap_layer_destroy(bingo_cell->layer);
  text_layer_destroy(bingo_cell->text_layer);
}

void bingo_cell_update(BingoCell* bingo_cell) {
  text_layer_set_text(&bingo_cell->text_layer, &bingo_cell->value_string);
  bingo_cell_rehighlight(bingo_cell);
}

void bingo_cell_set_value(BingoCell* bingo_cell, int16_t value) {
  if (bingo_cell->value != value) {
    bingo_cell->value = value;
    snprintf(&bingo_cell->value_string, sizeof(&bingo_cell->value_string), "%d", bingo_cell->value);
    bingo_cell_update(bingo_cell);
  } else {
    // All we need to do is re-highlight a layer if it's already got the right value
    bingo_cell_rehighlight(bingo_cell);
  }
}

void bingo_cell_rehighlight(BingoCell* bingo_cell) {
  // Bold if hour, everything else normal
  if (bingo_cell->hr_layer) {
    text_layer_set_font(&bingo_cell->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_30_BLACK));
  } else {
    text_layer_set_font(&bingo_cell->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  }

  // White text on black bg for time cells, black on clear for everything else
  if (bingo_cell->hr_layer || bingo_cell->min_layer) {
    bingo_cell_highlight(bingo_cell);
  } else {
    bingo_cell_unhighlight(bingo_cell);
  }
}

void bingo_cell_highlight(BingoCell* bingo_cell) {
  bitmap_layer_set_background_color(&bingo_cell->layer, GColorBlack);  
  text_layer_set_text_color(&bingo_cell->text_layer, GColorWhite);
}


void bingo_cell_unhighlight(BingoCell* bingo_cell) {
  bitmap_layer_set_background_color(&bingo_cell->layer, GColorClear);  
  text_layer_set_text_color(&bingo_cell->text_layer, GColorBlack);
}