#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <cJSON.h>
#include <cJSON_Utils.h>

#define SPRITE_FILE	"character.json"

typedef struct {
	int id;
	int x;
	int y;
	int w;
	int h;
} ALLEGRO_SPRITE_FRAME;

#define ALLEGRO_SPRITE_MAX_FRAMES	4

typedef struct {
	/* bitmap resource */
	char *image;
	int image_width;
	int image_height;
	ALLEGRO_BITMAP *bitmap;

	/* stand frames */
	int stand_frame_count;
	ALLEGRO_SPRITE_FRAME *stand_frames; /* down/up/right/left */
	/* walk frames */
	int walk_frame_count;
	ALLEGRO_SPRITE_FRAME *walk_frames; /* down/up/right/left */
	/* fight frames */
	int fight_frame_count;
	ALLEGRO_SPRITE_FRAME *fight_frames; /* down/up/right/left */
} ALLEGRO_SPRITE_LAYER;

typedef struct {
	int x;
	int y;

	int layer_count;
	ALLEGRO_SPRITE_LAYER *layers;
} ALLEGRO_SPRITE;

static void al_dump_sprite(ALLEGRO_SPRITE *s)
{
	int i, j;

	if (!s)
		return;

	printf("Sprite: pos (%d, %d), %d layers.\n", s->x, s->y, s->layer_count);

	for (i = 0; i < s->layer_count; i++) {
		printf("Layer %d:\n", i+1);
		printf("\tImage:\n");
		printf("\t\tfile: %s\n", s->layers[i].image);
		printf("\t\twidth:%d\n", s->layers[i].image_width);
		printf("\t\theight:%d\n", s->layers[i].image_height);
		printf("\tStand frames: %d\n", s->layers[i].stand_frame_count);
		for (j = 0; j < s->layers[i].stand_frame_count; j++) {
			printf("\t\t%02d: %d, %d, %d, %d\n", j,
					s->layers[i].stand_frames[j].x,
					s->layers[i].stand_frames[j].y,
					s->layers[i].stand_frames[j].w,
					s->layers[i].stand_frames[j].h);
		}
		printf("\tWalk frames: %d\n", s->layers[i].walk_frame_count);
		for (j = 0; j < s->layers[i].walk_frame_count; j++) {
			printf("\t\t%02d: %d, %d, %d, %d\n", j,
					s->layers[i].walk_frames[j].x,
					s->layers[i].walk_frames[j].y,
					s->layers[i].walk_frames[j].w,
					s->layers[i].walk_frames[j].h);
		}
		printf("\tFight frames: %d\n", s->layers[i].fight_frame_count);
		for (j = 0; j < s->layers[i].fight_frame_count; j++) {
			printf("\t\t%02d: %d, %d, %d, %d\n", j,
					s->layers[i].fight_frames[j].x,
					s->layers[i].fight_frames[j].y,
					s->layers[i].fight_frames[j].w,
					s->layers[i].fight_frames[j].h);
		}
	}
	printf("\n");
}

static int al_destroy_sprite(ALLEGRO_SPRITE *s)
{
	int i;

	if (!s->layers)
		return 0;

	for (i = 0; i < s->layer_count; i++) {
		if (s->layers[i].image)
			free(s->layers[i].image);

		if (s->layers[i].bitmap)
			al_destroy_bitmap(s->layers[i].bitmap);

		if (s->layers[i].stand_frames)
			free(s->layers[i].stand_frames);

		if (s->layers[i].walk_frames)
			free(s->layers[i].walk_frames);

		if (s->layers[i].fight_frames)
			free(s->layers[i].fight_frames);
	}

	free(s->layers);
	return 0;
}

static int al_parse_sprite_frame_pos(ALLEGRO_SPRITE_FRAME *frames, cJSON *obj)
{
	int i, size = 0;

	if (!obj || !cJSON_IsArray(obj))
		return -1;

	size = cJSON_GetArraySize(obj);
	for (i = 0; i < size; i++) {
		cJSON *item, *item_x, *item_y;

		item = cJSON_GetArrayItem(obj, i);
		if (!item || !cJSON_IsObject(item))
			return -1;

		item_x = cJSON_GetObjectItem(item, "x");
		if (!item_x || !cJSON_IsNumber(item_x))
			return -1;

		item_y = cJSON_GetObjectItem(item, "y");
		if (!item_y || !cJSON_IsNumber(item_y))
			return -1;

		frames[i].x = (int)item_x->valuedouble;
		frames[i].y = (int)item_y->valuedouble;
	}
	return 0;
}

static int al_parse_sprite_frames(ALLEGRO_SPRITE_FRAME **frames, int *count, cJSON *obj)
{
	cJSON *item_s, *item_t;
	int w, h, c, i;

	/* Parse frame tile count */
	item_s = cJSON_GetObjectItem(obj, "count");
	if (!item_s || !cJSON_IsNumber(item_s))
		return -1;
	c = (int)item_s->valuedouble;

	/* Parse frame tile size, width X height */
	item_s = cJSON_GetObjectItem(obj, "size");
	if (!item_s || !cJSON_IsObject(item_s))
		return -2;

	item_t = cJSON_GetObjectItem(item_s, "w");
	if (!item_t || !cJSON_IsNumber(item_t))
		return -3;
	w = (int)item_t->valuedouble;

	item_t = cJSON_GetObjectItem(item_s, "h");
	if (!item_t || !cJSON_IsNumber(item_t))
		return -4;
	h = (int)item_t->valuedouble;

	/* malloc frames memory */
	*frames = malloc(sizeof(ALLEGRO_SPRITE_FRAME) * c);
	if (!(*frames))
		return -5;
	*count = c;

	/* Parse frame tile position in image */
	item_s = cJSON_GetObjectItem(obj, "face_down");
	if (!item_s || !cJSON_IsArray(item_s))
		return -6;
	if (al_parse_sprite_frame_pos(*frames, item_s))
		return -7;

	item_s = cJSON_GetObjectItem(obj, "face_up");
	if (!item_s || !cJSON_IsArray(item_s))
		return -8;
	if (al_parse_sprite_frame_pos(*frames + c/4, item_s))
		return -9;

	item_s = cJSON_GetObjectItem(obj, "face_right");
	if (!item_s || !cJSON_IsArray(item_s))
		return -10;
	if (al_parse_sprite_frame_pos(*frames + c*2/4, item_s))
		return -11;

	item_s = cJSON_GetObjectItem(obj, "face_left");
	if (!item_s || !cJSON_IsArray(item_s))
		return -12;
	if (al_parse_sprite_frame_pos(*frames + c*3/4, item_s))
		return -13;

	/* Set frame tile size */
	for (i = 0; i < c; i++) {
		(*frames+i)->w = w;
		(*frames+i)->h = h;
	}
	return 0;
}

static int al_parse_sprite_layer(ALLEGRO_SPRITE_LAYER *layer, cJSON *obj)
{
	int ret = 0;
	cJSON *item;

	memset(layer, 0, sizeof(ALLEGRO_SPRITE_LAYER));

	/* Parse image info */
	item = cJSON_GetObjectItem(obj, "image");
	if (item && cJSON_IsObject(item)) {
		cJSON *item_s, *item_t;

		/* Image file path */
		item_s = cJSON_GetObjectItem(item, "file");
		if (!item_s || !cJSON_IsString(item_s))
			return -1;
		layer->image = malloc(strlen(item_s->valuestring)+2);
		if (!layer->image)
			return -2;
		strcpy(layer->image, item_s->valuestring);
		
		/* Image size, width X height */
		item_s = cJSON_GetObjectItem(item, "size");
		if (!item_s || !cJSON_IsObject(item_s))
			return -3;

		item_t = cJSON_GetObjectItem(item_s, "w");
		if (!item_t || !cJSON_IsNumber(item_t))
			return -4;
		layer->image_width = (int)item_t->valuedouble;

		item_t = cJSON_GetObjectItem(item_s, "h");
		if (!item_t || !cJSON_IsNumber(item_t))
			return -5;
		layer->image_height = (int)item_t->valuedouble;
	} else
		return -6;

	/* Parse stand frames */
	item = cJSON_GetObjectItem(obj, "stand_frames");
	if (!item && !cJSON_IsObject(item))
		return -7;
	ret = al_parse_sprite_frames(&layer->stand_frames, &layer->stand_frame_count, item);
	if (ret)
		return ret;

	/* Parse walk frames */
	item = cJSON_GetObjectItem(obj, "walk_frames");
	ret = al_parse_sprite_frames(&layer->walk_frames, &layer->walk_frame_count, item);
	if (ret)
		return ret;

	/* Parse fight frames */
	item = cJSON_GetObjectItem(obj, "fight_frames");
	ret = al_parse_sprite_frames(&layer->fight_frames, &layer->fight_frame_count, item);
	if (ret)
		return ret;

	return 0;
}

static int al_parse_sprite(ALLEGRO_SPRITE *s, cJSON *obj)
{
	int i, ret = 0;
	cJSON *item;

	if (!s || !obj)
		return -1;

	memset(s, 0, sizeof(ALLEGRO_SPRITE));

	item = cJSON_GetObjectItem(obj, "layers");
	if (!item || !cJSON_IsArray(item))
		return -2;

	s->layer_count = cJSON_GetArraySize(item);
	s->layers = malloc(sizeof(ALLEGRO_SPRITE_LAYER) * s->layer_count);
	if (!s->layers)
		return -3;

	for (i = 0; i < s->layer_count; i++) {
		ret = al_parse_sprite_layer(&s->layers[i], cJSON_GetArrayItem(item, i));
		if (ret)
			goto error;
	}

	return 0;
error:
	al_destroy_sprite(s);
	return ret;
}

static cJSON *al_json_parse(const char *file_name)
{
	FILE *fp = NULL;
	char *data = NULL;
	int size = 0;
	cJSON *json = NULL;

	fp = fopen(file_name, "r");
	if (!fp) {
		fprintf(stderr, "Open file [%s] error!\n", file_name);
		goto exit;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	data = malloc(size);
	if (!data) {
		fprintf(stderr, "Can't malloc memory, %d bytes\n", size);
		goto exit;
	}

	fread(data, 1, size, fp);

	json = cJSON_Parse(data);
	if (!json)
		fprintf(stderr, "Parse json data error!\n");

exit:
	if (fp)
		fclose(fp);
	if (data)
		free(data);
	return json;
}

static int al_init_sprite(ALLEGRO_SPRITE *s, const char *file_name)
{
	cJSON *json = al_json_parse(file_name);
	return al_parse_sprite(s, json);
}

int main(int argc, char **argv)
{
	int ret;
	ALLEGRO_SPRITE s;

	ret = al_init_sprite(&s, SPRITE_FILE);
	if (ret) {
		printf("Load sprite failed. %d\n", ret);
		return -1;
	}

	al_dump_sprite(&s);
	al_destroy_sprite(&s);
	return 0;
}
