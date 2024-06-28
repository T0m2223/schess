#include <schess/gen.h>
#include <schess/move.h>
#include <schess/types.h>
#include <schess/utils.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  board_state board;
  irreversable_state meta;


  parse_FEN(argv[1], &board, &meta);
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
