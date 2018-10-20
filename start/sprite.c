#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#define IMAGE_FILE	"bat.png"

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_EVENT event;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_BITMAP *bmp = NULL;

	/* Init allegro engine */
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	/* Init keyboard & mouse support */
	al_install_keyboard();
	al_install_mouse();

	/* Init image addon */
	al_init_image_addon();

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

	/* Load image */
	bmp = al_load_bitmap(IMAGE_FILE);
	if (!bmp) {
		fprintf(stderr, "failed to load image "IMAGE_FILE"!\n");
		return -1;
	}

	/* Create timer, 0.3 second */
	timer = al_create_timer(0.3);
	if (!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "Hello Sprite!");
	/* Set background color, black */
	al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
	/* Show the window */
	al_flip_display();

	/* Register event source */
	al_register_event_source(eventq, al_get_keyboard_event_source());
	al_register_event_source(eventq, al_get_display_event_source(display));
	al_register_event_source(eventq, al_get_timer_event_source(timer));

	al_start_timer(timer);

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
			if (event.type == ALLEGRO_EVENT_TIMER) {
				//printf("Got TIMER event, counter %ld\n", event.timer.count);
				/* Face UP */
				al_set_clipping_rectangle(100, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 0, 32, 32,
							100, 100, 0);
				/* Face UP */
				al_set_clipping_rectangle(100, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 0, 32, 32,
							100, 100, 0);
				/* Face UP */
				al_set_clipping_rectangle(100, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 0, 32, 32,
							100, 100, 0);
				/* Face UP */
				al_set_clipping_rectangle(100, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 0, 32, 32,
							100, 100, 0);
				/* Face UP */
				al_set_clipping_rectangle(100, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 0, 32, 32,
							100, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 0, 32, 32,
							100, 100, 0);
				/* Face LEFT */
				al_set_clipping_rectangle(200, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 32, 32, 32,
							200, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 32, 32, 32,
							200, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 32, 32, 32,
							200, 100, 0);
				/* Face DOWN */
				al_set_clipping_rectangle(300, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 64, 32, 32,
							300, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 64, 32, 32,
							300, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 64, 32, 32,
							300, 100, 0);
				/* Face RIGHT */
				al_set_clipping_rectangle(400, 100, 32, 32);
				al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
				if ((event.timer.count+1) % 3 == 1)
					al_draw_bitmap_region(bmp, 0, 96, 32, 32,
							400, 100, 0);
				else if ((event.timer.count+1) % 3 == 2)
					al_draw_bitmap_region(bmp, 32, 96, 32, 32,
							400, 100, 0);
				else if ((event.timer.count+1) % 3 == 0)
					al_draw_bitmap_region(bmp, 64, 96, 32, 32,
							400, 100, 0);
				al_flip_display();
			}
		}
	}

	al_stop_timer(timer);

	/* Release resource */
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_bitmap(bmp);
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
