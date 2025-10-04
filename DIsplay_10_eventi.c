{
//   example of macro to read data from an ascii file and
//   create a root file with an histogram and an ntuple.
gROOT->Reset();
gStyle->SetOptStat(1111111);
gStyle->SetOptFit(111);
#include "TInterpreter.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TNtuple.h"
#include "TPaveLabel.h"
#include "TPaveText.h"
#include "TFrame.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "Riostream.h"
#include <vector>
#define max_row  3008
#define max_col  3008
#define max_sensors  4

int n_file; //variabili che mi serveno da leggere nel file conf
int i, j, k, l, n, itemp;
char conf_file[700], datafile[700], filelist[700], fileio[700], fileframe[700], fileped[700], fileout[700], filenoise[700], pippo[700], aa[10];

strcpy(conf_file,"./conf_analisi.txt"); //copia in una stringa l'indirizzo del file configurazion.
 printf(" string %s \n",conf_file); //da provare
 
 ifstream inc;
 ifstream in;
 ifstream in_n;
 ifstream in_data;
 ifstream ino;
 ifstream inped;

 //vado ad aprire il file di configurazione e alloco in inc i valori di settaggio
 inc.open(conf_file); //mi legge il dato in quella riga  
 inc >> n_file;

 int V[max_row][max_col]; //setto il valore del singolo pixel in cordinate row col
 printf(" n_file %d \n",n_file);
 int n_row;
 int n_col;
 inc >> n_row;
 inc >> n_col;
 printf(" i %d n_row %d n_col %d\n",i,n_row,n_col); //mi stampa i parametri che ho impostato

 inc >> pippo; //prende in considerazione i file input(lista) e output (risultato) 
 strcpy(fileio,pippo);
 printf(" fileio %s \n",fileio);
 inc >> pippo;
 strcpy(fileout,pippo);
 printf(" fileout %s \n",fileout);
 inc.close();

 printf(" n file %d fileio %s \n",n_file,fileio);

 FILE *infile;
 FILE *outfile;
 in.open(fileio);
 outfile = fopen(fileout,"a");

short int temp = 0;

std::vector<TH1F*> V_single;
std::vector<TH2F*> V_map;


 inc.open(fileio);    // apre file con lista di frames 
 printf("fileio %s \n",fileio);

//matrice dei singoli pixel

for(n= 0; n< n_file; n++){
  TH1F * h = new TH1F(" Single pixel value","",256,0.,256.); 
  TH2F * k = new TH2F("Heatmap", "",3008,0.,3008.,3008 ,0.,3008.);
  V_single.push_back(h);
  V_map.push_back(k);

}



 for(n=0; n<n_file; n++) //lo cilca per numero di frame, cambia il seconco in n_file
      { 

        inc >> datafile;
        printf("read data datafile %s\n",datafile);
        ino.open(datafile);

        for(i=0; i<n_row; i++) 
        {    
          for(j=0; j<n_col; j++)
          {
            ino >> temp;
            V[i][j] = temp;
            if(V[i][j] > 0){
              V_single[n]->Fill(V[i][j]);
            }
            V_map[n]->Fill(i,j,V[i][j]);
        }
     }
     

    ino.close(); 


  }
  
  

  inc.close();  



 in.close();
 fclose(outfile); // chiudi file di output
  ino.close(); 
  printf(" fine del ASCII file %s \n",pippo);


}// chiudi  programma
