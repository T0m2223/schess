#include <schess/gen.h>
#include <schess/move.h>
#include <schess/types.h>
#include <schess/utils.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static const char *move_names[9] =
{
  "NORMAL",
  "DOUBLE_PAWN",
  "EN_PASSANT",
  "CASTLE_KING",
  "CASTLE_QUEEN",
  "PROMOTION_KNIGHT",
  "PROMOTION_BISHOP",
  "PROMOTION_ROOK",
  "PROMOTION_QUEEN",
};
const char *
move_name(enum MOVE_TYPE mt)
{
  return move_names[mt];
}

static const char *piece_names[PT_COUNT] =
{
  " ",
  "P", "N", "B", "R", "Q", "K",
  "p", "n", "b", "r", "q", "k",
};
const char *
piece_name(piece_type pt)
{
  return piece_names[pt];
}

static const char *square_names[NUM_SQUARES] =
{
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};
const char *
square_name(square sq)
{
  return square_names[sq];
}

int
parse_board(const char *board_string, board_state *board)
{
  size_t string_pos;
  size_t board_pos = 0;

  memset(board, 0, sizeof(*board));

  char c;
  unsigned file, rank;
  square sq;

  for (string_pos = 0; board_string[string_pos] != ' '; ++string_pos)
  {
    c = board_string[string_pos];

#define PARSE_REGISTER_PIECE_CASE(lit, PT) \
  case (lit): \
    { \
      file = board_pos & 0x7; \
      rank = 0x7 - (board_pos >> 3); \
      sq   = (rank * 8) + file; \
      board->bitboards[PT] |= sq2bb(sq); \
      board->types[sq] = PT; \
      ++board_pos; \
      break; \
    }

    switch (c)
    {
      PARSE_REGISTER_PIECE_CASE('P', PT_WP);
      PARSE_REGISTER_PIECE_CASE('N', PT_WN);
      PARSE_REGISTER_PIECE_CASE('B', PT_WB);
      PARSE_REGISTER_PIECE_CASE('R', PT_WR);
      PARSE_REGISTER_PIECE_CASE('Q', PT_WQ);
      PARSE_REGISTER_PIECE_CASE('K', PT_WK);
      PARSE_REGISTER_PIECE_CASE('p', PT_BP);
      PARSE_REGISTER_PIECE_CASE('n', PT_BN);
      PARSE_REGISTER_PIECE_CASE('b', PT_BB);
      PARSE_REGISTER_PIECE_CASE('r', PT_BR);
      PARSE_REGISTER_PIECE_CASE('q', PT_BQ);
      PARSE_REGISTER_PIECE_CASE('k', PT_BK);

    case '/':
      if (board_pos % 8 != 0) return 2;
      break;
    default:
      if (c > '0' && c <= '8')
      {
        board_pos += c - '0';
        break;
      }
      return 3;
    }

#undef PARSE_REGISTER_PIECE_CASE

  }

  return 0;
}

int
parse_FEN(const char *FEN, game_state *game, irreversable_state *meta)
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

  // ACTIVE
  game->active = sections[ACTIVE][0] == 'w' ? COLOR_WHITE : COLOR_BLACK;

  if (sections[EN_PASSANT][0] == '-')
  {
    game->en_passant_potential = 0;
  }
  else
  {
    square sq = ((sections[EN_PASSANT][0] - 'a') * 2) + (sections[EN_PASSANT][1] - '1');
    game->en_passant_potential = (sq2bb(sq + 8) | sq2bb(sq - 8)) & 0x000000FFFF000000;
  }

  // HALFMOVE_CLOCK
  meta->halfmove_clock = sections[HALFMOVE_CLOCK][0] - '0';
  if (sections[HALFMOVE_CLOCK][1] >= '0' && sections[HALFMOVE_CLOCK][1] <= '9')
  {
    meta->halfmove_clock *= 10;
    meta->halfmove_clock += sections[HALFMOVE_CLOCK][1] - '0';
  }

  // CASTLING
  const char *c;
  for (c = sections[CASTLING]; *c != ' '; ++c)
  {
    switch (*c)
    {
    case 'K':
      meta->castling_rights |= 0x0000000000000040;
      break;
    case 'Q':
      meta->castling_rights |= 0x0000000000000004;
      break;
    case 'k':
      meta->castling_rights |= 0x4000000000000000;
      break;
    case 'q':
      meta->castling_rights |= 0x0400000000000000;
      break;
    case '-':
    default: // TODO: maybe error?
      break;
    }
  }

  // BOARD
  const char *board_string = sections[BOARD];

  return parse_board(board_string, &game->board);
}

void
print_board(board_state *board)
{
  piece_type type;

  for (size_t r = 0; r < 8; ++r)
  {
    printf("+---+---+---+---+---+---+---+---+\n|");
    for (size_t c = 0; c < 8; ++c)
    {
      type = board->types[((7 - r) * 8) + c];
      printf(" %s |", piece_names[type]);
    }
    printf("\n");
  }
  printf("+---+---+---+---+---+---+---+---+\n");
}

void
print_moves(board_state *board, struct move_buffer *mbuf)
{
  size_t i;

  for (i = 0; i < mbuf->size; ++i)
  {
    move m = mbuf->moves[i];
    printf("[%3zu]:  ", i);
    print_move(board, m);
    printf("\n");
  }
}

void
print_move(board_state *board, move m)
{
  printf("(%s) %s -> %s (%s)  | %s",
      piece_names[board->types[m.from]],
      square_names[m.from], square_names[m.to],
      piece_names[m.capture], move_names[m.type]);
}

static enum PIECE_REL
piece_from_symbol(const char name)
{
  switch (name) {
  case 'N': return PR_N;
  case 'B': return PR_B;
  case 'R': return PR_R;
  case 'Q': return PR_Q;
  case 'K': return PR_K;
  }
  return PR_P;
}

static square
square_from_name(const char file, const char rank)
{
  return ((file - 'a') * 8) + (rank - '1');
}

int
SAN_to_squares(const char *SAN, game_state *game, square *from_out, square *to_out, piece_type *promotion)
{
  square sq1;
  size_t len = strlen(SAN);
  *promotion = PT_NONE;
  color active = game->active;

  if (len < 2) return 1;

  if (len == 2) // pawn push
  {
    *to_out = square_from_name(SAN[0], SAN[1]);
    switch (active)
    {
    case COLOR_WHITE:
      if (game->board.types[*to_out - 8] == PT_WP)
      {
        *from_out = *to_out - 8;
        return 0;
      }
      if (*to_out >= 16 && game->board.types[*to_out - 16] == PT_WP)
      {
        *from_out = *to_out - 16;
        return 0;
      }
      return 1;
    case COLOR_BLACK:
      if (game->board.types[*to_out + 8] == PT_BP)
      {
        *from_out = *to_out + 8;
        return 0;
      }
      if (*to_out < 48 && game->board.types[*to_out + 16] == PT_BP)
      {
        *from_out = *to_out + 16;
        return 0;
      }
      return 1;
    }
  }

  if (len == 3 && SAN[0] >= 'a') // promotion push
  {

  }

  if (SAN[0] >= 'a') // pawn move
  {
    if (SAN[1] == 'x') // disambiguious pawn capture
    {
      if (len < 4) return 1;
      *to_out = square_from_name(SAN[2], SAN[3]);

      if (len >= 5)
        *promotion = game->active + piece_from_symbol(SAN[4]);

      return 0;
    }

  }

}
