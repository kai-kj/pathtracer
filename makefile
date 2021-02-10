#==============================================================================#
# basic makefile for binary creation                                           #
#==============================================================================#

#------------------------------------------------------------------------------#
# important variables                                                          #
#------------------------------------------------------------------------------#

# binary name 
BIN := pathtracer

# libraries 
LIBS := -lm -lpthread -lOpenCL -llua

# flags 
FLAGS := -Wall -Wextra -DCL_TARGET_OPENCL_VERSION=300

#------------------------------------------------------------------------------#
# other variables                                                              #
#------------------------------------------------------------------------------#

# commands 
CC := gcc
MV := mv
RM := rm -rf
CP := cp
MKDIR := mkdir
ECHO := @echo

# folders 
BUILDDIR := build
SRCDIR := src/c
LIBDIR := lib

# source files 
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

#------------------------------------------------------------------------------#
# main rules                                                                   #
#------------------------------------------------------------------------------#

.PHONY: all run clean

# compile project (default) 
compile: $(BIN)

# compile and run project 
run: $(BIN)
	./$<

# compile for debugging
compile_debug: FLAGS += -g
compile_debug: $(BIN)

# compile and debug project
debug: FLAGS += -g
debug: $(BIN)
	gdb $<

# compile and profile project
profile: $(BIN)
	valgrind --tool=callgrind ./$<

# clean project 
clean:
	$(RM) $(BIN)
	$(RM) $(BUILDDIR)
	$(RM) *.out.*
	$(RM) *.png *.jpg *.bmp
	$(RM) *.txt

#------------------------------------------------------------------------------#
# build process                                                                #
#------------------------------------------------------------------------------#

$(BIN): $(OBJS)
	$(CC) $(FLAGS) -L $(LIBDIR) $(OBJS) $(LIBS) -o $(BIN)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)
	$(CC) $(FLAGS) -c $< -o $@

$(BUILDDIR):
	$(MKDIR) $(BUILDDIR)