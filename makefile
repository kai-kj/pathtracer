#------------------------------------------------------------------------------#
# important variables                                                          #
#------------------------------------------------------------------------------#

# binary name 
BIN := pathtracer

# include dir
INC := include

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
CC := gcc $(FLAGS) $(LIBS) $(DEFS)
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

$(BUILD):
	$(MKDIR) $(BUILD)
	$(MKDIR) $(BUILD)/renderer

$(BIN): $(BUILD)
	$(CC) -c $(SRC)/renderer/renderer.c -o $(BUILD)/renderer/renderer.a
	$(CC) -c $(SRC)/renderer/kernel.c -o $(BUILD)/renderer/kernel.a
	$(CC) -c $(SRC)/main.c -o $(BUILD)/main.o

	$(CC) $(BUILD)/main.o $(BUILD)/renderer/* -o $(BIN)

clean:
	$(RM) $(BIN) $(BUILD) log.txt
