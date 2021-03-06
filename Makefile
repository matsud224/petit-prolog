#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = -Wall
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = -lreadline

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/petit-prolog

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/petit-prolog

OBJ_DEBUG = $(OBJDIR_DEBUG)/etc.o $(OBJDIR_DEBUG)/gc.o $(OBJDIR_DEBUG)/interpret.o $(OBJDIR_DEBUG)/lexer.o $(OBJDIR_DEBUG)/main.o $(OBJDIR_DEBUG)/parser.o $(OBJDIR_DEBUG)/symbol.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/etc.o $(OBJDIR_RELEASE)/gc.o $(OBJDIR_RELEASE)/interpret.o $(OBJDIR_RELEASE)/lexer.o $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/parser.o $(OBJDIR_RELEASE)/symbol.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/etc.o: etc.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c etc.c -o $(OBJDIR_DEBUG)/etc.o

$(OBJDIR_DEBUG)/gc.o: gc.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c gc.c -o $(OBJDIR_DEBUG)/gc.o

$(OBJDIR_DEBUG)/interpret.o: interpret.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c interpret.c -o $(OBJDIR_DEBUG)/interpret.o

$(OBJDIR_DEBUG)/lexer.o: lexer.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c lexer.c -o $(OBJDIR_DEBUG)/lexer.o

$(OBJDIR_DEBUG)/main.o: main.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.c -o $(OBJDIR_DEBUG)/main.o

$(OBJDIR_DEBUG)/parser.o: parser.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c parser.c -o $(OBJDIR_DEBUG)/parser.o

$(OBJDIR_DEBUG)/symbol.o: symbol.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c symbol.c -o $(OBJDIR_DEBUG)/symbol.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/etc.o: etc.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c etc.c -o $(OBJDIR_RELEASE)/etc.o

$(OBJDIR_RELEASE)/gc.o: gc.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c gc.c -o $(OBJDIR_RELEASE)/gc.o

$(OBJDIR_RELEASE)/interpret.o: interpret.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c interpret.c -o $(OBJDIR_RELEASE)/interpret.o

$(OBJDIR_RELEASE)/lexer.o: lexer.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c lexer.c -o $(OBJDIR_RELEASE)/lexer.o

$(OBJDIR_RELEASE)/main.o: main.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.c -o $(OBJDIR_RELEASE)/main.o

$(OBJDIR_RELEASE)/parser.o: parser.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c parser.c -o $(OBJDIR_RELEASE)/parser.o

$(OBJDIR_RELEASE)/symbol.o: symbol.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c symbol.c -o $(OBJDIR_RELEASE)/symbol.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

