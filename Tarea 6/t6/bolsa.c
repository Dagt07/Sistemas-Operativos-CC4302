#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "pss.h"
#include "bolsa.h"
#include "spinlocks.h"

// Declare aca sus variables globales
int mutex = OPEN;
int min_price = INT_MAX;
char *global_vendor_name = NULL;
char *global_buyer_name = NULL;

int *wait_state = NULL; 
int *global_vendor_spinlock = NULL;

enum {ESPERA, RECHAZADO, ADJUDICADO};

int vendo(int precio, char *vendedor, char *comprador) {
  spinLock(&mutex);

  if (precio >= min_price) { // Oferta es peor que la anterior
    spinUnlock(&mutex);
    return 0;
  }

  // JUAN MATA A PEDRO XD
  if (global_vendor_name != NULL) { // Hay un vendedor
    spinUnlock(global_vendor_spinlock);
    *wait_state = RECHAZADO;
  }
  
  min_price = precio;
  global_vendor_name = vendedor;
  global_buyer_name = comprador;

  int VL = ESPERA;
  wait_state = &VL; //Estamos esperando
     
  int wait = CLOSED;
  global_vendor_spinlock = &wait;
  spinUnlock(&mutex);
  spinLock(&wait);
  
  spinLock(&mutex);

  if(VL == ADJUDICADO){
    spinUnlock(&mutex);
    return 1;
  }

  spinUnlock(&mutex);
  return 0;
}
  

int compro(char *comprador, char *vendedor) {
  spinLock(&mutex);

  //if(min_price==0) { // No hay vendedor
  if(global_vendor_name == NULL){ // No hay vendedor
    min_price = INT_MAX;
    spinUnlock(&mutex); 
    return 0;
  }

  // Si hay vendedor
  int price_paid = min_price;
  strcpy(vendedor, global_vendor_name); // quien le vendio
  strcpy(global_buyer_name, comprador); //enunc 4, quien compro
  
  global_vendor_name = NULL;
  min_price = INT_MAX;
  *wait_state = ADJUDICADO;

  spinUnlock(global_vendor_spinlock);
  spinUnlock(&mutex);
  return price_paid;
}
