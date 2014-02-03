#include <pebble.h>
#include <bingo_cell.h>

void bingo_cell_init(BingoCell *bingo_cell, GPoint pos) {
  // Set the parent layer up
  bingo_cell->layer = layer_create((GRect){ .origin = {pos.x, pos.y}, .size = {46, 46} });
  
  // Set the highlight layer up
  bingo_cell->highlight_layer = bitmap_layer_create((GRect) { .origin = {0,0}, .size = {46,46} });
  bitmap_layer_set_bitmap(bingo_cell->highlight_layer, cell_active_bitmap);
  bingo_cell_unhighlight(bingo_cell); // Sets defaults for hr / min layer booleans and hides highlight layer

  // Set the text layer up
  bingo_cell->text_layer = text_layer_create((GRect){ .origin = {0, 5}, .size = {46, 41} });

  // Set text defaults
  text_layer_set_text_alignment(bingo_cell->text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(bingo_cell->text_layer, GColorClear);

  // Add the layers
  layer_add_child(bingo_cell->layer, bitmap_layer_get_layer(bingo_cell->highlight_layer));
  layer_add_child(bingo_cell->layer, text_layer_get_layer(bingo_cell->text_layer));
}


void bingo_cell_update(BingoCell* bingo_cell) {
  // Set some defaults
  text_layer_set_font(bingo_cell->text_layer, bingo_font_normal);
  text_layer_set_text_color(bingo_cell->text_layer, GColorBlack);

  // Override some settings if this is an hour or minute cell
  if (bingo_cell->hr_layer || bingo_cell->min_layer) {
    bingo_cell_highlight(bingo_cell);
  }

  // Finally add the new value
  text_layer_set_text(bingo_cell->text_layer, bingo_cell->value_string);
}

void bingo_cell_highlight(BingoCell* bingo_cell) {
  text_layer_set_text_color(bingo_cell->text_layer, GColorWhite);
  if (bingo_cell->hr_layer) {
    text_layer_set_font(bingo_cell->text_layer, bingo_font_extra_bold);
  }
  layer_set_hidden(bitmap_layer_get_layer(bingo_cell->highlight_layer), false);
}

void bingo_cell_unhighlight(BingoCell* bingo_cell) {
  bingo_cell->hr_layer = false;
  bingo_cell->min_layer = false;
  layer_set_hidden(bitmap_layer_get_layer(bingo_cell->highlight_layer), true);
}

void bingo_cell_set_value(BingoCell* bingo_cell, int value) {
  bingo_cell->value = value;
  snprintf(bingo_cell->value_string, 16, "%d", bingo_cell->value);
  bingo_cell_update(bingo_cell);
}

Layer* bingo_cell_get_layer(BingoCell* bingo_cell) {
  return bingo_cell->layer;
}

void bingo_cell_destroy(BingoCell* bingo_cell) {
  bitmap_layer_destroy(bingo_cell->highlight_layer);
  text_layer_destroy(bingo_cell->text_layer);
  layer_destroy(bingo_cell->layer);
}

void bingo_cells_load_resources() {
  bingo_font_normal = fonts_load_custom_font(resource_get_handle((RESOURCE_ID_OPEN_SANS_28_REGULAR)));
  bingo_font_extra_bold = fonts_load_custom_font(resource_get_handle((RESOURCE_ID_OPEN_SANS_28_EXTRA_BOLD)));
  cell_active_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CELL_ACTIVE_HIGHLIGHT);
}

void bingo_cells_unload_resources() {
  free(bingo_font_normal);
  free(bingo_font_extra_bold);
  gbitmap_destroy(cell_active_bitmap);
}