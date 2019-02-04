//
// Created by luigi wu on 2018/9/20.
//

#ifndef PROJECT_BUILTIN_H
#define PROJECT_BUILTIN_H
#include <string.h>
#include <stdio.h>
#include "glo_value.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
int if_out = 0;

void my_exit(char *comma) {
  // printf("debug_builtin:%s\n",comma);
  if (strcmp(comma, "exit") == 0)
    if_out = 1;

  // printf("debug_builtin: if_out = %d\n",if_out);
}

void my_pwd(char *comma) {
  if (strcmp(comma, "pwd") == 0) {
    char org[Command_Length];
    memset(org, 0, Command_Length);
    getcwd(org, Command_Length);
    printf("%s\n", org);
    fflush(stdout);
  }
}
void my_cd(char *comma, char *directory) {
  if (strcmp(comma, "cd") == 0) {
    strcat(directory, "/");
    //strcat(directory, "/");
    int re = chdir(directory);
    if(re == -1){

      directory[strlen(directory)-1]=0;
      printf("%s: No such file or directory\n",directory);
      fflush(stdout);
    }
  }
}

void my_jobs(char *comma,
             char list1[Back_Num][Command_Length],
             int list2[Back_Num][Pipe_NUM], int * pip_nums,int back_iter) {
  if (strcmp(comma, "jobs") == 0) {
    for (int i = 0; i < back_iter; i++) {
      int sta = 0;
      if(pip_nums[i]==0){
        sta = waitpid(list2[i][0], NULL, WNOHANG);
      }
      for(int j=0;j< pip_nums[i];j++) {
        sta = sta & waitpid(list2[i][j], NULL, WNOHANG);
      }
      //printf("sta = %d\n",sta);
      if(sta != 0){
        printf("[%d] done ",i+1);
      }
      else if(sta == 0){
        printf("[%d] running ",i+1);
      }
      for(int j = 0; j< Command_Length;j++){
        printf("%c",list1[i][j]);
        if(list1[i][j]=='&'){
          break;
        }
      }
      printf("\n");
    }

  }
}

#endif //PROJECT_BUILTIN_H
