#ifndef __OUTPUT_H
#define __OUTPUT_H
#include <stdint.h>

void output_cast_creature(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success, uint8_t illusion);
void output_cast_disbelieve(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success);
void output_cast(uint8_t current_player, uint8_t source_index, uint8_t spell, uint8_t success);
void output_cast_success(uint8_t current_player, uint8_t source_index, uint8_t spell);
void output_cast_fail(uint8_t current_player, uint8_t source_index, uint8_t spell);
void output_wizard_all_creatures_destroyed(uint8_t player, uint8_t index);
void output_creature_killed(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t illusion);
void output_wizard_killed(uint8_t player, uint8_t index);
void output_magic_attack(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success);
void output_wizard_dismounted(uint8_t player, uint8_t index);
void output_movement_creature(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t flying);
void output_movement_wizard(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t flying);
#endif