#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../h_files/mmu.h"
#include "../h_files/utils.h"
//Handles page fault
void MMU_exception(MMU* mmu, int pos){
    printf("Page fault in posizione %d\n\n", pos);
    //Calcoliamo Id  ~
    int page_id = pos/PAGE_SIZE;
    int page_evicted = -1;

    //Se il numero di frame liberi rimasti è > 0, possiamo trovarlo e allocare la pagina lì
    if(mmu->tables[0].pages_left > 0){
        printf("Found free frame: alllocating page...\n");
        int frame_id;
        for(int i = 0; i<NUM_PAGES;i++){
            if(mmu->track_alloc_frames[i] == 0){
                frame_id = i;
                break;
            }
        }
        //Alloca la pagina nel primo frame disponibile
        mmu->tables[0].pages_left--;
        mmu->tables[0].pages[page_id].flags |= Valid | Write | Read;
        mmu->tables[0].pages[page_id].phy_page_id = frame_id;
        printf("Done.\n\n");
        return;
    }

    int listHead = mmu->listHead;

    //IMPLEMENTAZIONE SECOND  CHANCE ALGORITHM
   //1.Cerco la pagina logica non mappata in RAM su FILE
   //2.La cambio con la prima pagina sfrattabile su memoria fisica

    //1
    int offs = page_id*PAGE_SIZE;
    fseek(mmu->swap_file, offs, SEEK_SET);
    char buffer[PAGE_SIZE];
    size_t bytes_read = fread(buffer, sizeof(char),PAGE_SIZE, mmu->swap_file);

    //2
    //Si assume che tutta la memoria sia allocata all'inizio
    //Saltiamo le pagine unswappable iniziali della tabella
    while(mmu->tables[0].pages[listHead].flags & Unswappable){
        listHead++;
    }
    while(1){
        //Se il bit di validità è a 0, la pagina non e allocata e si cerca di allocarla
        //IPOTESI MEMORIA GIÀ ALLOCATA E MAPPATA DALL'INIZIO-> NON C'È QUESTO CASO
        //Se il bit di validita e a 1, si controlla il bit di write e di read: se sono entrambi 0, si sostituisce
        if((mmu->tables[0].pages[listHead].flags & Valid) && !(mmu->tables[0].pages[listHead].flags & Read) && !(mmu->tables[0].pages[listHead].flags & Write) && !(mmu->tables[0].pages[listHead].flags & Unswappable)){
            //Pagina ne letta, ne scritta di recente->sostituzione
            page_evicted = mmu->tables[0].pages[listHead].page_id;
            int offset = page_evicted*PAGE_SIZE;
            fseek(mmu->swap_file,offset, SEEK_SET);
            uint8_t frame_evicted = mmu->tables[0].pages[listHead].phy_page_id;
            fwrite(&(mmu->memory->frames[frame_evicted].mem), PAGE_SIZE, 1, mmu->swap_file); //scriviamo il buffer sul file
            mmu->tables[0].pages[listHead].flags = 0;//Pagina non più allocata

            //Copiamo i dati letti da swap_file nel frame
            memcpy(mmu->memory->frames[frame_evicted].mem, buffer, FRAME_SIZE);

            printf("Evicted page with id %d at frame %d\n\n", page_evicted, frame_evicted);
            //Mettiamo la pagina nuova "page_id" mappandola nel frame sfrattato
            mmu->tables->pages[page_id].flags |= Valid | Write | Read; //setto questi bit a 1
            mmu->tables->pages[page_id].phy_page_id = frame_evicted;
            printf("New page in frame %d is id %d\n\n", frame_evicted, page_id);
            mmu->listHead = listHead;
            break;
        }
        //Se solo uno dei due bit e impostato a 1, si setta a 0 se si mette in code la page
        else if((mmu->tables[0].pages[listHead].flags & Valid) && (mmu->tables[0].pages[listHead].flags & Read) && !(mmu->tables[0].pages[listHead].flags & Write) && !(mmu->tables[0].pages[listHead].flags & Unswappable)){
           mmu->tables[0].pages[listHead].flags &= ~Read; //setto read a 0
            printf("Set read bit of page %d to %d\n\n",mmu->tables[0].pages[listHead].page_id, mmu->tables[0].pages[listHead].flags & Read);
            //Sposto l'elemento in coda

        }
        else if((mmu->tables[0].pages[listHead].flags & Valid) && !(mmu->tables[0].pages[listHead].flags & Read) && (mmu->tables[0].pages[listHead].flags & Write) && !(mmu->tables[0].pages[listHead].flags & Unswappable)){
            mmu->tables[0].pages[listHead].flags &= ~Write; //setto read a 0
            printf("Set written(dirty) bit of page %d to %d\n\n",mmu->tables[0].pages[listHead].page_id, mmu->tables[0].pages[listHead].flags & Write);
            //Sposto l'elemento in coda

        }
        else if((mmu->tables[0].pages[listHead].flags & Valid) && (mmu->tables[0].pages[listHead].flags & Read) && (mmu->tables[0].pages[listHead].flags & Write) && !(mmu->tables[0].pages[listHead].flags & Unswappable)){
            //Se sia il bit di read che di write sono a 1, la pagina è usata molto: si setta uno dei bit a 0 e ha un'altra second chance
           mmu->tables[0].pages[listHead].flags &= ~Write; //setto read a 0
            printf("Both read and write bits were 1: change write bit of page %d to %d\n\n",mmu->tables[0].pages[listHead].page_id, mmu->tables[0].pages[listHead].flags & Write);
            //Sposto l'elemento in coda

        }
        //Pagina non sfrattata, si va avanti
        listHead++;
        if(listHead >= 4096){
            listHead = 0;
        }
    }

}

//Writes single byte c, accessing to memory in position pos
void MMU_writeByte(MMU* mmu, int pos, char c){
    if(pos >= (1<<24) || pos<0){
        printf("Errore, posizione illegale! L'istruzione sarà ignorata.\n\n");
        return;
    }

    //Calcoliamo Id e offset del processo a cui accedere ~
    int page_id = pos/PAGE_SIZE;
    int offset  = pos % PAGE_SIZE;

    if(mmu->tables[0].pages[page_id].flags & Valid){
        printf("Scrivo '%c' in memoria in posizione %d in pagina %d, con offset %d\n", c, pos, page_id, offset);
        mmu->tables[0].pages[page_id].flags |= Write; //Impostiamo bit di scrittura e lettura a 1

        //Scriviamo su memoria fisica: prendo l'id del frame dalla entry della tabella delle pagine e così trovo il frame fisico
        int frame_id = mmu->tables[0].pages[page_id].phy_page_id;
        mmu->memory->frames[frame_id].mem[offset] = c; 
        printf("Fatto\n\n");
    }else{
        if(page_id < 4){
            //non scriviamo nella pgtable
            return;
        }
        //Eccezione per accesso a memoria invalido
        MMU_exception(mmu,pos);
        //Riproviamo a scrivere dopo aver allocato/sostituito la pagina
        MMU_writeByte(mmu,pos,c);
    }

}

//Reads single byte, accessing to memory in position pos
char* MMU_readByte(MMU* mmu, int pos ){

    if(pos >= (1<<24) || pos<0){
        printf("Errore, posizione illegale! L'istruzione sara ignorata.\n\n");
        return NULL;
    }
    //Calcoliamo Id e offset del processo a cui accedere ~
    int page_id = pos/PAGE_SIZE;
    int offset  = pos % PAGE_SIZE;

    if(mmu->tables[0].pages[page_id].flags & Valid){
        printf("Leggo da memoria in posizione %d in pagina %d, con offset %d\n", pos, page_id, offset);
        mmu->tables[0].pages[page_id].flags |= Read; 
        int frame_id = mmu->tables[0].pages[page_id].phy_page_id;
        printf("Fatto\n\n");
        return &(mmu->memory->frames[frame_id].mem[offset]);
    }else{
        if(page_id < 4){
            //non leggiamo dalla pgtable
            return NULL;
        }
        //Se sto leggendo da memoria invalida cerco sul file il frame
        MMU_exception(mmu,pos);
        //e riprovo la lettura
        return MMU_readByte(mmu,pos);
    }
    return NULL;

}
