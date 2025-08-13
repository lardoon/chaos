#ifndef __OUTPUT_H
#define __OUTPUT_H
#include <stdint.h>

void output_cast_creature(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success, uint8_t illusion);
void output_cast_disbelieve(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success);
void output_cast(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t spell, uint8_t success);
void output_cast_success(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t spell);
void output_cast_fail(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t spell);
void output_wizard_all_creatures_destroyed(int8_t chaos, uint8_t round, uint8_t player, uint8_t index);
void output_creature_killed(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t illusion);
void output_wizard_killed(int8_t chaos, uint8_t round, uint8_t player, uint8_t index);
void output_magic_attack(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success);
void output_wizard_dismounted(int8_t chaos, uint8_t round, uint8_t player, uint8_t index);
void output_movement_creature(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t flying);
void output_movement_wizard(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t flying);
void output_attack(int8_t chaos, uint8_t round, uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t ranged, uint8_t success);
void output_win(int8_t chaos, uint8_t round, uint8_t player);
void output_draw(int8_t chaos, uint8_t round, int count, uint8_t* players);
void output_destroyed(int8_t chaos, uint8_t round, uint8_t index);
void output_new_spell(int8_t chaos, uint8_t round, uint8_t player, uint8_t index, uint8_t spell);
#endif