#include<iostream>
#include<omp.h>
using namespace std;

int main(){
    cout << "the begin of loop" << endl;

    #pragma omp parallel for
    for (int i = 0; i < 10; ++i) {
        cout << i;
    }
    cout << endl << "the end of loop" << endl;

}