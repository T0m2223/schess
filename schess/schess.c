#include <errno.h>
#include <schess/gen.h>
#include <schess/move.h>
#include <schess/search.h>
#include <schess/types.h>
#include <schess/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  printf("SCHESS ENGINE by Kilian Chung\n");
  move_gen_init_LUTs();

  if (argc == 1) return EXIT_SUCCESS;
  if (argc != 3) return EXIT_FAILURE;

  FILE *fp = fopen (argv[1], "rb");
  size_t length;
  char *FEN = 0;

  if (!fp)
  {
    fprintf(stderr, "Error opening %s: %s\n", argv[1], strerror(errno));
    return EXIT_FAILURE;
  }

  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  FEN = malloc(length);
  if (!FEN)
  {
    fprintf(stderr, "Error allocating string buffer: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  fread(FEN, 1, length, fp);
  fclose (fp);

  unsigned depth = strtoul(argv[2], NULL, 10);

  game_state game;
  irreversable_state meta;

  parse_FEN(FEN, &game, &meta);
  move best = search_best_move(&game, meta, depth);
  print_move(&game.board, best);
  printf("\n");

  return EXIT_SUCCESS;
}
