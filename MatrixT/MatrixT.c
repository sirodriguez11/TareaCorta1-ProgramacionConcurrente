#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAP_ANONYMOUS 0x20
#define ITERATIONS 100

int N;
int *m2;
int *m1;
int *resultado;


/* Función que ejecuta cada hilo para la multiplicación de matrices. */
void* multi(void *i)
{
    int a = *((int *) i);

    for (int x = 0; x < N; x++) {
        int suma = 0;
        for (int j = 0; j < N; j++) {
            suma += *(m1+N*a+j) * *(m2+N*j+x);
        }
        *(resultado+N*a+x) = suma;
    }
    return NULL;
}

/* Función requerida para eliminar un directorio y su contenido. */
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, 
        struct FTW *ftwbuf) {
    int rv = remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}

int main(){

    int n;
    double time_spent = 0.0;
    double accumulator = 0.0;

    char *filename = malloc(10);
    char *stats_filename = "Stats";
    char iteration[2];
    const char *folder = "Mat_R";
    const char *prefix = "Mat_R/Mat_";
    
    
    FILE* output_file;
    FILE* stats_file;
    int i, cell;
    struct stat sb;
    

    srand (time(0));

    printf("Ingrese el valor de N para la matriz: ");
    scanf("%d",&n);  
    N = n;
    
    size_t matrix_size = sizeof(int)*N*N;
    m1 = malloc(matrix_size);  
    m2 = malloc(matrix_size); 
    resultado = malloc(matrix_size);  

    /* Si el directorio Mat_R existe, entonces eliminar. */
    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        nftw(folder, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
    }

    /* Si el archivo Stats existe, entonces eliminar. */
    remove(stats_filename);

    mkdir(folder, S_IRWXU);

    pthread_t threads[N];

    for(int j = 0; j < ITERATIONS; j++) {

        /* Se inicializan las dos matrices, y la de resultado. */
        for (int x = 0; x < N; x++){
            for (int y = 0; y < N; y++){
                int numero = rand() % (N);
                *(m1+N*x+y) = numero;

                // inicialicemos la matriz de resultado de una vez
                *(resultado+N*x+y) = 0;
            }
        }

        for (int x = 0; x < N; x++){
            for (int y = 0; y < N; y++){
                int numero = rand() % (N);
                *(m2+N*x+y) = numero;
            }
        }

        clock_t begin = clock();

        int *arg; 

        /*--------------------------------------------------------------*/
        for(int i = 0; i < N; i++){
            arg = (int *) malloc(sizeof(int));
            *arg = i;
            int rc = pthread_create(&threads[i], NULL, multi, (void *) arg);  
        }    
        for(int i = 0; i < N; i++){
            (void) pthread_join(threads[i], NULL);
        }
        /*--------------------------------------------------------------*/
   
        clock_t end = clock();
        
        
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
                fprintf(output_file, "matriz[%d][%d] = %d \t", x, y, *(resultado+N*x+y));
            }
            fprintf(output_file, "\n");
        }

        fclose(output_file);

    }

    /* Registrar el promedio de los tiempos en Stats. */

    stats_file = fopen(stats_filename, "a");

    if(stats_file == NULL) {
        printf("Error al abrir archivo Stats.");
        exit(1);
    }

    fprintf(stats_file, "\nPromedio de las 100 iteraciones: %f segundos.\n", accumulator/ITERATIONS);

    fclose(stats_file);

    free(m1);
    free(m2);
    free(resultado);


    return 0;
}