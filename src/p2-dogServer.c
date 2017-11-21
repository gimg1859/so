#include "mainMethods.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "pthread.h"
#include "pthread.h"
#define PORT 3566
#define MAX_CLIENTS 32


int total_clients = 0;
//log and dat global 
int writeLog = 0, writeInFile = 0;

void logWrite(int type,char registry[32],int socket);
void *clientManager(void *socket);
//server operetion
void server() {

  struct sockaddr_in server, client;
  int serverSocket, r, optval, s;
  socklen_t optlen = sizeof(optval);
  socklen_t size = sizeof(struct sockaddr_in);
  pthread_t hilo; //instance thearth

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(serverSocket == -1) {
    perror("Socket perror creacion");
    exit(-1);
  }
//complete information conection
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = INADDR_ANY;
  bzero(server.sin_zero, 8);

  r = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);
  if(r == -1) {
    perror("Setsockopt perror");
    exit(-1);
  }

  r = bind(serverSocket, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
  if(r == -1) {
    perror("Bind perror ");
    exit(-1);
  }

  while(1) {
    if(total_clients == MAX_CLIENTS) {
      continue;
    }

    r = listen(serverSocket, MAX_CLIENTS);
    if(r == -1) {
      perror("Listen perror ");
      exit(-1);
    }

    r = accept(serverSocket, (struct sockaddr*)&client, (socklen_t*)&size);
    if(r == -1) {
      perror("Accept perror ");
      exit(-1);
    }
  //comunication thread with client
    int s = pthread_create(&hilo, NULL, (void*)clientManager, (void*)&r);
    total_clients ++; 
        sleep(2);
    if(s != 0) {
      perror("Thread perror creacion");
      exit(-1);
    }
  }
}

//thread create to manage comunication with a client
void *clientManager(void *socket) {
  int serverfd = *(int*)socket;
  int r, option, flag = 1, verification = 1, validation = 1, total_reg, num_reg, val;
  char buffer[32];
  FILE* f;

  while(verification == 1) {
    //if dont open file dat, write in file
    if(writeInFile == 0) { 
      writeInFile = 1;
      verification = 0;
    }
  }

  f = fopen(file, "r+");
 //if exist file, else create new file
  if(f == NULL) {
    int zero = 0, r;
    f = fopen(file, "w");
    if(f == NULL) {  
      perror("fopen perror clientManager");
    }//indicator is located in the first row and read
    r = fseek(f, 0, SEEK_SET); 
    if(r != 0) {
      perror("fseek perror clientManager");
      exit(-1);
    }
    fwrite(&zero, sizeof(int), ELEMENTS, f);
    r = fclose(f);
    if (r != 0) {
      perror("perror fclose clientManager");
      exit(-1);
    }
  }
  while(flag == 1) {
    r = recv(serverfd, &option, sizeof(int), 0);
    if(r != sizeof(int)) {
      perror("Recv perror");
      exit(-1);
    }



    //option enter register
    if(option == 1) {  
      struct dogType *dog;
      dog = malloc(sizeof(struct dogType));
      r = recv(serverfd, dog, sizeof(struct dogType), 0);
      if(r != sizeof(struct dogType)) {
        perror("Recv perror ingresa registro");
        exit(-1);
      }
      logWrite(1, dog->name, serverfd);
      writeRegistry(dog);
      r = send(serverfd, &validation, sizeof(int), 0);
      if(r == -1) {
        perror("Send perror ingresa registro");
        exit(-1);
      }
      free(dog);

    }
    //option view register
    else if (option == 2) {  

      struct dogType *dog;
      bool want_clin_hist;
      int fileSize = -1, i;
      dog = malloc(sizeof(struct dogType));
      total_reg = calculateNumberRegistrys();

      r = send(serverfd, &total_reg, sizeof(int), 0);
      if(r == -1) {
        perror("Send perror ver registro");
        exit(-1);
      }
      //receive register
      r = recv(serverfd, &num_reg, sizeof(int), 0);
      if(r != sizeof(int)) {
        perror("Recv Error recibir registro");
        exit(-1);
      }
      if(num_reg != -1) {
        showRegistry(num_reg, dog);
        sprintf(buffer, "%d", num_reg);
        logWrite(2, buffer, serverfd);
        r = send(serverfd, dog, sizeof(struct dogType), 0);
        if(r == -1) {
          perror("Send perror recibir registro");
          exit(-1);
        }

        r = recv(serverfd, &want_clin_hist, sizeof(bool), 0);
        if(r != sizeof(bool)) {
          perror("Recv perror serverfd");
          exit(-1);
        }

        if(want_clin_hist) {
          char s[100];
          strcpy(s, getClinicalHistoryName(num_reg, dog));
          //while the cynical story is busy, wait
          while(!isClinicalHistoryAvailable(s)) { 
            r = send(serverfd, &fileSize, sizeof(int), 0);
            sleep(1);
          }

          FILE *a = fopen(s, "r");
          fseek(a, 0, SEEK_END);
          //get the file size, -1 from validation field
          fileSize = ftell(a) -1;  
             //position pointer, first from 1 because validation   
          fseek(a, 1, SEEK_SET);       
          //sent tama clinic history
          r = send(serverfd, &fileSize, sizeof(int), 0); 
     if(fileSize != 0) { 
      //sent history character by character 
            char tmp;
            for (i = 0; i < fileSize; i++) {
              r = fread(&tmp, 1, 1, a);
              if(r != ELEMENTS) {
                printf("r = %d\n", r);
                perror("fread perror server");
                exit(-1);
              }
              r = send(serverfd, &tmp, sizeof(char), 0);
              if(r == -1) {
                perror("Send perror");
                exit(-1);
              }
            }
          }
          r = fclose(a);
          if(r != 0) {
            perror("error fclose server");
            exit(-1);
          }
          //donts exist history, create 
          a = fopen(s, "w+"); 
          if(a == NULL) {
            perror("error fopen server historia clinica");
            exit(-1);
          }
          bool available = false;
          //comprovate open
          r = fwrite(&available, sizeof(bool), 1, a);
          if(r != 1) {
            perror("error fwrite server comprovacion");
            exit(-1);
          }
          r = fclose(a);
          if(r != 0) {
            perror("error fclose server comprovacion");
            exit(-1);
          }
           //recives size  histoy 
          r = recv(serverfd, &fileSize, sizeof(int), 0);
          char clinicalHistory[fileSize];

          char command[100];
          strcpy(command, "rm ");
          strcat(command, s);
          system(command);
          //read history character by character
          if(fileSize != 0) { 
            char tmp;
            for (i = 0; i < fileSize; i++) {
              r = recv(serverfd, &tmp, sizeof(char), 0);
              if(r != sizeof(char)) {
                perror("Recv error comprovacion");
                exit(-1);
              }
              clinicalHistory[i] = tmp;
            }
          }
          copyInFile(s, clinicalHistory, fileSize);
        }
      }
      free(dog);

    }
   //option delete register
    else if (option == 3) {       
      total_reg = calculateNumberRegistrys();
      r = send(serverfd, &total_reg, sizeof(int), 0);
      if(r == -1) {
        perror("Send error delete");
        exit(-1);
      }
      r = recv(serverfd, &num_reg, sizeof(int), 0);
      if(r != sizeof(int)) {
        perror("Recv Error delete ");
        exit(-1);
      }
      if(num_reg != -1) {
        deleteClinicalHistory(num_reg,-1);
        sleep(1);
        deleteRegistry(num_reg);
        sprintf(buffer, "%d", num_reg);
        r = send(serverfd, &validation, sizeof(int), 0);
        if(r == -1){
          perror("Send error deleteClinicalHistory");
          exit(-1);
        }
        logWrite(3, buffer, serverfd);
      }

    }
     //option search register
    else if (option == 4) { 

      total_reg = calculateNumberRegistrys();
      r = send(serverfd, &total_reg, sizeof(int), 0);
      if(r == -1){
        perror("Send error search");
        exit(-1);
      }
      r = recv(serverfd, &num_reg, sizeof(int), 0);
      if(r != sizeof(int)) {
        perror("Recv Error search");
        exit(-1);
      }
      if(num_reg != -1) {
        r = recv(serverfd, buffer, sizeof(buffer), 0);
        if(r != sizeof(buffer)) {
          perror("Recv Error search ");
          exit(-1);
        }
        searchRegistry(buffer, serverfd);
        logWrite(4, buffer, serverfd);
      }

    }
    //option exit close sockts
    else if(option == 5) {           
      total_clients--;
      sleep(3);
      r = recv(serverfd, &val, sizeof(int), 0);
      if(r != sizeof(int)) {
        perror("Recv Error close ");
        exit(-1);
      }
      if(val == -1) {
        break;
      }

    } else {
      r = recv(serverfd, &val, sizeof(int), 0);
      if(val == -1) {
        continue;
      }
    }
    writeInFile = 0;

  }
//close sockets client
  close(serverfd); 

}
//fution flie log
void logWrite(int type, char registry[32], int socket) { 
  int verification = 1, r;

  while(verification == 1) {
    //verefication if log open
    if(writeLog == 0){ 
      writeLog = 1;
      verification = 0;
    }
  }

  //open log in append
  FILE *fileLog = fopen("serverDogs.log", "a");
  if(fileLog == NULL) {
    perror("Error al abrir o crear el archivo log: ");
    exit(-1);
  }
  //Ip client and date time 
  time_t timer;
  struct tm* tm_info;
  char date[26];
  time(&timer);
  tm_info = localtime(&timer);
  strftime(date, 26, "%Y:%m:%d %H:%M:%S", tm_info);

  //take sistem information
  struct sockaddr_storage addr;
  char clientip[INET_ADDRSTRLEN];
  socklen_t len = sizeof addr;
  getpeername(socket, (struct sockaddr*) &addr, &len);
  struct sockaddr_in *s = (struct sockaddr_in *) &addr;
  inet_ntop(AF_INET, &s->sin_addr, clientip, sizeof clientip);
  char option[15];
  //save chain
  if(type == 1) {
    strcpy(option, "Insercion");
  } else if(type == 2) {
    strcpy(option, "Lectura");
  } else if(type == 3) {
    strcpy(option, "Borrado");
  } else {
    strcpy(option, "Busqueda");
  }

  //save information in file log
  r = fprintf(fileLog, "|%s|	Cliente [%s] |%s| |%s|\n", date, clientip, option, registry);
  if(r < 1) {
    perror("Fprintf error");
  }
  //close file log because proceses wait write 
  fclose(fileLog);

  writeLog = 0;
  verification = 1;
}

int main() {
  server();
  return 0;
}
