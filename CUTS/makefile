IDIR = include

TARGET ?= linux

ifeq ($(TARGET),linux)
	CC = gcc
	EXE = CUTS
else ifeq ($(TARGET),win64)
	CC = x86_64-w64-mingw32-gcc
	EXE = CUTS.exe
else ifeq ($(TARGET),win32)
	CC = i686-w64-mingw32-gcc
	EXE = CUTS.exe
endif

CFLAGS = -Wall -Wextra -O2

Headers = $(wildcard $(IDIR)/*.h)

ODIR = obj

SDIR = src

SRCS = $(wildcard $(SDIR)/*.c)

OBJS = $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(SRCS))

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(OBJS) -o $@

$(ODIR)/%.o: $(SDIR)/%.c $(HEADERS) | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR):
	mkdir -p $(ODIR)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(ODIR)
