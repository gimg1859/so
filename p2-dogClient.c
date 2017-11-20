#include "../mainMethods.c"
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

  printf("\n\t\t:::::::::      MENU DE GESTION DE LA VETERINARIA 2.0       ::::::::\n\n\n");

  printf("\t\t    #####    \t\t  #                 #        \n");
  printf("\t\t  #########  \t\t  ##               ##        \n");
  printf("\t\t ########### \t\t  ###             ###        \n");
  printf("\t\t#############\t\t  ###################        \n");
  printf("\t\t#############\t\t  ###  #######  #####        \n");
  printf("\t\t#############\t\t   #################         \n");
  printf("\t\t ########### \t\t   #######  ########         \n");
  printf("\t\t  #########  \t\t   ######    #######         \n");
  printf("\t\t   #######   \t\t   #################         \n");
  printf("\t\t      ..     \t\t   #####      ######         \n");
  printf("\t\t     ..      \t\t   #################         \n");
  printf("\t\t    ..       \t\t    ###############          \n");
  printf("\t\t    ..       \t\t    #####    ######          \n");
  printf("\t\t     ..      \t\t    ######  #######          \n");
  printf("\t\t      ..     \t\t   ######    #######         \n");
  printf("\t\t       ..    \t\t  #######    ########        \n");
  printf("\t\t       ..    \t\t  #######    ########        \n");
  printf("\t\t      ..     \t\t  #######    ########        \n");
  printf("\t\t     ..      \t\t  #######    ########        \n");
  printf("\t\t     ..      \t\t  #######    ########        \n");
  printf("\t\t     ..      \t\t  ###################        \n");
  printf("\t\t     ..      \t\t   #################         \n");
  printf("\t\t             \t\t    ###############          \n");
  printf("\t\t             \t\t   #################         \n");
  printf("\t\t             \t\t  ##  #  ####   #  ##        \n");
  printf("\t\t             \t\t  ## ### ##### ### ##        \n");
  printf("1. Ingresar Registro\n");
  printf("2. Ver Registro\n");
  printf("3. Borrar Registro\n");
  printf("4. Buscar Registro\n");
  printf("5. Salir\n\n\n");
  printf("Por favor ingrese el numero de la opcion que desea ejecutar y presione Enter: ");
  fflush(stdout);
  scanf("%d", &sel);

  r = send(clientfd, &sel, sizeof(int), 0);
  if(r == -1) {
    perror("Send error 1");
    exit(-1);
  }




  if(sel == 1) {          //opcion de ingresar registro
    struct dogType *dog;
    dog = malloc(sizeof(struct dogType));

    enterRegistry(dog);

    r = send(clientfd, dog, sizeof(struct dogType), 0);
    if(r == -1) {
      perror("Send error 2");
      exit(-1);
    }

    r = recv(clientfd, &confirmation, sizeof(int), 0);
    if(r != sizeof(int)){
      perror("Recv error 1");
      exit(-1);
    }

    if(confirmation == 1) {
      printf("\n\n- Su registro ha sido guardado con exito!\n\n\n");
    }

    free(dog);
  }




  else if(sel == 2) {          //opcion de ver registro
    struct dogType *dog;
    dog = malloc(sizeof(struct dogType));
    bool want_clin_hist;
    int fileSize;

    r = recv(clientfd, &total_reg, sizeof(int), 0);
    if(r != sizeof(int)){
      perror("Recv error 2");
      exit(-1);
    }

    if(total_reg == 0) {
      printf("\n:::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
      printf("\n\n- Lo sentimos! Aun no se ha ingresado algun registro en el sistema.\n");
      r = send(clientfd, &error, sizeof(int), 0);
      if(r == -1) {
        perror("Send error 4");
        exit(-1);
      }

    } else {
      printf("\n\n::::::::::::::::::::::::::::::::::  Cantidad de registros en el sistema:  %d  :::::::::::::::::::::::::::::::::::::::\n", total_reg);
      printf("\n\n- Ingrese el numero del registro que desea consultar: ");
      scanf("%d", &num_reg);
      if(num_reg >= 1 && num_reg <= total_reg)
        val = 1;
      else
        val = 0;

      if(val != 0) {
        r = send(clientfd, &num_reg, sizeof(int), 0); //envía el registro a consultar
        if(r == -1) {
          perror("Send error 5");
          exit(-1);
        }
        printf("\n\n\n::::::::::::::::::::::::::::::::::::::::::::  Mostrando registro N°  %d  :::::::::::::::::::::::::::::::::::::::::::\n", num_reg);

        r = recv(clientfd, dog, sizeof(struct dogType), 0);
        if(r != sizeof(struct dogType)) {
          perror("Recv error 3");
          exit(-1);
        }
        showRegs(dog);

        want_clin_hist = wantClinicalHistory();
        r = send(clientfd, &want_clin_hist, sizeof(bool), 0);
        if(r == -1) {
          perror("Send error 6");
          exit(-1);
        }

        if(want_clin_hist) {
          r = recv(clientfd, &fileSize, sizeof(int), 0); //recibe el tamaño de la historia clinica
          while(fileSize == -1) { //Mientras que la hisoria cínica esté ocupada, espere
            printf("La historia clínica ha sido abierta por otro cliente. Espere unos minutos.\n");
            printf(">> Esperando a que la historia clínica deje de ser editada .............................\n");
            r = recv(clientfd, &fileSize, sizeof(int), 0);
            if(r != sizeof(int)) {
              perror("Recv error 45");
              exit(-1);
            }
          }

          char clinicalHistory[fileSize];
          strcpy(clinicalHistory, "");

          if(fileSize != 0) {
            char tmp;
            for (i = 0; i < fileSize; i++) { //lee la historia clinica caracter por caracter
              r = recv(clientfd, &tmp, sizeof(char), 0);
              if(r != sizeof(char)) {
                perror("Recv error 4");
                exit(-1);
              }
              strcat(clinicalHistory, &tmp);
            }
          }
          printf("\n\n\n>> La historia clínica está lista ser editada .......................................\n");
          printf(">> Abriendo la historia clinica .........................................................\n");
          sleep(1);
          openClinicalHistory(num_reg, dog, clinicalHistory, fileSize); //crea y abre la historia clinica localmente

          FILE *a = fopen("historiaClinica.txt", "r");
          fseek(a, 0, SEEK_END);
          fileSize = ftell(a);
          fseek(a, 0, SEEK_SET);
          r = send(clientfd, &fileSize, sizeof(int), 0); //envia el tamaño de la historia clinica

          if(fileSize != 0) { //envia la historia clinica caracter por caracter
            char tmp;
            for (i = 0; i < fileSize; i++) {
              r = fread(&tmp, 1, 1, a);
              if(r != ELEMENTS) {
                printf("r = %d\n", r);
                perror("fread error getClinicalHistory 2");
                exit(-1);
              }
              // printf("%c\n", tmp);
              r = send(clientfd, &tmp, sizeof(char), 0);
              if(r == -1) {
                perror("Send error 4");
                exit(-1);
              }
            }
          }

          system("rm historiaClinica.txt");
        }
      } else {
        printf("\n:::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        printf("\n\n- ¡Ha ingresado un número de registro incorrecto! Por favor ingrese un número entre 1 y %d\n", total_reg);
        r = send(clientfd, &error, sizeof(int), 0);
        if(r == -1){
          perror("Send error 7");
          exit(-1);
        }
      }
    }
    free(dog);

  }




  else if(sel == 3) {          //opcion de borrar registro

    r = recv(clientfd, &total_reg, sizeof(int), 0);
    if(r != sizeof(int)) {
      perror("Recv error 4");
      exit(-1);
    }

    if(total_reg == 0) {
      printf("\n:::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
      printf("\n\n- Lo sentimos! Aun no se ha ingresado algun registro en el sistema.\n");
      r = send(clientfd, &error, sizeof(int), 0);
      if(r == -1) {
        perror("Send error 7");
        exit(-1);
      }
    } else {

      printf("\n\n::::::::::::::::::::::::::::::::::Cantidad de registros en el sistema:  %d :::::::::::::::::::::::::::::::::::::::\n", total_reg);
      printf("\n\n- Ingrese el numero del registro que desea borrar: ");
      scanf("%d", &num_reg);
      if(num_reg >= 1 && num_reg <= total_reg)
        val = 1;
      else
        val = 0;

      if(val != 0) {
        r = send(clientfd, &num_reg, sizeof(int), 0);
        if(r == -1){
          perror("Send error 8");
          exit(-1);
        }
        r = recv(clientfd, &confirmation, sizeof(int), 0);
        if(r != sizeof(int)) {
          perror("Recv error 5");
          exit(-1);
        }
        if(confirmation == 1) {
          printf("\n:::::::::::::::::::::::::::::::::::::::::::::CONFIRMACION DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::");
          printf("\n\n\n- Su registro ha sido eliminado con exito!\n\n\n");
        }

      } else {
        r = send(clientfd, &error, sizeof(int), 0);
        if(r == -1){
          perror("Send error 9");
          exit(-1);
        }
        printf("\n:::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        printf("\n\n- ¡Ha ingresado un número de registro incorrecto! Por favor ingrese un número entre 1 y %d\n", total_reg);
      }
    }

  }




  else if(sel == 4) {          //opcion de buscar registro
    struct dogType *dog;
    dog = malloc(sizeof(struct dogType));
    r = recv(clientfd, &total_reg, sizeof(int), 0);
    if(r != sizeof(int)) {
      perror("Recv error 6");
      exit(-1);
    }

    if(total_reg != 0) {
      r = send(clientfd, &total_reg, sizeof(int), 0);
      if(r == -1) {
        perror("Send error 10");
        exit(-1);
      }
      printf("\n- Ingrese el registro que quiere buscar: \n\n");
      scanf("%s", search);
      r = send(clientfd, search, sizeof(search), 0);
      if(r == -1) {
        perror("Send error 11");
        exit(-1);
      }
      a = 1;
      printf("\n::::::::::::::::::::::::::::::::::::::::::RESULTADOS DE LA BUSQUEDA:::::::::::::::::::::::::::::::::::::::::::::::::::\n\n\n");
      while(a == 1) {
        r = recv(clientfd, &i, sizeof(int), 0);
        if(r != sizeof(int)) {
          perror("Recv error 7");
          exit(-1);
        }
        if(i != -1) {

          r = recv(clientfd, dog, sizeof(struct dogType), 0);
          if(r != sizeof(struct dogType)){
            perror("Recv error 8");
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
          printf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
          control++;
        }

        r = recv(clientfd, &a, sizeof(int), 0);
        if(r != sizeof(int)) {
          perror("Recv error 9");
          exit(-1);
        }
      }
      if(control == 0) {
        printf("\n:::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        printf("\n\n\nNo hay ninguna coincidencia para la busqueda realizada!\n\n\n");
      }

    } else {
      r = send(clientfd, &error, sizeof(int), 0);
      if(r == -1) {
        perror("Send error 12");
        exit(-1);
      }
      printf("\n:::::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
      printf("\n\n\n- Lo sentimos, aun no hay registros en el sistema\n\n\n");
    }

    free(dog);

  }




  else if(sel == 5) {            //opcion de salir del sistema
    printf("\n\n\n>> Saliendo del sistema.........................................................\n");
    sleep(1);
    printf(">> Finalizando conexion con el servidor.........................................\n");
    printf(">> Cerrando socket..............................................................\n");
    printf(">> STATUS: Successfully completed...............................................\n");
    sleep(1);
    printf(">> OK...........................................................................\n\n\n");
    exit_ = -1;
    r = send(clientfd, &exit_, sizeof(int), 0);
    if(r == -1) {
      perror("Send error 13");
      exit(-1);
    }
    close(clientfd);
    exit(-1);
  } else {
    printf("\n:::::::::::::::::::::::::::::::::::::::::::::MENSAJE DEL SISTEMA::::::::::::::::::::::::::::::::::::::::::::::::::::::");
    printf("\n\nIngrese una opcion correcta!!....\n\n\n");
    r = send(clientfd, &error, sizeof(int), 0);
    if(r == -1){
      perror("Send error 14");
      exit(-1);
    }
  }
  printf("\n\nPor favor pulse cualquier tecla para volver al menu principal...");
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
