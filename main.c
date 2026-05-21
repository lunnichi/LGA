#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lga.h"

int main(int argc, char *argv[])
{
    // Параметры симуляции:
    int width = 500;
    int height = 160;
    int iters = 5000;
    float rho_in = 1.5;
    int diameter = 20;
    int seed = 42;
    int viz_interval = 100;

    srand(seed);
    init_collision_table();

    // Выделяем память для двух решеток:
    Cell **gridA = malloc(height * sizeof(Cell *));
    Cell **gridB = malloc(height * sizeof(Cell *));
    
    if (!gridA || !gridB)
    {
        printf("Memory allocation error\n");
        return 1;
    }

    for (int y = 0; y < height; y++)
    {
        gridA[y] = malloc(width * sizeof(Cell));
        gridB[y] = malloc(width * sizeof(Cell));
        
        if (!gridA[y] || !gridB[y])
        {
            printf("Memory allocation error at row %d\n", y);
            return 1;
        }
    }

    // Инициализируем начальное состояние и препятствие:
    init_grid(gridA, width, height, rho_in, diameter);

    Cell **current = gridA;
    Cell **next = gridB;

    // Главный цикл симуляции:
    for (int step = 0; step < iters; step++)
    {
        collide(current, width, height);
        stream(current, next, width, height, rho_in);

        // Сохраняем кадры с заданным интервалом:
        if (step % viz_interval == 0)
            save_ppm(next, width, height, step);

        // Смена буферов:
        Cell **temp = current;
        current = next;
        next = temp;
    }

    double avg_rho = calc_avg_density(current, width, height);
    printf("Average density: %f\n", avg_rho);
    save_bin(current, width, height, "grid.bin");

    for (int y = 0; y < height; y++)
    {
        free(gridA[y]);
        free(gridB[y]);
    }
    free(gridA);
    free(gridB);

    return 0;
}