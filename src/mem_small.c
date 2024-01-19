/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void * emalloc_small(unsigned long size)
{
    if (arena.chunkpool == NULL){
        unsigned long tailleOctet = mem_realloc_small();
        unsigned long * ptr_small_mem = (unsigned long *) arena.chunkpool;
        void *next_ptr;

        // écrit les adresses pour la listes !
        do {
            // On avance de 96 octets pour faire référence au prochain élement
            next_ptr = (void *) ptr_small_mem;
            next_ptr += 96;

            *ptr_small_mem = (unsigned long) next_ptr;
            ptr_small_mem = (unsigned long *) next_ptr;
            tailleOctet -= 96;
        } while( tailleOctet > 96 );
    }

    // Chunk pool valide, on prend le 1er !
    void* ptr_chunk_start = arena.chunkpool;
    unsigned long * ptr_to_val_next_prt = (unsigned long *) ptr_chunk_start;

    arena.chunkpool = (void *) (*ptr_to_val_next_prt);

    return mark_memarea_and_get_user_ptr(ptr_chunk_start,96,SMALL_KIND);
}

void efree_small(Alloc a) {

}
