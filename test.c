#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "lga.h"


// Проверка законов физики:
int count_total_mass(Cell **grid, int w, int h) 
{
    int total_particles = 0;
    for (int y = 0; y < h; y++) 
    {
        for (int x = 0; x < w; x++) 
        {
            if (!grid[y][x].is_obstacle) 
            {
                for (int i = 0; i < 7; i++) 
                    if (grid[y][x].state & (1 << i)) total_particles++;
            }
        }
    }
    return total_particles;
}

// Первый модульный тест - проверка корректности генерации препятствия:
void test_unit_obstacle(void)
{
    int w = 20, h = 20, d = 10;
    
    Cell **grid = malloc(h * sizeof(Cell *));
    for (int y = 0; y < h; y++)
        grid[y] = malloc(w * sizeof(Cell));
        
    init_grid(grid, w, h, 0.0f, d);
    
    assert(grid[10][10].is_obstacle == true);
    assert(grid[0][0].is_obstacle == false);
    
    for (int y = 0; y < h; y++) free(grid[y]);
    free(grid);
    
    printf("[UNIT TEST 1] Passed: Obstacle is generated correctly.\n");
}

// Еще один модульный тест - проверка математики расчета плотности:
void test_unit_density(void)
{
    int w = 5, h = 5;
    Cell **grid = malloc(h * sizeof(Cell *));
    for (int y = 0; y < h; y++) {
        grid[y] = malloc(w * sizeof(Cell));
        for (int x = 0; x < w; x++) {
            grid[y][x].state = 0;
            grid[y][x].is_obstacle = false;
        }
    }
    
    grid[2][2].state = (1 << 0) | (1 << 1) | (1 << 2);
    
    // Плотность: 3 частицы / 25 клеток = 0.12:
    double avg = calc_avg_density(grid, w, h);
    assert(avg > 0.11 && avg < 0.13);
    
    for (int y = 0; y < h; y++) free(grid[y]);
    free(grid);
    
    printf("[UNIT TEST 2] Passed: Density calculation math is correct.\n");
}

// Первый итерационный тест:
void test_iter_mass_conservation(void)
{
    int w = 20, h = 20;
    Cell **grid1 = malloc(h * sizeof(Cell *));
    Cell **grid2 = malloc(h * sizeof(Cell *));
    for (int y = 0; y < h; y++) {
        grid1[y] = malloc(w * sizeof(Cell));
        grid2[y] = malloc(w * sizeof(Cell));
    }
    
    init_grid(grid1, w, h, 0.0f, 0);
    init_grid(grid2, w, h, 0.0f, 0); 
    
    
    grid1[5][5].state = 127;
    int initial_mass = count_total_mass(grid1, w, h);
    assert(initial_mass == 7); 

    Cell **src = grid1;
    Cell **dst = grid2;
    
    for (int step = 0; step < 3; step++) 
    {
        stream(src, dst, w, h, 0.0f); 
        collide(dst, w, h);
        
        Cell **tmp = src;
        src = dst;
        dst = tmp;
    }
    
    int final_mass = count_total_mass(src, w, h);
    assert(initial_mass == final_mass); 


    for (int y = 0; y < h; y++) 
    {
        free(grid1[y]);
        free(grid2[y]);
    }
    free(grid1);
    free(grid2);
    
    printf("[ITER TEST 1] Passed: Mass conservation (Physics law) holds over iterations.\n");
}

// Еще один итерационный тест:
void test_iter_multi_step(void)
{
    int w = 32, h = 16;
    Cell **grid1 = malloc(h * sizeof(Cell *));
    Cell **grid2 = malloc(h * sizeof(Cell *));
    for (int y = 0; y < h; y++) {
        grid1[y] = malloc(w * sizeof(Cell));
        grid2[y] = malloc(w * sizeof(Cell));
    }
    
    init_grid(grid1, w, h, 0.0f, 4);
    init_grid(grid2, w, h, 0.0f, 4);
    
    Cell **src = grid1;
    Cell **dst = grid2;
    
    // 10 итераций подряд:
    for (int step = 0; step < 10; step++) 
    {
        stream(src, dst, w, h, 0.3f);
        collide(dst, w, h);
        
        Cell **tmp = src;
        src = dst;
        dst = tmp;
    }
    
    // Проверяем, что поток за 10 шагов успешно продвинулся дальше:
    double final_density = calc_avg_density(src, w, h);
    assert(final_density > 0.0);
    
    for (int y = 0; y < h; y++) {
        free(grid1[y]);
        free(grid2[y]);
    }
    free(grid1);
    free(grid2);
    
    printf("[ITER TEST 2] Passed: 10 iteration loops finished successfully.\n");
}

int main(void)
{
    printf("RUNNING PROJECT TESTS:\n");

    init_collision_table();
    test_unit_obstacle();
    test_unit_density();
    test_iter_mass_conservation();
    test_iter_multi_step();
    
    printf("=============================\n");
    printf("SUCCESS: 2 unit and 2 iteration tests passed.\n");
    return 0;
}