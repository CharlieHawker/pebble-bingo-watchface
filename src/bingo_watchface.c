#include <pebble.h>
#include <bingo_cell.h>

static Window *window;
static BitmapLayer *grid_layer;
static GBitmap *grid_bitmap;
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
  int min = tick_time->tm_min;
  if (min != 0)
  {
    bingo_cells[min_layer_index].min_layer = true;
    used_numbers[1] = min; // Add the min int to array of used integers for grid
  }
  else
  {
    bingo_cells[min_layer_index].min_layer = false;
    used_numbers[1] = 1 + (rand() % 98);
  }
  bingo_cell_set_value(&bingo_cells[min_layer_index], used_numbers[1]);

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

  // Get information about the root layer
  Layer *window_layer = window_get_root_layer(window);
  //GRect bounds = layer_get_frame(window_layer);

  // Now add the grid
  grid_layer = bitmap_layer_create((GRect){ .origin = {0, 0}, .size = {144, 168} });
  grid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BINGO_GRID);
  bitmap_layer_set_bitmap(grid_layer, grid_bitmap);

  // Add the cells
  GPoint origins[9] = {
    (GPoint){2,26},
    (GPoint){49,26},
    (GPoint){96,26},
    (GPoint){2,73},
    (GPoint){49,73},
    (GPoint){96,73},
    (GPoint){2,120},
    (GPoint){49,120},
    (GPoint){96,120}
  };
  for (int i=0;i<9;++i) {
    // Add a bingo cell to one of the grid origins
    app_log(1, "bingo_watchface.c", 106, "Creating cell at: %i,%i", origins[i].x, origins[i].y);
    bingo_cell_init(&bingo_cells[i], origins[i]);
    layer_add_child(bitmap_layer_get_layer(grid_layer), bingo_cell_get_layer(&bingo_cells[i]));
  }

  // Update 2 random cells to show the time
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);

  // Subscribe to the tick timer service
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  // Add the grid layer as a child of the window layer
  layer_add_child(window_layer, bitmap_layer_get_layer(grid_layer));
}

void handle_deinit() {
  // Unsubscribe from services
  tick_timer_service_unsubscribe();

  // Destroy the layers
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
