#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <cJSON.h>
#include <cJSON_Utils.h>

#include "sprite.h"

#define ERROR_RETURN(x) \
	do{	\
		fprintf(stderr, "ALLEGRO_SPRITE ERROR [%s(), LINE %d in %s]\n", __func__, __LINE__, __FILE__); \
		return x; \
	} while(0)

typedef struct {
	int x;
	int y;
} ALLEGRO_SPRITE_TILE;

typedef struct {
	/* bitmap resource */
	char *image_file;
	int image_width;
	int image_height;
	ALLEGRO_BITMAP *bitmap;

	int tile_count;
	int tile_width;
	int tile_height;
	ALLEGRO_SPRITE_TILE *tiles;

	ALLEGRO_SPRITE_TILE *tiles_down;  /* pointer to tiles[0] */
	ALLEGRO_SPRITE_TILE *tiles_up;    /* pointer to tiles[tile_count * 1 / 4] */
	ALLEGRO_SPRITE_TILE *tiles_right; /* pointer to tiles[tile_count * 2 / 4] */
	ALLEGRO_SPRITE_TILE *tiles_left;  /*/pointer to tiles[tile_count * 3 / 4] */
} ALLEGRO_SPRITE_TILE_LAYER;

typedef struct {
	int layer_count;
	ALLEGRO_SPRITE_TILE_LAYER *layers;
} ALLEGRO_SPRITE_TILESET;

typedef struct {
	int id; /* Action ID */
	bool running; /* Is running? */

	int tileset_id; /* Tileset select */

	/* Animation frames control */
	int counter; /* Frame counter, 0 -> terminal */
	int counter_max; /* Frame counter max number */
	int interval; /* FPS internal, control action animation speed. */
	bool stopable; /* If action can be stopped. */
} ALLEGRO_SPRITE_ACTION;

#define MAX_ACTIONS	10

struct _ALLEGRO_SPRITE {
	/* Bitmap tilesets */
	int tileset_count;
	ALLEGRO_SPRITE_TILESET *tilesets;

	/* The size of map that sprite is in */
	int map_width;
	int map_height;

	/* Position in map. */
	int x;
	int y;

	/* Current tile size, related to tileset. */
	int w;
	int h;

	/* The direction sprite face to, tile selecting */
	ALLEGRO_SPRITE_DIRECTION direction;

	ALLEGRO_SPRITE_ACTION actions[MAX_ACTIONS];
	ALLEGRO_SPRITE_ACTION *action; /* Selected action, pointer to actions[?] */
	int action_count;
};

void al_dump_sprite(ALLEGRO_SPRITE *s)
{
	int i, j, t;

	if (!s)
		return;

	printf("Sprite: pos (%d, %d), %d tilesets.\n", s->x, s->y, s->tileset_count);

	for (i = 0; i < s->tileset_count; i++) {
		printf("\nTileset %2d: %d layers\n", i+1, s->tilesets[i].layer_count);
		for (j = 0; j < s->tilesets[i].layer_count; j++) {
			printf("\tLayer %2d: %d tiles\n", j+1, s->tilesets[i].layers[j].tile_count);
			printf("\t\tImage file:   %s\n", s->tilesets[i].layers[j].image_file);
			printf("\t\tImage Width:  %4d\n", s->tilesets[i].layers[j].image_width);
			printf("\t\tImage Height: %4d\n", s->tilesets[i].layers[j].image_height);
			printf("\t\tTile width:   %4d\n", s->tilesets[i].layers[j].tile_width);
			printf("\t\tTile height:  %4d\n", s->tilesets[i].layers[j].tile_height);
			printf("\t\tTile %2d:\n", j+1);
			for (t = 0; t < s->tilesets[i].layers[j].tile_count; t++) {
				printf("\t\t\t\t%02d: %4d, %4d\n", t+1,
						s->tilesets[i].layers[j].tiles[t].x,
						s->tilesets[i].layers[j].tiles[t].y);
			}
		}
	}
	printf("\n");
}

int al_destroy_sprite(ALLEGRO_SPRITE *s)
{
	int i, j;

	if (!s->tilesets)
		return 0;

	for (i = 0; i < s->tileset_count; i++) {
		if (!s->tilesets[i].layers)
			continue;

		for (j = 0; j < s->tilesets[i].layer_count; j++) {
			if (s->tilesets[i].layers[j].image_file)
				free(s->tilesets[i].layers[j].image_file);

			if (s->tilesets[i].layers[j].bitmap)
				al_destroy_bitmap(s->tilesets[i].layers[j].bitmap);

			if (s->tilesets[i].layers[j].tiles)
				free(s->tilesets[i].layers[j].tiles);
		}
		free(s->tilesets[i].layers);
	}

	free(s->tilesets);
	free(s);
	return 0;
}


/***************************************************************************************/
/********** Parse Sprite Data From JSON File *******************************************/
/***************************************************************************************/

static int al_parse_sprite_tiles(ALLEGRO_SPRITE_TILE *tiles, cJSON *obj)
{
	int i, size = 0;

	if (!obj || !cJSON_IsArray(obj))
		ERROR_RETURN(-1);

	size = cJSON_GetArraySize(obj);
	for (i = 0; i < size; i++) {
		cJSON *item, *item_x, *item_y;

		item = cJSON_GetArrayItem(obj, i);
		if (!item || !cJSON_IsObject(item))
			ERROR_RETURN(-1);

		item_x = cJSON_GetObjectItem(item, "x");
		if (!item_x || !cJSON_IsNumber(item_x))
			ERROR_RETURN(-1);

		item_y = cJSON_GetObjectItem(item, "y");
		if (!item_y || !cJSON_IsNumber(item_y))
			ERROR_RETURN(-1);

		tiles[i].x = (int)item_x->valuedouble;
		tiles[i].y = (int)item_y->valuedouble;
	}
	return 0;
}

static int al_parse_sprite_layer(ALLEGRO_SPRITE_TILE_LAYER *layer, cJSON *obj)
{
	cJSON *item, *item_s, *item_t;
	int w, h, c;

	memset(layer, 0, sizeof(ALLEGRO_SPRITE_TILE_LAYER));

	/* Parse image info */
	item = cJSON_GetObjectItem(obj, "image");
	if (!item || !cJSON_IsObject(item))
		ERROR_RETURN(-1);

	/* Image file path */
	item_s = cJSON_GetObjectItem(item, "file");
	if (!item_s || !cJSON_IsString(item_s))
		ERROR_RETURN(-1);
	layer->image_file = malloc(strlen(item_s->valuestring)+2);
	if (!layer->image_file)
		ERROR_RETURN(-1);
	strcpy(layer->image_file, item_s->valuestring);

	/* Image size, width X height */
	item_s = cJSON_GetObjectItem(item, "size");
	if (!item_s || !cJSON_IsObject(item_s))
		ERROR_RETURN(-1);

	item_t = cJSON_GetObjectItem(item_s, "w");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	layer->image_width = (int)item_t->valuedouble;

	item_t = cJSON_GetObjectItem(item_s, "h");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	layer->image_height = (int)item_t->valuedouble;

	/* Parse tile info */
	item = cJSON_GetObjectItem(obj, "tiles");
	if (!item || !cJSON_IsObject(item))
		ERROR_RETURN(-1);

	/* Parse tile count */
	item_s = cJSON_GetObjectItem(item, "count");
	if (!item_s || !cJSON_IsNumber(item_s))
		ERROR_RETURN(-1);
	c = (int)item_s->valuedouble;

	/* Parse tile size */
	item_s = cJSON_GetObjectItem(item, "size");
	if (!item_s || !cJSON_IsObject(item_s))
		ERROR_RETURN(-1);

	item_t = cJSON_GetObjectItem(item_s, "w");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	w = (int)item_t->valuedouble;

	item_t = cJSON_GetObjectItem(item_s, "h");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	h = (int)item_t->valuedouble;

	/* malloc tiles array */
	layer->tiles = malloc(sizeof(ALLEGRO_SPRITE_TILE) * c);
	if (!layer->tiles)
		ERROR_RETURN(-1);

	layer->tile_count = c;
	layer->tile_width = w;
	layer->tile_height = h;
	layer->tiles_down = &(layer->tiles[0]);
	layer->tiles_up = &(layer->tiles[c/4]);
	layer->tiles_right = &(layer->tiles[c*2/4]);
	layer->tiles_left = &(layer->tiles[c*3/4]);

	/* Parse tiles */
	item_s = cJSON_GetObjectItem(item, "face_down");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(layer->tiles_down, item_s))
		ERROR_RETURN(-1);

	item_s = cJSON_GetObjectItem(item, "face_up");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(layer->tiles_up, item_s))
		ERROR_RETURN(-1);

	item_s = cJSON_GetObjectItem(item, "face_right");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(layer->tiles_right, item_s))
		ERROR_RETURN(-1);

	item_s = cJSON_GetObjectItem(item, "face_left");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(layer->tiles_left, item_s))
		ERROR_RETURN(-1);

	/* Load image file to bitmap */
	layer->bitmap = al_load_bitmap(layer->image_file);
	if (!layer->bitmap)
		ERROR_RETURN(-1);
	return 0;
}

static int al_parse_sprite_tileset(ALLEGRO_SPRITE_TILESET *tileset, cJSON *obj)
{
	int ret = 0;
	int count = 0, i;
	cJSON *item, *item_s;

	memset(tileset, 0, sizeof(ALLEGRO_SPRITE_TILESET));

	item = cJSON_GetObjectItem(obj, "layers");
	if (!item || !cJSON_IsArray(item))
		ERROR_RETURN(-1);

	count = cJSON_GetArraySize(item);

	/* Alloc layers array */
	tileset->layers = malloc(sizeof(ALLEGRO_SPRITE_TILE_LAYER) * count);
	if (!tileset->layers)
		ERROR_RETURN(-1);
	tileset->layer_count = count;

	/* Parse layers */
	for (i = 0; i < count; i++) {
		item_s = cJSON_GetArrayItem(item, i);
		ret = al_parse_sprite_layer(&tileset->layers[i], item_s);
		if (ret)
			ERROR_RETURN(-1);
	}
	return 0;
}

static int al_parse_sprite(ALLEGRO_SPRITE *s, cJSON *obj)
{
	int i, ret = 0;
	int count = 0;
	cJSON *item;

	if (!s || !obj)
		ERROR_RETURN(-1);

	memset(s, 0, sizeof(ALLEGRO_SPRITE));

	item = obj->child;
	if (!item)
		ERROR_RETURN(-1);

	/* Get tilesets count */
	while(item) {
		count++;
		item = item->next;
	}

	s->tilesets = malloc(sizeof(ALLEGRO_SPRITE_TILESET) * count);
	if (!s->tilesets)
		ERROR_RETURN(-1);
	s->tileset_count = count;

	item = obj->child;
	for (i = 0; i < s->tileset_count; i++) {
		ret = al_parse_sprite_tileset(&s->tilesets[i], item);
		if (ret)
			ERROR_RETURN(ret);
		item = item->next;
	}
	return 0;
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

ALLEGRO_SPRITE *al_load_sprite(const char *file_name)
{
	int ret;
	cJSON *json;
	ALLEGRO_SPRITE *s;

	s = malloc(sizeof(ALLEGRO_SPRITE));
	if (!s)
		return NULL;

	json = al_json_parse(file_name);

	ret = al_parse_sprite(s, json);
	if (ret) {
		al_destroy_sprite(s);
		return NULL;
	}

	//al_dump_sprite(s);
	return s;
}

/***************************************************************************************/
/********** Sprite Draw ****************************************************************/
/***************************************************************************************/

int al_draw_sprite(ALLEGRO_SPRITE *s)
{
	int i;
	int tileset_id;
	int tile_id;
	ALLEGRO_SPRITE_TILESET *tileset;
	ALLEGRO_SPRITE_TILE *tiles;

	if (!s->action)
		return -1;

	tileset_id = s->action->tileset_id;
	tileset = &(s->tilesets[tileset_id]);

	for (i = 0; i < tileset->layer_count; i++) {
		switch (s->direction) {
			case ALLEGRO_SPRITE_DOWN:
				tiles = tileset->layers[i].tiles_down;
				break;
			case ALLEGRO_SPRITE_UP:
				tiles = tileset->layers[i].tiles_up;
				break;
			case ALLEGRO_SPRITE_RIGHT:
				tiles = tileset->layers[i].tiles_right;
				break;
			case ALLEGRO_SPRITE_LEFT:
				tiles = tileset->layers[i].tiles_left;
			default:
				break;
		}
		tile_id = s->action->counter % (tileset->layers[i].tile_count / 4);

		al_draw_bitmap_region(tileset->layers[i].bitmap,
				tiles[tile_id].x,
				tiles[tile_id].y,
				tileset->layers[i].tile_width,
				tileset->layers[i].tile_height,
				s->x, s->y, 0);
	}
	return 0;
}

/***************************************************************************************/
/********** Sprite Position Control ****************************************************/
/***************************************************************************************/

void al_sprite_set_map_size(ALLEGRO_SPRITE *s, int map_w, int map_h)
{
	s->map_width = map_w;
	s->map_height = map_h;
}

void al_sprite_move_to(ALLEGRO_SPRITE *s, int x, int y)
{
	s->x = x;
	s->y = y;
}

int al_sprite_get_x(ALLEGRO_SPRITE *s)
{
	return s->x;
}

int al_sprite_get_y(ALLEGRO_SPRITE *s)
{
	return s->y;
}

int al_sprite_get_width(ALLEGRO_SPRITE *s)
{
	return s->w;
}

int al_sprite_get_height(ALLEGRO_SPRITE *s)
{
	return s->h;
}

void al_sprite_set_direction(ALLEGRO_SPRITE *s, int d)
{
	s->direction = d;
}

int al_sprite_get_direction(ALLEGRO_SPRITE *s)
{
	return s->direction;
}

void al_sprite_move_step(ALLEGRO_SPRITE *s, int step_x, int step_y)
{
	int x = s->x + step_x;
	int y = s->y + step_y;

	if (y + s->h > s->map_height)
		y = s->map_height - s->h;
	if (y < 0)
		y = 0;

	if (x + s->w > s->map_width)
		x = s->map_width - s->w;
	if (x < 0)
		x = 0;

	s->x = x;
	s->y = y;
}

/***************************************************************************************/
/********** Sprite Action Control ******************************************************/
/***************************************************************************************/

int al_sprite_add_action(ALLEGRO_SPRITE *s, int id, int tileset_id,
					int counter_max, int fps_interval, bool stopable)
{
	int idx = s->action_count;
	if (idx >= MAX_ACTIONS)
		return -1;

	s->actions[idx].id = id;
	s->actions[idx].running = false;
	s->actions[idx].tileset_id = tileset_id;
	s->actions[idx].counter = 0;
	s->actions[idx].counter_max = counter_max;
	s->actions[idx].interval = fps_interval;
	s->actions[idx].stopable = stopable;

	s->action_count++;
	return 0;
}

int al_sprite_start_action(ALLEGRO_SPRITE *s, int id)
{
	if (id < 0 || id > s->action_count)
		return -1;

	s->action = &s->actions[id];
	s->w = s->tilesets[s->action->tileset_id].layers[0].tile_width;
	s->h = s->tilesets[s->action->tileset_id].layers[0].tile_height;

	s->action->running = true;
	s->action->counter = 0;
	return 0;
}

void al_sprite_update_action(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return;
	if (!s->action->running)
		return;

	s->action->counter++;
	if (s->action->counter >= 2520)
		s->action->counter = 0;
}

void al_sprite_stop_action(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return;
	s->action->running = false;
	s->action->counter = 0;
}

bool al_sprite_action_running(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return 0;
	return s->action->running;
}

int al_sprite_action_id(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return -1;
	return s->action->id;
}

int al_sprite_action_counter(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return -1;
	return s->action->counter;
}

void al_sprite_action_set_counter(ALLEGRO_SPRITE *s, int counter)
{
	if (!s->action)
		return;
	s->action->counter = counter;
}

int al_sprite_action_counter_max(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return -1;
	return s->action->counter_max;
}

int al_sprite_action_fps_interval(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return -1;
	return s->action->interval;
}

bool al_sprite_action_stopable(ALLEGRO_SPRITE *s)
{
	if (!s->action)
		return false;
	return s->action->stopable;
}


