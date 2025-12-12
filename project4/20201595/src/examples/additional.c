#include<stdio.h>
#include<string.h>
#include<syscall.h>
#include<stdlib.h>

int main(int argc, char *argv[]){

  if(argc == 5){
    int n[4];
    for(int i=1;i<5;i++){
      n[i-1] = atoi(argv[i]);
    }
    if(n[0]== -1){
      printf("Wrong fibonacci input!\n");
    }
    else{
      printf("fibonaci (%d): %d\n", n[0], fibonacci(n[0]));
    }
    printf("max_of_four_int (%d, %d, %d, %d): %d\n", n[0], n[1], n[2],n[3], max_of_four_int(n[0],n[1],n[2],n[3]));
  }

  else{
    printf("Wrong input!\n");
  }
  return 0;
}
