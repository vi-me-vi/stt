CC       := cc
CFLAGS := -std=c11 -O3 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
LDFLAGS  := -L/usr/lib -lm
BIN      := ./bin
OBJ_DIR  := ./obj
TARGET   := stt
INCLUDE  := -Iinclude/
SRC      := ./src
SOURCES  :=               \
   $(wildcard SRC/*.c)    \
   $(wildcard SRC/**/*.c) \

OBJECTS  := $(SOURCES:SRC/%.c=$(OBJ_DIR)/%.o)
DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(BIN)/$(TARGET)

$(OBJ_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(BIN)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $(BIN)/$(TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean info

build:
	@mkdir -p $(BIN)
	@mkdir -p $(OBJ_DIR)

# debug: CFLAGS += -DDEBUG -g
# debug: all

# release: CFLAGS += -O2
# release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(BIN)/*

info:
	@echo $(OBJECTS)
	@echo "[*] Application dir: ${APP_DIR}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Sources:         ${SOURCES}     "
	@echo "[*] Objects:         ${OBJECTS}     "
	@echo "[*] Dependencies:    ${DEPENDENCIES}"
