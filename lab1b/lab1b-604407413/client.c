#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <mcrypt.h>
#include <sys/fcntl.h>


//-----------GLOBAL VARS------------
struct termios original_attr;
int sockfd;
int logflag = 0;
int encryptflag = 0;
int portflag = 0;
int fp;

char key[16];// = "0123456789101112";
int keysize = 16;
char* IV;
MCRYPT td;

//-----------TERMIO FUNC-------------
void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &original_attr);
}

void set_input_mode (void)
{
  struct termios temp_attr;

  //check if the file descriptor is set to the terminal
  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  // Save the terminal attributes
  tcgetattr (STDIN_FILENO, &original_attr);
  atexit (reset_input_mode);
  // Set the terminal
  tcgetattr (STDIN_FILENO, &temp_attr);
  temp_attr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  temp_attr.c_cc[VMIN] = 1;
  temp_attr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &temp_attr);
}

//-----------------THREAD FUNC---------------

void error(char *msg)
 {
  perror(msg);
  exit(0);
}


void* readfromsocket(){

  int n;
  char buffer[256];
  int bufsize = 0;
  while(1){

    bzero(buffer,256);

    
    n = read(sockfd, buffer, 255);
    if (n < 0)
      error("ERROR writing to socket");

    if(encryptflag){
      for(int i = 0; i < n; i++)
	mdecrypt_generic (td, &buffer[i], 1);
    }
    
    write(1, buffer, n);
    if (n < 0)
      error("ERROR reading from socket");

    if(logflag == 1){
      for(int i =0; i<n; i++){
      write(fp, "RECEIVED 1 byte: ", 17);
      write(fp, &buffer[i] , 1);
      write(fp, "\n", 1);
  }
    }
  }
}


//-----------SIG HANDLER--------------

void signal_handler(int signum){

  if(signum == SIGINT){
    reset_input_mode();
    close(sockfd);
    exit(1);
  }

}

//--------------MAIN----------------

int main(int argc, char *argv[])
{
  int portno, n, threadcheck;
  char c;
  set_input_mode();
   
  signal(SIGINT, signal_handler);


  
  while(1){
    static struct option long_options[] =
      {
	{"port", required_argument, 0, 'p'},
	{"log", required_argument, 0, 'l'},
	{"encrypt", no_argument, &encryptflag, 1}
      };

    int option_index = 0;
    c = getopt_long (argc, argv, "p:l:", long_options, &option_index);

    if (c == -1)
      break;

    switch (c){

    case 'p':
      portno = atoi(optarg);
      portflag = 1;
      break;
    case 'l':
      
      fp = open(optarg, O_WRONLY);   
      logflag =1;
      break;
		     
    }
  }

  //---encrypt---

  if(encryptflag){

    int i = 0;
    
    int x = open("my.key", O_RDONLY);
    read (x, key, 16);

    td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    if (td==MCRYPT_FAILED) {
      return 1;
    }
    IV = malloc(mcrypt_enc_get_iv_size(td));
    for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
      IV[i]=rand();
    }
    i = mcrypt_generic_init( td, key, keysize, IV);
    if (i < 0) {
      mcrypt_perror(i);
      return 1;
    }



  }

  //---SOCKET---
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char input_buffer[256];


  if(portflag == 0)
    portno = 8989;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");
  server = gethostbyname("localhost");
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,	(char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  //read from socket to stdout
  pthread_t socketreader;
  threadcheck = pthread_create(&socketreader, 0, &readfromsocket, NULL);
  if (threadcheck)
    {
      fprintf(stderr, "Error: thread creation\n");
    }
  
  while(1){

    //read from stdin to the socket
    bzero(input_buffer, 256);
    n = read(STDIN_FILENO, input_buffer, 255);
    if (n < 0)
      error("ERROR reading from keyboard");

    
    
    for(int i = 0; i < n; i++){

      if(input_buffer[i] == 0X04){
	reset_input_mode();
	close(sockfd);
     	exit(0);

      }

      //control c
      if(input_buffer[i] == 0X03){
	reset_input_mode();
	close(sockfd);
	exit(1);
      }
      

      write( 1, &input_buffer[i], 1);

	if(encryptflag){
	    mcrypt_generic (td, &input_buffer[i], 1);
	}
	  write(sockfd, &input_buffer[i], 1);	  

	if (n < 0)
	  error("ERROR writing to socket");
    }
	if(logflag == 1){
	  for(int i =0; i<n; i++){
	    write(fp, "SENT 1 byte: ", 13);
	    write(fp, &input_buffer[i] , 1);
	    write(fp, "\n", 1);
	  }
	}	
  }
}
