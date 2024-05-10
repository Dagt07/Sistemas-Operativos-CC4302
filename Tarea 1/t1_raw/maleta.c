#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h> //faltaba este include

#include "maleta.h"

//No es necesario hacer un DEFINE K = 8?
#define P 8

// ---- Defina aca las estructuras y funciones adicionales que necesite ----

// Función original ya definida y importada con maleta.h por ende solo la usamos


// Typdef Args para guardar lo que necesito
typedef struct {
    double *w;
    double *v;
    int *z;
    int n;
    double maxW;
    int k;
    double res;
} Args;

// Función opaca
void *opaca(void *p){
    Args *args = (Args *)p;
    // hacemos asignaciones necesarias a los campos de args
    double *w = args->w;
    double *v = args->v;
    int *z = args->z;
    int n = args->n;
    double maxW = args->maxW;
    int k = args->k;
    
    args->res = llenarMaletaSec(w, v, z, n, maxW, k);

    return NULL;
}

double llenarMaletaPar(double w[], double v[], int z[], int n,
                       double maxW, int k) {
    // ... Modifique esta funcion ...
    pthread_t pid[P];
    Args args[P];
    
    //dividimos el intervalo de forma que podamos invocar llenarMaletaSec(..., k/8) cuando llamemos a la función opaca
    int intervalo = k/8;

    // for para crear y lanzar threads
    for (int i = 0; i < P; i++){
        //asignamos a cada args[i] lo que necesitamos, cada thread de acuerdo a estos args va a trabajar en su intervalo correspondiente
        args[i].w = w;
        args[i].v = v;
        args[i].z = malloc(sizeof(int)*n);
        args[i].n = n;
        args[i].maxW = maxW;
        args[i].k = intervalo;
        //llamamos a pthread_create
        pthread_create(&pid[i], NULL, opaca, &args[i]);
    }

    double resultado = -1;
    
    // for para recolectar resultados y enterrar threads
    for (int i = 0; i < P; i++){
        pthread_join(pid[i], NULL);
        if (args[i].res != 0){ //se queda con el último que haya encontrado sol 
            //se copia el arreglo del thread exitoso al z principal
            resultado = args[i].res;
            for (int j = 0; j < n; j++){
                z[j] = args[i].z[j];
            }
        }
        free(args[i].z);
    }

    return resultado;
}
