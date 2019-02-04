//
// Created by luigi wu on 2018/9/29.
//

#ifndef PROJECT_EXEC_H
#define PROJECT_EXEC_H
#include "parse.h"
#include <unistd.h>
#include "builtin.h"
#include "glo_value.h"
void close_all(int *pipe_fd) {
  for (int i = 0; i < Pipe_NUM; i++) {
    close((*pipe_fd) + 2 * i);
    close((*pipe_fd) + 2 * i + 1);
  }
}

void dup_right(int *pipe_fd, int iter, int redirection_kind) {
  //if dup out redirection (dup for write)
  if (redirection_kind == 1) {
    int test;
    test = dup2(*pipe_fd + 2 * iter + 1, 1);
//    printf("iter == %d, test_out = %d\n", iter, test);
  }
  //if dup in redirection (dup for read)
  if (redirection_kind == 0) {
    int test;
    test = dup2(*pipe_fd + 2 * (iter - 1), 0);
//    printf("iter == %d, test_in = %d\n", iter, test);
  }
}

void process_exec(struct pip *a,
                  int iter,
                  int pip_num,
                  int *pipe_fd,
                  int *pcsid,
                  int back_process_id[Back_Num][Pipe_NUM],
                  int if_back,
                  int back_iter) {

  if (iter > pip_num) return;
  if (strcmp("cd", a[iter].group[0]) == 0 && iter == pip_num) {
    if (strcmp("cd", a[iter].group[0]) == 0 && a[iter].group[1] == (NULL)) {
      clear(a);
      return;
    }
    my_cd(a[iter].group[0], a[iter].group[1]);
  }

  pipe(pipe_fd + 2 * iter);
  pcsid[iter] = 1;
  if (strcmp("cd", a[iter].group[0]) != 0) {
    pcsid[iter] = fork();
  }
  if (pcsid[iter] == 0) {

    if (a[iter].num_out != 0) {
      if (a[iter].num_out > 1 || iter != pip_num) {
        printf("error: duplicated output redirection\n");
        fflush(stdout);
        clear(a);
        exit(1);
      }
      for (int i = 0; i < a[iter].num_out; i++) {
        FILE *ree = NULL;
        fclose(stdout);
        if (a[iter].kind_of_out[i] == 1)
          ree = freopen(a[iter].output_file[i], "w", stdout);
        else if (a[iter].kind_of_out[i] == 2)
          ree = freopen(a[iter].output_file[i], "ab", stdout);
        if (ree == NULL) {
          fprintf(stderr, "%s: Permission denied\n", a[iter].output_file[i]);
          clear(a);
          exit(1);
        }
      }

    }
    if (a[iter].num_in != 0) {
      if (a[iter].num_in > 1 || pip_num > 0) {
        printf("error: duplicated input redirection\n");
        fflush(stdout);
        clear(a);
        exit(1);
      }
      for (int i = 0; i < a[iter].num_in; i++) {
        fclose(stdin);

        FILE *re = freopen(a[iter].input_file[i], "r", stdin);
        if (re == NULL) {
          printf("%s: No such file or directory\n", a[iter].input_file[i]);
          fflush(stdout);
          clear(a);
          exit(1);
        }
      }
    }


    //use the pipe to redirection with input situation
    if (a[iter].num_in == 0) {
      if (iter > 0) {
        dup_right(pipe_fd, iter, Input);
        if (a[iter].num_out != 0 || iter >= pip_num) {
          close_all(pipe_fd);
        }
      }
    }

    //use the pipe to redirection with output situation
    if (a[iter].num_out == 0) {
      if (iter < pip_num) {
        dup_right(pipe_fd, iter, Output);
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
      //printf("no!!!\n");
      //deal with the error
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
    my_exit(a[iter].group[0]);

    //dertermine the situation that may not start the next pipe
    if (if_out == 1) return;
    if (a[iter].num_out != 0) { return; }

    //start the next pipe
    if (a[iter].num_out == 0 || iter == pip_num) {
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
}

#endif //PROJECT_EXEC_H
