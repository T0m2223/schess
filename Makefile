SRC_DIR := schess
OBJ_DIR := obj
TARGET_DIR := target
LUT_DIR := $(TARGET_DIR)/LUTs

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
BIN := $(TARGET_DIR)/schess
LUT := knightLUT.bin kingLUT.bin bishop_rookLUT.bin bishop_mask.bin rook_mask.bin bishop_offset.bin rook_offset.bin
LUT := $(addprefix $(LUT_DIR)/, $(LUT))
LUT_GEN := $(TARGET_DIR)/genLUTs
LUT_SRC := $(SRC_DIR)/lut.c

CFLAGS := -Wall -Wextra -O3 -I.
CFLAGS += -mbmi2

.PHONY: all debug clean run

all: $(LUT) $(BIN)

run: all
	$(BIN)

debug: CFLAGS := $(filter-out -O3, $(CFLAGS))
debug: CFLAGS += -ggdb
debug: $(BIN)

$(LUT): $(LUT_GEN) | $(LUT_DIR)
	$(LUT_GEN) $(LUT)

$(LUT_GEN): CFLAGS += -DLUT_GEN_EXEC
$(LUT_GEN): $(LUT_SRC) | $(TARGET_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

$(BIN): $(OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_DIR) $(OBJ_DIR) $(LUT_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(TARGET_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
