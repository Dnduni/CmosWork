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
    int *buffer = new int[buff_size];
    int i;
    for(i = 0 ;  i < buff_size ; i ++){


        buffer[i] = (std::rand() % 256);

    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int NativeResX = 3008;
    int a = 0;
    int b = 0;
    std::stringstream ss;
    int threshold = 200;
    ofstream stream;
    ss << "Active_pixels_tresh_" << threshold << ".txt";
    std::string s = ss.str();
    
    stream.open(s, std::fstream::out);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!stream){
		std::cout << "Cannot open output file" << std::endl;
	}
    
    
    for(a = 0; a < buff_size; a++){
        if((a+1)%NativeResX == 0){
            
            b += 1;
        }
        

        
        if(buffer[a] > threshold){

            stream << (int)buffer[a] << "\t" << a << "\t" << b << std::endl;

        }
    }
    stream.close();







}
