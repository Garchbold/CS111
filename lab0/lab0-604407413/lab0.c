#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static int segfault_flag;
static int catch_flag;

void sig_handler(int signum)
{
    fprintf(stderr, "caught SIGSEGV\n");
    exit(3);
}


int main ( int argc, char** argv){
  int c = 0;
  int fd = 0;
  char character;
  int input;
  char* fault = NULL;
  
  while (1)
    {
      static struct option long_options[] =
	{
	  {"input", required_argument, 0, 'i'},
	  {"output", required_argument, 0, 'j'},
	  {"segfault", no_argument, &segfault_flag, 1},
	  {"catch", no_argument, &catch_flag, 1}
	};

      int option_index = 0;      
      c = getopt_long (argc, argv, "i:j:", long_options, &option_index); 

      if (c == -1)
	break;

      switch (c){

        case 'i':
	  
	  close(0);

	  fd = open(optarg, O_RDONLY);
	  dup(fd);
	    
          if(fd == -1){
	    perror("Error ");
	    exit(1);
	  }
	  break;

        case 'j':
	  close(1);
	  fd = creat(optarg, S_IRWXU);
	  dup(fd);
	  
	  if(fd == -1){
	    perror("Error ");
	    exit(2);
	  }
	  break;
	  
	  default:
	    if(segfault_flag == 0 && catch_flag == 0){
	      exit(1);
	    }
      }

    }

  if (catch_flag == 1 && segfault_flag == 1){
    signal(SIGSEGV, sig_handler);
  }

  if (segfault_flag == 1){
    *fault = 'a';
  }

  

  while(1){

    input = read(0, &character, 1);
    if (input > 0 && character != EOF) {
      write(1, &character, 1);
    }
    else{
      break;

    }
  }
}
