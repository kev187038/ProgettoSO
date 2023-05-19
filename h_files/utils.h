//ALTRE FUNZIONI 
#pragma once
#include "mmu.h"
#define MAX_NUM_PROCESSES 10


//FUNZIONI DI UTILITÀ PER IL MANAGEMENT DELLA PAGE TABLE
int allocNewTable(MMU * mmu);
void deallocTable(MMU * mmu, int tableIndex);
int createSwapFile(MMU * mmu);
void insertPage(PageTableEntry*entry, MMU* mmu);
void removeLinkedList(MMU * mmu);
void printLinkedList(MMU * mmu);