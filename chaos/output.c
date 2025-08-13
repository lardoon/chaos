#include "output.h"
#include "jWrite.h"
#include <stdio.h>
#include <stdint.h>

int get_player_at(int);
void get_xy(int, uint8_t*, uint8_t*);
int get_owner(int);
int get_spell(int);

#define OUTPUT_ACTION_CAST 0
#define OUTPUT_ACTION_KILLED 1
#define OUTPUT_ACTION_ALL_CREATURES_DESTROYED 2
#define OUTPUT_ACTION_WIZARD_KILLED 3
#define OUTPUT_ACTION_MAGIC_ATTACK 4
#define OUTPUT_ACTION_DISMOUNT 5
#define OUTPUT_ACTION_MOVE 6
#define OUTPUT_ACTION_ATTACK 7
#define OUTPUT_WIN 8
#define OUTPUT_DRAW 9
#define OUTPUT_DESTROYED 10
#define OUTPUT_NEW_SPELL 11

#define OUTPUT_WIZARD_KEY "wizard"
#define OUTPUT_TARGET_WIZARD_KEY "target_wizard"
#define OUTPUT_SOURCE_POSITION_KEY "source_position"
#define OUTPUT_ACTION_KEY "action"
#define OUTPUT_SPELL_KEY "spell"
#define OUTPUT_TARGET_POSITION_KEY "target_position"
#define OUTPUT_TARGET_KEY "target"
#define OUTPUT_SUCCESS_KEY "success"
#define OUTPUT_ILLUSION_KEY "illusion"
#define OUTPUT_FLYING_KEY "flying"
#define OUTPUT_RANGED_ATTACK_KEY "ranged"
#define OUTPUT_SOURCE_KEY "source"
#define OUTPUT_ROUND_KEY "round"
#define OUTPUT_CHAOS_KEY "chaos"

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

void output_cast_creature(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success, uint8_t illusion) {

	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_CAST);
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
		jw_key(OUTPUT_ILLUSION_KEY); jw_bool(illusion);
	jwClose();
	write(json);
}

void output_cast_disbelieve(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success) {
	int target_wizard = get_owner(target_index);
	int target = get_spell(target_index);
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
	jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_CAST);
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_KEY);	jw_int(target);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(target_wizard);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
	jwClose();
	write(json);
}

void output_cast(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t spell, uint8_t success) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
	jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_CAST);
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
	jwClose();
	write(json);
}

void output_cast_success(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t spell) {
	output_cast(chaos, round, current_player, source_index, spell, 1);
}

void output_cast_fail(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t spell) {
	output_cast(chaos, round, current_player, source_index, spell, 0);
}

void output_wizard_all_creatures_destroyed(int8_t chaos, uint8_t round, uint8_t player, uint8_t index) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
	jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(player);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_ALL_CREATURES_DESTROYED);
	jwClose();
	write(json);
}

void output_creature_killed(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t illusion) {
	int target_wizard = get_owner(target_index);
	int target = get_spell(target_index);
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_KILLED);
		jw_key(OUTPUT_TARGET_KEY); jw_int(target);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(target_wizard);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_ILLUSION_KEY); jw_bool(illusion);
	jwClose();
	write(json);
}

void output_wizard_killed(int8_t chaos, uint8_t round, uint8_t player, uint8_t index) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(player);
		jw_key(OUTPUT_TARGET_POSITION_KEY); coordinates(index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_WIZARD_KILLED);
	jwClose();
	write(json);
}

void output_magic_attack(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success) {
	int target_wizard = get_owner(target_index);
	int target = get_spell(target_index);
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_MAGIC_ATTACK);
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_KEY);	jw_int(target);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(target_wizard);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
	jwClose();
	write(json);
}

void output_wizard_dismounted(int8_t chaos, uint8_t round, uint8_t player, uint8_t index) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(player);
		jw_key(OUTPUT_TARGET_POSITION_KEY); coordinates(index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_DISMOUNT);
	jwClose();
	write(json);
}

void output_movement_creature(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t flying) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_MOVE);
		jw_key(OUTPUT_SPELL_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_FLYING_KEY); jw_bool(flying);
	jwClose();
	write(json);
}

void output_movement_wizard(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t flying) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_MOVE);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_FLYING_KEY); jw_bool(flying);
	jwClose();
	write(json);
}

void output_attack(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t ranged, uint8_t success) {
	int target_wizard = get_owner(target_index);
	int target = get_spell(target_index);
	int source = get_spell(source_index);
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(current_player);
		jw_key(OUTPUT_SOURCE_KEY);	jw_int(source);
		jw_key(OUTPUT_SOURCE_POSITION_KEY);	coordinates(source_index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_ACTION_ATTACK);
		jw_key(OUTPUT_TARGET_KEY);	jw_int(target);
		jw_key(OUTPUT_TARGET_WIZARD_KEY); jw_int(target_wizard);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(target_index);
		jw_key(OUTPUT_SUCCESS_KEY); jw_bool(success);
		jw_key(OUTPUT_RANGED_ATTACK_KEY); jw_bool(ranged);
	jwClose();
	write(json);
}

void output_win(int8_t chaos, uint8_t round, uint8_t player) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(player);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_WIN);
	jwClose();
	write(json);
}

void output_draw(int8_t chaos, uint8_t round, int count, uint8_t* players) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY);
			jw_array();
				for (int c = 0; c < count; c++) {
					jw_int(players[c]);
				}
			jwEnd();
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_DRAW);
	jwClose();
	write(json);
}


void output_destroyed(int8_t chaos, uint8_t round, uint8_t index) {
	uint8_t owner = get_owner(index);
	uint8_t spell = get_spell(index);
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(owner);
		jw_key(OUTPUT_TARGET_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_DESTROYED);
	jwClose();
	write(json);
}

void output_new_spell(int8_t chaos, uint8_t round, uint8_t player, uint8_t index, uint8_t spell) {
	char json[OUTPUT_BUFFER_SIZE];
	jwOpen(json, OUTPUT_BUFFER_SIZE, JW_OBJECT, JW_COMPACT);
		jw_key(OUTPUT_CHAOS_KEY); jw_int(chaos);
		jw_key(OUTPUT_ROUND_KEY); jw_int(round);
		jw_key(OUTPUT_WIZARD_KEY); jw_int(player);
		jw_key(OUTPUT_TARGET_KEY); jw_int(spell);
		jw_key(OUTPUT_TARGET_POSITION_KEY);	coordinates(index);
		jw_key(OUTPUT_ACTION_KEY); jw_int(OUTPUT_NEW_SPELL);
	jwClose();
	write(json);
}