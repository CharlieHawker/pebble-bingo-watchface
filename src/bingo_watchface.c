#include <pebble.h>
#include "bingo_cell.h"

PBL_APP_INFO(MY_UUID,
             "Bingo Watchface", "Charlie Hawker",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
BmpContainer grid_bg;
TextLayer title_layer;
BingoCell bingo_cells[9];

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
  int hour = tm->tm_hour;
  if ( !clock_is_24h_style() && hour > 12 ) {
    hour = hour - 12;
  }
  bingo_cell_set_value(&bingo_cells[hr_layer_index], hour);
  used_numbers[0] = hour; // Add the hour int to array of used integers for grid

  // Choose the layer for the minute and set it
  int min_layer_index = random_layer();
  bingo_cells[min_layer_index].min_layer = true;
  bingo_cell_set_value(&bingo_cells[min_layer_index], tm->tm_min);
  used_numbers[1] = tm->tm_min; // Add the min int to array of used integers for grid

  // Generate set of 7 unique random numbers
  for (int i=2;i<9;i++) { // We start from 2 because 0 and 1 contain hr and min
    int instances = 1;
    int number;
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


void handle_init(AppContextRef ctx) {
  window = window_create();

  window_stack_push(&window, true /* Animated */);
  resource_init_current_app(&APP_RESOURCES);

  // The grid outline
  bmp_init_container(RESOURCE_ID_BINGO_GRID, &grid_bg);
  layer_set_frame(&grid_bg.layer.layer, GRect(1, 1, 142, 166));
  layer_add_child(&window.layer, &grid_bg.layer.layer);

  // The text title
  title_layer = text_layer_create(GRect(2, 2, 140, 22));

  text_layer_set_background_color(&title_layer, GColorClear);
  text_layer_set_text_color(&title_layer, GColorWhite);
  text_layer_set_text_alignment(&title_layer, GTextAlignmentCenter);
  text_layer_set_text(&title_layer, "BINGO!");
  text_layer_set_font(&title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(&window.layer, &title_layer.layer);

  int pos_x;
  int pos_y;
  int offset_x;
  int offset_y;
  for (int i=0;i<9;++i) {
    offset_x = (i % 3) + 1;
    offset_y = ((i / 3) % 3) + 25;
    pos_x = ((i % 3) * 46) + offset_x;
    pos_y = (((i / 3) % 3) * 46) + offset_y;
    bingo_cells[i] = bingo_cell_create(GPoint(pos_x, pos_y));
    bingo_cell_set_value(&bingo_cells[i], 1 + (rand() % 98));
    layer_add_child(&grid_bg.layer.layer, &bingo_cells[i].layer.layer);
  }
}

void handle_deinit(AppContextRef ctx) {
  // Unsubscribe from services
  tick_timer_service_subscribe();

  // De-init resources
  bmp_deinit_container(&grid_bg);

  // Destroy the layers
  text_layer_destroy(title_layer);
  for (int i=0;i<9;++i) {
    bingo_layer_destroy(bingo_cells[i]);
  }
  window_destroy(window);
}

void main(void) {
  handle_init();
  app_event_loop();
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  app_event_loop();
  handle_deinit();  
}
