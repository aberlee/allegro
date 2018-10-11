#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define FONT_FILE	"wqy-microhei.ttc"

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_EVENT event;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_FONT *font = NULL;

	/* Init allegro engine */
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	/* Init keyboard & mouse support */
	al_install_keyboard();
	al_install_mouse();

	/* Init font & ttf addon */
	al_init_font_addon();
	al_init_ttf_addon();

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

	/* Load font file */
	font = al_load_font(FONT_FILE, 30, 0); /* 30 is the text size to show */
	if (!font) {
		fprintf(stderr, "failed to load font "FONT_FILE"!\n");
		return -1;
	}

	/* Create timer, 1 second */
	timer = al_create_timer(1.0);
	if (!timer) {
		fprintf(stderr, "failed to create timer!\n");
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
			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				printf("Got MOUSE DOWN event, x %d, y %d, buttong %u\n",
						event.mouse.x, event.mouse.y, event.mouse.button);
			}
			if (event.type == ALLEGRO_EVENT_TIMER) {
				printf("Got TIMER event, counter %ld\n", event.timer.count);
				if ((event.timer.count+1) % 2 == 0) {
					al_draw_text(font, al_map_rgb(0xff, 0xff, 0xff),
							10, 100, ALLEGRO_ALIGN_LEFT,
							"<-HELLO WORLD!");
					al_draw_text(font, al_map_rgb(0xff, 0xff, 0xff),
							300, 200, ALLEGRO_ALIGN_CENTER,
							"你好世界！");
					al_draw_textf(font, al_map_rgb(0xff, 0xff, 0xff),
							620, 300, ALLEGRO_ALIGN_RIGHT,
							"TIMER COUNT %ld->", event.timer.count);
					al_flip_display();
				} else {
					al_clear_to_color(al_map_rgb(0, 0, 0));
					al_flip_display();
				}
			}
		}
	}

	al_stop_timer(timer);

	/* Release resource */
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_font(font);
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
