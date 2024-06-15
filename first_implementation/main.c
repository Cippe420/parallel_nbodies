#include <stdio.h>

void showpointers(int *num1, int *num2, int *result){

    while(*result < 20){

        *result += *num1;
        *result += *num2;

    }


}

int main(){
    int num1 = 2;
    int num2 = 1;

    int result=0;

    showpointers(&num1,&num2,&result);
    printf("result: %d ",result);
}