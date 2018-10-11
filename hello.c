#include <stdio.h>
#include <allegro5/allegro.h>

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;

	/* Init allegro engine */
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	/* Create a 640x480 window */
	display = al_create_display(640, 480);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}

	/* Set window title */
	al_set_window_title(display, "Hello World!");
	/* Set background color, black */
	al_clear_to_color(al_map_rgb(0, 0, 0));
	/* Show the window */
	al_flip_display();

	/* Set a 10s timer */
	al_rest(10.0);

	/* Release the window resource */
	al_destroy_display(display);
	return 0;
}
