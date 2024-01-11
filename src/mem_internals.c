/******************************************************
 * Copyright Grégory Mounié 2018-2022                 *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    // Réalisation Magic Number
    unsigned long magic_number = knuth_mmix_one_round( (unsigned long)ptr);
    magic_number = (magic_number & ~(0b11UL)) + (k & 0b11UL);
    printf(" \n UWU??? [%lu] [%d] \n ",magic_number,k);
    // Initialisation Pointeur Début
    unsigned long *ptr_debut = (unsigned long *) ptr;
    *ptr_debut = size;

    // Magic Number Début
    unsigned long *ptr_magic_number_debut = ptr_debut + 1;
    *ptr_magic_number_debut = magic_number;

    // Magic Number Fin
    unsigned long *ptr_magic_number_fin = ptr_debut + size - 2;
    *ptr_magic_number_fin = magic_number; 
    
    // Size Fin
    unsigned long *ptr_size_fin = ptr_magic_number_fin + 1;
    *ptr_size_fin = size;

    // Réalisation Pointeur Début Mémoire
    void *ptr_debut_memoire = ptr_debut + 2;

    return ptr_debut_memoire;
}

Alloc mark_check_and_get_alloc(void *ptr) {
    unsigned long *ptr_debut = (unsigned long *) ptr;

    // Size début
    unsigned long size_debut = *(ptr_debut - 4);
    unsigned long magic_debut = *(ptr_debut - 2);

    unsigned long magic_fin = *(ptr_debut + size_debut - 8);
    //unsigned long size_fin = *(ptr_debut + size - 4);

    MemKind k = magic_debut & 0b11UL;
    assert( magic_debut == magic_fin );

    Alloc a = {};
    a.size = size_debut;
    a.kind = k;
    a.ptr = (ptr - 4);

    return a;
}


unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1UL << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
