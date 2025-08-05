#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <getopt.h>
#include <omp.h> // Enable OpenMP for parallelism

// #define MAX_ROW 3008
// #define MAX_COL 3008
//  #define SEED_CENTER_CUTOFF 4
//   #define CLUSTER_EDGE 3
//  #define BAD_THR 1     // Valore sopra il quale è bad una volta
//  #define BAD_THR_REP 4 // Se è bad piu volte di cosi allora è bad
//  #define N_FILES_FOR_BAD 10

struct SeedCandidate // TODO: In realtà poi è usato anche come oggetto per il singolo pixel, giusto?
{
    int x, y, val, frame_number;
};

int cluster_edge = 3;       // Cluster size
int seed_center_cutoff = 3; // Pixel value above which a pixel can be a seed center
int bad_thr = 1;            // To define a pixel as bad in a single frame
int bad_thr_rep = 5;        // To define a pixel as bad if above bad_thr for at least these times // TODO: Attenti che questo chiaramente è parente di n_frames_for_bad ...
int n_frames_for_bad = 10;  // Frames to anal for bad pixels
int n_row = 3008;
int n_col = 3008;
int n_frames = 1000;
std::string input_file = "";
std::string output_file = "output.txt";

int find_maximum(const std::vector<SeedCandidate> &valarray)
{
    int max_val = 0;
    int index = -1;
    for (int i = 0; i < valarray.size(); ++i)
    {
        if (valarray[i].val > max_val)
        {
            max_val = valarray[i].val;
            index = i;
        }
    }
    return index;
}

bool nearby_max(const SeedCandidate &new_seed, const SeedCandidate &previous_seed)
{
    return (new_seed.x >= previous_seed.x - cluster_edge && new_seed.x <= previous_seed.x + cluster_edge &&
            new_seed.y >= previous_seed.y - cluster_edge && new_seed.y <= previous_seed.y + cluster_edge &&
            new_seed.frame_number == previous_seed.frame_number && new_seed.val <= previous_seed.val);
}

void print_usage(const char *prog_name) // TODO: Da sistemare
{
    std::cout << "Usage: " << prog_name << " [options]\n";
    std::cout << "  -e, --edge       CLUSTER_EDGE (default: 3)\n";
    std::cout << "  -t, --threshold  THRESHOLD (default: 5)\n";
    std::cout << "  -i, --input      Input file path\n";
    std::cout << "  -o, --output     Output file path\n";
    std::cout << "  -h, --help       Show this message\n";
}

int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"cluster_edge", required_argument, 0, 'e'},
        {"seed_thr", required_argument, 0, 's'},
        {"bad_thr", required_argument, 0, 't'},
        {"bad_thr_rep", required_argument, 0, 'r'},
        {"n_frames_for_bad", required_argument, 0, 'b'},
        {"input_list", required_argument, 0, 'i'},
        {"output_file", required_argument, 0, 'o'},
        {"n_frames", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "e:s:t:r:b:i:o:h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'e':
            cluster_edge = std::atoi(optarg);
            break;
        case 's':
            seed_center_cutoff = std::atoi(optarg);
            break;
        case 't':
            bad_thr = std::atoi(optarg);
            break;
        case 'r':
            bad_thr_rep = std::atoi(optarg);
            break;
        case 'b':
            n_frames_for_bad = std::atoi(optarg);
            break;
        case 'n':
            n_frames = std::atoi(optarg);
            break;
        case 'i':
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
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

    // std::ifstream config_file("./conf_analisi.txt");
    // if (!config_file)
    // {
    //     std::cerr << "Failed to open config file.\n";
    //     return 1;
    // }

    // int n_file, n_row, n_col;
    // std::string fileio, fileout;
    // config_file >> n_file >> n_row >> n_col >> fileio >> fileout;
    // config_file.close();

    std::vector<std::string> datafiles(n_frames);
    std::ifstream file_list(input_file);
    for (int i = 0; i < n_frames; ++i)
        file_list >> datafiles[i];
    file_list.close();
    // Now inside datafiles there are n_frames as string

    std::vector<std::vector<int>> badmask(n_row, std::vector<int>(n_col, 0));
    std::vector<std::vector<bool>> is_bad(n_row, std::vector<bool>(n_col, false));

    // ################ LOOK FOR BAD PIXELS
    // ########

    // Determine bad pixels: loop on first n_frames_for_bad frames and look for pixels counting >= bad_thr, incrementing in this case the corresponding value in badmask
    for (int this_frame_number = 0; this_frame_number <= n_frames_for_bad && this_frame_number < n_frames; ++this_frame_number) // TODO: Pescarne N a caso, non i primi
    {
        std::ifstream this_frame(datafiles[this_frame_number]); // Read the frame
        for (int this_row = 0; this_row < n_row; ++this_row)
            for (int this_col = 0; this_col < n_col; ++this_col)
            {
                int value;
                this_frame >> value; // If this pixel value is above bad_thr, increment its mask value
                if (value >= bad_thr)
                {
                    badmask[this_row][this_col]++;
                }
            }
        this_frame.close();
    }

    // At this stage, we populated badmask, i.e. a frame-like object in which each pixel has as value the number of times it exceeded bad_thr

    // Now loop on this badmask: if a pixel was above bad_thr at least bad_thr_rep times, call it bad, and write so in is_bad
    for (int this_row = 0; this_row < n_row; ++this_row)
    {
        for (int this_col = 0; this_col < n_col; ++this_col)
        {
            if (badmask[this_row][this_col] >= bad_thr_rep)
            {
                is_bad[this_row][this_col] = true;
            }
        }
    }

    // At this stage, we populated is_bad, i.e. a frame-like object in which each pixel which exceeded bad_thr at least bad_thr_rep is marked as true and thus bad pixel

    // ########
    // ################

    // ################ LOOK FOR SEEDS
    // ########

    std::vector<SeedCandidate> seed_candidates; // Create the list of seed_candidates to be found
#pragma omp parallel
    {
        std::vector<SeedCandidate> local_candidates;  // Local in the sense of "of this thread"
        std::vector<SeedCandidate> this_seed_cluster; // TODO: Questo nome secondo me fa un po' confusione, dovrebbe essere tipo `this_seed_cluster_pixels`?
        std::vector<int> cluster_vals;
#pragma omp for
        for (int this_frame_number = 0; this_frame_number < n_frames; ++this_frame_number)
        {

            // Read all "good pixels" values
            std::cout << "Processing frame: " << this_frame_number << " / " << n_frames << std::endl;
            std::vector<std::vector<int>> this_frame_values(n_row, std::vector<int>(n_col)); // Matrix container to host all values of this frame
            std::ifstream this_frame(datafiles[this_frame_number]);
            for (int this_row = 0; this_row < n_row; ++this_row)
            {
                for (int this_col = 0; this_col < n_col; ++this_col)
                {
                    this_frame >> this_frame_values[this_row][this_col]; // Store in this_frame_values all values of this frame
                    if (is_bad[this_row][this_col] || this_frame_values[this_row][this_col] > 254)
                    {
                        this_frame_values[this_row][this_col] = 0; // If this pixel was marked as bad, or if it has a no sense value, put 0 //TODO: Sicuri non crei problemi/bias mettere a zero i bad pixel? da ragionare
                    }
                }
            }
            this_frame.close();

            // At this stage, for this frame we stored in this_frame_values all pixels that are not bad of no-sense

            // Now proceed to look for seeds, with a new loop on rows and cols (except edge)

            for (int this_row = cluster_edge; this_row < n_row - cluster_edge; ++this_row)
            {
                for (int this_col = cluster_edge; this_col < n_col - cluster_edge; ++this_col)
                {
                    if (this_frame_values[this_row][this_col] >= seed_center_cutoff) // When i find a pixel which value is at least seed_center_cutoff
                    {
                        SeedCandidate this_seed_candidate{this_row, this_col, this_frame_values[this_row][this_col], this_frame_number}; // Create a SeedCandidate object
                        while (true)
                        {
                            this_seed_cluster.resize(0); // Empty the list of clusters of this seed

                            // Loop on all pixels around the seed candidate, adding to this_seed_cluster all pixels values
                            for (int pixel_x = this_seed_candidate.x - cluster_edge; pixel_x <= this_seed_candidate.x + cluster_edge; ++pixel_x)
                                for (int pixel_y = this_seed_candidate.y - cluster_edge; pixel_y <= this_seed_candidate.y + cluster_edge; ++pixel_y)
                                    if (pixel_x > 0 && pixel_x < n_row && pixel_y > 0 && pixel_y < n_col) // Check di sicurezza
                                    {
#pragma omp critical
                                        this_seed_cluster.push_back({pixel_x, pixel_y, this_frame_values[pixel_x][pixel_y], this_frame_number});
                                    }
                            // At this stage, for this_seed_candidate, we addedd to this_seed_cluster all its sorrounding pixels values

                            // Find the highes pixel value in the cluster
                            int max_pixel_index = find_maximum(this_seed_cluster);
                            if (max_pixel_index == -1)
                                break; // Something went wrong, proceed to look for next SeedCandidate

                            const SeedCandidate &max_pixel = this_seed_cluster[max_pixel_index]; // Store (w/o copying) as SeedCandidate the highest pixel
                            if ((max_pixel.x != this_seed_candidate.x || max_pixel.y != this_seed_candidate.y) && max_pixel.val > this_seed_candidate.val)
                            {
                                this_seed_candidate = max_pixel; // If the highest pixel is not the original SeedCandidate, replace the seed candidate with the highest one
                                continue;
                            }
                            else if ((max_pixel.x != this_seed_candidate.x || max_pixel.y != this_seed_candidate.y) && max_pixel.val < this_seed_candidate.val)
                            {
                                break; // Inconsistent max
                            }
                            else
                            { // Enter here only if the original this_seed_candidate was already the highest pixel
                                bool found = false;
                                for (const auto &other_seed_candidate : local_candidates)      // Loop on all other possible SeedCandidates we already saw
                                    if (nearby_max(this_seed_candidate, other_seed_candidate)) // Se questo altro candidato seed è nello stesso frame, con il centro dentro il cluster del vecchio e valore minore... cioè se è compreso in quello vecchio, allora segno che l'avevo già trovato
                                    {
                                        found = true;
                                        break;
                                    }
                                if (!found) // If we did not already found this seed ...
#pragma omp critical
                                    local_candidates.push_back(this_seed_candidate); // ... add it to the list of local_candidates
                                break;
                            }
                        }
                    }
                }
            }
        } // Close the loop on all frames
#pragma omp critical
        seed_candidates.insert(seed_candidates.end(), local_candidates.begin(), local_candidates.end()); // Add at the end of seeed_candidates these local_candidates (i.e. candidates of this thread)
    }

    std::ofstream clusterfile(output_file); // Write on output file the list of seed candidates
    for (const auto &this_seed : seed_candidates)
        clusterfile << this_seed.x << "\t" << this_seed.y << "\t" << this_seed.val << "\t" << this_seed.frame_number << "\n";
    clusterfile.close();

    std::cout << "Clustering complete. Output written to " << output_file << "\n ";

    size_t dot_pos = output_file.find_last_of('.');
    std::string bad_pixel_filename;

    if (dot_pos != std::string::npos)
    {
        bad_pixel_filename = output_file.substr(0, dot_pos) + "_bad" + output_file.substr(dot_pos);
    }
    else
    {
        bad_pixel_filename = output_file + "_bad";
    }

    std::ofstream badPixelFile(bad_pixel_filename); // Write on bad_output file the list of bad pixels
    int bad_pixel_counter = 0;
    for (int this_row = 0; this_row < n_row; ++this_row)
    {
        for (int this_col = 0; this_col < n_col; ++this_col)
        {
            if (is_bad[this_row][this_col])
            {
                badPixelFile << bad_pixel_counter << "\t" << this_row << "\t" << this_col << "\t" << badmask[this_row][this_col] << "\n";
                bad_pixel_counter++;
            }
        }
    }
    badPixelFile.close();

    return 0;
}
