#ifndef LGA_H
#define LGA_H

#include <stdint.h>
#include <stdbool.h>

// Структура клетки: хранит состояния направлений и метка препятствия
typedef struct
{
    // Битовая маска: биты c 0-5 отвечают за направления, бит 6 - частица покоя
    uint8_t state;
    bool is_obstacle;
} Cell;

void init_collision_table();
void init_grid(Cell **grid, int w, int h, float rho, int d);
void collide(Cell **grid, int w, int h);
void stream(Cell **src, Cell **dst, int w, int h, float rho_in);
void save_ppm(Cell **grid, int w, int h, int step);
void save_bin(Cell **grid, int w, int h, const char *filename);
double calc_avg_density(Cell **grid, int w, int h);

#endif
