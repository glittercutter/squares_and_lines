OBJDIR = ./o/gnu
SRCDIR = ./src

# Automatic dependency
DEPS := $(wildcard $(OBJDIR)/*.d)

EXE = c_square 

CFLAGS = -Wall -pedantic -Werror -std=c99 -pipe
LFLAGS = -pipe `sdl-config --cflags --libs` -lSDL -lSDL_gfx -lSDL_ttf -lSDL_net -lpthread

# used to display message in debug build
DEBUG_INFO =

ifdef DNDEBUG
	OBJDIR += "/release"
	CFLAGS += -D DNDEBUG -O6 -ffast-math -funroll-loops \
		-fomit-frame-pointer -fexpensive-optimizations
else
	OBJDIR += "/debug"
	CFLAGS += -g -pg
	LFLAGS += -g -pg
	DEBUG_INFO += @echo " - Define 'DNDEBUG=1' in argument for release build"
endif

CC = gcc
DO_CC=$(CC) -MD $(CFLAGS) -o $@ -c $<
# -MD flag generate dependency

# top-level rules
all : create_dir $(EXE)

# create missing directory
create_dir :
	@test -d $(OBJDIR) || mkdir $(OBJDIR)

#############################################################################
# FILES
#############################################################################

#############################################################################
OBJ = \
	$(OBJDIR)/common.o \
	$(OBJDIR)/client.o \
	$(OBJDIR)/editor.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/fx.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/input.o \
	$(OBJDIR)/init.o \
	$(OBJDIR)/main.o \
	$(OBJDIR)/menu.o \
	$(OBJDIR)/net.o \
	$(OBJDIR)/net_z.o \
	$(OBJDIR)/parse.o \
	$(OBJDIR)/server.o \
	$(OBJDIR)/ui.o \


$(OBJDIR)/common.o : $(SRCDIR)/common.c; $(DO_CC)
$(OBJDIR)/client.o : $(SRCDIR)/client.c; $(DO_CC)
$(OBJDIR)/editor.o : $(SRCDIR)/editor.c; $(DO_CC)
$(OBJDIR)/draw.o : $(SRCDIR)/draw.c; $(DO_CC)
$(OBJDIR)/fx.o : $(SRCDIR)/fx.c; $(DO_CC)
$(OBJDIR)/game.o : $(SRCDIR)/game.c; $(DO_CC)
$(OBJDIR)/input.o : $(SRCDIR)/input.c; $(DO_CC)
$(OBJDIR)/init.o : $(SRCDIR)/init.c; $(DO_CC)
$(OBJDIR)/main.o : $(SRCDIR)/main.c; $(DO_CC)
$(OBJDIR)/menu.o : $(SRCDIR)/menu.c; $(DO_CC)
$(OBJDIR)/net.o : $(SRCDIR)/net.c; $(DO_CC)
$(OBJDIR)/net_z.o : $(SRCDIR)/net_z.c; $(DO_CC)
$(OBJDIR)/parse.o : $(SRCDIR)/parse.c; $(DO_CC)
$(OBJDIR)/server.o : $(SRCDIR)/server.c; $(DO_CC)
$(OBJDIR)/ui.o : $(SRCDIR)/ui.c; $(DO_CC)



#############################################################################

# Automatic dependency
-include $(DEPS)

$(EXE) : $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LFLAGS)
	$(DEBUG_INFO)

clean:
	rm $(OBJDIR)*.o
# DO NOT DELETE
