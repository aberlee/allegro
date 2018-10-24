#include <stdio.h>
#include <stdlib.h>
#include <cJSON.h>
#include <cJSON_Utils.h>

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
			printf("\n");
		}
		c = c->next;
	}
}

int main(int argc, char **argv)
{
	char *fn = argv[1];
	FILE *fp = NULL;
	char *data = NULL;
	int size = 0;
	cJSON *json = NULL;

	if (!fn) {
		printf("usage: %s fn\n", argv[0]);
		return -1;
	}

	fp = fopen(fn, "r");
	if (!fp) {
		printf("Open file [%s] error!\n", fn);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	data = malloc(size);
	if (!data) {
		printf("Can't malloc memory, %d bytes\n", size);
		fclose(fp);
		return -1;
	}

	fread(data, 1, size, fp);
	fclose(fp);

	json = cJSON_Parse(data);
	if (!json) {
		printf("Parse json data error!\n");
		goto exit;
	}

	cJSON_Travel(json, 0);
	cJSON_Delete(json);

exit:
	if (data)
		free(data);
	return 0;
}
