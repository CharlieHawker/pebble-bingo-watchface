typedef struct {
  BitmapLayer layer;
  TextLayer text_layer;
  bool hr_layer;
  bool min_layer;
  int16_t value;
  char value_string;
} BingoCell;

void bingo_cell_create(GPoint pos);
void bingo_cell_destroy(BingoCell* bingo_cell);
void bingo_cell_set_value(BingoCell* bingo_cell, int16_t value);
void bingo_cell_update(BingoCell* bingo_cell);
void bingo_cell_rehighlight(BingoCell* bingo_cell);
void bingo_cell_highlight(BingoCell* bingo_cell);
void bingo_cell_unhighlight(BingoCell* bingo_cell);