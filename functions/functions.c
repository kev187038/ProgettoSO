#include <stdio.h>
#include <stdlib.h>
#include "../h_files/mmu.h"
void MMU_exception(MMU* mmu, int pos){
    printf("Page fault in posizione %d\n", pos);
    //Calcoliamo Id  ~
    int page_id = pos/PAGE_SIZE;
    int page_evicted = -1;
    PageElement * listHead = mmu->pages_list;

    //IMPLEMENTAZIONE SECOND  CHANCE ALGORITHM
    //Si assume che tutta la memoria sia allocata all'inizio
    //Saltiamo le pagine unswappable iniziali della tabella
    while(listHead->element->flags & Unswappable){
        listHead = listHead->next;
    }
    while(listHead != mmu->pages_list){
        //Se il bit di validità è a 0, la pagina non è allocata e si cerca di allocarla

    }

}

void MMU_writeByte(MMU* mmu, int pos, char c){

    //Se non ci sono pagine libere, page fault
    if(mmu->tables[0].pages_left == 0){
        MMU_exception(mmu, pos);
        return;
    }

    //Calcoliamo Id e offset del processo a cui accedere ~
    int page_id = pos/PAGE_SIZE;
    int offset  = pos % PAGE_SIZE;

    if(mmu->tables[0].pages[page_id].flags & Valid){
        printf("Scrivo 'a' in memoria in posizione %d in pagina %d, con offset %d\n",pos, page_id, offset);
        mmu->tables[0].pages[page_id].flags | Write; //Impostiamo bit di scrittura a 1
        mmu->tables[0].pages[page_id].flags | Read; 

        //Scriviamo su memoria fisica: prendo l'id del frame dalla entry della tabella delle pagine e così trovo il frame fisico
        int frame_id = mmu->tables[0].pages[page_id].phy_page_id;
        mmu->memory->frames[frame_id].mem[offset] = 'a'; 
    }else{
        //Eccezione per accesso a memoria invalido
        MMU_exception(mmu,pos);
        //Riproviamo a scrivere dopo aver allocato/sostituito la pagina
        printf("Scrivo 'a' in memoria in posizione %d in pagina %d, con offset %d\n",pos, page_id, offset);
        mmu->tables[0].pages[page_id].flags | Write; //Impostiamo bit di scrittura a 1
        mmu->tables[0].pages[page_id].flags | Read; 
        int frame_id = mmu->tables[0].pages[page_id].phy_page_id;
        mmu->memory->frames[frame_id].mem[offset] = 'a'; 
    }

}
char* MMU_readByte(MMU* mmu, int pos );
