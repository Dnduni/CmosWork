#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
	using std::ofstream;
#include <limits>
#include <thread>
#include <chrono>

int main(){
    int buff_size = 3008*3008;
    int buffer[buff_size];
    int i;
    for(i = 0 ;  i < buff_size ; i ++){


        buffer[i] = std::rand()/(RAND_MAX) * 255;


    }

    std::cout << "Hello world" << std::endl;
    std::cout.flush();
    return 0;









}
