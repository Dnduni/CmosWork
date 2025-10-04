#include <iostream>
#include <vector>
#include <cstring>
using namespace std;
#include <fstream>
#include <string.h>
#include <omp.h>
#define max_row  3008
#define max_col  3008
#define max_sensors  4
#define cutoff 6
#define cluster_edge 7


struct candidate {
    int x;
    int y;
    int val;
    int frame;
};

int find_maximum(std::vector<candidate> valarray) {
    int roller;
    int temp = 0;
    int ind = 0;
    for (roller = 0; roller < int(valarray.size()); roller++) {
        if (valarray[roller].val > temp) { 
            temp = valarray[roller].val;
            ind = roller;
        }

    
    }
    return ind;
}


bool nearby_max(candidate apixel, candidate amaximum) {

    if ((apixel.x >= (amaximum.x - cluster_edge)) && (apixel.x <= (amaximum.x + cluster_edge)) && (apixel.y >= (amaximum.y - cluster_edge)) && (apixel.y <= (amaximum.y + cluster_edge)) && (apixel.frame == amaximum.frame) && (apixel.val <= amaximum.val)) {
        return true;
    
    }
    else {
        return false;
    }

    
}





int main(){
//   example of macro to read data from an ascii file and
//   create a root file with an histogram and an ntuple.



int n_file; //variabili che mi serveno da leggere nel file conf
int i = 0, j, n;
char conf_file[700], datafile[700], filelist[700], fileio[700], fileframe[700], fileped[700], fileout[700], filenoise[700], pippo[700], aa[10];

strcpy_s(conf_file,"./conf_analisi.txt"); //copia in una stringa l'indirizzo del file configurazion.
 printf(" string %s \n",conf_file); //da provare
 
 std::ifstream inc;
 std::ifstream in;
 std::ifstream in_n;
 std::ifstream in_data;
 std::ifstream ino;
 std::ifstream inped;

 //vado ad aprire il file di configurazione e alloco in inc i valori di settaggio
 inc.open(conf_file); //mi legge il dato in quella riga  
 inc >> n_file;

  //setto il valore del singolo pixel in cordinate row col
 printf(" n_file %d \n",n_file);
 int n_row;
 int n_col;
 inc >> n_row;
 inc >> n_col;
 printf(" i %d n_row %d n_col %d\n",i,n_row,n_col); //mi stampa i parametri che ho impostato

 inc >> pippo; //prende in considerazione i file input(lista) e output (risultato) 
 strcpy_s(fileio,pippo);
 printf(" fileio %s \n",fileio);
 inc >> pippo;
 strcpy_s(fileout,pippo);
 printf(" fileout %s \n",fileout);
 inc.close();

 printf(" n file %d fileio %s \n",n_file,fileio);


 in.open(fileio);

 std::vector<std::vector<bool>> is_bad(n_row, std::vector<bool>(n_col, false));

candidate newcand;
std::vector<candidate> candidates;

int exists;

std::vector<std::vector<int>> V(n_row, std::vector<int>(n_col, 0));
int k,l,a = 0;
int sum = 0;
 inc.open(fileio);    // apre file con lista di frames 
 printf("fileio %s \n",fileio);
 bool max_found = false;
 int index;
 int bad_check;
 std::vector<std::vector<int>> badmask(n_row, std::vector<int>(n_col, 0));

 candidate bad_pixel;

 std::vector<candidate> bads;



 for (i = 0; i < n_row; i++) { //inizializza a 0 la matrice conta bad pixel
     for (j = 0; j < n_col; j++) {
        
         badmask[i][j] = 0; 
     }
 }




 std::vector<candidate> cluster;
 candidate pixel;
 candidate seed;
 std::vector<int> cluster_vals;

 int n_bad = 19;

 if (n_file < 20) { n_bad = n_file; }

#pragma omp parallel
 {  


     for (n = 0; n < n_bad; n++) { //Conta pixel accesi piu' volte con un valore > 5
         inc >> datafile;
         ino.open(datafile);
         std::cout << "Checking bad pixels: " << n << " done" << std::endl;
         for (i = 0; i < n_row; i++) {

             for (j = 0; j < n_col; j++) {

                 ino >> bad_check;
                 if (bad_check > 2) {


                     badmask[i][j] += 1;
                 }
             }

         }

         ino.close();

     }

     for (i = 0; i < n_row; i++) { //salva bad pixel in un vettore. un pixel viene scelto come bad se appare piu' di 5 volte.

         for (j = 0; j < n_col; j++) {

             if (badmask[i][j] > 5) {

                 bad_pixel.x = i;
                 bad_pixel.y = j;
                 bads.push_back(bad_pixel);
                 std::cout << "Found bad pixel at x = " << i << " y = " << j << std::endl;


             }

         }
     }

     for (auto& b : bads) {
         is_bad[b.x][b.y] = true;
     }
 }







 inc.close();
 inc.open(fileio);
 int b;
#pragma omp parallel
 {

     for (n = 0; n < n_file; n++)
     {

         inc >> datafile;
         printf("read data datafile %s\n", datafile);
         ino.open(datafile);
         newcand.x = 0;
         newcand.y = 0;

         for (i = 0; i < n_row; i++) {
             for (j = 0; j < n_col; j++) {

                 ino >> V[i][j];
                 if (is_bad[i][j]) { V[i][j] = 0; }
                 if (V[i][j] > 254) { V[i][j] = 0; }
             }
         }





         for (i = 0; i < n_row; i++)  //Comincia clustering
         {
             for (j = 0; j < n_col; j++)
             {





                 if ((V[i][j] > cutoff) && (i >= cluster_edge) && (j >= cluster_edge) && (i < (n_row - cluster_edge)) && (j < (n_col - cluster_edge))) { //escludo i bordi. Se pixel value > cutoff cerco un maxs

                     seed.x = i;
                     seed.y = j;
                     seed.val = V[i][j];
                     seed.frame = n;

                     while (true) { //Ciclo ripetuto fino a quando un max non viene trovato

                         k = 0;
                         l = 0;
                         for (k = seed.x - cluster_edge; k <= seed.x + cluster_edge; k++) {

                             for (l = seed.y - cluster_edge; l <= seed.y + cluster_edge; l++) {

                                 if (k >= 0 && k < n_row && l >= 0 && l < n_col) {

                                     pixel.x = k;
                                     pixel.y = l;
                                     pixel.val = V[k][l];
                                     pixel.frame = n;
                                     cluster.push_back(pixel); //Salva tutti pixel del cluster in un vettore
                                     cluster_vals.push_back(pixel.val); //Salva i valori di intensita' dei pixel

                                 }
                             }

                         }




                         index = find_maximum(cluster); //trova pixel massimo nel cluster
                         if (index == -1) {
                             std::cout << "Sciagura" << std::endl;
                             return 1;
                         }
                         if ((cluster[index].x != seed.x || cluster[index].y != seed.y) && (cluster[index].val > seed.val)) {

                             seed = cluster[index];
                             cluster.clear();
                             cluster_vals.clear();
                             continue;

                         }


                         else if ((cluster[index].x != seed.x || cluster[index].y != seed.y) && (cluster[index].val < seed.val)) {

                             std::cout << "Sconfitta morte e rovina" << std::endl;
                             return 1;

                         }

                         else {

                             if (int(candidates.size()) > 0) {
                                 exists = 0;
                                 for (b = 0; b < int(candidates.size()); b++) {
                                     if (nearby_max(seed, candidates[b])) {
                                         cluster.clear();
                                         cluster_vals.clear();
                                         exists = 1;
                                         break;
                                     }
                                 }
                                 if (exists == 1) {

                                     break;

                                 }
                                 else {
                                     candidates.push_back(seed);
                                     cluster.clear();
                                     cluster_vals.clear();
                                     break;
                                 }
                             }

                             else {


                                 candidates.push_back(seed);
                                 cluster.clear();
                                 cluster_vals.clear();
                                 break;


                             }
                         }
                     } //chiusura while

                 }//chiusura if

             }
         }




         ino.close();
     }
 }

  std::ofstream clusterfile;
  clusterfile.open("./clusters.txt");
  for(i = 0; i < int(candidates.size()); i++){

      clusterfile << candidates[i].x << "\t" << candidates[i].y << "\t" << candidates[i].val << "\t" << candidates[i].frame << std::endl;

  }

  clusterfile.close();
  inc.close();  



 in.close();
  ino.close(); 
  printf(" fine del ASCII file %s \n",pippo);


}// chiudi  programma
