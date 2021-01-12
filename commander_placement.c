#define DEBUG 0 //true/false
#define INPUT_SHOW 0//true/false
#define SUCCESS 0
#include <pthread.h>
#include <semaphore.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SUM(x, y) (x + y)

typedef struct gb {
    int **game_field;
    int *i_array;
    int *j_array;
} GAMEBOARD;

typedef struct input {
    int N;
    int L;
    int K;
} ARGS;

const char *inf = { "input.txt" };
const char *ouf = { "output.txt" };
const char *f_time = { "time.txt" };
FILE *f;

int thrds_qnt, answer;
GAMEBOARD gameboard;
ARGS args;
pthread_t *threads;
pthread_mutex_t mutex;

int error(const char *err);
int get_args(void);
void show_args(void);
void every_struct_init(void);
void every_struct_deinit(void);
void create_threads(void);
void* thread_entry(void *param);
int WarleaderPlacement(int *value, int row, int col, int figures_num);
bool position_check(int row, int col);
void chess(void);

int main(void) {
    if (get_args())
        return error("fopen failed.");

    create_threads();
    chess();
    every_struct_deinit();

    return 0;
}

int error(const char *err) {
    printf("ERROR: %s", err);
    return -1;
}

int get_args(void) {
    if (DEBUG)
        printf("\nNow in -> get_args\n");

    int i = 0;
    char line1[20], line_buf[20];
    if ((f = fopen(inf, "rb")) == NULL)            
        return 1;
    fscanf(f, "%d", &thrds_qnt);
    fscanf(f, "%d %d %d", &args.N, &args.L, &args.K);

    every_struct_init();

    while (i < args.K) {
        fscanf(f, "%d %d", &gameboard.i_array[i], &gameboard.j_array[i]);
        i++;
    }

    fclose(f);
    if (DEBUG)
        show_args();

    return 0;
}

void show_args(void) {
    printf("\nArgs: thrds_qnt = %d, board_size (N) = %d\n----need to place (L) = %d, already placed (K) = %d\n",
        thrds_qnt, args.N, args.L, args.K);
    if (INPUT_SHOW) {
        printf("with input arr:\n");
        for (int i = 0; i < args.K; i++)
            printf("%d %d\n", gameboard.i_array[i], gameboard.j_array[i]);
    }
}

void every_struct_init(void) {
    if (DEBUG)
        printf("\nNow in -> every_struct_init\n");

    int i, j;
    answer = 0;
    gameboard.i_array = (int*)malloc(sizeof(int) * args.K);
    gameboard.j_array = (int*)malloc(sizeof(int) * args.K);
    threads = (pthread_t*)malloc(sizeof(pthread_t) * thrds_qnt);
    pthread_mutex_init(&mutex, 0);
    gameboard.game_field = (int**)malloc(sizeof(int*) * args.N);
    for (i = 0; i < args.N; i++) {
        gameboard.game_field[i] = (int*)malloc(sizeof(int) * args.N);
        for (j = 0; j < args.N; j++) {
            gameboard.game_field[i][j] = 0;
        }
    }
}

void every_struct_deinit(void) {
    if (DEBUG)
        printf("\nNow in -> every_struct_deinit\n");
    pthread_mutex_destroy(&mutex);
    free(threads);
    free(gameboard.i_array);
    free(gameboard.j_array);
    for (int i = 0; i < args.N; i++)
        free(gameboard.game_field[i]);
    free(gameboard.game_field);
}

void create_threads(void) {
    if (DEBUG)
        printf("\nNow in -> create_threads\n");
    int ind = 0, status;
    while (ind < thrds_qnt)
    {
        status = pthread_create(&threads[ind], 0, thread_entry, (void*)(ind));
        if (status != SUCCESS)
            exit(printf("\nthread_creation ERROR!\n"));
        ind++;
    }
}

void* thread_entry(void *param) {
    int index, returned_value = 0;

    index = (int)((long)param);
    if (DEBUG)
        printf("Thread #%d created. Got mutex.\n", index + 1);

    pthread_mutex_lock(&mutex);
    WarleaderPlacement(&returned_value, 0, 0, 0);
    pthread_mutex_unlock(&mutex);
    if (DEBUG)
        printf("Thread #%d released mutex.\n", index + 1);

    if (SUCCESS == index) {
        pthread_mutex_lock(&mutex);
        answer += returned_value;
    }
    pthread_mutex_unlock(&mutex);
    if (DEBUG)
        printf("Thread #%d finished.\n", index + 1);
}

int WarleaderPlacement(int *value, int row, int col, int figures_num) {
    int cnt, i, j, offset;
    int coord1, coord2;
    bool flag, check_res;

    cnt = SUM(args.L, args.K);
    if (figures_num == cnt) {
        flag = true;
        i = 0;
        while (i < args.K) {
            coord1 = gameboard.i_array[i];
            coord2 = gameboard.j_array[i];
            if (gameboard.game_field[coord1][coord2] != 1) {
                flag = false;
                break;
            }
            ++i;
        }

        if (flag)
            *value += 1;        
    }

    if (row == args.N)
        return 0;

    j = col;
    while (j < args.N) {
        check_res = position_check(row, j);
        if (check_res == true) {
            offset = row;
            col = SUM(j, 1);
            gameboard.game_field[row][j] = 1;
            if (col == args.N) {
                col = 0;
                offset++;
            }
            
            WarleaderPlacement(value, offset, col, figures_num + 1);
            gameboard.game_field[row][j] = 0;
        }
        j++;
    }

    for (i = row + 1; i < args.N; i++) {
        for (j = 0; j < args.N; j++) {
            check_res = position_check(i, j);

            if (check_res == true) {
                offset = i;
                col = SUM(j, 1);
                gameboard.game_field[i][j] = 1;

                if (col == args.N) {
                    col = 0;
                    offset++;
                }

                WarleaderPlacement(value, offset, col, figures_num + 1);
                gameboard.game_field[i][j] = 0;
            }
        }
    }
}

bool position_check(int row, int col) {
    bool ret = false;

    if ((col - 1) >= 0 && 
        gameboard.game_field[row][col - 1])
        return ret;
    if ((row - 1) >= 0  && 
        gameboard.game_field[row - 1][col])
        return ret;
    if ((col - 1) >= 0 && (row - 1) >= 0 && 
        gameboard.game_field[row - 1][col - 1])
        return ret;
    if ((col - 2) >= 0 && (row - 1) >= 0 && 
        gameboard.game_field[row - 1][col - 2])
        return ret;
    if ((col - 1) >= 0 && (row - 2) >= 0 && 
        gameboard.game_field[row - 2][col - 1])
        return ret;
    if ((col + 1) < args.N && 
        gameboard.game_field[row][col + 1])
        return ret;
    if ((col + 1) < args.N && (row - 1) >= 0 && 
        gameboard.game_field[row - 1][col + 1])
        return ret;
    if ((col + 2) < args.N && (row - 1) >= 0 && 
        gameboard.game_field[row - 1][col + 2])
        return ret;
    if ((col + 1) < args.N && (row - 2) >= 0 && 
        gameboard.game_field[row - 2][col + 1])
        return ret;
    if ((row + 1) < args.N && 
        gameboard.game_field[row + 1][col])
        return ret;
    if ((col + 1) < args.N && (row + 1) < args.N && 
        gameboard.game_field[row + 1][col + 1])
        return ret;
    if ((col + 1) < args.N && (row + 2) < args.N && 
        gameboard.game_field[row + 2][col + 1])
        return ret;
    if ((col + 2) < args.N && (row + 1) < args.N && 
        gameboard.game_field[row + 1][col + 2])
        return ret;
    if ((col - 1) >= 0 && (row + 1) < args.N && 
        gameboard.game_field[row + 1][col - 1])
        return ret;
    if ((col - 1) >= 0 && (row + 2) < args.N && 
        gameboard.game_field[row + 2][col - 1])
        return ret;
    if ((col - 2) >= 0 && (row + 1) < args.N && 
        gameboard.game_field[row + 1][col - 2])
        return ret;

    ret = !ret;
    return ret;
}

void join_all_threads(void) {
    int i = 0;
    while (i < thrds_qnt) {
        pthread_join(threads[i], 0);
        i++;
    }
}

void chess(void) {
    int i;
    clock_t time1, time2, time_spent;

    if (DEBUG)
        printf("\nStart placing warleaders!\n");

    time1 = clock();
    join_all_threads();
    time2 = clock();

    if (DEBUG)
        printf("\Placing finished!\n");

    f = fopen(ouf, "wb");
    fprintf(f, "%d", answer);
    fclose(f);
    if (DEBUG)
        printf("\nRes: %d\n", answer);

    f = fopen(f_time, "wb");
    time_spent = (time2 - time1) / (CLOCKS_PER_SEC / 1000);
    if (DEBUG)
        printf("\nNow in out.\nPlacing time: %u\n", time_spent);
    fprintf(f, "%u", time_spent);
    fclose(f);
}
