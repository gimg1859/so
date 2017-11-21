#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainMethods.c"

//funcion para generar un nombre aleatorio
char* generateName(FILE *f) {
  char* name = (char*)malloc(sizeof(char)*32);
  int maxRan = fileSize(f);

  int ran = rand() % maxRan;
  fseek(f, ran, SEEK_SET);
  if(fgets(name, 32, f)) {
    fgets(name, 32, f);
  }

  return name;
}

//funcion para generar un tipo de animal aleatoriamente
char* generateType() {
  int ran = rand() % 6;
  char *types[7] = {"perro", "gato", "mapache", "conejo", "pez","ave","serpiente"};
  return types[ran];
}

//funcion para generar una raza aleatoria
char* generateBreed(char animal[]) {
  int ran = rand() % 4;
  char *dogB[5] = {"husky", "labrador", "beagle", "pug", "bulldog"};
  char *catB[5] = {"siames", "persa", "angora", "abisinio", "ragdoll"};
  char *hamsterB[5] = {"ruso", "enano", "dorado", "de angora", "albino"};
  char *rabbitB[5] = {"mini lop", "californiano", "holandes", "jersey wooly", "arlequin"};
  char *fishB[5] = {"dorado", "pez gato", "pez payaso", "bailarina", "guppy"};

  if (animal == "perro") {
    return dogB[ran];
  }
  else if (animal == "gato") {
    return catB[ran];
  }
  else if (animal == "hamster") {
    return hamsterB[ran];
  }
  else if (animal == "conejo") {
    return rabbitB[ran];
  }
  else if (animal == "pez") {
    return fishB[ran];
  }
  else{
    return "bravo";
  }

}

//funcion para generar el sexo del animal aleatoriamente
char generateSex() {
  int ran = rand() % 1;
  if(ran)
    return 'H';
  else
    return 'M';
}

int main() {
  struct dogType p;
  //open records file
  FILE* f = fopen(file,"r+");

  //se verifica la existencia del archivo, si no existe, se crea
  if(f == NULL) {
    int num_reg = 10000000, r;
    f = fopen(file,"w");

    if(f == NULL) {  // Validamos errores
      perror("fopen error main generateDogs");
    }

    r = fseek(f,0,SEEK_SET); //se situa el indicador de pos. al inicio del archivo para escribir el num de registros

    if(r!=0) {
      perror("fseek error main generateDogs");
      exit(-1);
    }

    fwrite(&num_reg,sizeof(int),ELEMENTS,f); //se pone el numero de registros a escribir
  }

  //se abre el archivo que contiene los nombres de los animales para generar las estucturas
  FILE *n;
  n = fopen("petNames.txt", "r");
  if(n == NULL) {
    perror("error fopen main generateDogs 2");
    exit(-1);
  }

  int i;
  for(i = 0; i<1e07; i++) { //se generan 10'000.000 de estructuras y se escriben en el archivo dataDogs.dat
    char * name = generateName(n);
    name[strlen(name)-1] = 0;
    char * type = generateType();
    int age = rand() % 14;
    char * breed = generateBreed(type);
    int height = rand() % 120;
    float weight = (rand() % 60) + (rand() % 9)*0.1;
    char sex = generateSex();

    p.id = i;
    strcpy(p.name, name);
    strcpy(p.type, type);
    p.age = age;
    strcpy(p.breed, breed);
    p.height = height;
    p.weight = weight;
    p.sex = sex;

    int r = fwrite(&p, sizeof(struct dogType), ELEMENTS, f);
    if(r != ELEMENTS){
      perror("error fwrite main generateDogs");
      exit(-1);
    }
  }

  //close records file
  int r = fclose(f);
  if (r != 0) {
    perror("error fclose main generateDogs f");
    exit(-1);
  }
  //close names file
  r = fclose(n);
  if (r != 0) {
    perror("error fclose main generateDogs n");
    exit(-1);
  }

  return 0;
}
