#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>

static void draw_rect(int x, int y,
				int w, int h, ALLEGRO_COLOR color)
{
	int i, j;

	for (i = 0; i <= w; i++) {
		for (j = 0; j <= h; j++)
			al_draw_pixel(x+i, y+j, color);
	}
}

static void draw_rect_xy(int x1, int y1,
				int x2, int y2, ALLEGRO_COLOR color)
{
	int x = (x1 < x2) ? x1 : x2;
	int y = (y1 < y2) ? y1 : y2;
	int w = abs(x2 - x1);
	int h = abs(y2 - y1);
	draw_rect(x, y, w, h, color);
}

static void draw_line(int x1, int y1,
				int x2, int y2, ALLEGRO_COLOR color)
{
	int x, y;
	int x_step, y_step;

	x_step = (x1 < x2) ? 1 : -1;
	y_step = (y1 < y2) ? 1 : -1;

	if (x1 == x2) {
		for (y = y1; (y_step > 0) ? (y <= y2) : (y >= y2); y += y_step)
			al_draw_pixel(x1, y, color);
		return;
	}

	if (y1 == y2) {
		for (x = x1; (x_step > 0) ? (x <= x2) : (x >= x2); x += x_step)
			al_draw_pixel(x, y1, color);
		return;
	}

	for (x = x1; (x_step > 0) ? (x <= x2) : (x >= x2); x += x_step) {
		y = (x - x1) * (y2 - y1) / (x2 - x1) + y1;
		al_draw_pixel(x, y, color);
	}

	for (y = y1; (y_step > 0) ? (y <= y2) : (y >= y2); y += y_step) {
		x = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
		al_draw_pixel(x, y, color);
	}
}

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_EVENT event;
	int line_start = 0;
	int line_start_x = 0;
	int line_start_y = 0;

	/* Init allegro engine */
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	/* Init keyboard & mouse support */
	al_install_keyboard();
	al_install_mouse();

	/* Create a 640x480 window */
	display = al_create_display(640, 480);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}

	/* Create event queue */
	eventq = al_create_event_queue();
	if (!eventq) {
		fprintf(stderr, "failed to create event queue!\n");
		return -1;
	}

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "Hello World!");
	/* Set background color, black */
	al_clear_to_color(al_map_rgb(0, 0, 0));
	/* Show the window */
	al_flip_display();

	/* Register event source */
	al_register_event_source(eventq, al_get_mouse_event_source());
	al_register_event_source(eventq, al_get_keyboard_event_source());
	al_register_event_source(eventq, al_get_display_event_source(display));

	/* main loop */
	while (1) {
		if (!al_is_event_queue_empty(eventq)) {
			al_wait_for_event(eventq, &event);
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				printf("Got DISPLAY CLOSE event!\n");
				break;
			}
			if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
				printf("Got KEYBOARD event, %u\n", event.keyboard.keycode);
				if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					break;
			}
			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				printf("Got MOUSE DOWN event, x %d, y %d, buttong %u\n",
						event.mouse.x, event.mouse.y, event.mouse.button);
				if (!line_start) {
					line_start_x = event.mouse.x;
					line_start_y = event.mouse.y;
					line_start = 1;
				} else {
					line_start = 0;
					draw_rect_xy(line_start_x, line_start_y,
							event.mouse.x, event.mouse.y,
							al_map_rgb(0xff,0xff, 0xff));
					al_flip_display();
				}
			}
		}
	}

	/* Release resource */
	al_destroy_display(display);
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
