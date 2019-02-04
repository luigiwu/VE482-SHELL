//
// Created by luigi wu on 2018/9/18.
//

#ifndef PROJECT_PARSE_H
#define PROJECT_PARSE_H
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "glo_value.h"

static int record_last[Pipe_NUM];

typedef struct pip {
  char *group[Word_COUNT];
  char input_file[Word_COUNT][Word_COUNT];
  int num_in;
  char output_file[Word_COUNT][Word_COUNT];
  int num_out;
  int kind_of_out[Word_COUNT];
} pip_t;

//0 for proper
// 1 for syntax error
//2 for missing program
int if_syntax(char test){
  if(test == '<' | test == '&' | test == '|' | test == '>') return 1;
  else return 0;
}

int error_scan(char* command,int syntax_pointer){


  if(command[syntax_pointer+1]=='>' && command[syntax_pointer]=='>') return 0;
  for(int i = syntax_pointer + 1;i<Command_Length-1;i++){
    if(command[i]=='\0') return 0;
    else if(if_syntax(command[i])==1){
      if(if_syntax(command[i+1])!=1){
        if(command[syntax_pointer]=='|' && command[i]=='|'){
          printf("error: missing program\n");
          return 2;
        }
        else{
          printf("syntax error near unexpected token `%c'\n",command[i]);
          return 1;
        }

      } else{
        if(if_syntax(command[i])=='>' && if_syntax(command[i+1])=='>'){
          printf("syntax error near unexpected token `>>'\n");
        }
        else{
          printf("syntax error near unexpected token `%c'\n",command[i]);
        }
        return 1;
      }
      return 1;
    }
    else if(command[i]!=' '){
      return 0;
    }
  }

  return 0;
}


void initial(struct pip *a) {
  a->num_in = 0;
  a->num_out = 0;
}
//judge whether a char is < or > which will be used for the direction
int if_Background(char test) {
  if (test == '&') return 1;
  return 0;
}

//judge whether a char is < or > which will be used for the direction
int if_IOdirection(char test) {
  if (test == '<' | test == '>') return 1;

  return 0;
}
int if_pipe_op(char test) {
  if (test == '|') return 1;
  return 0;
}

//clear the malloc before the process terminate
void clear(struct pip *a) {
  for (int j = 0; j < Pipe_NUM; j++) {
    for (int i = 0; i < record_last[j]; i++) {
      free((a + j)->group[i]);
    }
  }
}

void quate_scan(char *result,
                char *cammander,
                int *pointer,
                char qua, int *count) {
  while (cammander[*pointer] == qua) {
    (*pointer)++;
  }
  while (cammander[*pointer] != qua) {
    if (cammander[*pointer] == qua) {
      break;
    } else {
      result[(*count)++] = cammander[(*pointer)++];
    }
  }
  (*pointer)++;
  result[(*count)] = '\0';

}

// get the specified groups of charactors
void scan(char *result,
          char *cammander,
          int *pointer,
          int *pip_num,
          int *if_quate_special) {
  *if_quate_special = 0;
  int count = 0;
  while (cammander[*pointer] == ' ' | cammander[*pointer] == '\t') {
    (*pointer)++;
  }
  while (cammander[*pointer] != ' ' && cammander[*pointer] != '\t') {
    if (cammander[*pointer] == '\0') {
      break;
    } else if (if_IOdirection(cammander[*pointer]) == 1) {
      if (count == 0) {
        result[count++] = cammander[(*pointer)++];
        if (if_IOdirection(cammander[(*pointer)]) == 1) {
          result[count++] = cammander[(*pointer)++];
        }
        result[count] = '\0';
      }
      break;
    } else if (if_pipe_op(cammander[*pointer]) == 1) {
      if (count == 0) {
        result[count++] = cammander[(*pointer)++];
        (*pip_num) = (*pip_num) + 1;
        if (if_pipe_op(cammander[(*pointer)]) == 1) {
          result[count++] = cammander[(*pointer)++];
        }
        result[count] = '\0';
      }
      break;
    } else if (if_Background(cammander[*pointer]) == 1) {
      if (count == 0) {
        result[count++] = cammander[(*pointer)++];
        if (if_Background(cammander[(*pointer)]) == 1) {
          result[count++] = cammander[(*pointer)++];
        }
        result[count] = '\0';
      }
      break;
    } else {
      if (cammander[*pointer] == '\'') {
        if (cammander[*pointer + 1] == '\'') {
          *pointer = *pointer + 2;
        } else {
          *if_quate_special = 1;
          quate_scan(result, cammander, pointer, '\'', &count);
        }
      } else if (cammander[*pointer] == '\"') {
        if (cammander[*pointer + 1] == '\"') {
          *pointer = *pointer + 2;
        } else {
          *if_quate_special = 1;
          quate_scan(result, cammander, pointer, '\"', &count);
        }
      } else {
        result[count++] = cammander[(*pointer)++];
      }
    }
  }

  result[count] = '\0';
  //printf("%s\n",result);
}



//

//Requirement: The word should not be empty
//after scan, cut the getting word into smaller piece of word,not only
//the space is the reference for cut
void word_divide(char *word, int *pointer, char *group[Word_COUNT]) {

  group[*pointer] = (char *) malloc(sizeof(char) * Word_LENGTH);
  strcpy(group[*pointer], word);
  //printf("debug:%s\n", group[*pointer]);
  (*pointer)++;
  //       printf("%d\n", *pointer);

}

void parse(char *str,
           struct pip *a,
           int *pip_num, int* back_iter,int* if_back) {
  int if_quate_special = 0;
  int pointer = 0;
  char result[Word_LENGTH];
  int count[Pipe_NUM];
  int flag = 0;
  //initial the count array
  for (int i = 0; i < Pipe_NUM; i++) {
    count[i] = 0;
  }

  //free the memory that is allocated at the last time
  for (int j = 0; j < Pipe_NUM; j++) {
    for (int i = 0; i < record_last[j]; i++) {
      free((a + j)->group[i]);
    }
  }
  //make all the memory be null in case of the big allocation
  for (int j = 0; j < Pipe_NUM; j++) {
    for (int i = 0; i < Word_COUNT; i++) {
      (a + j)->group[i] = (NULL);
    }
  }
  while (str[pointer] != '\0') {

    scan(result, str, &pointer, pip_num,&if_quate_special);
//    printf("%s ",result);
    if(if_quate_special==0) {
      if (strcmp(result, ">") == 0) {
        flag = 1;
      }
      if (strcmp(result, ">>") == 0) {
        flag = 2;
      }
      if (strcmp(result, "<") == 0) {
        flag = 3;
      }
      if (strcmp(result, "|") == 0 || strcmp(result, "||") == 0) {
        flag = 4;
      }
      if (strcmp(result, "&") == 0 || strcmp(result, "&&") == 0) {
        flag = 5;
      }
    }

    if (result[0] == '\0') {
      record_last[*pip_num] = count[*pip_num];
      break;
    }

    //----------word divide----------
    //printf("get_word = %s\n", result);
//    printf("flag = %d\n",flag);
    if (flag == 0) {
      word_divide(result, count + *pip_num, (a + *pip_num)->group);
      record_last[*pip_num] = count[*pip_num];
    } else if (flag == 1 & strcmp(result, ">") != 0) {
      flag = 0;
      if (result[0] == '\0') {
        printf("No such a file!\n");
        exit(1);
      }
//      word_divide(result, num_out, output_file);
      ((a + *pip_num)->num_out) = ((a + *pip_num)->num_out) + 1;
      (a + *pip_num)->kind_of_out[((a + *pip_num)->num_out) - 1] = 1;
      strcpy((a + *pip_num)->output_file[((a + *pip_num)->num_out) - 1],
             result);
//      printf("output file: %s\nnum of out: %d\n",
//             output_file[*num_out - 1],
//             *num_out);
    } else if (flag == 2 & strcmp(result, ">>") != 0) {
      flag = 0;
      if (result[0] == '\0') {
        printf("No such a file!\n");
        exit(1);
      }
//      word_divide(result, num_out, output_file);
      ((a + *pip_num)->num_out) = ((a + *pip_num)->num_out) + 1;
      (a + *pip_num)->kind_of_out[((a + *pip_num)->num_out) - 1] = 2;
      strcpy((a + *pip_num)->output_file[((a + *pip_num)->num_out) - 1],
             result);
//      printf("output file2: %s\nnum of out2: %d\n",
//             output_file[*num_out - 1],
//             *num_out);
    } else if (flag == 3 & strcmp(result, "<") != 0) {
      flag = 0;
      if (result[0] == '\0') {
        printf("No such a file!\n");
        exit(1);
      }
      ((a + *pip_num)->num_in) = ((a + *pip_num)->num_in) + 1;
//      word_divide(result, num_in, input_file);
      strcpy((a + *pip_num)->input_file[((a + *pip_num)->num_in) - 1], result);
//      printf("input file2: %s\nnum of in2: %d\n",
//             input_file[*num_in - 1],
//             *num_in);
    } else if (flag == 4) {
      flag = 0;
    } else if (flag == 5) {
      *back_iter = *back_iter + 1;
      *if_back = 1;
//      printf("back iter = %d\n",*back_iter);
//      printf("if back = %d\n",*if_back);
      break;
    }
  }

//  printf("pipe_num = %d\n", *pip_num);
//  for(int i = 0;i < Pipe_NUM;i++){
//    printf("%d command, last_record == %d\n",i,record_last[i]);
//  }
}

#endif //PROJECT_PARSE_H
