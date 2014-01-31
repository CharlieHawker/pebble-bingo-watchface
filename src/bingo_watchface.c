#include <pebble.h>
#include <bingo_cell.h>

static Window *window;
static BitmapLayer *grid_layer;
static GBitmap *grid_bitmap;
static TextLayer *title_layer;
static BingoCell bingo_cells[9];

// Pick a random layer not occupied by a time value
int random_layer() {
  int index = rand() % 8;
  if (bingo_cells[index].hr_layer || bingo_cells[index].min_layer) {
    return random_layer();
  } else {
    return index;
  }
}

// Called every minute
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  int used_numbers[9]; // This will contain the grid values and keep track of what numbers have been used

  // Choose the layer for the hour and set it
  int hr_layer_index = random_layer();
  bingo_cells[hr_layer_index].hr_layer = true;

  int hour = tick_time->tm_hour;
  if ( !clock_is_24h_style() && hour > 12 ) {
    hour = hour - 12;
  }
  bingo_cell_set_value(&bingo_cells[hr_layer_index], hour);
  used_numbers[0] = hour; // Add the hour int to array of used integers for grid

  // Choose the layer for the minute and set it
  int min_layer_index = random_layer();
  bingo_cells[min_layer_index].min_layer = true;
  bingo_cell_set_value(&bingo_cells[min_layer_index], tick_time->tm_min);
  used_numbers[1] = tick_time->tm_min; // Add the min int to array of used integers for grid

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

  // Now fill in the other spaces with random numbers
  int number_index = 2; // Starts from 2 as 0 and 1 are hr and min
  for (int k=0;k<9;k++) {
    if (k != hr_layer_index && k != min_layer_index) {
      // Need to reset the hr & min layer booleans to false for these layers
      bingo_cells[k].hr_layer = false;
      bingo_cells[k].min_layer = false;
      // Now set the value for the cell to a random value
      bingo_cell_set_value(&bingo_cells[k], used_numbers[number_index]);
      number_index++;
    }
  }
}


void handle_init() {
  window = window_create();

  window_stack_push(window, true /* Animated */);

  app_log(1, "bingo_watchface.c", 76, "%s", "The window was generated");

  // Get information about the root layer
  Layer *window_layer = window_get_root_layer(window);
  //GRect bounds = layer_get_frame(window_layer);

  // Now add the grid
  grid_layer = bitmap_layer_create((GRect){ .origin = {1, 1}, .size = {142, 166} });
  grid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BINGO_GRID);
  bitmap_layer_set_bitmap(grid_layer, grid_bitmap);

  app_log(1, "bingo_watchface.c", 87, "%s", "The grid layer was generated");

  // Add the grid layer as a child of the window layer
  layer_add_child(window_layer, bitmap_layer_get_layer(grid_layer));

  app_log(1, "bingo_watchface.c", 92, "%s", "The grid layer was assigned to the window layer");

  // Add the cells
  int offset_x, offset_y, pos_x, pos_y;
  for (int i=0;i<9;++i) {
    offset_x = (i % 3) + 2;
    offset_y = ((i / 3) % 3) + 26;
    pos_x = ((i % 3) * 46) + offset_x;
    pos_y = (((i / 3) % 3) * 46) + offset_y;

    app_log(1, "bingo_watchface.c", 101, "%s %i %s%i %s%i", "Generating cell", i+1, "of 9 at position: x=", pos_x, "y=", pos_y);
    bingo_cell_init(&bingo_cells[i], (GPoint){ pos_x, pos_y });
    bingo_cell_set_value(&bingo_cells[i], 1 + (rand() % 98));    

    layer_add_child(window_layer, bingo_cell_get_layer(&bingo_cells[i]));
    app_log(1, "bingo_watchface.c", 106, "%s", "The cell was assigned to the window layer");
  }

  // Set the text title
  title_layer = text_layer_create((GRect){ .origin = {2, 2}, .size = {140, 22} });
  text_layer_set_background_color(title_layer, GColorClear);
  text_layer_set_text_color(title_layer, GColorWhite);
  text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
  text_layer_set_text(title_layer, "BINGO!");
  text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  // Add the title layer as a child of the window layer
  layer_add_child(window_layer, text_layer_get_layer(title_layer));

  // Subscribe to the tick timer service
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit() {
  // Unsubscribe from services
  tick_timer_service_unsubscribe();

  // Destroy the layers
  text_layer_destroy(title_layer);
  bitmap_layer_destroy(grid_layer);
  gbitmap_destroy(grid_bitmap);

  // Destroy the cells
  for (int i=0;i<9;++i) {
    bingo_cell_destroy(&bingo_cells[i]);
  }

  // Destroy the window
  window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();  
}
