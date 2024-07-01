#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reservar.h"

// Defina aca las variables globales y funciones auxiliares que necesite

#define N_EST 10

// ------------------------- podria ir en un typdef ------------------------------
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int estacionamientos[N_EST]; 
// quiza campo donde se guarde cual es su primer estacionamiento ocupado?
// -------------------------------------------------------------------------------

int ticket_dist = 0, display = 0; //distribución de tickets & display


void initReservar() {
}

void cleanReservar() {
}

int check_disponible(int k){
  int contador = 0;
  for(int i = 0; i < N_EST; i++){
    if (estacionamientos[i] == 0)
    {
      contador++;
      if(contador == k){
        return i-k+1;
      }
    }
    else{
      contador = 0;
    }   
  }

  return -1; // no lo logró, retorno un num especial
}

void completar_reserva(int ocupar_desde, int nro_estacionamientos){
  int cont = 0;
  int i = ocupar_desde;
  while(cont < nro_estacionamientos){
    estacionamientos[i] = 1;
    i++;
    cont++;
  }
}

void completar_liberar(int desocupar_desde, int nro_estacionamientos){
  int cont = 0;
  int i = desocupar_desde;
  while(cont < nro_estacionamientos){
    estacionamientos[i] = 0;
    i++;
    cont++;
  }
}

int reservar(int k) {
  // ---- sección critica ----
  pthread_mutex_lock(&m);
  int my_num = ticket_dist++; //esto es post incremento, EJ: para el primero guarda 0 como su ticket y luego lo aumenta
  // Si hay k contiguos disponibles, se reservan
  while(my_num != display || check_disponible(k)==-1){ //agregar not (hay suficientes estacionamientos disponibles)
    pthread_cond_wait(&cond, &m);
  }
  // Si lo consiguio, entonces los reserva y luego retorna
  int ocupando_desde = check_disponible(k);
  //printf("toca estacionar en: %d\n", ocupando_desde);
  completar_reserva(ocupando_desde, k);
  display++;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&m);
  // ---- sección critica ----
  
  return ocupando_desde; //retorna la identificación del primer estacionamiento que ocupa
}

void liberar(int e, int k) { // e : primer estacionamiento que ocupaba, k : cuantos se le habían otorgado
  // ---- sección critica ----
  pthread_mutex_lock(&m);
  completar_liberar(e, k);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&m);
  // ---- sección critica ----
} 
