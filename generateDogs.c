#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainMethods.c"

//funcion para generar un nombre aleatorio
char* generateName(FILE *fileOpen) {
  char* name = (char*)malloc(sizeof(char)*32);
  int maxRan = fileSize(fileOpen);

  int random = rand() % maxRan;
  fseek(fileOpen, random, SEEK_SET);
  if(fgets(name, 32, fileOpen)) {
    fgets(name, 32, fileOpen);
  }

  return name;|

char* generateType() {
  int random = rand() % 6;
  char *types[7] = {"perro", "gato", "hamster", "conejo" , "pez" , "ave" ,"serpiente"};
  return types[random];
}

char* generateBreed(char animal[]) {
  int random = rand() % 4;
  char *breedDog[5] = {"alabardero", "labrador", "lobo", "criollo", "ingles"};
  char *breedCat[5] = {"aragon", "otomano", "persa", "criollo", "nipon"};
  char *breedHamster[5] = {"criollo", "rata", "arkanto", "puchim", "laboratorio"};
  char *breedRabbit[5] = {"cilico", "criollo", "blanco", "swet", "hikiko"};
  char *breedFish[5] = {"criollo", "doradito", "nemo", "lunario", "globo"};
  char *breedBird[5] = {"crow", "tukan", "canario", "loro", "paloma"};

  if (animal == "perro") {
    return breedDog[random];
  }
  else if (animal == "gato") {
    return breedCat[random];
  }
  else if (animal == "hamster") {
    return breedHamster[random];
  }
  else if (animal == "conejo") {
    return breedRabbit[random];
  }
  else if (animal == "pez") {
    return breedFish[random];
  }
  else if (animal == "ave") {
    return breedBird[random];
  }
  else{
    return "serpiente";
  }

}

char generateSex() {
  int random = rand() % 1;
  if(random)
    return 'H';
  else
    return 'M';
}

int main() {
  struct dogType pou;
  FILE* fileOpen = fopen(file,"r+");
  if(fileOpen == NULL) { //if this file dont exist
    int num_reg = 10000000, r;
    fileOpen = fopen(file,"w");

    if(fileOpen == NULL) 
      perror("error generando archivo random");
    }
  r = fseek(fileOpen,0,SEEK_SET); //row in this file cursor
    if(r!=0) {
      perror("error del fseek ");
      exit(-1);
    }

    fwrite(&num_reg,sizeof(int),ELEMENTS,fileOpen); //count elements in this file
  }
  FILE *n;
  n = fopen("petNames.txt", "r"); //open petnames of animals
  if(n == NULL) {
    perror("error fopen petNames");
    exit(-1);
  }

  int i;
  for(i = 0; i<1e07; i++) { //generate 10milliones
    char * name = generateName(n);
    name[strlen(name)-1] = 0;
    char * type = generateType();
    int age = rand() % 14;
    char * breed = generateBreed(type);
    int height = rand() % 120;
    float weight = (rand() % 60) + (rand() % 9)*0.1;
    char sex = generateSex();
    pou.id = i;
    strcpy(pou.name, name);
    strcpy(pou.type, type);
    pou.age = age;
    strcpy(pou.breed, breed);
    pou.height = height;
    pou.weight = weight;
    pou.sex = sex;

    int r = fwrite(&pou, sizeof(struct dogType), ELEMENTS, fileOpen);
    if(r != ELEMENTS){
      perror("error fwrite permisos plis");
      exit(-1);
    }
  }
  int r = fclose(fileOpen);
  if (r != 0) {
    perror("error fclose");
    exit(-1);
  }
  //close names file
  r = fclose(n);
  if (r != 0) {
    perror("error fclose");
    exit(-1);
  }

  return 0;
}
