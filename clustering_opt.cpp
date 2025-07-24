#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <omp.h> // Enable OpenMP for parallelism

#define MAX_ROW 3008
#define MAX_COL 3008
#define SEED_CENTER_CUTOFF 4
#define CLUSTER_EDGE 3
#define BAD_THR 1     // Valore sopra il quale è bad una volta
#define BAD_THR_REP 4 // Se è bad piu volte di cosi allora è bad
#define N_FILES_FOR_BAD 10

struct SeedCandidate
{
    int x, y, val, frame;
};

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

bool nearby_max(const SeedCandidate &a, const SeedCandidate &b)
{
    return (a.x >= b.x - CLUSTER_EDGE && a.x <= b.x + CLUSTER_EDGE &&
            a.y >= b.y - CLUSTER_EDGE && a.y <= b.y + CLUSTER_EDGE &&
            a.frame == b.frame && a.val <= b.val);
}

int main()
{
    std::ifstream config_file("./conf_analisi.txt");
    if (!config_file)
    {
        std::cerr << "Failed to open config file.\n";
        return 1;
    }

    int n_file, n_row, n_col;
    std::string fileio, fileout;
    config_file >> n_file >> n_row >> n_col >> fileio >> fileout;
    config_file.close();

    std::vector<std::string> datafiles(n_file);
    std::ifstream file_list(fileio);
    for (int i = 0; i < n_file; ++i)
        file_list >> datafiles[i];
    file_list.close();

    std::vector<std::vector<int>> badmask(n_row, std::vector<int>(n_col, 0));
    std::vector<std::vector<bool>> is_bad(n_row, std::vector<bool>(n_col, false));

    // Determine bad pixels: loop on first N_FILES_FOR_BAD files and look for pixels counting >= BAD_THR, incrementing in this case the corresponding value in badmask
    for (int this_file = 0; this_file <= N_FILES_FOR_BAD && this_file < n_file; ++this_file)
    {
        std::ifstream data(datafiles[this_file]);
        for (int this_row = 0; this_row < n_row; ++this_row)
            for (int this_col = 0; this_col < n_col; ++this_col)
            {
                int value;
                data >> value;
                if (value >= BAD_THR)
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
            if (badmask[this_row][this_col] >= BAD_THR_REP)
            {
                is_bad[this_row][this_col] = true;
            }
        }
    }

    std::vector<SeedCandidate> seed_candidates;
#pragma omp parallel
    {
        std::vector<SeedCandidate> local_candidates;
        std::vector<SeedCandidate> clusters;
        std::vector<int> cluster_vals;
#pragma omp for
        for (int this_file_number = 0; this_file_number < n_file; ++this_file_number)
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
            //  Ora cerco i cluster, scorrendo di nuovo tutte le righe e le colonne (edge escluso)

            for (int this_row = CLUSTER_EDGE; this_row < n_row - CLUSTER_EDGE; ++this_row)
            {
                for (int this_col = CLUSTER_EDGE; this_col < n_col - CLUSTER_EDGE; ++this_col)
                {
                    if (this_file_values[this_row][this_col] >= SEED_CENTER_CUTOFF) // Quando trovo un pixel sopra SEED_CENTER_CUTOFF
                    {
                        SeedCandidate this_seed{this_row, this_col, this_file_values[this_row][this_col], this_file_number}; // Creo un oggetto SeedCandidate
                        while (true)
                        {
                            clusters.resize(0); // Svuota la lista di clusters
                            for (int pixel_x = this_seed.x - CLUSTER_EDGE; pixel_x <= this_seed.x + CLUSTER_EDGE; ++pixel_x)
                                for (int pixel_y = this_seed.y - CLUSTER_EDGE; pixel_y <= this_seed.y + CLUSTER_EDGE; ++pixel_y)
                                    if (pixel_x > 0 && pixel_x < n_row && pixel_y > 0 && pixel_y < n_col) // Check di sicurezza
                                    {
#pragma omp critical
                                        clusters.push_back({pixel_x, pixel_y, this_file_values[pixel_x][pixel_y], this_file_number});
                                        // Aggiunge alla lista dei clusters di questo frame un nuovo SeedCandidate
                                    }
                            int index = find_maximum(clusters);
                            if (index == -1)
                                break;

                            const SeedCandidate &max_pixel = clusters[index];
                            if ((max_pixel.x != this_seed.x || max_pixel.y != this_seed.y) && max_pixel.val > this_seed.val)
                            {
                                this_seed = max_pixel;
                                continue;
                            }
                            else if ((max_pixel.x != this_seed.x || max_pixel.y != this_seed.y) && max_pixel.val < this_seed.val)
                            {
                                break; // Inconsistent max
                            }
                            else
                            {
                                bool found = false;
                                for (const auto &c : local_candidates)
                                    if (nearby_max(this_seed, c))
                                    {
                                        found = true;
                                        break;
                                    }
                                if (!found)
#pragma omp critical
                                    local_candidates.push_back(this_seed);
                                break;
                            }
                        }
                    }
                }
            }
        }
#pragma omp critical
        seed_candidates.insert(seed_candidates.end(), local_candidates.begin(), local_candidates.end());
    }

    std::ofstream clusterfile("clusters.txt");
    for (const auto &c : seed_candidates)
        clusterfile << c.x << "\t" << c.y << "\t" << c.val << "\t" << c.frame << "\n";
    clusterfile.close();

    std::cout << "Clustering complete. Output written to clusters.txt\n";
    return 0;
}
