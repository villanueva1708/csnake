#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "food_sound.h" // Embedded MP3 file as a C array

#define WIDTH 40
#define HEIGHT 20
#define DELAY 100000 // Microseconds

typedef struct Snake {
    int x, y;
    struct Snake *next;
} Snake;

typedef struct {
    int x, y;
} Food;

Snake *head = NULL;
Food food;
int direction = KEY_RIGHT;
int score = 0;
Mix_Music *food_sound = NULL;

void init_game();
void draw_game();
void move_snake();
void generate_food();
int check_collision();
void end_game();
void draw_borders();
void init_audio();
void play_food_sound();
void cleanup_audio();

int main() {
    int ch;
    init_game();
    init_audio();
    
    while (1) {
        timeout(100);
        ch = getch();
        if (ch == 'q') break;
        if (ch == KEY_UP || ch == KEY_LEFT || ch == KEY_DOWN || ch == KEY_RIGHT)
            direction = ch;
        
        move_snake();
        if (check_collision()) break;
        draw_game();
        usleep(DELAY);
    }
    
    cleanup_audio();
    end_game();
    return 0;
}

void init_game() {
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    srand(time(0));
    
    head = (Snake *)malloc(sizeof(Snake));
    head->x = WIDTH / 2;
    head->y = HEIGHT / 2;
    head->next = NULL;
    
    generate_food();
}

void init_audio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return;
    }
    
    // Write embedded MP3 data to a temporary file
    FILE *fp = fopen("/tmp/food.mp3", "wb");
    if (fp) {
        fwrite(food_mp3, 1, food_mp3_len, fp);
        fclose(fp);
    }
    
    // Load the MP3 from the temporary file
    food_sound = Mix_LoadMUS("/tmp/food.mp3");
    if (!food_sound) {
        printf("Failed to load food sound! Mix_Error: %s\n", Mix_GetError());
    }
}

void play_food_sound() {
    if (food_sound) {
        Mix_PlayMusic(food_sound, 1);
    }
}

void cleanup_audio() {
    if (food_sound) {
        Mix_FreeMusic(food_sound);
    }
    Mix_CloseAudio();
    SDL_Quit();
}

void draw_game() {
    clear();
    draw_borders();
    
    mvprintw(0, WIDTH + 5, "Score: %d", score);
    
    mvprintw(food.y, food.x, "F");
    
    attron(COLOR_PAIR(1));
    Snake *current = head;
    while (current) {
        mvprintw(current->y, current->x, "O");
        current = current->next;
    }
    attroff(COLOR_PAIR(1));
    
    refresh();
}

void draw_borders() {
    for (int i = 0; i < WIDTH + 2; i++) {
        mvprintw(0, i, "#");
        mvprintw(HEIGHT + 1, i, "#");
    }
    for (int i = 0; i < HEIGHT + 2; i++) {
        mvprintw(i, 0, "#");
        mvprintw(i, WIDTH + 1, "#");
    }
}

void move_snake() {
    int new_x = head->x, new_y = head->y;
    switch (direction) {
        case KEY_UP: new_y--; break;
        case KEY_DOWN: new_y++; break;
        case KEY_LEFT: new_x--; break;
        case KEY_RIGHT: new_x++; break;
    }
    
    Snake *new_head = (Snake *)malloc(sizeof(Snake));
    new_head->x = new_x;
    new_head->y = new_y;
    new_head->next = head;
    head = new_head;
    
    if (new_x == food.x && new_y == food.y) {
        play_food_sound();  // Play sound when food is eaten
        score += 10;
        generate_food();
    } else {
        Snake *temp = head;
        while (temp->next->next) temp = temp->next;
        free(temp->next);
        temp->next = NULL;
    }
}

void generate_food() {
    food.x = (rand() % WIDTH) + 1;
    food.y = (rand() % HEIGHT) + 1;
}

int check_collision() {
    if (head->x <= 0 || head->x >= WIDTH + 1 || head->y <= 0 || head->y >= HEIGHT + 1)
        return 1;
    
    Snake *temp = head->next;
    while (temp) {
        if (temp->x == head->x && temp->y == head->y)
            return 1;
        temp = temp->next;
    }
    return 0;
}

void end_game() {
    endwin();
    while (head) {
        Snake *temp = head;
        head = head->next;
        free(temp);
    }
    printf("Game Over! Final Score: %d\n", score);
}
