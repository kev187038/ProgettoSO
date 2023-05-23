#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../h_files/mmu.h"
#include "../h_files/utils.h"

//FUNZIONI DI GESTIONE LISTA COLLEGATA




//Prints in order all list nodes with the Valid bit set to 1
void printLinkedListValid(MMU * mmu){
    int listHead = mmu->listHead;
    printf("\nSTART PRINTING VALID NODES OF LINKED LIST\n");

    while(listHead < 4096){

        if(mmu->tables[0].pages[listHead].flags & Valid){
            printf("%d->", mmu->tables[0].pages[listHead].page_id);
        }
        listHead++;
    }
    printf("\nEND OF LINKED LIST\n");
}
//Prints in order all list nodes with the Read bit set to 1
void printLinkedListRead(MMU * mmu){
    int listHead = mmu->listHead;
    printf("\nSTART PRINTING VALID NODES OF LINKED LIST\n");

    while(listHead < 4096){

        if(mmu->tables[0].pages[listHead].flags & Read){
            printf("%d->", mmu->tables[0].pages[listHead].page_id);
        }
        listHead++;
    }
    printf("\nEND OF LINKED LIST\n");
}

//Prints in order all list nodes with the Write bit set to 1
void printLinkedListWrite(MMU * mmu){
    int listHead = mmu->listHead;
    printf("\nSTART PRINTING VALID NODES OF LINKED LIST\n");

    while(listHead < 4096){

        if(mmu->tables[0].pages[listHead].flags & Write){
            printf("%d->", mmu->tables[0].pages[listHead].page_id);
        }
        listHead++;
    }
    printf("\nEND OF LINKED LIST\n");
}

//Prints the list reversed
void printLinkedListReverse(MMU * mmu){
    int listHead = 4095;
    while(listHead > 0){

        if(mmu->tables[0].pages[listHead].flags & Valid){
            printf("%d->", mmu->tables[0].pages[listHead].page_id);
        }
        listHead--;
    }
    printf("\nEND OF LINKED LIST\n");
}

//FUNZIONI GESTIONE MMU

//Allocates page table with all possible pages already mapped onto frames
int allocNewTable(MMU * mmu){
    RAM * memory = mmu->memory;
    if(!mmu || !memory){
        perror("No mmu or ram allocated!");
        return 1;
    }
    int i;
    //In realtà il ciclo viene eseguito 1 volta, assumo di dover allocare 1 tabella
    for(i = 0; i<MAX_NUM_PROCESSES; i++){
        //Al primo spazio libero alloco la tabella
        if ( mmu->tables[i].pages == NULL){
            //Alloco le pagine
            printf("Allocating space for %d pages, one table\n\n", VM_NUM_PAGES);
            mmu->tables[i].pages = malloc(sizeof(PageTableEntry)*VM_NUM_PAGES);
             //Assume all memory allocated 
            mmu->tables[i].pages_left = 0;  
            for(int j = 0; j<NUM_PAGES;j++){
                mmu->track_alloc_frames[j] = 1;//Occupiamo tutta la memoria(caso interessante per fare swap)
            }
            for(int j = 4; j<VM_NUM_PAGES; j++){
                mmu->tables[i].pages[j].flags = 0;
                mmu->tables[i].pages[j].page_id = j;
                //per semplicità facciamo mappatura non randomica partendo dal frame dopo i frame per la table
                if(j < 256){ //la mappatura avviene solo per i primi 256 frame possibili gli altri sono allocati ma non validi
                    mmu->tables[i].pages[j].phy_page_id = (uint8_t )j;
                    mmu->tables[i].pages[j].flags |= Valid;
                }
                //Mappiamo tutti gli indirizzi sui frame, assumendo che tutta la memoria virtuale sia gia allocata

            }

            //printLinkedList(mmu);

            //Facciamo gestire i primi frame dedicati alla tabella delle pagine alla tabella stessa:
            for(int j = 0; j<4; j++){
                mmu->tables[i].pages[j].page_id = j;
                mmu->tables[i].pages[j].phy_page_id = j;
                mmu->tables[i].pages[j].flags = 0;
                //Le pagine corrispondenti ai frame della page table non sono scambiabili!
                mmu->tables[i].pages[j].flags |= Unswappable;
            }

            //La tabella delle pagine viene posta all'inizio della Memoria. Assumo che ogni tabella abbia dimensione fissa.
            //Viene mappata con costo NUM_PAGES*ENTRY_SIZE + pages_left_counter_size
            //ENTRY SIZE e idealmente 24bit e abbiamo 2^12 pagine allocabili(2^8 frame occupabili). 3byte per entry, ogni frane da 4kb ospita fino a 1365 entry.
            //Allo stesso tempo il contatore singolarmente occupa 4byte di un frame e le entry totali sono 4096.
            //Mettiamo quindi la tabella in memoria fisica
            //Per semplicità: 4096/1365 = 4 frame arrotondando per eccesso su 256
            //Per ora assumo di allocare una sola tabella che inizia dai primi 4 frame, quindi la memoria dei processi non verrà mappata in questi indirizzi
            //Settiamo questi primi byte a 1 per simulare il fatto che la tabella copre questo spazio.
            for(int k = 0; k<4; k++){
                for(int y = 0; y < 4096; y++){ //A causa della page table, vengono occupati 4 frame
                    memory->frames[k].mem[y] = 1;
                }
            }
            printf("Done!\n\n");
            break;
        }
    }
    return 0;
}

//Deallocates the table
void deallocTable(MMU * mmu, int tableIndex){
    RAM * memory = mmu->memory;
    printf("Deallocating table\n");
    //Liberiamo la memoria fisica precedentemente occupata dalla page table
    for(int k = 0; k<4; k++){
        for(int _ = 0; _ < 4096; _++){ 
            memory->frames[k].mem[_] = 0;
        }
    }
    free(mmu->tables[tableIndex].pages);

   printf("Table deallocated successfully\n");
}

//Creates or opens swap file of 16MB
int createSwapFile(MMU * mmu){
    if(!mmu){
        perror("mmu not initialized!\n");
        return 1;
    }
    mmu->swap_file = fopen("swap_file.bin", "w+b");
    char * val = (char*)malloc(sizeof(char)*SWAPPING_DISK_SIZE);
    memset(val, 0, SWAPPING_DISK_SIZE);
    fwrite(val, sizeof(char), SWAPPING_DISK_SIZE, mmu->swap_file);
    free(val);
    return 0;
}
