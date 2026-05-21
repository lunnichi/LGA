#include <stdio.h>
#include "lga.h"

void save_ppm(Cell **grid, int w, int h, int step)
{
    char filename[64];
    sprintf(filename, "frame_%04d.ppm", step);
    FILE *f = fopen(filename, "wb");
    if (!f)
        return;

    fprintf(f, "P6\n%d %d\n255\n", w, h);

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            uint8_t color[3] = {0, 0, 0}; // Черный фон
            
            if (grid[y][x].is_obstacle)
            {
                // Рисуем препятствие серым цветом:
                color[0] = 128; color[1] = 128; color[2] = 128;
            }
            else
            {
                // Считаем частицы для отрисовки газа:
                int particles = 0;
                for (int i = 0; i < 7; i++) {
                    if (grid[y][x].state & (1 << i))
                        particles++;
                }

                if (particles > 0) {
                    // Сине-голубой цвет газа. Чем больше частиц в клетке, тем он светлее:
                    color[0] = 0;
                    color[1] = particles * 40; 
                    color[2] = particles * 35 + 50; 
                    
                    // Защита от переполнения цвета:
                    if (color[1] > 255) color[1] = 255;
                    if (color[2] > 255) color[2] = 255;
                }
            }
            fwrite(color, 1, 3, f);
        }
    }
    fclose(f);
}

void load_bin(Cell **grid, int w, int h, const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return;

    int file_w, file_h;
    fread(&file_w, sizeof(int), 1, f);
    fread(&file_h, sizeof(int), 1, f);

    if (file_w == w && file_h == h)
    {
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                fread(&grid[y][x], sizeof(Cell), 1, f);
    }

    fclose(f);
}

void save_bin(Cell **grid, int w, int h, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
        return;

    fwrite(&w, sizeof(int), 1, f);
    fwrite(&h, sizeof(int), 1, f);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            fwrite(&grid[y][x], sizeof(Cell), 1, f);

    fclose(f);
}

double calc_avg_density(Cell **grid, int w, int h)
{
    long long total = 0;
    int cells = 0;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if (!grid[y][x].is_obstacle)
            {
                for (int i = 0; i < 7; i++)
                    if (grid[y][x].state & (1 << i))
                        total++;
                cells++;
            }
        }
    }
    return (double)total / cells;
}