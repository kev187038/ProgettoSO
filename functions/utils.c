#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../h_files/mmu.h"
#include "../h_files/utils.h"

//FUNZIONI DI GESTIONE LISTA COLLEGATA
void insertPage(PageTableEntry* entry, MMU*mmu){
    PageElement * new = (PageElement*)malloc(sizeof(PageElement));
    new->element = entry;
    if(mmu->pages_list == NULL){
        mmu->pages_list = new;
        new->next = new;
        new->previous = new;
        return;
    }

    PageElement * el = mmu->pages_list;
    while(el->next != mmu->pages_list){
        el = el->next;
    }
    el->next = new;
    new->next = mmu->pages_list;
    new->previous = el;
    mmu->pages_list->previous = new;
    return;
}

void removeLinkedList(MMU * mmu){
    PageElement * el = mmu->pages_list;
    PageElement * nextnode;
    while(el != mmu->pages_list){
        nextnode = el->next;
        free(el);
        el = nextnode;
    }
}

void printLinkedList(MMU * mmu){
    PageElement * el = mmu->pages_list;
    printf("START PRINTING LINKED LIST\n");
    printf("%d->", el->element->page_id);
    while(el->next != mmu->pages_list){
        el = el->next;
        printf("%d->", el->element->page_id);
    }
    printf("\nEND OF LINKED LIST\n");
}

void printLinkedListReverse(MMU * mmu){
    PageElement * el = mmu->pages_list->previous;
    printf("START PRINTING LINKED LIST\n");
    printf("%d->", el->element->page_id);
    while(el != mmu->pages_list){
        el = el->previous;
        printf("%d->", el->element->page_id);
    }
    printf("\nEND OF LINKED LIST\n");
}
//Sposta in coda alla lista circolare qualsiasi elemento to_move
void inTail(MMU * mmu, PageElement * to_move){
    PageElement * listHead = mmu->pages_list;
    PageElement * el = to_move;

    PageElement * prev_of_head = listHead->previous;
    PageElement * nxt = el->next;
    PageElement * prev = el->previous;

    prev->next = nxt;
    nxt->previous = prev;

    listHead->previous = el;
    prev_of_head->next = el;
    el->next = listHead;
    el->previous = prev_of_head;
}

//FUNZIONI GESTIONE MMU
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
            printf("Allocating space for %d pages, one table\n", VM_NUM_PAGES);
            mmu->tables[i].pages = malloc(sizeof(PageTableEntry)*VM_NUM_PAGES);
            mmu->tables[i].pages_left = VM_NUM_PAGES - 4;   
            for(int j = 4; j<VM_NUM_PAGES; j++){
                mmu->tables[i].pages[j].flags = 0;
                mmu->tables[i].pages[j].page_id = j;
                //per semplicità facciamo mappatura non randomica partendo dal frame dopo i frame per la table
                if(j < 256){ //la mappatura avviene solo per i primi 256 frame possibili gli altri sono allocati ma non validi
                    mmu->tables[i].pages[j].phy_page_id = (uint8_t )j;
                    mmu->tables[i].pages[j].flags |= Valid;
                }
                //Mappiamo tutti gli indirizzi sui frame, assumendo che tutta la memoria virtuale sia già allocata

                //creiamo la lista collegata associata
                insertPage(&(mmu->tables[i].pages[j]), mmu);

            }

            //printLinkedList(mmu);

            //Facciamo gestire i primi frame dedicati alla tabella delle pagine alla tabella stessa:
            for(int j = 0; j<4; j++){
                mmu->tables[i].pages[j].page_id = j;
                mmu->tables[i].pages[j].phy_page_id = j;
                mmu->tables[i].pages[j].flags = 0;
                //Le pagine corrispondenti ai frame della page table non sono scambiabili!
                mmu->tables[i].pages[j].flags |= Unswappable;
                mmu->tables[i].pages[j].flags |= Valid;
            }

            //La tabella delle pagine viene posta all'inizio della Memoria. Assumo che ogni tabella abbia dimensione fissa.
            //Viene mappata con costo NUM_PAGES*ENTRY_SIZE + pages_left_counter_size
            //ENTRY SIZE è idealmente 24bit e abbiamo 2^12 pagine allocabili(2^8 frame occupabili). 3byte per entry, ogni frane da 4kb ospita fino a 1365 entry.
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
            printf("Done!\n");
            break;
        }
    }
    return 0;
}

//Dealloca la table singola
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

    //Rimuoviamo la lista collegata associata
   removeLinkedList(mmu);
}

//crea file di swap di 16MB
int createSwapFile(MMU * mmu){
    if(!mmu){
        perror("mmu not initialized!\n");
        return 1;
    }
    mmu->swap_file = fopen("swap_file.bin", "w+b");
    return 0;
}
