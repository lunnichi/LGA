#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lga.h"

int main(int argc, char *argv[])
{
    // Параметры симуляции:
    int width = 500; // ширина сетки (аэродинамической трубы)
    int height = 160; // высота сетки
    int iters = 2000; // общее количество итераций
    float rho_in = 1.5; // плотность
    int diameter = 20; // диаметр
    int seed = 42; // генерация случайных чисел
    int viz_interval = 100; // интервал сохранения кадров

    srand(seed);
    init_collision_table(); // таблица физических столкновений перед стартом

    // Выделяем память для двух решеток:
    Cell **gridA = malloc(height * sizeof(Cell *));
    Cell **gridB = malloc(height * sizeof(Cell *));
    
    if (!gridA || !gridB) // проверка на выделение памяти
    {
        printf("Memory allocation error\n"); // выводим сообщение об ошибке
        return 1;
    }

    for (int y = 0; y < height; y++) // проходимся п окаждой строке по высоте
    {
        gridA[y] = malloc(width * sizeof(Cell)); // выделяем память под столбцы решетки А
        gridB[y] = malloc(width * sizeof(Cell)); // выделяем память под столбцы решетки B
        
        if (!gridA[y] || !gridB[y])
        {
            printf("Memory allocation error at row %d\n", y);
            return 1;
        }
    }

    // Инициализируем начальное состояние и препятствие:
    init_grid(gridA, width, height, rho_in, diameter);

    Cell **current = gridA; // Указатель current указывает на текущее состояние (сейчас это А)
    Cell **next = gridB; // Указатель next указывает на буфер для записи следующего шага (сейчас это В)

    // Главный цикл симуляции:
    for (int step = 0; step < iters; step++)
    {
        collide(current, width, height); // Рассчитываем столкновения внутри каждой клетки
        stream(current, next, width, height, rho_in); // Перемещаем частицы из current в соседние клетки в next

        // Сохраняем кадры с заданным интервалом:
        if (step % viz_interval == 0) // Если остаток от деления шага на 100 равен нулю
            save_ppm(next, width, height, step); // Отрисовываем и сохраняем текущий кадр в формате PPM

        // Смена буферов:
        Cell **temp = current; // Временно сохраняем указатель на старую сетку
        current = next; // Теперь новая сетка (next) становится текущей
        next = temp; // А старая сетка становится пустым буфером для следующего шага
    }

    double avg_rho = calc_avg_density(current, width, height); // Считаем среднюю плотность частиц в конце симуляции
    printf("Average density: %f\n", avg_rho); // Выводим плотность в консоль
    save_bin(current, width, height, "grid.bin"); // Сохраняем финальное состояние в бинарный файл

    for (int y = 0; y < height; y++) // Цикл очистки памяти
    {
        free(gridA[y]);
        free(gridB[y]);
    }
    free(gridA);
    free(gridB);

    return 0;
}