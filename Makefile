TARGET       := stt

# Default paths can be overridden
CC          ?= gcc
INCLUDE_DIR ?= /usr/include
LIB_DIR     ?= /usr/lib

CFLAGS      := -std=c23 -O0 -g -Wall -Wextra

# Check for Cc23 support
CHECK_C23 := $(shell $(CC) -std=c23 -dM -E - < /dev/null > /dev/null 2>&1 && echo "yes" || echo "no")

ifeq ($(CHECK_C23),no)
    CFLAGS := -std=c11 -O0 -g -Wall -Wextra
    $(info C23 standard not supported, falling back to C11)
endif

LDFLAGS      := -L$(LIB_DIR) -lm -lcurl
INCLUDE      := -Iinclude/ -I$(INCLUDE_DIR)

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

.PHONY: all build clean info help

build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(BIN_DIR)/*

info:
	@echo "[*] Library dir:     ${LIB_DIR}     "
	@echo "[*] Include dir:     ${INCLUDE_DIR} "
	@echo "[*] Application dir: ${APP_DIR}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Object dir:      ${SRC_DIR}     "
	@echo "[*] Object dir:      ${SOURCES}     "
	@echo "[*] Object dir:      ${OBJECTS}     "

help:
	@echo "Usage:"
	@echo "  make [INCLUDE_DIR=/path/to/curl/include] [LIB_DIR=/path/to/curl/lib] [CC=gcc]"

