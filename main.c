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

    //printf("The linked list of all allocated Pages is:\n");
    //printLinkedList(mmu);

    //Scriviamo qualcosa in posizione 7000+4*PAGE_SIZE, mi aspetto che la scrittura vada senza page fault, visto che fino a 1MB di spazio logico è tutto allocato
    MMU_writeByte(mmu, 7000+4*PAGE_SIZE, 'A');
    char * c = MMU_readByte(mmu, 3500+4*PAGE_SIZE);
    //printLinkedListValid(mmu);
    printLinkedListRead(mmu);
    printLinkedListWrite(mmu);

    //Scriviamo ora in un'area non mappata
    MMU_writeByte(mmu, 2000000, 'H');

    //Leggiamo da un'area non esistente
    c = MMU_readByte(mmu, (1<<25));

    c = MMU_readByte(mmu, 7000+4*PAGE_SIZE);
    printf("Il byte letto dalla posizione in cui abbiamo scritto prima è proprio: %c\n\n", *c);

    //Leggiamo dall'area su cui c'è stato un page fault prima
    c = MMU_readByte(mmu,2000000);
    printf("Il byte letto dall'area che aveva causato PF è proprio %c\n\n",*c);

    MMU_writeByte(mmu,3500+4*PAGE_SIZE,'B');

    printLinkedListValid(mmu);
    printf("Altri test:)\n\n");
    //Per i prossimi test, poniamoci nella condizione in cui ogni pagina ha almeno un bit a 1: così potremo
    //testare il programma quando deve mettere una pagina (la 7, perché ora si trova all'inizio della lista) su disco e poi riperscarla
    //per farlo, scriviamo qualcosa su tutte le altre pagine, così che ricevano una seconda chance e A venga sfrattata
    //da notare che la 6 è stata sostituita con un'altra pagina, quindi non deve essere considerata, altrimenti generiamo page fault a cascata
    for(int i = 7; i<NUM_PAGES; i++){
        MMU_writeByte(mmu,i*PAGE_SIZE, 'C');
    }
    //Scriviamo in maniera tale da generare page fault
    MMU_writeByte(mmu, 500*PAGE_SIZE, 'D');
    //Ora che la pagina 7 è stata rimpiazzata, proviamo a leggere di nuovo il carattere C in 7: generiamo un altri page fault
    c = MMU_readByte(mmu, 7*PAGE_SIZE);
    printf("Carattere letto da pagina ripescata da swap file è proprio %c\n\n", *c);

    //Libero memoria della tabella '0'
    deallocTable(mmu, 0);
    free(memory);
    free(mmu->tables);
    fclose(mmu->swap_file);
    free(mmu);

    printf("TEST ENDED!!\n");
    return 0;
}