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

static int
parse_board(const char *board_string, board_state *board_out, const char **string_pos_out)
{
  size_t string_pos;
  square board_pos;
  board_state board = { 0 };

  char c;
  unsigned file, rank;
  square sq;

  // error catching variables
  unsigned last_slash = 0;
  size_t last_digit = 71; // max board_string size

  for (string_pos = 0, board_pos = a1; board_pos < NUM_SQUARES; ++string_pos)
  {
    c = board_string[string_pos];

    switch (c)
    {

#define PARSE_BOARD_REGISTER_PIECES_CASE(lit, PT) \
  case (lit): \
    { \
      file = board_pos & 7; \
      rank = 7 - (board_pos >> 3); \
      sq   = (rank * 8) + file; \
      board.bitboards[PT] |= sq2bb(sq); \
      board.types[sq] = PT; \
      ++board_pos; \
      break; \
    }
      PARSE_BOARD_REGISTER_PIECES_CASE('P', PT_WP);
      PARSE_BOARD_REGISTER_PIECES_CASE('N', PT_WN);
      PARSE_BOARD_REGISTER_PIECES_CASE('B', PT_WB);
      PARSE_BOARD_REGISTER_PIECES_CASE('R', PT_WR);
      PARSE_BOARD_REGISTER_PIECES_CASE('Q', PT_WQ);
      PARSE_BOARD_REGISTER_PIECES_CASE('K', PT_WK);
      PARSE_BOARD_REGISTER_PIECES_CASE('p', PT_BP);
      PARSE_BOARD_REGISTER_PIECES_CASE('n', PT_BN);
      PARSE_BOARD_REGISTER_PIECES_CASE('b', PT_BB);
      PARSE_BOARD_REGISTER_PIECES_CASE('r', PT_BR);
      PARSE_BOARD_REGISTER_PIECES_CASE('q', PT_BQ);
      PARSE_BOARD_REGISTER_PIECES_CASE('k', PT_BK);
#undef PARSE_BOARD_REGISTER_PIECES_CASE

    case '/':
      // invalid slash position
      if (board_pos != last_slash + 8) return 2;

      last_slash = board_pos;
      break;
    default:
      if (c > '0' && c <= '8')
      {
        // two digits in a row
        if (string_pos == last_digit + 1) return 3;

        last_digit = string_pos;
        board_pos += c - '0';
        break;
      }
      // undefined character
      return 1;
    }

    // missing slash
    if (board_pos > last_slash + 8) return 4;
  }

  *board_out = board;
  *string_pos_out = &board_string[string_pos];
  return 0;
}

static int
parse_active(const char *active_string, game_state *game_out, const char **string_pos_out)
{
  switch (*active_string)
  {
  case 'w':
    game_out->active = COLOR_WHITE;
    break;
  case 'b':
    game_out->active = COLOR_BLACK;
    break;
  default:
    return 1;
  }

  *string_pos_out = &active_string[1];
  return 0;
}

static int
parse_castling(const char *castling_string, irreversable_state *meta_out, const char **string_pos_out)
{
  size_t string_pos = 0;
  bitboard castling_rights = 0;

  if (castling_string[string_pos] == '-')
  {
    meta_out->castling_rights = castling_rights;
    *string_pos_out = &castling_string[1];
    return 0;
  }

  if (castling_string[string_pos] == 'K')
  {
    castling_rights |= 0x0000000000000040;
    ++string_pos;
  }
  if (castling_string[string_pos] == 'Q')
  {
    castling_rights |= 0x0000000000000004;
    ++string_pos;
  }
  if (castling_string[string_pos] == 'k')
  {
    castling_rights |= 0x4000000000000000;
    ++string_pos;
  }
  if (castling_string[string_pos] == 'q')
  {
    castling_rights |= 0x0400000000000004;
    ++string_pos;
  }

  // no valid character found
  if (!string_pos) return 1;

  meta_out->castling_rights = castling_rights;
  *string_pos_out = &castling_string[string_pos];
  return 0;
}

static int
is_valid_square_name(char file, char rank)
{
  return (file >= 'a' && file <= 'h' &&
          rank >= '1' && rank <= '8');
}
static square
square_from_name(char file, char rank)
{
  return ((rank - '1') * 8) + (file - 'a');
}
static int
parse_en_passant(const char *en_passant_string, game_state *game_out, const char **string_pos_out)
{
  if (en_passant_string[0] == '-')
  {
    game_out->en_passant_potential = 0;
    *string_pos_out = &en_passant_string[1];
    return 0;
  }

  if (!is_valid_square_name(en_passant_string[0], en_passant_string[1]))
      return 1;

  game_out->en_passant_potential = sq2bb(square_from_name(en_passant_string[0], en_passant_string[1]));
  *string_pos_out = &en_passant_string[2];
  return 0;
}

static int
parse_halfmove_clock(const char *halfmove_clock_string, irreversable_state *meta_out, const char **string_pos_out)
{
  if (halfmove_clock_string[0] < '0' || halfmove_clock_string[0] > '9') return 1;
  meta_out->halfmove_clock = halfmove_clock_string[0] - '0';
  *string_pos_out = &halfmove_clock_string[1];

  if (halfmove_clock_string[1] < '0' || halfmove_clock_string[1] > '9') return 0;
  meta_out->halfmove_clock *= 10;
  meta_out->halfmove_clock += halfmove_clock_string[1] - '0';
  *string_pos_out = &halfmove_clock_string[2];

  return 0;
}

static int
parse_fullmove_number(const char *fullmove_numer_string, game_state *game_out, const char **string_pos_out)
{
  size_t i;

  if (fullmove_numer_string[0] < '0' || fullmove_numer_string[0] > '9') return 1;
  game_out->fullmove = fullmove_numer_string[0] - '0';
  *string_pos_out = &fullmove_numer_string[1];

  for (i = 1; i < 4; ++i) // max 4 digit fullmove number
  {
    if (fullmove_numer_string[i] < '0' || fullmove_numer_string[i] > '9') break;
    game_out->fullmove *= 10;
    game_out->fullmove += fullmove_numer_string[i] - '0';
    *string_pos_out = &fullmove_numer_string[i + 1];
  }

  return 0;
}

int
parse_FEN(const char *FEN, game_state *game_out, irreversable_state *meta_out)
{
  game_state game = { 0 };
  irreversable_state meta = { 0 };
  const char *string_ptr = FEN;
  int err;

  err = parse_board(string_ptr, &game.board, &string_ptr);
  if (err) return err;
  if (*string_ptr++ != ' ') return 1;

  err = parse_active(string_ptr, &game, &string_ptr);
  if (err) return err;
  if (*string_ptr++ != ' ') return 1;

  err = parse_castling(string_ptr, &meta, &string_ptr);
  if (err) return err;
  if (*string_ptr++ != ' ') return 1;

  err = parse_en_passant(string_ptr, &game, &string_ptr);
  if (err) return err;

  // optional fields
  do
  {
    if (*string_ptr   != '\0') break;
    if (*string_ptr++ != ' ') return 1;

    err = parse_halfmove_clock(string_ptr, &meta, &string_ptr);
    if (err) return err;
    if (*string_ptr   != '\0') break;
    if (*string_ptr++ != ' ') return 1;

    err = parse_fullmove_number(string_ptr, &game, &string_ptr);
    if (err) return err;
    if (*string_ptr != '\0') return 1;
  } while (0);

  *game_out = game;
  *meta_out = meta;
  return 0;
}

void
print_board(board_state *board)
{
  size_t r, c;
  piece_type type;

  for (r = 0; r < 8; ++r)
  {
    printf("+---+---+---+---+---+---+---+---+\n|");
    for (c = 0; c < 8; ++c)
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


int
parse_SAN(const char *SAN, game_state *game, irreversable_state meta, square *from_out, square *to_out, piece_type *promotion_out)
{
  square from = 0, to;
  piece_type piece, promotion = PT_NONE;
  size_t len = strlen(SAN), i;
  struct move_buffer *mbuf;
  irreversable_state meta_dump;
  move m;

  if (len < 2) return 1;

  if (SAN[len - 1] == '+' ||
      SAN[len - 1] == '#')
    --len;

  if (SAN[0] >= 'a' && SAN[0] <= 'h') // pawn move
  {
    switch (len)
    {
    case 2: // push
      if (!is_valid_square_name(SAN[0], SAN[1]))
        return 1;

      to = square_from_name(SAN[0], SAN[1]);

      switch (game->active)
      {
      case COLOR_WHITE:
        if (game->board.types[to - 8] == PT_WP)
          from = to - 8;
        else if (to >= 16 && game->board.types[to - 16] == PT_WP)
          from = to - 16;
        else return 1;
        break;
      case COLOR_BLACK:
        if (game->board.types[to + 8] == PT_BP)
          from = to + 8;
        else if (to < 48 && game->board.types[to + 16] == PT_BP)
          from = to + 16;
        else return 1;
        break;
      default:
        return 1;
      }
      break;
    case 4:
      if (SAN[1] != 'x') // capture
      {
        if (!is_valid_square_name(SAN[2], SAN[3]))
          return 1;
        to   = square_from_name(SAN[2], SAN[3]);
        from = square_from_name(SAN[1], SAN[3]);

        switch (game->active)
        {
        case COLOR_WHITE:
          from -= 8;
          break;
        case COLOR_BLACK:
          from += 8;
          break;
        }
      }
      else // no capture, promotion
      {
        if (!is_valid_square_name(SAN[0], SAN[1]) ||
            SAN[2] != '=' ||
            piece_from_symbol(SAN[3]) == PR_P)
          return 1;

        to = square_from_name(SAN[0], SAN[1]);

        switch (game->active)
        {
        case COLOR_WHITE:
          if (game->board.types[to - 8] == PT_WP)
            from = to - 8;
          else return 1;
          break;
        case COLOR_BLACK:
          if (game->board.types[to + 8] == PT_BP)
            from = to + 8;
          else return 1;
          break;
        }

        promotion = game->active + piece_from_symbol(SAN[3]);
      }
      break;
    case 5:
      if (SAN[1] == 'x') // capture, promotion, disambiguous
      {
        if (!is_valid_square_name(SAN[2], SAN[3]) || piece_from_symbol(SAN[4]) == PR_P)
          return 1;

        to   = square_from_name(SAN[2], SAN[3]);
        from = square_from_name(SAN[1], SAN[3]);

        switch (game->active)
        {
        case COLOR_WHITE:
          from -= 8;
          break;
        case COLOR_BLACK:
          from += 8;
          break;
        }

        promotion = game->active + piece_from_symbol(SAN[4]);
      }
      else if (SAN[2] == 'x') // capture
      {
        if (!is_valid_square_name(SAN[0], SAN[1]) ||
            !is_valid_square_name(SAN[3], SAN[4]))
          return 1;

        from = square_from_name(SAN[0], SAN[1]);
        to   = square_from_name(SAN[3], SAN[4]);
      }
      else return 1;
      break;
    case 6: // capture, promotion
      if (SAN[1] != 'x' || SAN[4] != '=' ||
          !is_valid_square_name(SAN[2], SAN[3]) ||
          piece_from_symbol(SAN[5]) == PR_P)
        return 1;

      from = square_from_name(SAN[0], SAN[3]);
      to   = square_from_name(SAN[2], SAN[3]);

      switch (game->active)
      {
      case COLOR_WHITE:
        from -= 8;
        break;
      case COLOR_BLACK:
        from += 8;
        break;
      }

      promotion = game->active + piece_from_symbol(SAN[5]);
      break;
    default:
      return 1;
    }
  }
  else // piece move
  {
    if (piece_from_symbol(SAN[0]) == PR_P || len == 2) return 1;
    piece = game->active + piece_from_symbol(SAN[0]);

    move_gen_init_LUTs();
    mbuf = move_buffer_create(1);
    generate_moves(game, meta, mbuf);

    switch (len)
    {
    case 3:
      if (!is_valid_square_name(SAN[1], SAN[2]))
        goto parse_SAN_fail_free_mbuf;

      to = square_from_name(SAN[1], SAN[2]);

      for (i = 0; i < mbuf->size; ++i)
      {
        m = mbuf->moves[i];
        if (m.to != to ||
            game->board.types[m.from] != piece ||
            game->board.types[m.to]   != PT_NONE)
          continue;

        move_make(&m, game, &meta_dump);
        if (is_board_legal(&game->board, game->active))
        {
          from = m.from;
          move_unmake(&m, game);
          break;
        }
        move_unmake(&m, game);
      }

      if (i == mbuf->size) goto parse_SAN_fail_free_mbuf;
      break;
    case 4:
      if (!is_valid_square_name(SAN[2], SAN[3]))
        goto parse_SAN_fail_free_mbuf;

      to = square_from_name(SAN[2], SAN[3]);
      if (SAN[1] == 'x') // capture, disambiguous
      {
        for (i = 0; i < mbuf->size; ++i)
        {
          m = mbuf->moves[i];
          if (m.to != to ||
              game->board.types[m.from] != piece ||
              game->board.types[m.to]   == PT_NONE)
            continue;

          move_make(&m, game, &meta_dump);
          if (is_board_legal(&game->board, game->active))
          {
            from = m.from;
            move_unmake(&m, game);
            break;
          }
          move_unmake(&m, game);
        }

        if (i == mbuf->size) goto parse_SAN_fail_free_mbuf;
      }
      else // piece move, half ambiguous
      {
        if (SAN[1] >= 'a' && SAN[1] <= 'h') // file hint
        {
          for (i = 0; i < mbuf->size; ++i)
          {
            m = mbuf->moves[i];
            if (m.to != to || (m.from >> 3) != SAN[1] - 'a' ||
                game->board.types[m.from] != piece)
              continue;

            move_make(&m, game, &meta_dump);
            if (is_board_legal(&game->board, game->active))
            {
              from = m.from;
              move_unmake(&m, game);
              break;
            }
            move_unmake(&m, game);
          }

          if (i == mbuf->size) goto parse_SAN_fail_free_mbuf;
        }
        else if (SAN[1] >= '1' && SAN[1] <= '8') // rank hint
        {
          for (i = 0; i < mbuf->size; ++i)
          {
            m = mbuf->moves[i];
            if (m.to != to || (m.from & 7) != SAN[1] - '1' ||
                game->board.types[m.from] != piece)
              continue;

            move_make(&m, game, &meta_dump);
            if (is_board_legal(&game->board, game->active))
            {
              from = m.from;
              move_unmake(&m, game);
              break;
            }
            move_unmake(&m, game);
          }

          if (i == mbuf->size) goto parse_SAN_fail_free_mbuf;
        }
        else goto parse_SAN_fail_free_mbuf;
      }
      break;
    case 5:
      if (!is_valid_square_name(SAN[3], SAN[4]))
        goto parse_SAN_fail_free_mbuf;

      to = square_from_name(SAN[3], SAN[4]);

      if (SAN[2] == 'x') // capture, half disambiguous
      {
        if (SAN[1] >= 'a' && SAN[1] <= 'h') // file hint
        {
          for (i = 0; i < mbuf->size; ++i)
          {
            m = mbuf->moves[i];
            if (m.to != to || (m.from >> 3) != SAN[1] - 'a' ||
                game->board.types[m.from] != piece ||
                game->board.types[m.to]   == PT_NONE)
              continue;

            move_make(&m, game, &meta_dump);
            if (is_board_legal(&game->board, game->active))
            {
              from = m.from;
              move_unmake(&m, game);
              break;
            }
            move_unmake(&m, game);
          }

          if (i == mbuf->size) goto parse_SAN_fail_free_mbuf;
        }
        else if (SAN[1] >= '1' && SAN[1] <= '8') // rank hint
        {
          for (i = 0; i < mbuf->size; ++i)
          {
            m = mbuf->moves[i];
            if (m.to != to || (m.from & 7) != SAN[1] - '1' ||
                game->board.types[m.from] != piece ||
                game->board.types[m.to]   == PT_NONE)
              continue;

            move_make(&m, game, &meta_dump);
            if (is_board_legal(&game->board, game->active))
            {
              from = m.from;
              move_unmake(&m, game);
              break;
            }
            move_unmake(&m, game);
          }

          if (i == mbuf->size) goto parse_SAN_fail_free_mbuf;
        }
        else goto parse_SAN_fail_free_mbuf;
      }
      else
      {
        if (!is_valid_square_name(SAN[1], SAN[2]))
          goto parse_SAN_fail_free_mbuf;

        from = square_from_name(SAN[1], SAN[2]);
      }
      break;
    case 6: // capture, ambiguous
      if (!is_valid_square_name(SAN[1], SAN[2]) ||
          !is_valid_square_name(SAN[4], SAN[5]) ||
          SAN[3] != 'x')
        goto parse_SAN_fail_free_mbuf;

      from = square_from_name(SAN[1], SAN[2]);
      to   = square_from_name(SAN[4], SAN[5]);
      break;
    default:
parse_SAN_fail_free_mbuf:
      move_buffer_destroy(mbuf);
      return 1;
    }

    move_buffer_destroy(mbuf);
  }

  *to_out = to;
  *from_out = from;
  *promotion_out = promotion;
  return 0;
}
