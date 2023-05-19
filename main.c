#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "h_files/mmu.h"
#include "h_files/utils.h"

//MAIN DI TESTING
int main(){
    MMU * mmu = (MMU*)malloc(sizeof(MMU));
    mmu->tables = (PageTable*)malloc(sizeof(PageTable)*MAX_NUM_PROCESSES); //spazio per tabelle
    mmu->pages_list = NULL;
    //RAM su cui si poggia la tabella delle pagine e il resto della memoria richiesta dai processi
    RAM * memory = (RAM*)malloc(sizeof(RAM));

    //Creiamo i frame di memoria con i loro id, ogni frame ha 16byte e la memoria totale è 1MB
    for(int i = 0; i<PHYSICAL_MEMORY/FRAME_SIZE; i++){
        memset(memory->frames[i].mem, 0, FRAME_SIZE);
    }
    mmu->memory = memory;
    mmu->swap_file = NULL;

    if(allocNewTable(mmu)){
        perror("Error allocating table!");
        free(memory);
        free(mmu->tables);
        free(mmu);
        return 1;
    }

    

    //Creiamo Swap File su DISCO
    if(createSwapFile(mmu)){
        perror("Error allocating swap disk!");
        free(memory);
        free(mmu->tables);
        free(mmu);
        return 1;
    }

    //Scriviamo qualcosa in posizione 7000, mi aspetto che la scrittura vada senza page fault, visto che fino a 1MB di spazio logico è tutto allocato


    //Libero memoria della tabella '0'
    deallocTable(mmu, 0);
    free(memory);
    free(mmu->tables);
    fclose(mmu->swap_file);
    free(mmu);

    printf("TEST ENDED");
    return 0;
}