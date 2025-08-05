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

struct SeedCandidate
{
    int x, y, val, frame_number;
};

int cluster_edge = 3;
int seed_center_cutoff = 3;
int bad_thr = 1;
int bad_thr_rep = 5;
int n_files_for_bad = 10;
int n_row = 3008;
int n_col = 3008;
int n_files = 1000;
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
        {"n_files_for_bad", required_argument, 0, 'b'},
        {"input_list", required_argument, 0, 'i'},
        {"output_file", required_argument, 0, 'o'},
        {"n_files", required_argument, 0, 'n'},
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
            n_files_for_bad = std::atoi(optarg);
            break;
        case 'n':
            n_files = std::atoi(optarg);
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
              << "  N_FILES_FOR_BAD     = " << n_files_for_bad << "\n"
              << "  N_FILES     = " << n_files << "\n"
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

    std::vector<std::string> datafiles(n_files);
    std::ifstream file_list(input_file);
    for (int i = 0; i < n_files; ++i)
        file_list >> datafiles[i];
    file_list.close();

    std::vector<std::vector<int>> badmask(n_row, std::vector<int>(n_col, 0));
    std::vector<std::vector<bool>> is_bad(n_row, std::vector<bool>(n_col, false));

    // Determine bad pixels: loop on first N_FILES_FOR_BAD files and look for pixels counting >= BAD_THR, incrementing in this case the corresponding value in badmask
    for (int this_file = 0; this_file <= n_files_for_bad && this_file < n_files; ++this_file)
    {
        std::ifstream data(datafiles[this_file]);
        for (int this_row = 0; this_row < n_row; ++this_row)
            for (int this_col = 0; this_col < n_col; ++this_col)
            {
                int value;
                data >> value;
                if (value >= bad_thr)
                {
                    badmask[this_row][this_col]++;
                }
            }
        data.close();
    }

    // Now loop on this badmask: if a pixel was above BAD_THR at least BAD_THR_REP times, call it bad, and write so in is_bad
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

    std::vector<SeedCandidate> seed_candidates;
#pragma omp parallel
    {
        std::vector<SeedCandidate> local_candidates;
        std::vector<SeedCandidate> this_seed_cluster;
        std::vector<int> cluster_vals;
#pragma omp for
        for (int this_file_number = 0; this_file_number < n_files; ++this_file_number)
        {
            std::cout << "Processing frame: " << this_file_number << std::endl;
            std::vector<std::vector<int>> this_file_values(n_row, std::vector<int>(n_col));
            std::ifstream data(datafiles[this_file_number]);
            for (int this_row = 0; this_row < n_row; ++this_row)
                for (int this_col = 0; this_col < n_col; ++this_col)
                {
                    data >> this_file_values[this_row][this_col]; // Appoggio in this_file_values tutti i valori di questo frame
                    if (is_bad[this_row][this_col] || this_file_values[this_row][this_col] > 254)
                    {
                        this_file_values[this_row][this_col] = 0; // Se questo pixel era segnato come Bad, o se ha un numero insensato, ci metto 0
                    }
                }
            data.close();

            // Arrivato qui, per questo frame/file, ho salvato in this_file_values tutti i pixel che non sono Bad o insensati
            // Ora cerco i seed, scorrendo di nuovo tutte le righe e le colonne (edge escluso)

            for (int this_row = cluster_edge; this_row < n_row - cluster_edge; ++this_row)
            {
                for (int this_col = cluster_edge; this_col < n_col - cluster_edge; ++this_col)
                {
                    if (this_file_values[this_row][this_col] >= seed_center_cutoff) // Quando trovo un pixel sopra SEED_CENTER_CUTOFF
                    {
                        SeedCandidate this_seed{this_row, this_col, this_file_values[this_row][this_col], this_file_number}; // Creo un oggetto SeedCandidate
                        while (true)
                        {
                            this_seed_cluster.resize(0); // Svuota la lista di clusters di questo seed
                            // Scorro tutti i pixel attorno al possibile seed, calcolando il valore totale del cluster
                            for (int pixel_x = this_seed.x - cluster_edge; pixel_x <= this_seed.x + cluster_edge; ++pixel_x)
                                for (int pixel_y = this_seed.y - cluster_edge; pixel_y <= this_seed.y + cluster_edge; ++pixel_y)
                                    if (pixel_x > 0 && pixel_x < n_row && pixel_y > 0 && pixel_y < n_col) // Check di sicurezza
                                    {
#pragma omp critical
                                        this_seed_cluster.push_back({pixel_x, pixel_y, this_file_values[pixel_x][pixel_y], this_file_number});
                                        // Aggiunge al cluster di questo frame un nuovo oggetto SeedCandidate (qui usato per il singolo pixel)
                                    }

                            // Trovo all'interno del cluster il pixel più alto
                            int max_pixel_index = find_maximum(this_seed_cluster);
                            if (max_pixel_index == -1)
                                break;

                            const SeedCandidate &max_pixel = this_seed_cluster[max_pixel_index]; // Salvo (senza copiare) come SeedCandidate il pixel di maggiore valore
                            if ((max_pixel.x != this_seed.x || max_pixel.y != this_seed.y) && max_pixel.val > this_seed.val)
                            {
                                this_seed = max_pixel; // Se non era quello da cui ero partito il pixel più alto, lo rimpiazzo con quello più alto
                                continue;
                            }
                            else if ((max_pixel.x != this_seed.x || max_pixel.y != this_seed.y) && max_pixel.val < this_seed.val)
                            {
                                break; // Inconsistent max
                            }
                            else
                            { // Qui ci entro solo se avevo subito beccato il pixel più alto del cluster
                                bool found = false;
                                for (const auto &other_seed_candidate : local_candidates) // Scorro tutti i candidati seed già visti
                                    if (nearby_max(this_seed, other_seed_candidate))      // Se questo altro candidato seed è nello stesso frame, con il centro dentro il cluster del vecchio e valore minore... cioè se è compreso in quello vecchio, allora segno che l'avevo già trovato
                                    {
                                        found = true;
                                        break;
                                    }
                                if (!found) // Se non lo avevo già trovato
#pragma omp critical
                                    local_candidates.push_back(this_seed); // ... lo aggiungo alla lista dei candidati seed
                                break;
                            }
                        }
                    }
                }
            }
        } // Chiude il ciclo su tutti i files/frames
#pragma omp critical
        seed_candidates.insert(seed_candidates.end(), local_candidates.begin(), local_candidates.end()); // aggiungo in fondo a seed_candidates i candidati locali (cioè di questo thread)
    }

    std::ofstream clusterfile(output_file); // Scrivo su file di output txt la lista dei seed
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

    std::ofstream badPixelFile(bad_pixel_filename); // Scrivo su file di output txt la lista dei bad pixel
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
