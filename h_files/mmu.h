#pragma once
#include <stdint.h>

//Spazio di memoria virtuale 16MB
#define VIRTUAL_MEMORY (1<<24)

//Page size di 4kb ognuna
#define PAGE_SIZE (1<<12)

//Memoria fisica 1MB
#define PHYSICAL_MEMORY (1<<20)

//SWAPPING DISK SIZE DI 16MB
# define SWAPPING_DISK_SIZE (1<<24)

//NUMERO BIT PER LE FLAG
#define PAGE_FLAGS_NUMBITS 4

//NUMERO BIT PAGINA
#define PAGE_NUMBITS 12

//NUMERO BIT PER ADDRESS FISICO
#define PHYSICAL_ADDR_NUMBITS 8

//NUMERO BYTE PER FRAME FISICO: MISURA DELLA PAGINA
#define FRAME_SIZE PAGE_SIZE

//Num pagine mappabili alla volta: 256
#define NUM_PAGES (PHYSICAL_MEMORY / PAGE_SIZE)

//Num pagine della memoria virtuale totali: 4096
#define VM_NUM_PAGES (VIRTUAL_MEMORY/PAGE_SIZE)

//OFFSET
#define OFFSET 12

//File di swap
#define FILE_SIZE (1<<24)

//Dichiaro i bit per le pagine
typedef enum {
  Valid=0x1,
  Unswappable=0x2,
  Read=0x4,
  Write=0x8
} PageFlags;

//Entry della tabella delle pagine: idealmente 12bit per indirizzo logico, 8 per fisico e 4 bit per flag -> 24bit per entry
typedef struct PageTableEntry{
  uint32_t page_id: PAGE_NUMBITS;
  uint16_t phy_page_id: PHYSICAL_ADDR_NUMBITS;
  PageFlags flags: PAGE_FLAGS_NUMBITS;
}PageTableEntry;

//Tabella delle pagine, 40bit(idealmente) per entry 
typedef struct PageTable{
  PageTableEntry * pages; 
  uint32_t pages_left;
}PageTable;

//Frames di memoria fisica da 16byte ognuno
typedef struct Frame{
  char mem[FRAME_SIZE];
}Frame;

//Memoria fisica: buffer da 1MB su cui mappiamo la memoria
typedef struct RAM{
  Frame frames[PHYSICAL_MEMORY/FRAME_SIZE]; //1MB 
}RAM;

//Lista collegata di pagine implementata esternamente
typedef struct PageElement{
  PageTableEntry * element;
  struct  PageElement * next;
  struct PageElement * previous;
}PageElement;

//LA MMU conterrà la tabella delle pagine e un contatore per sapere se può soffisfare o meno una richiesta di pagine
typedef struct MMU {
  PageTable * tables;  //nuova tabella allocata per ogni processo
  RAM * memory;
  FILE * swap_file;
  PageElement * pages_list;
  char track_alloc_frames[NUM_PAGES];
} MMU;


//Funzioni da definire nella consegna
void MMU_writeByte(MMU * mmu, int pos, char c);
char * MMU_readByte(MMU * mmu, int pos );
void MMU_exception(MMU * mmu, int pos); //handling di page fault con algoritmo "second chance"