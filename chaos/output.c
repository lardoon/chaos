#include "output.h"
#include "jWrite.h"
#include <stdio.h>
#include <stdint.h>

int get_player_at(int);
void get_xy(int, uint8_t*, uint8_t*);
int get_owner(int);
int get_spell(int);

#define OUTPUT_WIZARD_KEY "wizard"
#define OUTPUT_TARGET_WIZARD_KEY "target_wizard"
#define OUTPUT_SOURCE_POSITION_KEY "source_position"
#define OUTPUT_ACTION_KEY "action"
#define OUTPUT_SPELL_KEY "spell"
#define OUTPUT_TARGET_POSITION_KEY "target_position"
#define OUTPUT_TARGET_KEY "target"
#define OUTPUT_SUCCESS_KEY "success"
#define OUTPUT_ILLUSION_KEY "illusion"

#define OUTPUT_FILE "output.json"

#define OUTPUT_BUFFER_SIZE 1024

static void coordinates(uint8_t index) {
	uint8_t x, y;
	get_xy(index, &x, &y);
	jw_array();
		jw_int(x);
		jw_int(y);
	jwEnd();
}

static void write(char* json) {
	FILE* f = fopen(OUTPUT_FILE, "a");
	fprintf(
		f, "%s\n", json
	);
	fclose(f);
}

void output_cast_creature(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success, uint8_t illusion) {

	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_string("cast");
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
		jw_key(OUTPUT_ILLUSION_KEY); jw_bool(illusion);
	jwClose();
	write(json);
}

void output_cast_disbelieve(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success) {
	int target_wizard = get_owner(target_index);
	int target = get_spell(target_index);
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_string("cast");
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_KEY);	jw_int(target);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(target_wizard);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
	jwClose();
	write(json);
}

void output_cast_fail(uint8_t current_player, uint8_t source_index, uint8_t spell) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_string("cast");
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(0);
	jwClose();
	write(json);
}

