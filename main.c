#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "parse.h"
#include "builtin.h"
#include <signal.h>
#include "exec.h"

static void sig_cancel(int);
static int pip_num;
static int go_on;
static int pcsid[Pipe_NUM];
static int single_quate_num;
static int double_quate_num;
static int if_pipe_end;
static char command[Command_Length];
static char back_command[Back_Num][Command_Length];
static int back_process_id[Back_Num][Pipe_NUM];
static int back_process_pipes[Back_Num];
static int if_syntax_error;
int if_odd_quate(char *command,
                 int *single_quate_num,
                 int *double_quate_num,
                 int *if_pipe_end, int *if_syntax_error) {
//  printf("double_quate_num = %d\n", *double_quate_num);
//  printf("single_quate_num = %d\n", *single_quate_num);

  for (int i = 0; i < Command_Length; i++) {
    if ((command[i] == '\'') && (*double_quate_num % 2 == 0)) {
      (*single_quate_num)++;
    }
    if ((command[i] == '\"') && (*single_quate_num % 2 == 0)) {
      (*double_quate_num)++;
    }
    if (*double_quate_num % 2 == 0 && *single_quate_num % 2 == 0) {
      if ((command[i] == '|') || (command[i] == '>') || (command[i] == '<')) {
        //printf("cone in1!\n");
        *if_syntax_error = error_scan(command, i);
        if (*if_syntax_error > 0) { break; }
        *if_pipe_end = 0;
      } else if (*if_pipe_end == 0) {
        if (command[i] == '\n') {
          command[i] = '\0';
          break;
        } else if (command[i] != ' ') {
          //printf("cone in2!\n");
          *if_pipe_end = 1;
        }
      }
    }

  }
  //printf("if_pipe_end = %d\n",*if_pipe_end);
//  printf("double_quate_num = %d\n", *double_quate_num);
//  printf("single_quate_num = %d\n", *single_quate_num);
//  printf("if_odd_quate = %d\n",
//         *double_quate_num % 2 == 1 || *single_quate_num % 2 == 1);
  if (*double_quate_num % 2 == 1 || *single_quate_num % 2 == 1) {
    return 1;
  } else {
    return 0;
  }
}

int main() {

  int back_iter = 0;
  signal(SIGINT, sig_cancel);
  pip_t a[Pipe_NUM];
  pip_num = 0;
  char buffer[Command_Length];
  for (int i = 0; i < Pipe_NUM; i++) {
    initial(&(a[i]));
  }
  //define a parameter for pipe
  //int pipe_result = -1;
  int pipe_fd[Pipe_NUM * 2];

  int iter = 0;
  int if_back;
  while (1) {
    if_back = 0;
    for (int i = 0; i < Pipe_NUM; i++) {
      pcsid[i] = 1;
    }
    int status;
    pip_num = 0;

    if (if_out == 1) {
      //here plase add the free logic .........
      clear(a);
      printf("exit\n");
      break;
    }

    //read the commands
    printf("mumsh $ ");
    fflush(stdout);
    char *re;
    go_on = 0;
//    int count_n = 0;
    single_quate_num = 0;
    double_quate_num = 0;
    memset(command, 0, Command_Length);
    if_pipe_end = 1;
    if_syntax_error = 0;
    while (1) {
      //printf("test_start\n");
      fflush(stdin);
      memset(buffer, 0, Command_Length);
      re = fgets(buffer, Command_Length, stdin);
      //printf("test_end\n");

      if (re == NULL) {
        //printf("error!\n");
        strcpy(buffer, "");
        strcpy(command, "");
        fflush(stdout);
        break;
      }
      //printf("end\n");
      fflush(stdin);

      int tmp = if_odd_quate(buffer,
                             &single_quate_num,
                             &double_quate_num,
                             &if_pipe_end, &if_syntax_error);
      if (if_syntax_error > 0) {
        break;
      }
      if (tmp == 0 && if_pipe_end == 1) {
        break;
      }
      printf("> ");
      fflush(stdout);
      strcat(command, buffer);
      // printf("%s\n",buffer);
    }
    if (if_syntax_error > 0) { continue; }
    strcat(command, buffer);
//    printf("%s\n",command);
    fflush(stdin);
    go_on = 1;
    if (re == NULL) { strcpy(command, "exit"); }
    int in_single = 0;
    int in_double = 0;
    for (int i = 0; i < Command_Length; i++) {
      if (command[i] == '\'' && in_double == 0) {
        if (in_single == 0) in_single = 1;
        else in_single = 0;
      } else if (command[i] == '\"' && in_single == 0) {
        if (in_double == 0) in_double = 1;
        else in_double = 0;
      }
      if (command[i] == '\n' | command[i] == '\0') {
        if (in_double == 0 && in_single == 0) {
          command[i] = '\0';
          break;
        }
      }
    }

    // initial for the next command
    for (int i = 0; i < Pipe_NUM; i++) {
      initial(&(a[i]));
    }

    parse(command,
          a,
          &pip_num, &back_iter, &if_back);
    if (if_back == 1) {
      memset(back_command[back_iter - 1], 0, Command_Length);
      for (int i = 0; i < Command_Length; i++) {
        back_command[back_iter - 1][i] = command[i];
        if (command[i] == '&') break;
      }
      back_process_pipes[back_iter - 1] = pip_num;
      printf("[%d] %s\n", back_iter, command);
      fflush(stdout);
    }
    //The situation for return
    if (a[0].group[0] == NULL) {
      continue;
    }



    //print all of the result in a
//    for (int i = 0; i <= pip_num; i++) {
//      printf("%d command, group[0] == %s\n", i, a[i].group[0]);
//      printf("%d command, num_in == %d\n", i, a[i].num_in);
//      printf("%d command, num_out == %d\n", i, a[i].num_out);
//    }
    iter = 0;
    for (int i = 0; i < Pipe_NUM; i++) {
      pipe(pipe_fd + 2 * i);
    }

    if (strcmp("cd", a[iter].group[0]) == 0 && iter == pip_num) {
      if (strcmp("cd", a[iter].group[0]) == 0 && a[iter].group[1] == (NULL)) {
        continue;
      }
      my_cd(a[iter].group[0], a[iter].group[1]);
    }
    if (strcmp("jobs", a[iter].group[0]) == 0 && iter == pip_num) {
      my_jobs(a[iter].group[0],
              back_command,
              back_process_id,
              back_process_pipes,
              back_iter);
      continue;
    }
    pcsid[iter] = 1;
    if (strcmp("cd", a[iter].group[0]) != 0
        && strcmp("jobs", a[iter].group[0]) != 0) {
      pcsid[iter] = fork();
    }
    if (pcsid[iter] == 0) {

      if (a[iter].num_out != 0) {
        if (a[iter].num_out > 1 || pip_num != 0) {
          printf("error: duplicated output redirection\n");
          fflush(stdout);
          clear(a);
          exit(1);
        }

        for (int i = 0; i < a[iter].num_out; i++) {
          FILE *ree = NULL;
          fclose(stdout);
          if (a[iter].kind_of_out[i] == 1) {
            ree = freopen(a[iter].output_file[i], "w", stdout);
          } else if (a[iter].kind_of_out[i] == 2) {
            ree = freopen(a[iter].output_file[i], "ab", stdout);
          }
          if (ree == NULL) {
            fprintf(stderr, "%s: Permission denied\n", a[iter].output_file[i]);
            clear(a);
            exit(1);
          }
        }

      }
      if (a[iter].num_in != 0) {
        if (a[iter].num_in > 1) {
          printf("error: duplicated input redirection\n");
          fflush(stdout);
          clear(a);
          exit(1);
        }
        for (int i = 0; i < a[iter].num_in; i++) {
          fclose(stdin);
          FILE *ree = freopen(a[iter].input_file[i], "r", stdin);
          if (ree == NULL) {
            printf("%s: No such file or directory\n", a[iter].input_file[i]);
            fflush(stdout);
            clear(a);
            exit(1);
          }
        }
      }


      //use the pipe to redirection with output situation
      if (a[iter].num_out == 0) {
        if (iter < pip_num) {
          dup2(pipe_fd[1], 1);
          close_all(pipe_fd);
        }
      }
      //execute the command
      if (strcmp("pwd", a[iter].group[0]) == 0) {
        my_pwd(a[iter].group[0]);
        clear(a);
        exit(1);
      } else {
        execvp(a[iter].group[0], a[iter].group);
        if (strcmp("exit", a[iter].group[0]) != 0)
          printf("%s: command not found\n", a[iter].group[0]);
        if (a[iter].num_out != 0) {
          fclose(stdout);
        }
        if (a[iter].num_in != 0) {
          fclose(stdin);
        }
        clear(a);
        exit(1);
      }
    } else {
      if (if_back == 1) {
        back_process_id[back_iter - 1][iter] = pcsid[iter];
      }
      if (pip_num >= 1) {
        if (a[iter].num_out == 0) {
          process_exec(a,
                       iter + 1,
                       pip_num,
                       pipe_fd,
                       pcsid,
                       back_process_id,
                       if_back,
                       back_iter);
        }
      }

      my_exit(a[iter].group[0]);

//      printf("pip_num = %d\n", pip_num);
      //close all the sub pipe!
      close_all(pipe_fd);
      //deal with the wait logic
      if (if_back == 0) {
        if (pip_num == 0) {
          wait(NULL);
//          waitpid(pcsid[0],NULL,0);
        } else {
          for (int i = 0; i <= pip_num; i++) {
            wait(&status);
//            waitpid(pcsid[i],&status,0);
          }
        }
        usleep(800);
      } else {
        //usleep(100000);
        for (int i = 0; i <= pip_num; i++) {
          waitpid(-1, &status, WNOHANG);
        }
      }

    }
  }

  return 0;
}
static void sig_cancel(int signum) {
  signum = signum + 1;

  for (int i = 0; i <= pip_num + 1; i++) {
    if (pcsid[i] == 0) {
      exit(0);
    }
  }
  if (go_on == 0) {
    single_quate_num = 0;
    double_quate_num = 0;
    memset(command, 0, Command_Length);
    if_pipe_end = 1;
    printf("\nmumsh $ ");
    fflush(stdout);
  } else {
    printf("\n");
    fflush(stdout);
  }

}
