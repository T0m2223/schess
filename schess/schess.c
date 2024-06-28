#include <schess/gen.h>
#include <schess/move.h>
#include <schess/types.h>
#include <schess/utils.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  board_state board;
  irreversable_state meta;


  parse_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &board, &meta);
  printf("Board read in:\n");
  print_board(&board);


  struct move_buffer *mbuf = move_buffer_create(1);
  move_gen_init_LUTs();
  generate_moves(&board, meta, PT_WP, PT_BP, mbuf);
  printf("Possible moves:\n");
  print_moves(&board, mbuf);


  move_make(mbuf->moves, &board, &meta);
  printf("After making move 0:\n");
  print_board(&board);

  return EXIT_SUCCESS;
}
