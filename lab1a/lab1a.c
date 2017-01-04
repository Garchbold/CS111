#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>

struct termios original_attr;
int terminal_to_shell[2];
int shell_to_terminal[2];
char newline[2];
pid_t pid;

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

void * terminal_write(void * arg){
  //read input from the keyboard, echo it to stdout, and forward it to the shell                          
  char buffer;
  int bufsize;

  while(1){

    bufsize = read(STDIN_FILENO, &buffer, 1);

if(bufsize == -1)
      fprintf(stderr, "Error reading input in twrite");



      if(buffer == 0X04){
        reset_input_mode();
        fprintf(stderr, "Got controlD\n");
        close(terminal_to_shell[1]);
        close(shell_to_terminal[0]);
        exit(1);
      }

      if (buffer == 0x0d || buffer == 0x0a){
        write(1, &newline, 2);
        write(terminal_to_shell[1], "\n", 1);
        continue;
      }

      write(1, &buffer, 1);
      write(terminal_to_shell[1], &buffer, 1);
  }
}

void * terminal_read(){
  //read input from the shell pipe and write it to stdout                                                 
  char buffer;
  int bufsize;
  while(1){

      bufsize = read(shell_to_terminal[0], &buffer, 1);

      if(bufsize == -1){
        fprintf(stderr, "Error reading input in tread");
        break;
      }

      if(buffer == 0X04){
        reset_input_mode();
        fprintf(stderr, "Got controlD\n");
        close(terminal_to_shell[1]);
        close(shell_to_terminal[0]);
        exit(1);

    }

    write(1, &buffer, 1);


  }
  return;
}

int main(int argc, char** argv){



  char buffer;
  int bufsize;
  static int shell_flag = 0;
  int c = 0;

  newline[0] = 0x0d;
  newline[1] = 0x0a;

  set_input_mode();

  while(1){
  static struct option long_options[] =
    {
       {"shell", no_argument, &shell_flag, 1},
       {0, 0, 0, 0,}
    };

  int option_index = 0;
  c = getopt_long(argc, argv,"", long_options, &option_index);

  if(c == -1)
    break;
  }

 pipe(terminal_to_shell);
  pipe(shell_to_terminal);

  if (shell_flag == 1){

    pid = fork();

    if(pid == -1){
      fprintf(stderr, "Failed to fork processes.");
      exit(1);
    }
    else if (pid == 0){
      printf("child\n");

      close(terminal_to_shell[0]);
      close(shell_to_terminal[1]);

      dup2(terminal_to_shell[0], 0);
      dup2(shell_to_terminal[1], 1);
      dup2(shell_to_terminal[1], 2);

      printf("exec call\n");
      execvp("/bin/bash", NULL);

}
    else{
      printf("parent\n");

      close(terminal_to_shell[0]);
      close(shell_to_terminal[1]);

      pthread_t terminal_write_thread;
      pthread_t terminal_read_thread;

      pthread_create(&terminal_write_thread, 0, terminal_write, NULL);
      printf("write\n");

      pthread_create(&terminal_read_thread, 0, terminal_read, NULL);
      printf("read\n");

    }

  }
while (1)
    {

      bufsize = read(STDIN_FILENO, &buffer, 1);

      if(bufsize == -1){
        fprintf(stderr, "Error reading input in reg\n");
        break;
      }

        if(buffer == 0X04){
          reset_input_mode();
          exit(1);
        }

        if (buffer == 0x0a || buffer == 0x0d){
          write(1, &newline, 2);
          continue;
        }

        write(1, &buffer, 1);

    }
  reset_input_mode();
  return EXIT_SUCCESS;

}


