CC       = cc
CFLAGS   = -Wall -Wextra $(shell llvm-config --cflags)
LIBS     = $(shell llvm-config --libs) -lclang

TARGET   = parse_sdl
SRC      = parse_sdl.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
