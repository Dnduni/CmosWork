#include <iostream>
#include <vector>
#include <cstring>
using namespace std;
#include <fstream>
#include <string.h>
#define max_row  3008
#define max_col  3008
#define max_sensors  4
#define cutoff 15

int main(){
//   example of macro to read data from an ascii file and
//   create a root file with an histogram and an ntuple.



int n_file; //variabili che mi serveno da leggere nel file conf
int i = 0, j, n;
char conf_file[700], datafile[700], filelist[700], fileio[700], fileframe[700], fileped[700], fileout[700], filenoise[700], pippo[700], aa[10];
int temp;
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

 auto V = new int[max_row][max_col]; //setto il valore del singolo pixel in cordinate row col
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

 FILE *infile;
 FILE *outfile;
 in.open(fileio);
 outfile = fopen(fileout,"a");

struct candidate {
    int x;
    int y; 
    int val;
    int frame;
};
candidate newcand;
std::vector<candidate> candidates;

int cluster_edge = 3;
int max;
int k,l,a = 0;
int sum = 0;
 inc.open(fileio);    // apre file con lista di frames 
 printf("fileio %s \n",fileio);
 bool max_found = false;
 int index;

 std::vector<candidate> cluster;
 candidate pixel;
 candidate seed;

 for(n=0; n<n_file; n++) //lo cilca per numero di frame, cambia il seconco in n_file
      { 

        inc >> datafile;
        printf("read data datafile %s\n",datafile);
        ino.open(datafile);
        newcand.x = 0;
        newcand.y = 0;


        for(i=0; i<n_row; i++) 
        {    
          for(j=0; j<n_col; j++)
          {
            ino >> V[i][j];
            
            if(V[i][j] > 254){V[i][j] = 0;} //esclude saturazione
            sum = 0;
            if(V[i][j]> cutoff  && i >= cluster_edge && j >= cluster_edge && i < (3007 - cluster_edge) && j < (3007 -cluster_edge)){

                seed.x = i;
                seed.y = j;
                seed.val = V[i][j];
                seed.frame = n;
                max_found = false;
                while (max_found == false) {

                    for (k = seed.x - cluster_edge; k <= seed.x + cluster_edge; k++) {
                    
                        for (l = seed.y - cluster_edge; l <= seed.y + cluster_edge; l++) {
                            pixel.x = k;
                            pixel.y = l;
                            pixel.val = V[k][l];
                            cluster.push_back(pixel);
                        
                        }
                    
                    }
                    temp = 0;
                    for (a = 0; a < cluster.size(); a++) {
                        if (cluster[a].val > temp) {
                            temp = cluster[a].val;
                            index = a;
                        }
                    
                    }
                    if (cluster[index].x != seed.x && cluster[index].y != seed.y) {

                        seed.x = cluster[index].x;
                        seed.y = cluster[index].y;
                        seed.val = cluster[index].val;

                    }
                    else { 
                        
                        if (candidates.size() > 0) {
                        
                            if (seed.x != candidates[candidates.size() - 1].x && seed.y != candidates[candidates.size() - 1].y) {
                            
                                candidates.push_back(seed);
                            
                            }
                                

                        }

                        else { candidates.push_back(seed); }
                        
                        cluster.clear();
                        max_found = true; }
                
                
                
                
                
                
                
                
                
                
                }



                if (candidates.size() > 0) {
                    if (sum > 30 && newcand.x != candidates[int(candidates.size()) - 1].x && newcand.y != candidates[int(candidates.size()) - 1].y) { candidates.push_back(newcand); }
                }
                else {
                
                    if (sum > 30) { candidates.push_back(newcand); }

                }
            }
        }
     }
     

    ino.close(); 
  }
  
  for(i = 0; i < int(candidates.size()); i++){

    std::cout << "Found candidate at: " << candidates[i].x << " " << candidates[i].y << " " << " frame: " << candidates[i].frame << " value: " << candidates[i].val << std::endl;  

  }


  inc.close();  



 in.close();
 fclose(outfile); // chiudi file di output
  ino.close(); 
  printf(" fine del ASCII file %s \n",pippo);


}// chiudi  programma
