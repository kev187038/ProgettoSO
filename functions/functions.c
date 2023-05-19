#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../h_files/mmu.h"
#include "../h_files/utils.h"
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
        //IPOTESI MEMORIA GIÀ ALLOCATA -> NON C'È QUESTO CASO
        //Se il bit di validità è a 1, si controlla il bit di write e di read: se sono entrambi 0, si sostituisce
        if((listHead->element->flags & Valid) && !(listHead->element->flags & Read) && !(listHead->element->flags & Write)){
            //Pagina né letta, né scritta di recente->sostituzione
            page_evicted = listHead->element->page_id;
            int offset = page_evicted*PAGE_SIZE;
            fseek(mmu->swap_file,offset, SEEK_SET);
            uint8_t frame_evicted = listHead->element->phy_page_id;
            fwrite(&(mmu->memory->frames[frame_evicted].mem), PAGE_SIZE, 1, mmu->swap_file); //scriviamo il buffer sul file
            listHead->element->flags = 0;//Pagina non più allocata
            memset(mmu->memory->frames[frame_evicted].mem, 0, FRAME_SIZE );
            printf("Evicted page with id %d\n", page_evicted);
            //Mettiamo la pagina nuova "page_id" mappandola nel frame sfrattato
            mmu->tables->pages[page_id].flags |= Valid | Write | Read; //setto questi bit a 1
            mmu->tables->pages[page_id].phy_page_id = frame_evicted;
            break;
        }
        //Se solo uno dei due bit è impostato a 1, si setta a 0 se si mette in code la page
        else if((listHead->element->flags & Valid) && (listHead->element->flags & Read) && !(listHead->element->flags & Write) ){
            listHead->element->flags &= ~Read; //setto read a 0
            //Sposto l'elemento in coda
            inTail(mmu, listHead);
        }
        else if((listHead->element->flags & Valid) && !(listHead->element->flags & Read) && (listHead->element->flags & Write) ){
            listHead->element->flags &= ~Write; //setto read a 0
            //Sposto l'elemento in coda
            inTail(mmu, listHead);
        }
    }

}

void MMU_writeByte(MMU* mmu, int pos, char c){
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
