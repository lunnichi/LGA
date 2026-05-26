#include <stdio.h> // Подключаем библиотеку для файловых операций
#include "lga.h"   // Подключаем заголовок

void save_ppm(Cell **grid, int w, int h, int step) // Функция отрисовки кадра
{
    char filename[64]; // Массив для имени файла
    sprintf(filename, "frame_%04d.ppm", step); // Форматируем имя файла
    FILE *f = fopen(filename, "wb"); // Открываем файл для записи в бинарном режиме
    if (!f) // Защита от ошибки открытия файла
        return;

    fprintf(f, "P6\n%d %d\n255\n", w, h); // Записываем заголовок PPM-файла

    for (int y = 0; y < h; y++) // Проходим по пикселям высоты
    {
        for (int x = 0; x < w; x++) // Проходим по пикселям ширины
        {
            uint8_t color[3] = {0, 0, 0}; // Массив RGB для пикселя (по умолчанию Черный фон)
            
            if (grid[y][x].is_obstacle) // Если это клетка камня
            {
                // Рисуем препятствие серым цветом:
                color[0] = 128; color[1] = 128; color[2] = 128; // Задаем R, G, B
            }
            else // Если это газ или вакуум
            {
                // Считаем частицы для отрисовки газа:
                int particles = 0; // Счетчик частиц в этой клетке
                for (int i = 0; i < 7; i++) { // Проверяем 7 битов
                    if (grid[y][x].state & (1 << i)) // Если бит активен
                        particles++; // Увеличиваем счетчик частиц
                }

                if (particles > 0) {
                    // Сине-голубой цвет газа. Чем больше частиц в клетке, тем он светлее:
                    color[0] = 0;                    // Красный канал на нуле
                    color[1] = particles * 40;       // Зеленый канал растет от плотности
                    color[2] = particles * 35 + 50;  // Синий канал базовый + растет от плотности
                    
                    // Защита от переполнения цвета (значение не может быть > 255 в байте):
                    if (color[1] > 255) color[1] = 255;
                    if (color[2] > 255) color[2] = 255;
                }
            }
            fwrite(color, 1, 3, f); // Записываем 3 байта цвета (RGB) в файл
        }
    }
    fclose(f); // Закрываем файл кадра
}

void load_bin(Cell **grid, int w, int h, const char *filename) // Загрузка бинарного сохранения (для возобновления)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return;

    int file_w, file_h; // Переменные для чтения размеров из файла
    fread(&file_w, sizeof(int), 1, f); // Читаем сохраненную ширину
    fread(&file_h, sizeof(int), 1, f); // Читаем сохраненную высоту

    if (file_w == w && file_h == h) // Если размеры файла совпадают с нашими
    {
        for (int y = 0; y < h; y++) // Проходим по высоте
            for (int x = 0; x < w; x++) // Проходим по ширине
                fread(&grid[y][x], sizeof(Cell), 1, f); // Читаем структуру Cell целиком прямо в массив
    }

    fclose(f);
}

void save_bin(Cell **grid, int w, int h, const char *filename) // Сохранение состояния в бинарный файл
{
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&w, sizeof(int), 1, f); // Пишем ширину
    fwrite(&h, sizeof(int), 1, f); // Пишем высоту

    for (int y = 0; y < h; y++) // Проход по всем ячейкам
        for (int x = 0; x < w; x++)
            fwrite(&grid[y][x], sizeof(Cell), 1, f); // Сохраняем структуру Cell целиком как сырые байты

    fclose(f);
}

double calc_avg_density(Cell **grid, int w, int h) // Функция расчета средней плотности газа
{
    long long total = 0; // Общая сумма всех частиц
    int cells = 0;       // Счетчик свободных (не каменных) клеток

    for (int y = 0; y < h; y++) // Обходим сетку
    {
        for (int x = 0; x < w; x++)
        {
            if (!grid[y][x].is_obstacle) // Если это не препятствие
            {
                for (int i = 0; i < 7; i++) // Перебираем биты направлений
                    if (grid[y][x].state & (1 << i)) // Если есть частица
                        total++; // Плюсуем в общий
                cells++; // Увеличиваем счетчик свободных клеток
            }
        }
    }
    return (double)total / cells; // Делим общее число частиц на число клеток (получаем среднюю плотность)
}