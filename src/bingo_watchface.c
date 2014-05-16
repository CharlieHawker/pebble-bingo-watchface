#include <pebble.h>
#include <bingo_cell.h>

// Watch face resources
static Window *window;
static BitmapLayer *grid_layer;
static GBitmap *grid_bitmap;
static BingoCell bingo_cells[9];
static const GPoint origins[9] = {
  {2,26}, {49,26}, {96,26}, {2,73}, {49,73}, {96,73}, {2,120}, {49,120}, {96,120}
};

// Pick a random layer not occupied by a time value
int random_layer() {
  int index = rand() % 8;
  BingoCell *bingo_cell = &bingo_cells[index];
  if (bingo_cell->hr_layer || bingo_cell->min_layer) {
    return random_layer();
  } else {
    return index;
  }
}


// Called every minute
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  BingoCell *bingo_cell;
  int used_numbers[9]; // This will contain the grid values and keep track of what numbers have been used

  // Choose the layer for the hour and set it
  int hr_layer_index = random_layer();
  bingo_cell = &bingo_cells[hr_layer_index];
  bingo_cell->hr_layer = true;

  if ( !clock_is_24h_style() && tick_time->tm_hour > 12 ) {
    used_numbers[0] = tick_time->tm_hour - 12;
  } else {
    used_numbers[0] = tick_time->tm_hour; // Add the hour int to array of used integers for grid
  }
  bingo_cell_set_value(bingo_cell, used_numbers[0]);

  // Choose the layer for the minute and set it
  int min_layer_index = random_layer();
  bingo_cell = &bingo_cells[min_layer_index];

  // Assign the minute if not on the hour
  if (tick_time->tm_min != 0)
  {
    bingo_cell->min_layer = true;
    used_numbers[1] = tick_time->tm_min; // Add the min int to array of used integers for grid
  }
  else
  {
    bingo_cell->min_layer = false;
    used_numbers[1] = 1 + (rand() % 98);
  }
  bingo_cell_set_value(bingo_cell, used_numbers[1]);

  // Generate set of 7 unique random numbers
  int instances, number;
  for (int i=2;i<9;i++) { // We start from 2 because 0 and 1 contain hr and min
    instances = 1;
    while (instances > 0) {
      number = 1 + (rand() % 98);
      instances = 0;
      for (int j=0;j<9;j++) {
        if (used_numbers[j] == number)
          instances++;
      }
    }
    used_numbers[i] = number;
  }

  // Now fill in the other spaces with random number
  int number_index = 2; // Starts from 2 as 0 and 1 are hr and min
  for (int k=0;k<9;k++) {
    if (k != hr_layer_index && k != min_layer_index) {
      // Clear the bitmap highlight layer if this was the last hour / min layer
      bingo_cell = &bingo_cells[k];
      if (bingo_cell->hr_layer || bingo_cell->min_layer) {
        bingo_cell_unhighlight(bingo_cell);
      }
      // Now set the value for the cell to a random value
      bingo_cell_set_value(bingo_cell, used_numbers[number_index]);
      number_index++;
    }
  }
}


void handle_init() {
  bingo_cells_load_resources();

  // Seed the random number generator
  srand(time(NULL));
  
  // Add the cells
  for (int i=0;i<9;++i) {
    // Add a bingo cell to one of the grid origins
    BingoCell *bingo_cell = &bingo_cells[i];
    bingo_cell_init(bingo_cell, origins[i]);
    layer_add_child(window_get_root_layer(window), bingo_cell->layer);
  }

  // Now add the grid above
  grid_layer = bitmap_layer_create((GRect){ .origin = {0, 0}, .size = {144, 168} });
  grid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BINGO_GRID_BLACK);
  bitmap_layer_set_bitmap(grid_layer, grid_bitmap);
  bitmap_layer_set_compositing_mode(grid_layer, GCompOpClear);

  // Add the grid layer as a child of the window layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(grid_layer));  

  // Update 2 random cells to show the time
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);

  // Subscribe to the tick timer service
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit() {
  // Unsubscribe from services
  tick_timer_service_unsubscribe();

  // Destroy the cells
  for (int i=0;i<9;i++) {
    BingoCell *bingo_cell = &bingo_cells[i];
    bitmap_layer_destroy(bingo_cell->highlight_layer);
    text_layer_destroy(bingo_cell->text_layer);
    layer_destroy(bingo_cell->layer);
  }

  // Destroy the cell highlight resource
  bingo_cells_unload_resources();

  // Destroy the grid
  bitmap_layer_destroy(grid_layer);
  gbitmap_destroy(grid_bitmap);
}

int main(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = handle_init,
    .unload = handle_deinit,
  });
  window_stack_push(window, true /* Animated */);
  app_event_loop();
  window_destroy(window);
}
