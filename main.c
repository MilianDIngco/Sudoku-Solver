#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>


#define _POSIX_C_SOURCE 200809L //magic line idk what this does tbh


typedef struct {
    int index;
    int solution;
} Index;


int board[81];
int boardrows[81];
int boardcols[81];
int boardsquares[81];
int board_counter = 0;




/*sets a bit given a position and a bitmap*/
void set(int* bitmap, int pos) {
    *bitmap = *bitmap | (1 << pos);
}


void reset(int* bitmap, int pos) {
    *bitmap = *bitmap & ~(1 << pos);
}


int check(int* bitmap, int pos) {
    return *bitmap & (1 << pos);
}


void* parse_rows(void* param) {
    for(int i = 0; i < 81; i++) {
        boardrows[i] = board[i];
    }
	pthread_exit(0);
}


void* parse_cols(void* param) {
    for(int i = 0; i < 81; i++) {
        boardcols[i * 9 - (80 * (i / 9))] = board[i];
    }
	pthread_exit(0);
}


void* parse_squares(void* param) {
    for(int i = 0; i < 81; i++) {
        int mod_counter = i / 3;
        boardsquares[(mod_counter * 3 - (8 * (mod_counter / 3)) + 6 * (mod_counter / 9)) * 3 + (i % 3)] = board[i];
    }
	pthread_exit(0);
}


void* check_cols(void* param) {
    //unpack parameters
    Index* args = (Index*)param;
    args->solution = 1;
    int bitmap = 0;


    for(int i = args->index; (i < args->index + 9 && args->solution); i++) {
        if(check(&bitmap, boardcols[i]) == 0) {
            set(&bitmap, boardcols[i]);
        } else {
            args->solution = 0;
            break;
        }
    }


    pthread_exit(0);            
}


void* check_rows(void* param) {
    //unpack parameters
    Index* args = (Index*)param;
    args->solution = 1;
    int bitmap = 0;


    for(int i = args->index; (i < args->index + 9 && args->solution); i++) {
        if(check(&bitmap, boardrows[i]) == 0) {
            set(&bitmap, boardrows[i]);
        } else {
            args->solution = 0;
            break;
        }
    }


    pthread_exit(0);
}


void* check_squares(void* param) {
    //unpack parameters
    Index* args = (Index*)param;
    args->solution = 1;
    int bitmap = 0;


    for(int i = args->index; (i < args->index + 9 && args->solution); i++) {
        if(check(&bitmap, boardsquares[i]) == 0) {
            set(&bitmap, boardsquares[i]);
        } else {
            args->solution = 0;
            break;
        }
    }


    pthread_exit(0);
}




int main(int argc, char** argv) {


    if(argc < 2) {
        //printf("enter an option");
        //return 1;
    }


    //open input.txt as read only
    FILE* file = fopen("input.txt", "r");
    if(file == NULL) {
        perror("failed to open file");
    }
    //get to end of file and get file size from position
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);


    //read file
    char word[2];
    while(fscanf(file, "%1s", word) == 1) {
        //store data in int array
        board[board_counter] = atoi(word);
        board_counter++;
    }

    //solution flag
    int solution = 1;
    //param structs for threads
    Index col_param[9];
    Index row_param[9];
    Index square_param[9];
   
    struct timespec ts_start;
    struct timespec ts_end;
    clock_gettime(CLOCK_REALTIME, &ts_start);
    //printf("Seconds: %ld, Nanoseconds: %ld\n", ts_start.tv_sec, ts_start.tv_nsec);
    if(atoi(argv[1]) == 1) {
    /* ----------------------------------------------------Single Threaded check---------------------------------------------------- */
    //parse array
    for(int i = 0; i < 81; i++) {
        boardrows[i] = board[i];
        boardcols[i * 9 - (80 * (i / 9))] = board[i];
        int mod_counter = i / 3;
        boardsquares[(mod_counter * 3 - (8 * (mod_counter / 3)) + 6 * (mod_counter / 9)) * 3 + (i % 3)] = board[i];
    }

    //bitmaps for singlethreaded checks
    int columns[9] = { 0 };
    int rows[9] = { 0 };
    int squares[9] = { 0 };
    for(int i = 0; (i < 9 && solution); i++) {
        for(int n = 0; n < 9; n++) {
            //check columns
            if(check(&columns[i], boardcols[i * 9 + n]) == 0) {
                set(&columns[i], boardcols[i * 9 + n]);
            } else {
                solution = 0;
                break;
            }  
            //check rows
            if(check(&rows[i], boardrows[i * 9 + n]) == 0) {
                set(&rows[i], boardrows[i * 9 + n]);
            } else {
                solution = 0;
                break;
            }
            //check squares
            if(check(&squares[i], boardsquares[i * 9 + n]) == 0) {
                set(&squares[i], boardsquares[i * 9 + n]);
            } else {
                solution = 0;
                break;
            }
        }
    }


    } else if(atoi(argv[1]) == 2) {
    /* ----------------------------------------------------Multi Threaded check---------------------------------------------------- */  
    //create 3 threads to process int array
   
	// pthread_t tid_parse[3];
	// pthread_create(&tid_parse[0], NULL, parse_rows, NULL);
	// pthread_create(&tid_parse[1], NULL, parse_cols, NULL);
	// pthread_create(&tid_parse[2], NULL, parse_squares, NULL);
	// pthread_join(tid_parse[0], NULL);
	// pthread_join(tid_parse[1], NULL);
	// pthread_join(tid_parse[2], NULL);
	for(int i = 0; i < 81; i++) {
        boardrows[i] = board[i];
        boardcols[i * 9 - (80 * (i / 9))] = board[i];
        int mod_counter = i / 3;
        boardsquares[(mod_counter * 3 - (8 * (mod_counter / 3)) + 6 * (mod_counter / 9)) * 3 + (i % 3)] = board[i];
    }

    //create parameters for each thread
    //create column threads
    pthread_t tid_col[9];
    pthread_t tid_row[9];
    pthread_t tid_square[9];
    int col_valid = 1;
    int row_valid = 1;
    int square_valid = 1;
    //instantiate index and create threads
    for(int i = 0; i < 9; i++) {
        col_param[i].index = i * 9;    
        row_param[i].index = i * 9;
        square_param[i].index = i * 9;
        pthread_create(&tid_col[i], NULL, check_cols, (void*)&col_param[i]);
        pthread_create(&tid_row[i], NULL, check_rows, (void*)&row_param[i]);
        pthread_create(&tid_square[i], NULL, check_squares, (void*)&square_param[i]);
    }
    //join threads
    for(int i = 0; i < 9; i++) {
        pthread_join(tid_col[i], NULL);
        pthread_join(tid_row[i], NULL);
        pthread_join(tid_square[i], NULL);
    }  
    //check all returned solutions
    for(int i = 0; i < 9; i++) {
        col_valid = col_valid && col_param[i].solution;
        row_valid = row_valid && row_param[i].solution;
        square_valid = square_valid && square_param[i].solution;
    }
    //join all solutions
    solution = col_valid && row_valid && square_valid;


    } else if(atoi(argv[1]) == 3) {
    /* ----------------------------------------------------Multi Process check---------------------------------------------------- */  
    for(int i = 0; i < 81; i++) {
        boardrows[i] = board[i];
        boardcols[i * 9 - (80 * (i / 9))] = board[i];
        int mod_counter = i / 3;
        boardsquares[(mod_counter * 3 - (8 * (mod_counter / 3)) + 6 * (mod_counter / 9)) * 3 + (i % 3)] = board[i];
    }
    //create shared memory
    const char* name = "SOLUTION";
    const int SIZE = 4096;
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    int* ptr = (int*)mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);


    pid_t child[3];
    //create first process
    child[0] = fork();
    if(child[0] == 0) {
        int col_sol = 1;
        int columns[9] = { 0 };
        for(int i = 0; (i < 9 && solution); i++) {
            for(int n = 0; n < 9; n++) {
                //check columns
                if(check(&columns[i], boardcols[i * 9 + n]) == 0) {
                    set(&columns[i], boardcols[i * 9 + n]);
                } else {
                    col_sol = 0;
                    break;
                }
            }
        }
        ptr[0] = col_sol;
        exit(0);
    } else if(child[0] < 0) {
        perror("fork");
        exit(1);
    }
    //create second process
    child[1] = fork();
    if(child[1] == 0) {
        int row_sol = 1;
        int rows[9] = { 0 };
        for(int i = 0; (i < 9 && solution); i++) {
            for(int n = 0; n < 9; n++) {
                //check rows
                if(check(&rows[i], boardrows[i * 9 + n]) == 0) {
                    set(&rows[i], boardrows[i * 9 + n]);
                } else {
                    row_sol = 0;
                    break;
                }
            }
        }
        ptr[1] = row_sol;
        exit(0);
    } else if(child[1] < 0) {
        perror("fork");
        exit(1);
    }
    //create third process
    child[2] = fork();
    if(child[2] == 0) {
        int square_sol = 1;
        int squares[9] = { 0 };
        for(int i = 0; (i < 9 && solution); i++) {
            for(int n = 0; n < 9; n++) {
                //check squares
                if(check(&squares[i], boardsquares[i * 9 + n]) == 0) {
                    set(&squares[i], boardsquares[i * 9 + n]);
                } else {
                    square_sol = 0;
                    break;
                }
            }
        }
        ptr[2] = square_sol;
        exit(0);
    } else if(child[2] < 0) {
        perror("fork");
        exit(1);
    }
    //wait
    for(int i = 0; i < 3; i++) {
        waitpid(child[i], NULL, 0);
    }

    solution = ptr[0] && ptr[1] && ptr[2];
    shm_unlink(name);
    close(shm_fd);


    }
    clock_gettime(CLOCK_REALTIME, &ts_end);
    //printf("Seconds: %ld, Nanoseconds: %ld\n", ts_end.tv_sec, ts_end.tv_nsec);
	long diff_sec = (ts_end.tv_sec - ts_start.tv_sec);
	long diff_ns = (ts_end.tv_nsec - ts_start.tv_nsec);
   
    /* Check results */
    printf("BOARD STATE IN input.txt:\n");
    for(int i = 0; i < 81; i++) {
        printf("%d%s", boardrows[i], (((i + 1) % 9 == 0) ? "\n" : " "));
    }

    printf("SOLUTION: %s (%ld seconds, %ld nanoseconds)\n", ((solution) ? "YES" : "NO"),  diff_sec, diff_ns);
	// printf("%ld ", diff_ns);

    fclose(file);


    return 0;
}







