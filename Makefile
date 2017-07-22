VPATH = src
CFLAGS = -Wall -Werror -Wextra -std=c99 $$(pkg-config --cflags yaml-0.1) -D_DEFAULT_SOURCE
LFLAGS = -lm -lportmidi $$(pkg-config --libs yaml-0.1)
objects = app.o \
	chord.o \
	input.o \
	input_joystick.o \
	input_midi.o \
	main.o \
	player.o \
	program.o \
	program_factory.o \
	sequence.o \
	timer.o

MemfisMIDI: $(objects)
	cc $(objects) -o MemfisMIDI $(LFLAGS)

$(objects): %.o: %.c
	cc -c $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -f MemfisMIDI $(objects)
