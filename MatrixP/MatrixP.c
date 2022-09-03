#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <ftw.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/wait.h>

#define MAP_ANONYMOUS 0x20

/* Función requerida para crear un espacio de memoria compartida
   entre los procesos. */
void* create_shared_memory(size_t size) {
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    return mmap(NULL, size, protection, visibility, -1, 0);
}

/* Función requerida para eliminar un directorio y su contenido. */
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, 
        struct FTW *ftwbuf) {
    int rv = remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}

int main() {

    int N;
    char *filename = malloc(10);
    char *stats_filename = "Stats";
    char iteration[2];
    const char *folder = "Mat_R";
    const char *prefix = "Mat_R/Mat_";
    double time_spent = 0.0;
    double accumulator = 0.0;
    FILE* output_file;
    FILE* stats_file;
    int i, cell;

    printf("Ingrese el valor de N para la matriz: ");
    scanf("%d",&N);

    int pids[N];
    struct stat sb;
    size_t matrix_size = sizeof(int)*N*N;

    srand(time(0));

    /* Si el directorio Mat_R existe, entonces eliminar. */
    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        nftw(folder, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
    }

    /* Si el archivo Stats existe, entonces eliminar. */
    remove(stats_filename);

    mkdir(folder, S_IRWXU);

    int* shmem1 = create_shared_memory(matrix_size);
    int* shmem2 = create_shared_memory(matrix_size);
    int* shmem3 = create_shared_memory(matrix_size);

    int *matrix1 = malloc(matrix_size);
    int *matrix2 = malloc(matrix_size);
    int *matrix3 = malloc(matrix_size);

    /*---------------------------------------------------------*/

    for(int j = 0; j < 100; j++) {

        /* Crear primera matriz, segunda matriz y matriz resultado */

        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                int random = rand() % N;
                *(matrix1+N*x+y) = random;
            }
        }

        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                int random = rand() % N;
                *(matrix2+N*x+y) = random;
            }
        }

        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                *(matrix3+N*x+y) = 0;
            }
        }

    /*---------------------------------------------------------*/

        /* Colocar las matrices en memoria compartida. */
    
        memcpy(shmem1, matrix1, matrix_size);
        memcpy(shmem2, matrix2, matrix_size);
        memcpy(shmem3, matrix3, matrix_size);

        /* Calcular la matriz resultado tomando el tiempo. */

        clock_t begin = clock();

        for (i = 0; i < N; i++) {
            pids[i] = fork();

            if (pids[i] == -1) {
                printf("Error al crear un nuevo proceso. \n");
                return 1;
            }

            if(pids[i] == 0) {
                /* Proceso hijo */

                for (int row = 0; row < N; row++) {
                    cell = 0;
                    for (int col = 0; col < N; col++) {
                        cell += *(shmem1+N*i+col) * *(shmem2+N*col+row);
                    }
                    *(shmem3+N*i+row) = cell;
                }

                return 0;
            }
        }

        for (i = 0; i < N; i++) {
            wait(NULL);
        }

        clock_t end = clock();

    /*---------------------------------------------------------*/

        /* Registrar el tiempo de cada iteración en Stats. */

        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        accumulator += time_spent;

        stats_file = fopen(stats_filename, "a");

        if(stats_file == NULL) {
            printf("Error al abrir archivo Stats.");
            exit(1);
        }

        fprintf(stats_file, "Tiempo transcurrido en iteración %d: %f segundos.\n", j, time_spent);

        fclose(stats_file);

    /*---------------------------------------------------------*/

        /* Crear un archivo Mat_R para la iteración y registrar la 
            matriz resultado respectiva en el archivo. */

        sprintf(iteration, "%d", j);
        strcpy(filename, prefix);
        strcat(filename, iteration);

        output_file = fopen(filename,"w");

        if(output_file == NULL) {
            printf("Error al crear archivo %s.", filename);
            exit(1);
        }

        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                fprintf(output_file, "matriz[%d][%d] = %d \t", x, y, *(shmem3+N*x+y));
            }
            fprintf(output_file, "\n");
        }

        fclose(output_file);
    }

    /*---------------------------------------------------------*/

    /* Registrar el promedio de los tiempos en Stats. */

    stats_file = fopen(stats_filename, "a");

    if(stats_file == NULL) {
        printf("Error al abrir archivo Stats.");
        exit(1);
    }

    fprintf(stats_file, "\nPromedio de las 100 iteraciones: %f segundos.\n", accumulator/100);

    fclose(stats_file);

    /*---------------------------------------------------------*/

    free(filename);
    free(matrix1);
    free(matrix2);
    free(matrix3);
    munmap(shmem1, matrix_size);
    munmap(shmem2, matrix_size);
    munmap(shmem3, matrix_size);

    return 0;
}