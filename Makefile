SRC_DIR := schess
OBJ_DIR := obj
TARGET_DIR := target
LUT_DIR := $(TARGET_DIR)/LUTs
TEST_SRC_DIR := test
TEST_OBJ_DIR := $(OBJ_DIR)/test
DEPTH := 7
ARGS := FENs/init.fen $(DEPTH)

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
BIN := $(TARGET_DIR)/schess
LUT := knightLUT.bin kingLUT.bin bishop_rookLUT.bin bishop_mask.bin rook_mask.bin bishop_offset.bin rook_offset.bin
LUT := $(addprefix $(LUT_DIR)/, $(LUT))
LUT_GEN := $(TARGET_DIR)/genLUTs
LUT_SRC := $(SRC_DIR)/lut.c
TEST_SRC := $(wildcard $(TEST_SRC_DIR)/*.c)
TEST_OBJ := $(patsubst $(TEST_SRC_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(TEST_SRC))
TEST_BIN := $(TARGET_DIR)/schess_tests

CFLAGS := -Wall -Wextra -O3 -I.
CFLAGS += -mbmi2

.PHONY: all debug clean run test

all: $(LUT) $(BIN)

run: all
	$(BIN) $(ARGS)

test: $(TEST_BIN)
	$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $(filter-out $(OBJ_DIR)/schess.o, $^) $(LDLIBS) -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS := $(filter-out -O3, $(CFLAGS))
debug: CFLAGS += -ggdb
debug: $(BIN) $(TEST_BIN)

$(LUT): $(LUT_GEN) | $(LUT_DIR)
	$(LUT_GEN) $(LUT)

$(LUT_GEN): CFLAGS += -DLUT_GEN_EXEC
$(LUT_GEN): $(LUT_SRC) | $(TARGET_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

$(BIN): $(OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_DIR) $(OBJ_DIR) $(LUT_DIR) $(TEST_OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(TARGET_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
