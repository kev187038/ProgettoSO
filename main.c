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

    printf("The linked list of all allocated Pages is:\n");
    //printLinkedList(mmu);

    //Scriviamo qualcosa in posizione 7000+4*PAGE_SIZE, mi aspetto che la scrittura vada senza page fault, visto che fino a 1MB di spazio logico è tutto allocato
    MMU_writeByte(mmu, 7000+4*PAGE_SIZE, 'a');
    char * c = MMU_readByte(mmu, 3500+4*PAGE_SIZE);
    //printLinkedListValid(mmu);
    printLinkedListRead(mmu);
    printLinkedListWrite(mmu);

    //Scriviamo ora in un'area non mappata
    MMU_writeByte(mmu, 2000000, 'H');

    //Leggiamo da un'area non esistente
    c = MMU_readByte(mmu, (1<<25));


    //Libero memoria della tabella '0'
    deallocTable(mmu, 0);
    free(memory);
    free(mmu->tables);
    fclose(mmu->swap_file);
    free(mmu);

    printf("TEST ENDED!!\n");
    return 0;
}