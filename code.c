#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

/*GLOBAL VARIABLES*/
int pipe_count=0, fd;
static char* args[512];
//char *history_file;
char input_buffer[1024];
char *cmd_exec[100];
int flag, len;
char cwd[1024];
int flag_pipe=1;
pid_t pid;
int no_of_lines;
//int environmment_flag;
int flag_pipe, flag_without_pipe,  output_redirection, input_redirection;
int bang_flag;
int pid, status;
//char history_data[1000][1000];
char current_directory[1000];
char ret_file[3000];
//char his_var[2000];
char *input_redirection_file;
char *output_redirection_file;
//extern char** environ;


/*****************Harsh Ka code **********************/
#define SHELL_RL_BUFSIZE 1024

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
/**************************************/


/***************************Header Files Used*****************************/
void clear_variables();
//void fileprocess ();
//void filewrite();
//void bang_execute();
//void environmment();
//void set_environment_variables();
//void change_directory();
//void parent_directory();
//void echo_calling(char *echo_val);
//void history_execute_with_constants();
static char* skipwhite(char* s);
void tokenise_commands(char *com_exec);
void tokenise_redirect_input_output(char *cmd_exec);
void tokenise_redirect_input(char *cmd_exec);
void tokenise_redirect_output(char *cmd_exec);
char* skipcomma(char* str);
static int split(char *cmd_exec, int, int, int);
void with_pipe_execute();
static int command(int, int, int, char *cmd_exec);
//void prompt();
void sigintHandler(int sig_num);

/*************************************************************************/
void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    fflush(stdout);
}

/*********************** harsh functions ***********************/ 
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[])(char **)={
  &shell_cd,
  &shell_help,
  &shell_exit
};

int shell_num_builtins(){
  return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args)
{
  if (args[1] == NULL){
    fprintf(stderr, "shell: expected argument to \"cd\" \n");
  }
  else{
    if (chdir(args[1]) != 0)
    {
      perror("shell");
    }
  }
  return 1;
}

int shell_help(char **args)
{
  int i;
  printf("Welcome to My shell \n");
  printf("type program names and arguments and hit enter \n");
  printf("the following are built in : \n");

  for (i=0; i< shell_num_builtins() ; i++)
  {
    printf("%s\n",builtin_str[i]);
  }
  printf("use the man command for the information on other programs \n");
  return 1;
}
int shell_exit(char **args)
{
  return 0;
}

/****************************************************************/



void clear_variables()
{
  fd =0;
  flag=0;
  len=0;
  no_of_lines=0;
  pipe_count=0;
  flag_pipe=0;
  flag_without_pipe=0;
  output_redirection=0;
  input_redirection=0;
  input_buffer[0]='\0';
  cwd[0] = '\0';
  pid=0;
  //environmment_flag=0;
  //bang_flag=0;
}


static char* skipwhite(char* s)
{
  while (isspace(*s)) ++s;
  return s;
}
void tokenise_commands(char *com_exec)
{
int m=1;
args[0]=strtok(com_exec," ");       
while((args[m]=strtok(NULL," "))!=NULL)
        m++;
}
void tokenise_redirect_input_output(char *cmd_exec)
{
  char *io_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  io_token[0]=strtok(new_cmd_exec1,"<");       
  while((io_token[m]=strtok(NULL,">"))!=NULL)
        m++;
  /*printf("initial one io_token[1] %s\n",io_token[1]);
  printf("initial one io_token[2] %s\n",io_token[2]);*/

  io_token[1]=skipwhite(io_token[1]);
  io_token[2]=skipwhite(io_token[2]);

  /*printf("skip white one io_token[1] %s\n",io_token[1]);
  printf("skip white one io_token[2] %s\n",io_token[2]);*/
  

  input_redirection_file=strdup(io_token[1]);
  output_redirection_file=strdup(io_token[2]);
  tokenise_commands(io_token[0]);
  
}
void tokenise_redirect_input(char *cmd_exec)
{
  char *i_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  i_token[0]=strtok(new_cmd_exec1,"<");       
  while((i_token[m]=strtok(NULL,"<"))!=NULL)
        m++;
  i_token[1]=skipwhite(i_token[1]);
  input_redirection_file=strdup(i_token[1]);
  tokenise_commands(i_token[0]);
}
void tokenise_redirect_output(char *cmd_exec)
{
  char *o_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  o_token[0]=strtok(new_cmd_exec1,">");       
  while((o_token[m]=strtok(NULL,">"))!=NULL)
          m++;
  o_token[1]=skipwhite(o_token[1]);
  output_redirection_file=strdup(o_token[1]); 
  tokenise_commands(o_token[0]);   
  
}
char* skipcomma(char* str)
{
  int i=0, j=0;
  char temp[1000];
  while(str[i++]!='\0')
            {
              if(str[i-1]!='"')
                    temp[j++]=str[i-1];
            }
        temp[j]='\0';
        str = strdup(temp);
  
  return str;
}


/****** changes made ****************/

static int split(char *cmd_exec, int input, int first, int last)
{
    char *new_cmd_exec1;  
    new_cmd_exec1=strdup(cmd_exec);
        int m=1;
        args[0]=strtok(cmd_exec," ");       
        while((args[m]=strtok(NULL," "))!=NULL)
              m++;
        args[m]=NULL;
        if (args[0] != NULL) 
            {

            if(strcmp("cd",args[0])==0)
                    {
                    //change_directory();
                    return  (*builtin_func[0])(args);
                    //return 1;
                    }  
            if (strcmp(args[0], "exit") == 0) 
                    return  (*builtin_func[2])(args);
            if(strcmp("help",args[0])==0)
                    {
                    return  (*builtin_func[1])(args);
                    }
            if (strcmp(args[0], "echo") != 0) 
                    {
                      cmd_exec = skipcomma(new_cmd_exec1);
                      int m=1;
                      args[0]=strtok(cmd_exec," ");       
                      while((args[m]=strtok(NULL," "))!=NULL)
                                m++;
                      args[m]=NULL;

                    }
           
            }
    return command(input, first, last, new_cmd_exec1);
}


void with_pipe_execute()
{

int i, n=1, input, first;

input=0;
first= 1;

cmd_exec[0]=strtok(input_buffer,"|");

while ((cmd_exec[n]=strtok(NULL,"|"))!=NULL)
      n++;
cmd_exec[n]=NULL;
pipe_count=n-1;
for(i=0; i<n-1; i++)
    {
      input = split(cmd_exec[i], input, first, 0);
      first=0;
    }
input=split(cmd_exec[i], input, first, 1);
input=0;
return;

}
static int command(int input, int first, int last, char *cmd_exec)
{
  int mypipefd[2], ret, input_fd, output_fd;
  ret = pipe(mypipefd);
  if(ret == -1)
      {
        perror("pipe");
        return 1;
      }
  pid = fork();
 
  if (pid == 0) 
  {
    if (first==1 && last==0 && input==0) 
    {
      dup2( mypipefd[1], 1 );
    } 
    else if (first==0 && last==0 && input!=0) 
    {
      dup2(input, 0);
      dup2(mypipefd[1], 1);
    } 
    else 
    {
      dup2(input, 0);
    }
    if (strchr(cmd_exec, '<') && strchr(cmd_exec, '>')) 
            {
              input_redirection=1;
              output_redirection=1;
              tokenise_redirect_input_output(cmd_exec);
            }
   else if (strchr(cmd_exec, '<')) 
        {
          input_redirection=1;
          tokenise_redirect_input(cmd_exec);
        }
   else if (strchr(cmd_exec, '>')) 
        {
          output_redirection=1;
          tokenise_redirect_output(cmd_exec);
        }
    if(output_redirection == 1)
                {                    
                        output_fd= creat(output_redirection_file, 0644);
                        if (output_fd < 0)
                          {
                          fprintf(stderr, "Failed to open %s for writing\n", output_redirection_file);
                          return(EXIT_FAILURE);
                          }
                        dup2(output_fd, 1);
                        close(output_fd);
                        output_redirection=0;
                }
    if(input_redirection  == 1)
                  {
                         input_fd=open(input_redirection_file,O_RDONLY, 0);
                         if (input_fd < 0)
                          {
                          fprintf(stderr, "Failed to open %s for reading\n", input_redirection_file);
                          return(EXIT_FAILURE);
                          }
                        dup2(input_fd, 0);
                        close(input_fd);
                        input_redirection=0;
                  }
 
    if(execvp(args[0], args)<0) printf("%s: command not found\n", args[0]);
              exit(0);
  }
  else 
  {
     waitpid(pid, 0, 0);  
   }
 
  if (last == 1)
    close(mypipefd[0]);
  if (input != 0) 
    close(input);
  close(mypipefd[1]);
  return mypipefd[0];

}






/*void prompt()
{
  char shell[1000];
   if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
          strcpy(shell, "My_shell:");
          //strcat(shell, cwd);
          strcat(shell, "$ ");

          printf("%s", shell);
        }
   else
       perror("getcwd() error");

}*/

int main()
{   
    int status;
    //char ch[2]={"\n"};
    //getcwd(current_directory, sizeof(current_directory));
    signal(SIGINT, sigintHandler);
    while (1)
    {
      clear_variables();
      //prompt();
      printf(">");
      fgets(input_buffer, 1024, stdin);
      if(strcmp(input_buffer, "\n")==0)
            {
              continue;
            }
      
      len = strlen(input_buffer);
      input_buffer[len-1]='\0';
      //strcpy(his_var, input_buffer);
      if(strcmp(input_buffer, "exit") == 0) 
            {
              //flag = 1;
              exit(0); 
              break;
            }

      with_pipe_execute();
      waitpid(pid,&status,0);
         
    }  
    /*if(flag==1)
      {
      printf("Bye...\n");
      exit(0);       
      //return 0;
      }*/
return 0;
}