#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../h_files/mmu.h"
#include "../h_files/utils.h"

int allocNewTable(MMU * mmu, RAM * memory){

    if(!mmu || !memory){
        perror("No mmu or ram allocated!");
        return 0;
    }
    int i;
    for(i = 0; i<MAX_NUM_PROCESSES; i++){
        //Al primo spazio libero alloco la tabella
        if ( mmu->tables[i].pages == NULL){
            //Alloco le pagine
            printf("Allocating space for %d pages\n", NUM_PAGES);
            mmu->tables[i].pages = malloc(sizeof(PageTableEntry)*NUM_PAGES);
            mmu->tables[i].pages_left = NUM_PAGES;   
            for(int j = 0; j<NUM_PAGES; j++){
                mmu->tables[i].pages[j].page_id = j;
                //per semplicità facciamo mappatura non randomica partendo dal frame dopo i frame per la table
                mmu->tables[i].pages[j].phy_page_id = j + 6828;
                mmu->tables[i].pages[j].flags = 0;
                //Imposto bit di validità a 1
                mmu->tables[i].pages[j].flags |= Valid;
            }

            //Facciamo gestire i primi frame dedicati alla tabella delle pagine alla tabella stessa:
            for(int j = 0; j<6828; j++){
                mmu->tables[i].pages[j].page_id = j;
                mmu->tables[i].pages[j].phy_page_id = j + 6828;
                mmu->tables[i].pages[j].flags = 0;
                //Le pagine corrispondenti ai frame della page table non sono scambiabili!
                mmu->tables[i].pages[j].flags |= Unswappable;
            }

            //La tabella delle pagine viene posta all'inizio della Memoria. Assumo che ogni tabella abbia dimensione fissa.
            //Viene mappata con costo NUM_PAGES*ENTRY_SIZE + pages_left_counter_size
            //ENTRY SIZE è idealmente 40bit e abbiamo 2^16 pagine allocabili(2^16 frame occupabili). Ogni entry occupa quindi 1 frame e ogni frame può
            //ospitare fino a 3 entry(16byte l'uno). Allo stesso tempo il contatore singolarmente occupa 4byte di un frame.
            //Mettiamo quindi la tabella in memoria fisica
            //Per semplicità: (5*2^16/16)/3 + 1 = 6828 frame per tabella circa.
            //Per ora assumo di allocare una sola tabella che inizia dai primi 109228 frame, quindi la memoria dei processi non verrà mappata in questi indirizzi
            //Settiamo questi primi byte a 1 per simulare il fatto che la tabella copre questo spazio.
            for(int k = 0; k<6828; k++){
                for(int y = 0; y < 15; y++){ //A causa della page table, vengono occupati 15 byte per frame (3 entry)
                    memory->frames[k].mem[y] = 1;
                }
            }
            printf("Done!\n");
            break;
        }
    }
    return 1;
}

//Dealloca la table singola
void deallocTable(MMU * mmu, RAM * memory, int tableIndex){
    //Liberiamo la memoria fisica precedentemente occupata dalla page table
    for(int k = 0; k<109228; k++){
        for(int _ = 0; _ < 15; _++){ 
            memory->frames[k].mem[_] = 0;
        }
    }
    free(mmu->tables[tableIndex].pages);
}
