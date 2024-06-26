SRC_DIR := src
OBJ_DIR := obj
TARGET_DIR := target

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
BIN := $(TARGET_DIR)/schess

CFLAGS := -Wall -Wextra -O3 -Isrc
CFLAGS += -mbmi2

.PHONY: all debug clean run

all: $(BIN)

debug: CFLAGS := $(filter-out -O3, $(CFLAGS))
debug: $(BIN)

run: $(BIN)
	./$(BIN)

$(BIN): $(OBJ) | $(TARGET_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(TARGET_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
