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
        void* ptr_small_mem = arena.chunkpool;
        
        // écrit les adresses pour la listes !
        do {
            (*ptr_small_mem) = ptr_small_mem+96;
            ptr_small_mem += 96; // Bouge de 96 octets
            tailleOctet -= 96:
        } while( tailleOctet > 96 );
        //mark_memarea_and_get_user_ptr(ptr_small_mem,96,SMALL_KIND);
    }

    // Chunk pool valide, on prend le 1er !
    void* ptr_chunk_start = arena.chunkpool;
    arena.chunkpool = (*ptr_chunk_start);

    return mark_memarea_and_get_user_ptr(ptr_chunk_start,96,SMALL_KIND);
}

void efree_small(Alloc a) {
    /* ecrire votre code ici */
}
