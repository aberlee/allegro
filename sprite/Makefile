CC = gcc
CFLAGS =
LDFLAGS = -lm -lallegro -lallegro_image -lallegro_font \
		  -lallegro_ttf -lallegro_primitives -lallegro_tiled \
		  -lcjson -lcjson_utils 
APPS = basic_test game_frame

all: $(APPS)

%.o: %.c sprite.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(APPS): % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(APPS) *.o

