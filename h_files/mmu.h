#pragma once
#include <stdint.h>

//Spazio di memoria virtuale 16MB
#define VIRTUAL_MEMORY (1<<24)

//Page size di 16byte ognuna
#define PAGE_SIZE (1<<4)

//Memoria fisica 1MB
#define PHYSICAL_MEMORY (1<<20)

//SWAPPING DISK SIZE
# define SWAPPING_DISK_SIZE (1<<24)

//NUMERO BIT PER LE FLAG
#define PAGE_FLAGS_NUMBITS 4

//NUMERO BIT PAGINA
#define PAGE_NUMBITS 20

//NUMERO BIT PER ADDRESS FISICO
#define PHYSICAL_ADDR_NUMBITS (1<<4)

//NUMERO BYTE PER FRAME FISICO
#define FRAME_SIZE 16

//Num pages
#define NUM_PAGES (PHYSICAL_MEMORY / PAGE_SIZE)

//OFFSET
#define OFFSET 4

//File di swap
#define FILE_SIZE (1<<24)

//Dichiaro i bit per le pagine
typedef enum {
  Valid=0x1,
  Unswappable=0x2,
  Read=0x3,
  Write=0x4
} PageFlags;

//Indirizzo logico, idealmente a 24bit: 20bit per id, 4bit per offset
typedef struct LogicalAddress{
  uint32_t page_id: PAGE_NUMBITS;
  uint8_t offset: OFFSET;
} LogicalAddress;

//Indirizzo fisico a 20bit: 16 di id e 4 di offset
typedef struct PhysicalAddress{
  uint16_t phy_page_id: PHYSICAL_ADDR_NUMBITS;
  uint8_t offset:      OFFSET;
} PhysicalAddress;

//Entry della tabella delle pagine: 20bit per indirizzo logico, 16 per fisico e 4 bit per flag
typedef struct PageTableEntry{
  uint32_t page_id: PAGE_NUMBITS;
  uint16_t phy_page_id: PHYSICAL_ADDR_NUMBITS;
  PageFlags flags: PAGE_FLAGS_NUMBITS;
}PageTableEntry;

//Tabella delle pagine, 40bit(idealmente) per entry 
typedef struct PageTable{
  PageTableEntry *pages; 
  uint32_t pages_left;
}PageTable;

//LA MMU conterrà la tabella delle pagine e un contatore per sapere se può soffisfare o meno una richiesta di pagine
typedef struct MMU {
  PageTable * tables;  //nuova tabella allocata per ogni processo
} MMU;

//Frames di memoria fisica da 16byte ognuno
typedef struct Frame{
  uint16_t phy_frame_id: PHYSICAL_ADDR_NUMBITS; 
  char mem[FRAME_SIZE];
}Frame;

//Memoria fisica: buffer da 1MB su cui mappiamo la memoria
typedef struct RAM{
  Frame frames[PHYSICAL_MEMORY/PHYSICAL_ADDR_NUMBITS]; //1MB
}RAM;

//Struct per simulare il file di swap: i frame ora sono spazi su disco. La memoria verrà mappata su file e trattata tramite questa struct
typedef struct SwapFile{
  Frame frames[FILE_SIZE/PAGE_SIZE]; //16MB
}SwapFile;

//Funzioni da definire
void MMU_writeByte(MMU* mmu, int pos, char c);
char* MMU_readByte(MMU* mmu, int pos );
void MMU_exception(MMU* mmu, int pos); //handling di page fault con algoritmo "second chance"