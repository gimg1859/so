#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

const int SIZE = 2000; //tamano de la tabla hash

struct Node {
  int data;   //id unico de cada registro
  char *key;
  struct Node *PN; //nodo anterior
  struct Node *NN; //nodo siguiente
};


// Funcion que calcula el hash segun el nombre del registro
int hashCode(char* key) {

  int hash = 0, i;

  for (i = 0; i < strlen(key); i++) {
    hash += key[i] * (i * 2 + 1);
  }

  hash = hash % SIZE;

  while((hashArray[hash] != NULL) && hashArray[hash]->key != key) {//detectar colisiones y reasignar el hash
    hash = (hash + 7) % SIZE;
  }

  return hash;
}

//funcion para insertar un nodo

void insert(char* key, int data) {

  int hashIndex = hashCode(key);

  //printf("%d\n", hashIndex );

  struct Node *firstNode = (struct Node*) malloc(sizeof(struct Node));
  struct Node *newNode = (struct Node*) malloc(sizeof(struct Node));
  struct Node *walkerNode = (struct Node*) malloc(sizeof(struct Node));

  firstNode = hashArray[hashIndex];


  newNode->data = data; //se guarda el id del registro
  newNode->key = key;   //se guarda el nombre del registro

  if(firstNode==NULL){

    hashArray[hashIndex]=newNode;
    //printf("EL NODO ES EL PRIMER NODO QUE EXISTE EN ESTA LISTA\n");


  }else if(firstNode->key==key && firstNode->data==-1){ //ya se ha asignado esta ranura de la hash a un nombre especifico pero no hay ningun nodo porque los que habian han sido eliminados

    hashArray[hashIndex] = newNode;

    //printf("EL NODO ES EL PRIMER NODO DE LA LISTA LUEGO DE LA ELIMINACION DE LOS ANTERIORES\n");

  }else{


    walkerNode = firstNode;
    while(walkerNode->NN!=NULL){
      walkerNode=walkerNode->NN;
      //printf("SE HA PASADO POR UN NODO\n");
    }

    newNode->PN = walkerNode; // se asigna el nodo anterior del nuevo nodo como el primer nodo de la ranura

    walkerNode->NN = newNode; //se asigna el nuevo nodo como el siguiente nodo del primer nodo de la ranura

    //printf("EL NODO SE HA INSERTADO AL FINAL DE LA LISTA\n");


  }

}

//funcion para borrar un nodo de la Tabla hash
void delete(int data, char* key) {

  //char* key = getNameFromId(data);
  int hashIndex = hashCode(key);

  struct Node *firstNode = (struct Node*) malloc(sizeof(struct Node));
  struct Node *auxNode = (struct Node*) malloc(sizeof(struct Node));
  
  
  firstNode = hashArray[hashIndex];

  if(firstNode==NULL){

    //printf("NO HAY NADA PARA BORRAR\n");

  }else{

    //printf("ES POSIBLE QUE SE PUEDA BORRAR\n");


    if(firstNode->data == data && firstNode->NN == NULL){ //el nodo es el unico en la lista

      firstNode->data = -1;

      //printf("EL NODO SE BORRO, ERA EL UNICO EN LA LISTA\n");

    }else if(firstNode->data == data && firstNode->NN!=NULL){ //el nodo es el primero pero hay mas nodos en la lista

      hashArray[hashIndex] = firstNode->NN;

      //printf("EL NODO SE BORRO, ERA EL PRIMERO EN LA LISTA\n");


    }else{//el nodo esta en medio o al final de la lista

      struct Node *walkerNode = (struct Node*) malloc(sizeof(struct Node));
      walkerNode = firstNode;
      bool flag = true;

      while(flag){

        if(walkerNode->NN != NULL){

          if(walkerNode->data == data){//se borra del medio

            auxNode = walkerNode; 

            walkerNode = walkerNode->PN;

            walkerNode->NN = auxNode->NN;

            walkerNode = auxNode->NN;

            walkerNode->PN = auxNode->PN;

            //printf("EL NODO ESTABA EN UNA POS INTERMEDIA DE LA LISTA\n");

            flag = false;
          }

        }else{

          if(walkerNode->data == data){//se borra del final de la lista (ultimo)

            walkerNode = walkerNode->PN;
            walkerNode->NN = NULL;

            //printf("EL NODO ERA EL ULTIMO EN LA LISTA, SE BORRO\n");

            flag = false;
          }

          break;
        }

        walkerNode = walkerNode->NN;
       
      }

      if(flag){

        //printf("NO SE PUDO BORRAR NADA\n");

     }

    }
  }
}

//funcion para mostrar la hash

void showHashTable(){

  int i;
 
  struct Node *walkerNode = malloc(sizeof(struct Node));

  for (i=0; i<SIZE; i++){

    walkerNode = hashArray[i];

    if(walkerNode==NULL){

      printf("RANURA VACIA\n");

    }else{

      printf("( %s , %d )  ",walkerNode->key, walkerNode->data );

      while(walkerNode->NN != NULL){

        walkerNode = walkerNode->NN;

        printf("( %s , %d )  ",walkerNode->key, walkerNode->data );


      }

      printf("\n");

    }

  }

}
