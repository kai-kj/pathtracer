#------------------------------------------------------------------------------#
# important variables                                                          #
#------------------------------------------------------------------------------#

# binary name 
BIN := pathtracer

# include dir
INC := -I include/

# libraries 
LIBS := -lOpenCL -lm

# flags 
FLAGS := -Wall

# defines
DEFS := -D CL_TARGET_OPENCL_VERSION=300
#------------------------------------------------------------------------------#
# other variables                                                              #
#------------------------------------------------------------------------------#

# commands 
CC := gcc $(FLAGS) $(DEFS) $(INC)
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

$(BIN): $(BUILD)
	$(CC) -c $(SRC)/renderer/renderer.c -o $(BUILD)/renderer/renderer.a
	$(CC) -c $(SRC)/renderer/image.c -o $(BUILD)/renderer/image.a
	$(CC) -c $(SRC)/renderer/scene.c -o $(BUILD)/renderer/scene.a
	$(CC) -c $(SRC)/renderer/camera.c -o $(BUILD)/renderer/camera.a
	$(CC) -c $(SRC)/renderer/material.c -o $(BUILD)/renderer/material.a
	$(CC) -c $(SRC)/main.c -o $(BUILD)/main.o

	$(CC) $(BUILD)/main.o $(BUILD)/renderer/* -o $(BIN) $(LIBS)

clean:
	$(RM) $(BIN) $(BUILD) log.txt render.png
