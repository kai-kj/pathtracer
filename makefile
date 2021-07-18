#------------------------------------------------------------------------------#
# important variables                                                          #
#------------------------------------------------------------------------------#

# binary name 
BIN := pathtracer

# include dir
INC := -I include/

# library dir
LIB := -L lib/

# libraries 
LIBS := -lOpenCL -lm -lSDL2

# flags 
FLAGS := -Wall

# defines
DEFS := -D CL_TARGET_OPENCL_VERSION=300
#------------------------------------------------------------------------------#
# other variables                                                              #
#------------------------------------------------------------------------------#

# commands 
CC := gcc $(FLAGS) $(DEFS) $(INC) $(LIB)
MV := mv
RM := rm -rf
CP := cp
MKDIR := mkdir -p
ECHO := @echo

# folders 
BUILD := build
SRC := src

#------------------------------------------------------------------------------#
# build process                                                                #
#------------------------------------------------------------------------------#

.PHONY: default run clean

default: clean $(BIN)

run: clean $(BIN)
	./$(BIN)

debug: CC += -g
debug: clean $(BIN)

$(BUILD):
	$(MKDIR) $(BUILD)
	$(MKDIR) $(BUILD)/renderer
	$(MKDIR) $(BUILD)/gui

$(BIN): $(BUILD)
	$(CC) -c $(SRC)/renderer/renderer.c -o $(BUILD)/renderer/renderer.a
	$(CC) -c $(SRC)/renderer/image.c -o $(BUILD)/renderer/image.a
	$(CC) -c $(SRC)/renderer/scene.c -o $(BUILD)/renderer/scene.a
	$(CC) -c $(SRC)/renderer/camera.c -o $(BUILD)/renderer/camera.a
	$(CC) -c $(SRC)/renderer/material.c -o $(BUILD)/renderer/material.a
	$(CC) -c $(SRC)/renderer/global.c -o $(BUILD)/renderer/global.a
	$(CC) -c $(SRC)/gui/gui.c -o $(BUILD)/gui/gui.a
	$(CC) -c $(SRC)/main.c -o $(BUILD)/main.o

	$(CC) $(BUILD)/main.o $(BUILD)/renderer/* $(BUILD)/gui/* -o $(BIN) $(LIBS)

clean:
	$(RM) $(BIN) $(BUILD) log.txt
