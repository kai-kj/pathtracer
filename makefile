#==============================================================================#
# basic makefile for binary creation                                           #
#==============================================================================#

#------------------------------------------------------------------------------#
# important variables                                                          #
#------------------------------------------------------------------------------#

# binary name 
BIN := pathtracer

# libraries 
LIBS := -lm -lpthread -lOpenCL

# flags 
FLAGS := -Wall -Wextra

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

# compile for profiling
compile_profile: FLAGS += -pg
compile_profile: $(BIN)

# compile and profile project
profile: FLAGS += -pg
profile: $(BIN)
	./$<
	gprof $(BIN) gmon.out > profile.output
	gprof2dot profile.output -o profile.dot

# clean project 
clean:
	$(RM) $(BIN)
	$(RM) $(BUILDDIR)
	$(RM) gmon.out profile.dot profile.output
	$(RM) *.png *.jpg *.bmp
	$(RM) *log.txt

#------------------------------------------------------------------------------#
# build process                                                                #
#------------------------------------------------------------------------------#

$(BIN): $(OBJS)
	$(CC) $(FLAGS) -L $(LIBDIR) $(OBJS) $(LIBS) -o $(BIN)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)
	$(CC) $(FLAGS) -c $< -o $@

$(BUILDDIR):
	$(MKDIR) $(BUILDDIR)