#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "types.h"

int
parse_FEN(const char *FEN, board_state *board, irreversable_state *meta)
{
  size_t i, section = 0;
  const char *sections[6];
  enum SECTIONS { BOARD, ACTIVE, CASTLING, EN_PASSANT, HALFMOVE_CLOCK, FULLMOVE_NUMBER };

  for (i = 0; i < strlen(FEN); ++i)
  {
    if (FEN[i] == ' ') continue;

    sections[section++] = &FEN[i];
    for (; i < strlen(FEN) && FEN[i] != ' '; ++i);
  }

  if (section != 6) return 1;

  memset(meta, 0, sizeof(*meta));
  meta->halfmove_clock = sections[HALFMOVE_CLOCK][0] - '0';
  if (sections[HALFMOVE_CLOCK][1] >= '0' && sections[HALFMOVE_CLOCK][1] <= '9')
  {
    meta->halfmove_clock *= 10;
    meta->halfmove_clock += sections[HALFMOVE_CLOCK][1] - '0';
  }

  const char *board_string = sections[BOARD];

  size_t string_pos;
  size_t board_pos = 0;

  memset(board, 0, sizeof(*board));
  for (string_pos = 0; board_string[string_pos] != ' '; ++string_pos)
  {
    char c = board_string[string_pos];
    switch (c)
    {
    case 'P':
      board->bitboards[PT_WP] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_WP;
      break;
    case 'N':
      board->bitboards[PT_WN] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_WN;
      break;
    case 'B':
      board->bitboards[PT_WB] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_WB;
      break;
    case 'R':
      board->bitboards[PT_WR] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_WR;
      break;
    case 'Q':
      board->bitboards[PT_WQ] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_WQ;
      break;
    case 'K':
      board->bitboards[PT_WK] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_WK;
      break;
    case 'p':
      board->bitboards[PT_BP] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_BP;
      break;
    case 'n':
      board->bitboards[PT_BN] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_BN;
      break;
    case 'b':
      board->bitboards[PT_BB] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_BB;
      break;
    case 'r':
      board->bitboards[PT_BR] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_BR;
      break;
    case 'q':
      board->bitboards[PT_BQ] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_BQ;
      break;
    case 'k':
      board->bitboards[PT_BK] |= 1 << board_pos;
      board->types[63 - board_pos++] = PT_BK;
      break;
    case '/':
      if (board_pos % 8 != 0) return 2;
      break;
    default:
      if (c > '0' && c <= '8')
        board_pos += c - '0';
      break;
      return 3;
    }
  }

  return 0;
}

void
print_board(board_state *board)
{
  size_t i = 0;
  for (size_t c = 0; c < 8; ++c)
  {
    for (size_t r = 0; r < 8; ++r)
      printf("%d ", board->types[i++]);

    printf("\n");
  }
}
