TOP_OBJ_DIR = ./o/gnu
SRCDIR = ./src

# Automatic dependency
DEPS := $(wildcard $(OBJDIR)/*.d)

BIN = squares_and_lines

CFLAGS = -Wall -pedantic -Werror -std=c99 -pipe
LFLAGS =
LIBS = `sdl-config --cflags --libs` -lSDL -lSDL_gfx -lSDL_ttf -lSDL_net -lpthread -lm

DEBUG_INFO =

ifeq ($(build_type), debug)
	OBJDIR = ${TOP_OBJ_DIR}/debug
	CFLAGS += -g -pg
	LFLAGS += -g -pg
	BUILD_INFO += @echo "build_type=debug"
else
	OBJDIR = ${TOP_OBJ_DIR}/release
	CFLAGS += -D DNDEBUG -Os
	LFLAGS += -s
	BUILD_INFO += @echo "build_type=release"
endif

CC = gcc
DO_CC=$(CC) -MD $(CFLAGS) -o $@ -c $<
# -MD flag generate dependency

# top-level rules
all : create_dir $(BIN)

# create missing directory
create_dir :
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)

# FILES #####################################################################

OBJ = \
	$(OBJDIR)/common.o \
	$(OBJDIR)/client.o \
	$(OBJDIR)/editor.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/fx.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/input.o \
	$(OBJDIR)/main.o \
	$(OBJDIR)/menu.o \
	$(OBJDIR)/net.o \
	$(OBJDIR)/parser.o \
	$(OBJDIR)/server.o \
	$(OBJDIR)/ui.o \


$(OBJDIR)/common.o : $(SRCDIR)/common.c; $(DO_CC)
$(OBJDIR)/client.o : $(SRCDIR)/client.c; $(DO_CC)
$(OBJDIR)/editor.o : $(SRCDIR)/editor.c; $(DO_CC)
$(OBJDIR)/draw.o : $(SRCDIR)/draw.c; $(DO_CC)
$(OBJDIR)/fx.o : $(SRCDIR)/fx.c; $(DO_CC)
$(OBJDIR)/game.o : $(SRCDIR)/game.c; $(DO_CC)
$(OBJDIR)/input.o : $(SRCDIR)/input.c; $(DO_CC)
$(OBJDIR)/main.o : $(SRCDIR)/main.c; $(DO_CC)
$(OBJDIR)/menu.o : $(SRCDIR)/menu.c; $(DO_CC)
$(OBJDIR)/net.o : $(SRCDIR)/net.c; $(DO_CC)
$(OBJDIR)/parser.o : $(SRCDIR)/parser.c; $(DO_CC)
$(OBJDIR)/server.o : $(SRCDIR)/server.c; $(DO_CC)
$(OBJDIR)/ui.o : $(SRCDIR)/ui.c; $(DO_CC)


#############################################################################

# Automatic dependency
-include $(DEPS)

$(BIN) : $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LFLAGS) $(LIBS)
	$(BUILD_INFO)

clean:
	rm $(OBJDIR)*.o $(OBJDIR)*.d
