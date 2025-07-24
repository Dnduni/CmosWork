#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <omp.h> // Enable OpenMP for parallelism

#define MAX_ROW 3008
#define MAX_COL 3008
#define CUTOFF 0
#define CLUSTER_EDGE 3

struct Candidate {
    int x, y, val, frame;
};

int find_maximum(const std::vector<Candidate>& valarray) {
    int max_val = 0;
    int index = -1;
    for (int i = 0; i < valarray.size(); ++i) {
        if (valarray[i].val > max_val) {
            max_val = valarray[i].val;
            index = i;
        }
    }
    return index;
}

bool nearby_max(const Candidate& a, const Candidate& b) {
    return (a.x >= b.x - CLUSTER_EDGE && a.x <= b.x + CLUSTER_EDGE &&
        a.y >= b.y - CLUSTER_EDGE && a.y <= b.y + CLUSTER_EDGE &&
        a.frame == b.frame && a.val <= b.val);
}

int main() {
    std::ifstream config_file("./conf_analisi.txt");
    if (!config_file) {
        std::cerr << "Failed to open config file.\n";
        return 1;
    }

    int n_file, n_row, n_col;
    std::string fileio, fileout;
    config_file >> n_file >> n_row >> n_col >> fileio >> fileout;
    config_file.close();

    std::vector<std::string> datafiles(n_file);
    std::ifstream file_list(fileio);
    for (int i = 0; i < n_file; ++i) file_list >> datafiles[i];
    file_list.close();

    std::vector<std::vector<int>> badmask(n_row, std::vector<int>(n_col, 0));
    std::vector<std::vector<bool>> is_bad(n_row, std::vector<bool>(n_col, false));

    // Determine bad pixels
    for (int f = 0; f < 11 && f < n_file; ++f) {
        std::ifstream data(datafiles[f]);
        for (int i = 0; i < n_row; ++i)
            for (int j = 0; j < n_col; ++j) {
                int value;
                data >> value;
                if (value > 2) badmask[i][j]++;
            }
        data.close();
    }

    for (int i = 0; i < n_row; ++i)
        for (int j = 0; j < n_col; ++j)
            if (badmask[i][j] > 5)
                is_bad[i][j] = true;

    std::vector<Candidate> candidates;
#pragma omp parallel
    {
        std::vector<Candidate> local_candidates;
        std::vector<Candidate> cluster;
        std::vector<int> cluster_vals;
#pragma omp for 
        for (int f = 0; f < n_file; ++f) {
            std::cout << "Processing frame: " << f << std::endl;
            std::vector<std::vector<int>> V(n_row, std::vector<int>(n_col));
            std::ifstream data(datafiles[f]);
            for (int i = 0; i < n_row; ++i)
                for (int j = 0; j < n_col; ++j) {
                    data >> V[i][j];
                    if (is_bad[i][j] || V[i][j] > 254) V[i][j] = 0;
                }
            data.close();

            for (int i = CLUSTER_EDGE; i < n_row - CLUSTER_EDGE; ++i) {
                for (int j = CLUSTER_EDGE; j < n_col - CLUSTER_EDGE; ++j) {
                    if (V[i][j] > CUTOFF) {
                        Candidate seed{ i, j, V[i][j], f };
                        while (true) {
                            cluster.resize(0);
                            for (int k = seed.x - CLUSTER_EDGE; k <= seed.x + CLUSTER_EDGE; ++k)
                                for (int l = seed.y - CLUSTER_EDGE; l <= seed.y + CLUSTER_EDGE; ++l)
                                    if(k>0 && k < n_row && l > 0 && l < n_col ){
                                        #pragma omp critical
                                        cluster.push_back({ k, l, V[k][l], f });
                                    }
                            int index = find_maximum(cluster);
                            if (index == -1) break;

                            const Candidate& max_pixel = cluster[index];
                            if ((max_pixel.x != seed.x || max_pixel.y != seed.y) && max_pixel.val > seed.val) {
                                seed = max_pixel;
                                continue;
                            }
                            else if ((max_pixel.x != seed.x || max_pixel.y != seed.y) && max_pixel.val < seed.val) {
                                break; // Inconsistent max
                            }
                            else {
                                bool found = false;
                                for (const auto& c : local_candidates)
                                    if (nearby_max(seed, c)) {
                                        found = true;
                                        break;
                                    }
                                if (!found) 
                                #pragma omp critical
                                local_candidates.push_back(seed);
                                break;
                            }
                        }
                    }
                }
            }
        }
#pragma omp critical
        candidates.insert(candidates.end(), local_candidates.begin(), local_candidates.end());
    }

    std::ofstream clusterfile("clusters.txt");
    for (const auto& c : candidates)
        clusterfile << c.x << "\t" << c.y << "\t" << c.val << "\t" << c.frame << "\n";
    clusterfile.close();

    std::cout << "Clustering complete. Output written to clusters.txt\n";
    return 0;
}
