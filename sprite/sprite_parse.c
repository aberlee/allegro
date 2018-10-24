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
} ALLEGRO_SPRITE;

static int al_destroy_sprite(ALLEGRO_SPRITE *s)
{
	int i;

	if (s->image)
		free(s->image);

	if (s->bitmap)
		al_destroy_bitmap(s->bitmap);

	if (s->stand_frames)
		free(s->stand_frames);

	if (s->walk_frames)
		free(s->walk_frames);

	if (s->fight_frames)
		free(s->fight_frames);
	return 0;
}

static int al_init_sprite(ALLEGRO_SPRITE *s, cJSON *json)
{
	cJSON *obj1, *obj2, *obj3;
	int i;

	memset(s, 0, sizeof(ALLEGRO_SPRITE));

	/* image file info */
	obj1 = cJSON_GetObjectItem(json, "image");
	if (!obj1)
		goto error;

	obj2 = cJSON_GetObjectItem(obj1, "file");
	if (!obj2 || !cJSON_IsString(obj2))
		goto error;
	s->image = malloc(strlen(obj2->valuestring)+1);
	if (!s->image)
		goto error;
	strcpy(s->image, obj2->valuestring);

	obj2 = cJSON_GetObjectItem(obj1, "size");
	if (obj2) {
		obj3 = cJSON_GetObjectItem(obj2, "w");
		if (!obj3 || !cJSON_IsNumber(obj3))
			goto error;
		s->image_width = (int)obj3->valuedouble;
		obj3 = cJSON_GetObjectItem(obj2, "h");
		if (!obj3 || !cJSON_IsNumber(obj3))
			goto error;
		s->image_height = (int)obj3->valuedouble;
	} else
		goto error;


	/* get frame data */

	return 0;

error:
	al_destroy_sprite(s);
	return -1;
}

static void cJSON_Travel(cJSON *c, int level)
{
	int i;
	while(c) {
		for (i = 0; i < level; i++)
			printf("    ");
		if (c->string)
			printf("%s: ", c->string);
		if (cJSON_IsObject(c)) {
			printf("\n");
			if (c->child)
				cJSON_Travel(c->child, level+1);
		} else {
			if (cJSON_IsString(c))
				printf("%s", c->valuestring);
			else if (cJSON_IsNumber(c))
				printf("%f", c->valuedouble);
			else if (cJSON_IsBool(c))
				printf("%s", cJSON_IsTrue(c) ? "true" : "false");
			else if (cJSON_IsArray(c))
				printf("array[%d]", cJSON_GetArraySize(c));
			printf("\n");
		}
		c = c->next;
	}
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

int main(int argc, char **argv)
{
	cJSON *json = al_json_parse(SPRITE_FILE);
	if (!json)
		return -1;

	cJSON_Travel(json, 0);
	cJSON_Delete(json);
	return 0;
}
