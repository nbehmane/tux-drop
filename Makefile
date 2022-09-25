SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/tux
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

CPPFLAGS := -Iinclude -Iinternal-includes -MMD -MP $(shell pkg-config --libs glib-2.0 gobject-2.0 gio-2.0)
CFLAGS := -Wall $(shell pkg-config --cflags glib-2.0 gobject-2.0 gio-2.0)
LDFLAGS := -Llib $(shell pkg-config --libs-only-l glib-2.0 gobject-2.0 gio-2.0)

.PHONY: all clean

build: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)