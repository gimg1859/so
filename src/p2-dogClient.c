#include "mainMethods.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3566

const char ip[] = "127.0.0.1"; //mypc ip adress


int init() {

  struct sockaddr_in client;
  int clientfd, connection;
  char buffer[10];
  socklen_t size;

  clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if(clientfd == -1) {
    perror("Socket error");
    exit(-1);
  }
  //llenar campos
  client.sin_family = AF_INET;
  client.sin_port = htons(PORT);
  client.sin_addr.s_addr = inet_addr(ip);
  bzero(client.sin_zero, 8);

  size = sizeof(struct sockaddr_in);

  connection = connect(clientfd, (struct sockaddr*)&client, size);
  if(connection == -1) {
    perror("Connect error");
    exit(-1);
  }

  return clientfd;
}


void showMenu(int clientfd) {

  int sel = 0, r, confirmation, total_reg, num_reg, error = -1, val, i, control, a, exit_;
  char search[32];

  printf("\n\t\t----      Menu      ----\n\n\n");
  printf("\t1. Ingresar Registro\n");
  printf("\t2. Ver Registro\n");
  printf("\t3. Borrar Registro\n");
  printf("\t4. Buscar Registro\n");
  printf("\t5. Salir\n\n\n");
  printf("Por favor ingrese el numero de la opcion que desea ejecutar y presione Enter: ");
  fflush(stdout);
  scanf("%d", &sel);

  r = send(clientfd, &sel, sizeof(int), 0);
  if(r == -1) {
    perror("Send perror");
    exit(-1);
  }
//option create a new register
  if(sel == 1) {          
    struct dogType *dog;
    dog = malloc(sizeof(struct dogType));
    enterRegistry(dog);
    r = send(clientfd, dog, sizeof(struct dogType), 0);
    if(r == -1) {
      perror("Send perror opcion 1");
      exit(-1);
    }

    r = recv(clientfd, &confirmation, sizeof(int), 0);
    if(r != sizeof(int)){
      perror("Recv perror opcion 1");
      exit(-1);
    }

    if(confirmation == 1) {
      printf("\n\nregistro ha sido guardado con exito\n\n");
    }
    free(dog);
  }
//option view register use id
  else if(sel == 2) {          
    struct dogType *dog;
    dog = malloc(sizeof(struct dogType));
    bool want_clin_hist;
    int fileSize;

    r = recv(clientfd, &total_reg, sizeof(int), 0);
    if(r != sizeof(int)){
      perror("Recv perror opcion 2");
      exit(-1);
    }

    if(total_reg == 0) {
      printf("\nMensaje");
      printf("\n\nEl sistema no tiene ningun registro, lo invitamos a crear uno.\n");
      r = send(clientfd, &error, sizeof(int), 0);
      if(r == -1) {
        perror("Send perror opcion 2 registro");
        exit(-1);
      }

    } else {
      printf("\n\nCantidad registros del sistema:\t   %d  \n", total_reg);
      printf("\n\nIngrese el numero del registro que desea consultar: ");
      scanf("%d", &num_reg);
      if(num_reg >= 1 && num_reg <= total_reg)
        val = 1;
      else
        val = 0;

      if(val != 0) {
        r = send(clientfd, &num_reg, sizeof(int), 0); //sent search register
        if(r == -1) {
          perror("Send perror del reigstro a consultar");
          exit(-1);
        }
        printf("\n\n\nEl registro numero\t  %d \n", num_reg);

        r = recv(clientfd, dog, sizeof(struct dogType), 0);
        if(r != sizeof(struct dogType)) {
          perror("Recv perror clientfd");
          exit(-1);
        }
        showRegs(dog);

        want_clin_hist = wantClinicalHistory();
        r = send(clientfd, &want_clin_hist, sizeof(bool), 0);
        if(r == -1) {
          perror("Send perror historia clinica lista");
          exit(-1);
        }

        if(want_clin_hist) {
          //size of clinical history 
          r = recv(clientfd, &fileSize, sizeof(int), 0); 
          while(fileSize == -1) { 
          //if the clinical history is open, wait until it
            printf("La historia clinica ha sido abierta por otro cliente. Espere un rato\n");
            printf("---Esperando a que la historia clinica termine de editarse---\n");
            r = recv(clientfd, &fileSize, sizeof(int), 0);
            if(r != sizeof(int)) {
              perror("Recv perror historia clinica abierta");
              exit(-1);
            }
          }

          char clinicalHistory[fileSize];
          strcpy(clinicalHistory, "");

          if(fileSize != 0) {
            char tmp;
            //read history charter by charter
            for (i = 0; i < fileSize; i++) { 
              r = recv(clientfd, &tmp, sizeof(char), 0);
              if(r != sizeof(char)) {
                perror("Recv perror leyendo historia clinica");
                exit(-1);
              }
              strcat(clinicalHistory, &tmp);
            }
          }
          printf("\n\n\n----La historia clinica esta lista ser editada----\n");
          printf("----Abriendo la historia clinica----\n");
          sleep(1);
          openClinicalHistory(num_reg, dog, clinicalHistory, fileSize); 
          //create and open local history
          FILE *a = fopen("historiaClinica.txt", "r");
          fseek(a, 0, SEEK_END);
          fileSize = ftell(a);
          fseek(a, 0, SEEK_SET);
          r = send(clientfd, &fileSize, sizeof(int), 0); 
          //sent size of history 

          if(fileSize != 0) {
             //send history charter by charter
            char tmp;
            for (i = 0; i < fileSize; i++) {
              r = fread(&tmp, 1, 1, a);
              if(r != ELEMENTS) {
                printf("r = %d\n", r);
                perror("fread error getClinicalHistory 2");
                exit(-1);
              }
              r = send(clientfd, &tmp, sizeof(char), 0);
              if(r == -1) {
                perror("Send perror de la historia clinica");
                exit(-1);
              }
            }
          }

          system("rm historiaClinica.txt");
        }
      } else {
        printf("\n----     MENSAJE     ----");
        printf("\n\n- ingreso un numero incorrecto. Por favor ingrese un numero de 1 a %d\n", total_reg);
        r = send(clientfd, &error, sizeof(int), 0);
        if(r == -1){
          perror("Send perror numero registro");
          exit(-1);
        }
      }
    }
    free(dog);

  }




  else if(sel == 3) {          
//option delete register
    r = recv(clientfd, &total_reg, sizeof(int), 0);
    if(r != sizeof(int)) {
      perror("Recv error 4");
      exit(-1);
    }

    if(total_reg == 0) {
      printf("\n----      Menu      ----");
      printf("\n\n- Lo sentimos. Aun no se ha ingresado algun registro en el sistema\n");
      r = send(clientfd, &error, sizeof(int), 0);
      if(r == -1) {
        perror("Send perror registro");
        exit(-1);
      }
    } else {

      printf("\n\n----      registro  %d  ----\n", total_reg);
      printf("\n\nIngrese el numero del registro que desea borrar: ");
      scanf("%d", &num_reg);
      if(num_reg >= 1 && num_reg <= total_reg)
        val = 1;
      else
        val = 0;

      if(val != 0) {
        r = send(clientfd, &num_reg, sizeof(int), 0);
        if(r == -1){
          perror("Send perror registro");
          exit(-1);
        }
        r = recv(clientfd, &confirmation, sizeof(int), 0);
        if(r != sizeof(int)) {
          perror("Recv error 5");
          exit(-1);
        }
        if(confirmation == 1) {
          printf("\nInformacion borrado");
          printf("\n\n\nSu registro ha sido eliminado con exito\n\n\n");
        }

      } else {
        r = send(clientfd, &error, sizeof(int), 0);
        if(r == -1){
          perror("Send perror borrar registro");
          exit(-1);
        }
        printf("\n---- Mensaje  ----");
        printf("\n\nHa ingresado un número de registro incorrecto, por favor ingrese un numero de 1 a %d\n", total_reg);
      }
    }

  }
         //opction search register
  else if(sel == 4) { 
    struct dogType *dog;
    dog = malloc(sizeof(struct dogType));
    r = recv(clientfd, &total_reg, sizeof(int), 0);
    if(r != sizeof(int)) {
      perror("Recv perror buscar registro");
      exit(-1);
    }

    if(total_reg != 0) {
      r = send(clientfd, &total_reg, sizeof(int), 0);
      if(r == -1) {
        perror("Send perror buscar registro");
        exit(-1);
      }
      printf("\nIngrese el registro que quiere buscar:\n\n");
      scanf("%s", search);
      r = send(clientfd, search, sizeof(search), 0);
      if(r == -1) {
        perror("Send perror registro a buscar");
        exit(-1);
      }
      a = 1;
      printf("\n---- Busquedad ----\n\n\n");
      while(a == 1) {
        r = recv(clientfd, &i, sizeof(int), 0);
        if(r != sizeof(int)) {
          perror("Recv perror busquedad");
          exit(-1);
        }
        if(i != -1) {

          r = recv(clientfd, dog, sizeof(struct dogType), 0);
          if(r != sizeof(struct dogType)){
            perror("Recv perror busquedad");
            exit(-1);
          }

          printf("\nREGISTRO NUMERO : %d\n\n",i );
          printf("Nombre del animal:  %s\n", dog->name);
          printf("Tipo:  %s\n", dog->type);
          printf("Edad:  %i\n", dog->age);
          printf("Raza:  %s\n", dog->breed);
          printf("Estatura (cm):  %i\n", dog->height);
          printf("Peso (kg):  %0.1f\n", dog->weight);
          printf("Sexo:  %c\n\n", dog->sex);
          printf("----------\n");
          control++;
        }

        r = recv(clientfd, &a, sizeof(int), 0);
        if(r != sizeof(int)) {
          perror("Recv error 9");
          exit(-1);
        }
      }
      if(control == 0) {
        printf("\n---- Mensaje ----");
        printf("\n\n\nNo hay ninguna coincidencia para la busquedad realizada\n\n\n");
      }

    } else {
      r = send(clientfd, &error, sizeof(int), 0);
      if(r == -1) {
        perror("Send perror registro busquedad");
        exit(-1);
      }
      printf("\n---- Mensaje ----");
      printf("\n\n\nLo sentimos, aun no hay registros en el sistema \n\n\n");
    }

    free(dog);

  }
//exit client, close cosket
  else if(sel == 5) {           
    printf("\n\n\n--saliendo del sistema\n");
    sleep(1);
    printf("finalizando coneccion con el servidor\n");
    printf("cerrando socket\n");
    printf("----STATUS: Successfully completed----\n");
    sleep(1);
    printf("----Salida exitosa----\n\n\n");
    exit_ = -1;
    r = send(clientfd, &exit_, sizeof(int), 0);
    if(r == -1) {
      perror("Send perror exit");
      exit(-1);
    }
    close(clientfd);
    exit(-1);
  } else {
    printf("\n---- Mensaje ----");
    printf("\n\nIngrese una opcion correcta\n\n\n");
    r = send(clientfd, &error, sizeof(int), 0);
    if(r == -1){
      perror("Send perror opcion");
      exit(-1);
    }
  }
  printf("\n\n--Por favor pulse cualquier tecla para volver al menu principal--");
  fflush(stdout);
  char b;
  keypress(b);
  keypress(b);
}

int main() {
  int clientfd = init();

  while(1) {
    system("clear");
    showMenu(clientfd);
  }
  return 0;
}
