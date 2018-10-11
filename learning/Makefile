CC = gcc
CFLAGS =
LDFLAGS = -lm -lallegro -lallegro_image -lallegro_font \
		  -lallegro_ttf -lallegro_primitives
APPS = hello event draw move image font sprite spritemove

all: $(APPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(APPS): % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(APPS) *.o

