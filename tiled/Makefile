CC = gcc
CFLAGS =
LDFLAGS = -lm -lallegro -lallegro_image -lallegro_font \
		  -lallegro_ttf -lallegro_primitives -lallegro_tiled \
		  -lcjson -lcjson_utils
APPS = demo1 demo2 sprite1 sprite_map1

all: $(APPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(APPS): % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(APPS) *.o

