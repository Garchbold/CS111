#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <mcrypt.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

//-----------GLOBAL VARS------------
struct termios original_attr;
int terminal_to_shell[2];
int shell_to_terminal[2];
char newline[2];
pid_t pid;
int portflag = 0;
int encryptflag = 0;

char key[16];
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

//-------------THREAD FUNC-----------

void * shell_write(){

  //read from the socket and write to the shell
  
  char read_buffer;
  int read_bufsize;

  while(1){

    read_bufsize = read(0, &read_buffer, 1);

    if(encryptflag){
	mdecrypt_generic (td, &read_buffer, 1);
    }

    if(read_bufsize <= 0){
      perror("Error reading from terminal to shell");
      exit(1);
    }

    //control d
    if(read_buffer == 0X04){
      reset_input_mode();
      exit(1);
    }

    //control c
    if(read_buffer == 0X03){
      reset_input_mode();
      exit(1);
    }

    write(terminal_to_shell[1], &read_buffer, 1);
    if(read_bufsize <= 0)
      perror("Error writing from terminal to shell");
    
  }
}


void signal_handler(int signum){

  if(signum == SIGPIPE){
    reset_input_mode();
    exit(2);
  }

}



void error(char *msg)
{
  perror(msg);
  exit(1);
}

//----------------MAIN------------------
int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, clilen;
  char servbuffer[256];
  int servbuffersize;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  char c;


  while(1){
   static struct option long_options[] =
     {
       {"port", required_argument, 0, 'p'},
       {"encrypt", no_argument, &encryptflag, 1}
     };

  int option_index = 0;
  c = getopt_long (argc, argv, "p:", long_options, &option_index);

  if (c == -1)

    break;

  switch (c){

  case 'p':
    portno = atoi(optarg);
    portflag = 1;
    break;

  
  }
  }
  

  if(encryptflag){
    
    int i = 0;
    //key = calloc(1, keysize);
    td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    if (td==MCRYPT_FAILED) {
      return 1;
    }
    
    int y = open("my.key", O_RDONLY);
    read(y, key, 16);

    
    IV = malloc(mcrypt_enc_get_iv_size(td));
    for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
      IV[i]=rand();
    }
    i=mcrypt_generic_init( td, key, keysize, IV);
    if (i<0) {
      mcrypt_perror(i);
      return 1;
    }



  }
	
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  if(portflag == 0)
    portno = 8989;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	   sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0)
    error("ERROR on accept");
 

  
  pipe(terminal_to_shell);
  pipe(shell_to_terminal);
  pid = fork();
  if(pid == -1){
    fprintf(stderr, "Failed to fork processes.");
    exit(-1);
    }
  else if (pid == 0){

    close(terminal_to_shell[1]);
    close(shell_to_terminal[0]);

    dup2(terminal_to_shell[0], 0);
    dup2(shell_to_terminal[1], 1);
    dup2(shell_to_terminal[1], 2);

    execl("/bin/bash","/bin/bash",  NULL);
    }
  else{

    signal(SIGPIPE, signal_handler);
    
    close(terminal_to_shell[0]);
    close(shell_to_terminal[1]);

    dup2(newsockfd,0);
    dup2(newsockfd,1);
    dup2(newsockfd,2);
    
    pthread_t shell_write_thread;
    int thread_check = 0;

    thread_check = pthread_create(&shell_write_thread, NULL, &shell_write, NULL);
    if(thread_check < 0)
      fprintf(stderr, "Failed to create thread");
    
    while(1){

      //reading from the shell to the socket
      bzero(servbuffer, 256);
      n = read(shell_to_terminal[0], servbuffer,255);
      if (n <= 0) error("ERROR reading from socket");

      for(int i =0; i < n; i++){
      if(servbuffer[i] == 0X04){
	reset_input_mode();
	close(terminal_to_shell[1]);
	close(shell_to_terminal[0]);
	exit(2);
      }

      //control c
      if(servbuffer[i] == 0X03){
	reset_input_mode();
	close(terminal_to_shell[1]);
	close(shell_to_terminal[0]);
	exit(2);
      }
      }
      
      if(encryptflag){
	for(int i = 0; i < n; i++)
	  mcrypt_generic (td, &servbuffer[i], 1);
      }

      write( 1, servbuffer, n);
      if (n <= 0) error("ERROR writing to socket");
    }

  }
}


