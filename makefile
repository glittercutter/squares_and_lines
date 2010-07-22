O_DIR = ./o
SRC_DIR = ./src

EXEBASE = c_square 
EXE = $(EXEBASE)

CFLAGS = -Wall -pedantic -Werror -std=c99
LFLAGS = `sdl-config --cflags --libs` -lSDL -lSDL_gfx -lSDL_ttf

# used to display message
DEBUG_INFO =

ifdef NDEBUG
	CFLAGS += -D NDEBUG -O6 -ffast-math -funroll-loops \
		-fomit-frame-pointer -fexpensive-optimizations
else
	CFLAGS += -g -pg
	LFLAGS += -g -pg
	DEBUG_INFO += @echo " - Define 'NDEBUG=1' in argument for release build"
endif

CC = gcc -MD
DO_CC=$(CC) $(CFLAGS) -o $@ -c $<

# top-level rules
all : $(EXE)

#############################################################################
# FILES
#############################################################################

#############################################################################
GAME_OBJ = \
	$(O_DIR)/editor.o \
	$(O_DIR)/draw.o \
	$(O_DIR)/fx.o \
	$(O_DIR)/game.o \
	$(O_DIR)/input.o \
	$(O_DIR)/init.o \
	$(O_DIR)/main.o \
	$(O_DIR)/menu.o \
	$(O_DIR)/parse.o \
	$(O_DIR)/ui.o \

$(O_DIR)/editor.o : $(SRC_DIR)/editor.c; $(DO_CC)
$(O_DIR)/draw.o : $(SRC_DIR)/draw.c; $(DO_CC)
$(O_DIR)/fx.o : $(SRC_DIR)/fx.c; $(DO_CC)
$(O_DIR)/game.o : $(SRC_DIR)/game.c; $(DO_CC)
$(O_DIR)/input.o : $(SRC_DIR)/input.c; $(DO_CC)
$(O_DIR)/init.o : $(SRC_DIR)/init.c; $(DO_CC)
$(O_DIR)/main.o : $(SRC_DIR)/main.c; $(DO_CC)
$(O_DIR)/menu.o : $(SRC_DIR)/menu.c; $(DO_CC)
$(O_DIR)/parse.o : $(SRC_DIR)/parse.c; $(DO_CC)
$(O_DIR)/ui.o : $(SRC_DIR)/ui.c; $(DO_CC)


#############################################################################

OBJ = $(GAME_OBJ)

# linking
$(EXE) : $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LFLAGS)
	$(DEBUG_INFO)

clean:
	rm $(O_DIR)*.o
# DO NOT DELETE
