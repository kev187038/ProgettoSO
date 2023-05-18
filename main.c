#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "h_files/mmu.h"
#include "h_files/utils.h"

int main(){
    MMU * mmu = (MMU*)malloc(sizeof(MMU));
    mmu->tables = (PageTable*)malloc(sizeof(PageTable)*MAX_NUM_PROCESSES); //spazio per tabelle
    

    //RAM su cui si poggia la tabella delle pagine e il resto della memoria richiesta dai processi
    RAM * memory = (RAM*)malloc(sizeof(RAM));

    //Creiamo i frame di memoria con i loro id
    for(int i = 0; i<PHYSICAL_MEMORY/PHYSICAL_ADDR_NUMBITS; i++){
        memory->frames[i].phy_frame_id = i;
        memset(memory->frames[i].mem, 0, FRAME_SIZE);
    }


    if(!allocNewTable(mmu, memory)){
        perror("Error allocating table!");
        free(memory);
        free(mmu->tables);
        free(mmu);
        return 0;
    }

    //Creiamo Swap File su DISCO



    //Libero memoria
    free(memory);
    free(mmu->tables);
    free(mmu);

    printf("TEST ENDED");
    return 0;
}