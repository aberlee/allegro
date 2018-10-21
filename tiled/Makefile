CC = gcc
CFLAGS =
LDFLAGS = -lm -lallegro -lallegro_image -lallegro_font \
		  -lallegro_ttf -lallegro_primitives -lallegro_tiled
APPS = demo1 demo2 sprite1

all: $(APPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(APPS): % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(APPS) *.o

