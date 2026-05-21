#include <stdlib.h>
#include "lga.h"

// Таблица столкновений:
uint8_t col_table[2][128];

void init_collision_table()
{
    // Физические векторы скоростей для всех 7 направлений.
    // Умножаем их на константы, чтобы избежать дробей и использовать только целые числа:
    int px[7] = {2,  1, -1, -2, -1,  1, 0};
    int py[7] = {0, -2, -2,  0,  2,  2, 0};

    for (int i = 0; i < 128; i++)
    {
        // Считаем массу и импульс для текущего состояния клетки:
        int mass = 0, mx = 0, my = 0;
        for (int b = 0; b < 7; b++) {
            if (i & (1 << b)) {
                mass++;
                mx += px[b];
                my += py[b];
            }
        }

        // 2. Ищем все возможные состояния с точно такой же физикой:
        int valid_states[128];
        int count = 0;

        for (int j = 0; j < 128; j++)
        {
            int m2 = 0, mx2 = 0, my2 = 0;
            for (int b = 0; b < 7; b++) {
                if (j & (1 << b)) {
                    m2++;
                    mx2 += px[b];
                    my2 += py[b];
                }
            }
            // Если масса и импульс до и после совпадают - это допустимый исход столкновения:
            if (m2 == mass && mx2 == mx && my2 == my) 
            {
                valid_states[count++] = j; 
            }
        }

        // Выбираем случайный исход, чтобы заставить газ перемешиваться:
        if (count > 0) 
        {
            int chosen = valid_states[rand() % count];
            
            // Если у столкновения есть варианты, заставляем частицы изменить направление:
            if (count > 1) 
            {
                do {
                    chosen = valid_states[rand() % count];
                } while (chosen == i);
            }
            col_table[0][i] = chosen;
        } else 
        {
            col_table[0][i] = i; // Если вариантов нет, летят дальше.
        }
        col_table[1][i] = i;
    }
}

void init_grid(Cell **grid, int w, int h, float rho, int d)
{
    // Для стартового состояния игнорируем параметр rho, создавая вакуум:
    (void)rho; 
    
    double cx = w / 3.0; 
    double cy = h / 2.0;
    double r2 = (d / 2.0) * (d / 2.0);

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            // Делаем клетку абсолютно пустой (вакуум):
            grid[y][x].state = 0;
            grid[y][x].is_obstacle = false;

            double dx_c = (double)x - cx;
            double dy_c = (double)y - cy;
            
            // Устанавливаем цилиндр, газ не генерируем:
            if (dx_c * dx_c + dy_c * dy_c <= r2)
            {
                grid[y][x].is_obstacle = true;
            }
        }
    }
}

void collide(Cell **grid, int w, int h) 
{
    for (int y = 0; y < h; y++) 
    {
        for (int x = 0; x < w; x++) 
        {
            // Внутри препятствия столкновений газа нет:
            if (grid[y][x].is_obstacle) 
                continue;

            uint8_t current_state = grid[y][x].state;
            grid[y][x].state = col_table[0][current_state];
        }
    }
}

void stream(Cell **src, Cell **dst, int w, int h, float rho_in)
{
    (void)rho_in; 

    // Очистка целевой решетки перед перемещением:
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            dst[y][x].state = 0;
            dst[y][x].is_obstacle = src[y][x].is_obstacle;
        }
    }

    // Фаза перемещения:
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if (src[y][x].is_obstacle)
                continue; // Из препятствия ничего не вылетает.

            uint8_t st = src[y][x].state;
            
            // Перенос частицы покоя:
            if (st & (1 << 6))
                dst[y][x].state |= (1 << 6);

            int is_odd = y % 2; // Учет сдвига нечетных строк.

            for (int d = 0; d < 6; d++)
            {
                if (st & (1 << d))
                {
                    int nx = x;
                    int ny = y;

                    // Точный геометрический сдвиг координат для гексагональной сетки:
                    switch (d)
                    {
                        case 0: nx++; break;                            // Восток.
                        case 1: nx += is_odd; ny--; break;              // Северо-Восток.
                        case 2: nx += is_odd - 1; ny--; break;          // Северо-Запад.
                        case 3: nx--; break;                            // Запад.
                        case 4: nx += is_odd - 1; ny++; break;          // Юго-Запад.
                        case 5: nx += is_odd; ny++; break;              // Юго-Восток.
                    }

                    // Граничное условие «Верх/Низ»: Зеркальное скольжение/отражение:
                    if (ny < 0 || ny >= h)
                    {
                        int bounce_dir = (d + 3) % 6; // Разворот на 180 градусов.
                        dst[y][x].state |= (1 << bounce_dir); // Возвращаем частицу обратно.
                        continue;
                    }

                    // Левая граница - поглощение вылетающих влево частиц:
                    if (nx < 0)
                        continue;

                    // Правая граница - свободный выход газа:
                    if (nx >= w)
                        continue;

                    // Отскок от препятствия:
                    if (dst[ny][nx].is_obstacle)
                    {
                        int bounce_dir = (d + 3) % 6; // Разворот вектора обратно при ударе.
                        dst[y][x].state |= (1 << bounce_dir); 
                    }
                    else
                    {
                        // Обычный бесконфликтный перенос в целевую ячейку:
                        dst[ny][nx].state |= (1 << d);
                    }
                }
            }
        }
    }

    for (int y = 0; y < h; y++)
    {
        if (!dst[y][0].is_obstacle)
        {
            // Принудительно ставим частицы, летящие вправо, вверх-вправо и вниз-вправо:
            dst[y][0].state |= (1 << 0) | (1 << 1) | (1 << 5);
        }
    }
}