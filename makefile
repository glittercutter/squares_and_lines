OBJDIR = ./o
SRCDIR = ./src

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
all : create_dir $(EXE)

create_dir :
	@test -d $(OBJDIR) || mkdir $(OBJDIR)

#############################################################################
# FILES
#############################################################################

#############################################################################
GAME_OBJ = \
	$(OBJDIR)/editor.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/fx.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/input.o \
	$(OBJDIR)/init.o \
	$(OBJDIR)/main.o \
	$(OBJDIR)/menu.o \
	$(OBJDIR)/parse.o \
	$(OBJDIR)/ui.o \

$(OBJDIR)/editor.o : $(SRCDIR)/editor.c; $(DO_CC)
$(OBJDIR)/draw.o : $(SRCDIR)/draw.c; $(DO_CC)
$(OBJDIR)/fx.o : $(SRCDIR)/fx.c; $(DO_CC)
$(OBJDIR)/game.o : $(SRCDIR)/game.c; $(DO_CC)
$(OBJDIR)/input.o : $(SRCDIR)/input.c; $(DO_CC)
$(OBJDIR)/init.o : $(SRCDIR)/init.c; $(DO_CC)
$(OBJDIR)/main.o : $(SRCDIR)/main.c; $(DO_CC)
$(OBJDIR)/menu.o : $(SRCDIR)/menu.c; $(DO_CC)
$(OBJDIR)/parse.o : $(SRCDIR)/parse.c; $(DO_CC)
$(OBJDIR)/ui.o : $(SRCDIR)/ui.c; $(DO_CC)


#############################################################################

OBJ = $(GAME_OBJ)

$(EXE) : $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LFLAGS)
	$(DEBUG_INFO)

clean:
	rm $(OBJDIR)*.o
# DO NOT DELETE
