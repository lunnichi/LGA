#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "lga.h"

// Вспомогательная функция для тестов - подсчет абсолютной массы:
int count_total_mass(Cell **grid, int w, int h) 
{
    int total_particles = 0; // Счетчик
    for (int y = 0; y < h; y++) 
    {
        for (int x = 0; x < w; x++) 
        {
            if (!grid[y][x].is_obstacle) // Если не камень
            {
                for (int i = 0; i < 7; i++) // Считаем каждую частицу
                    if (grid[y][x].state & (1 << i)) total_particles++;
            }
        }
    }
    return total_particles; // Возвращаем точную цифру
}

// Первый модульный тест - проверка корректности генерации препятствия:
void test_unit_obstacle(void)
{
    int w = 20, h = 20, d = 10; // Искусственные маленькие размеры для теста
    
    Cell **grid = malloc(h * sizeof(Cell *)); // Выделение памяти под тестовую сетку
    for (int y = 0; y < h; y++)
        grid[y] = malloc(w * sizeof(Cell));
        
    init_grid(grid, w, h, 0.0f, d); // Заполняем решетку с цилиндром диаметром 10
    
    assert(grid[10][10].is_obstacle == true);  // Центр трубы (10x10) обязан быть внутри цилиндра (True)
    assert(grid[0][0].is_obstacle == false);   // Угол трубы (0x0) обязан быть пустым газом (False)
    
    for (int y = 0; y < h; y++) free(grid[y]); // Освобождение тестовой памяти
    free(grid);
    
    printf("[UNIT TEST 1] Passed: Obstacle is generated correctly.\n");
}

// Еще один модульный тест - проверка математики расчета плотности:
void test_unit_density(void)
{
    int w = 5, h = 5; // Сетка 5х5 (всего 25 клеток)
    Cell **grid = malloc(h * sizeof(Cell *));
    for (int y = 0; y < h; y++) {
        grid[y] = malloc(w * sizeof(Cell));
        for (int x = 0; x < w; x++) {
            grid[y][x].state = 0;            // Обнуляем
            grid[y][x].is_obstacle = false;  // Убираем камни
        }
    }
    
    grid[2][2].state = (1 << 0) | (1 << 1) | (1 << 2); // Вручную кладем ровно 3 частицы в центральную клетку
    
    // Плотность: 3 частицы / 25 клеток = 0.12:
    double avg = calc_avg_density(grid, w, h); // Запускаем расчет
    assert(avg > 0.11 && avg < 0.13); // Проверяем, что ответ правильный с учетом погрешности double
    
    for (int y = 0; y < h; y++) free(grid[y]); // Чистим память
    free(grid);
    
    printf("[UNIT TEST 2] Passed: Density calculation math is correct.\n");
}

// Первый интеграционный тест (Итерационный):
void test_iter_mass_conservation(void)
{
    int w = 20, h = 20;
    Cell **grid1 = malloc(h * sizeof(Cell *)); // Создаем две тестовые сетки
    Cell **grid2 = malloc(h * sizeof(Cell *));
    for (int y = 0; y < h; y++) {
        grid1[y] = malloc(w * sizeof(Cell));
        grid2[y] = malloc(w * sizeof(Cell));
    }
    
    init_grid(grid1, w, h, 0.0f, 0); // Инициализируем без препятствий
    init_grid(grid2, w, h, 0.0f, 0); 
    
    
    grid1[5][5].state = 127; // Вручную зажигаем ВСЕ 7 направлений в одной клетке
    int initial_mass = count_total_mass(grid1, w, h); // Считаем начальную массу
    assert(initial_mass == 7); // Проверяем, что в системе ровно 7 частиц

    Cell **src = grid1;
    Cell **dst = grid2;
    
    for (int step = 0; step < 3; step++) // Гоняем 3 шага симуляции
    {
        stream(src, dst, w, h, 0.0f);  // Движение (плотность нагнетания 0, чтобы новые частицы не появлялись)
        collide(dst, w, h);            // Столкновения
        
        Cell **tmp = src; // Смена буферов
        src = dst;
        dst = tmp;
    }
    
    int final_mass = count_total_mass(src, w, h); // Считаем массу после симуляции
    assert(initial_mass == final_mass); // ГЛАВНЫЙ ЗАКОН: Масса не могла исчезнуть, 7 должно остаться 7.


    for (int y = 0; y < h; y++) // Чистим память
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
    
    init_grid(grid1, w, h, 0.0f, 4); // Инициализация с цилиндром
    init_grid(grid2, w, h, 0.0f, 4);
    
    Cell **src = grid1;
    Cell **dst = grid2;
    
    // 10 итераций подряд:
    for (int step = 0; step < 10; step++) // Проверяем стабильность системы в динамике
    {
        stream(src, dst, w, h, 0.3f); // Нагнетаем поток плотностью 0.3
        collide(dst, w, h);
        
        Cell **tmp = src;
        src = dst;
        dst = tmp;
    }
    
    // Проверяем, что поток за 10 шагов успешно продвинулся дальше:
    double final_density = calc_avg_density(src, w, h);
    assert(final_density > 0.0); // Труба больше не вакуумная, плотность > 0
    
    for (int y = 0; y < h; y++) { // Очистка
        free(grid1[y]);
        free(grid2[y]);
    }
    free(grid1);
    free(grid2);
    
    printf("[ITER TEST 2] Passed: 10 iteration loops finished successfully.\n");
}

int main(void) // Главная функция для сборки тестов
{
    printf("RUNNING PROJECT TESTS:\n"); // Заголовок

    init_collision_table();         // Обязательно собираем таблицу перед тестами
    test_unit_obstacle();           // Запускаем тест препятствия
    test_unit_density();            // Запускаем тест расчетов
    test_iter_mass_conservation();  // Запускаем тест закона сохранения
    test_iter_multi_step();         // Запускаем тест 10-ти итераций
    
    printf("=============================\n");
    printf("SUCCESS: 2 unit and 2 iteration tests passed.\n"); // Итог
    return 0; // Конец программы
}