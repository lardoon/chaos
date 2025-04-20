#ifndef __OUTPUT_H
#define __OUTPUT_H
#include <stdint.h>

void output_cast_creature(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success, uint8_t illusion);
void output_cast_disbelieve(uint8_t current_player, uint8_t source_index, uint8_t target_index, uint8_t spell, uint8_t success);
void output_cast_fail(uint8_t current_player, uint8_t source_index, uint8_t spell);
#endif