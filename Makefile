TARGET       := stt

CC           := cc
CFLAGS       := -std=c11 -O0 -g -Wall -Wextra
#-Wpedantic -Wstrict-aliasing
LDFLAGS      := -Lusr/lib -lm
INCLUDE      := -Iinclude/

BIN_DIR      := ./bin
OBJ_DIR      := ./obj
SRC_DIR      := ./src

SOURCES      := $(shell find $(SRC_DIR) -name '*.c')
SRCS_F       := $(foreach src,$(SOURCES),$(subst $(SRC_DIR)/, , $(src)))
OBJECTS      := $(SRCS_F:%.c=$(OBJ_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)


all: build $(BIN_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean info

build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(BIN_DIR)/*

info:
	@echo "[*] Application dir: ${APP_DIR}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Object dir:      ${SRC_DIR}     "
	@echo "[*] Object dir:      ${SOURCES}     "
	@echo "[*] Object dir:      ${OBJECTS}     "
