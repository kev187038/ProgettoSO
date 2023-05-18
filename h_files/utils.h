//ALTRE FUNZIONI 
#pragma once
#include "mmu.h"
#define MAX_NUM_PROCESSES 10

//ALLOCA UNA NUOVA PAGETABLE PER IL PROCESSO
int allocNewTable(MMU * mmu, RAM * memory);
void deallocTable(MMU * mmu, RAM * memory, int tableIndex);