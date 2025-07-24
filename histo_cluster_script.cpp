{
//   example of macro to read data from an ascii file and
//   create a root file with an histogram and an ntuple.


#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TH1F.h"
#include "TBenchmark.h"
#include "TRandom.h"
#include "TSystem.h"
#include <fstream>
#include <omp.h>
using namespace std;
#define max_row  3008
#define max_col  3008
#define max_frames 100

struct pixel{
    int x;
    int y;
    int val;
    int frame;
};

std::stringstream path;
std::string string_path;


vector<pixel> centers;
std::ifstream clusterfile;
pixel seed;
int count = 0;
clusterfile.open("clustersdark.txt");
std::cout << "Reading cluster file" << std::endl;
int a,b,c,d;
TH1F *  seedhisto = new TH1F("seed", "seed", 200 , 0 ,255); //Histogram of seed value
while(clusterfile >> a >> b >> c >> d ){
    seed.x = a;
    seed.y = b;
    seed.val = c;
    seed.frame = d;
    centers.push_back(seed);
    seedhisto->Fill(seed.val);
}

clusterfile.close();

int i,j,frameind;
int cluster_sum;
int number;
std::vector<std::vector<int>> V(max_row, std::vector<int>(max_col, 0));
int edge3 = 1;
int edge5 = 2;
int edge7 = 3;
a,b,c,d = 0;
TH1F * histo3 = new TH1F("3x3 cluster value", "3x3 cluster value", 500, 0 , 200);
TH1F * histo5 = new TH1F("5x5 cluster value", "5x5 cluster value", 500, 0 , 200);
TH1F * histo7 = new TH1F("7x7 cluster value", "7x7 cluster value", 500, 0 , 200);
int sum3,sum5,sum7;
#pragma omp parallel
{
    #pragma omp for

for(frameind = 0; frameind < max_frames; frameind++){

    number = frameind;
    std::cout << "Working on frame " << number << std::endl;
    
    
    
    if(number < 10){
        path << "2025_05_21_dark_1s_gain_200/image0000" << number << ".txt";
        string_path = path.str();
        clusterfile.open(string_path);
        path.str("");
    }

    if(number >= 10 && number < 100){
        path << "2025_05_21_dark_1s_gain_200/image000" << number << ".txt";
        string_path = path.str();
        clusterfile.open(string_path);
        path.str("");
    }

    if(number >= 100 && number < 1000 ){
        path << "2025_05_21_dark_1s_gain_200/image00" << number << ".txt";
        string_path = path.str();
        clusterfile.open(string_path);
        path.str("");
    }


    if(number >= 1000){
        path << "2025_05_21_dark_1s_gain_200/image0" << number << ".txt";
        string_path = path.str();
        clusterfile.open(string_path);
        path.str("");
    }

    for(i = 0; i < max_row; i++ ){
        for(j = 0; j < max_col; j++){

            clusterfile >> V[i][j];
        }
    }





    for(a = 0; a < int(centers.size()); a++){

        if(centers[a].frame == number){
            sum3 = 0;
            sum5 = 0;
            sum7 = 0;
            for(b = centers[a].x - edge3; b <= centers[a].x + edge3; b++){
                for(c = centers[a].y - edge3; c <= centers[a].y + edge3; c++ ){
                    if (b >= 0 && b < max_row && c >= 0 && c < max_col) {
                        sum3 += V[b][c];
                    }   
                }


            }




            for(b = centers[a].x - edge5; b <= centers[a].x + edge5; b++){
                for(c = centers[a].y - edge5; c <= centers[a].y + edge5; c++ ){

                    if (b >= 0 && b < max_row && c >= 0 && c < max_col) {
                        sum5 += V[b][c];
                    }   
                    
                    }


                }




            for(b = centers[a].x - edge7; b <= centers[a].x + edge7; b++){
                for(c = centers[a].y - edge7; c <= centers[a].y + edge7; c++ ){

                    if (b >= 0 && b < max_row && c >= 0 && c < max_col) {
                        sum7 += V[b][c];
                    }   

                }


            }
            #pragma omp critical
            if(sum3>0){histo3->Fill(sum3);}
            if(sum5>0){histo5->Fill(sum5);}
            if(sum7>0){histo7->Fill(sum7);}
        }




    } 



    clusterfile.close();
}
}














}