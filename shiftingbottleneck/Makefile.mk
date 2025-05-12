CC = gcc
CFLAGS = -g -I../include -fdiagnostics-color=always
SOURCES = $(wildcard *.c)
OUTPUT = main.exe

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)

clean:
	del /Q $(OUTPUT)