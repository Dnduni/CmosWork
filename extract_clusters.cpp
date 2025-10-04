#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <getopt.h>
#include <omp.h> // Enable OpenMP for parallelism

struct SeedCandidate // TODO: In realtà poi è usato anche come oggetto per il singolo pixel, giusto?
{
    int x, y, val, frame_number;
};


int cluster_edge = 3;       // Cluster (half) size
int seed_center_cutoff = 0; // Pixel value above which a pixel can be a seed center
int bad_thr = 1;            // To define a pixel as bad in a single frame
int bad_thr_rep = 5;        // To define a pixel as bad if above bad_thr for at least these times // TODO: Attenti che questo chiaramente è parente di n_frames_for_bad ...
int n_frames_for_bad = 10;  // Frames to anal for bad pixels
int n_row = 3008;
int n_col = 3008;
int n_frames = 1000;
std::string input_file = "filelistSr90.txt";
std::string output_file = "output.txt";
std::string cluster_file = "clusters_debug.txt";

void print_usage(const char *prog_name) // TODO: Da sistemare
{
    std::cout << "Usage: " << prog_name << " [options]\n";
    std::cout << "  -n, --n_frames        Number of frames\n";
    std::cout << "  -i, --input_list      Data file path\n";
    std::cout << "  -h, --help       Show this message\n";
    std::cout << "  -c, --cluster_file      Cluster file path\n";
}


int main(int argc, char *argv[])
{
    static struct option long_options[] = {

        {"input_list", required_argument, 0, 'i'},
        {"n_frames", required_argument, 0, 'n'},
        {"cluster_file", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};





    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "i:o:n:c:h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'c':
            cluster_file = optarg;
            break;
        case 'n':
            n_frames = std::atoi(optarg);
            break;
        case 'i':
            input_file = optarg;
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    std::cout << "Parametri:\n"
              << "  CLUSTER_EDGE        = " << cluster_edge << "\n"
              << "  SEED_CENTER_CUTOFF  = " << seed_center_cutoff << "\n"
              << "  BAD_THR             = " << bad_thr << "\n"
              << "  BAD_THR_REP         = " << bad_thr_rep << "\n"
              << "  n_frames_for_bad     = " << n_frames_for_bad << "\n"
              << "  n_frames     = " << n_frames << "\n"
              << "  input_file          = \"" << input_file << "\"\n"
              << "  output_file         = \"" << output_file << "\"\n";

    std::vector<std::string> datafiles(n_frames); //Load data files
    std::ifstream file_list(input_file);
    for (int i = 0; i < n_frames; ++i)
        file_list >> datafiles[i];
    file_list.close();

    std::vector<SeedCandidate> cluster_list; //Load cluster file
    std::ifstream seed_list(cluster_file);
    int a,b,c,d = 0;
    SeedCandidate temp_seed;
    
    while(seed_list >> a >> b >> c >> d){ //Fill Seed Vector
        temp_seed.x = a;
        temp_seed.y = b;
        temp_seed.val = c;
        temp_seed.frame_number = d;
        cluster_list.push_back(temp_seed);
    }

    int i,j = 0;
    
    typedef std::vector<std::vector<int>> Matrix; //Definisco oggetto matrix per prendere il singolo cluster 7x7
    Matrix cluster_img(7, std::vector<int>(7));
    std::vector<Matrix> cluster_images; //Array delle matrici dei cluster


    #pragma omp parallel
    { //start parallelization
        std::vector<std::vector<int>> frame(n_row, std::vector<int>(n_col)); //Frame definition 
        #pragma omp for
        for(i = 0; i < n_frames ; i++){

            std::cout << "Working on frame: " << i << "/" << n_frames << std::endl; //Progress bar
            
            for(j = 0; j < cluster_list.size(); j++){
                int row = 0,col = 0;
                if(cluster_list[j].frame_number == i){ //When seed candidate frame matches current frame
                    
                    for(a = cluster_list[j].x - 3; a <= cluster_list[j].x + 3; a++){ //Draw a 7x7 around the seed coordinates and store it
                        for(b = cluster_list[j].y - 3;  b <= cluster_list[j].y +3; b++ ){

                            cluster_img[row][col] = cluster_list[j].val;
                            col++;
                    }
                    row++;
                    }
                #pragma omp critical
                cluster_images.push_back(cluster_img); //Store extrapolated 2D array

                }
            }
        }
    }
        std::ofstream outstream; //Open output stream

        for(i = 0; i < cluster_images.size(); i++){

            std::stringstream ss;
            ss << "./output/cluster" << i << ".txt"; //Name cluster file
            std::string address = ss.str();
            outstream.open(address, std::fstream::out);
            int row = 0,col = 0;
            for(row = 0; row < 7; row++){
                for(col = 0; col < 7; col++){
                outstream << cluster_images[i][row][col];
                outstream << " ";

                if(row == 6){
                    outstream << std::endl;
                }
                }
            }
        }
}