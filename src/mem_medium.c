/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"
#include <math.h>

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}


void * emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    
    // Prend en compte le marquage
    unsigned long sizeFinal = size + 32;
    // calcule l’indice de la TZL contenant la bonne taille 
    unsigned int minIndice = puiss2(sizeFinal); 
    uint32_t maxIndice = (FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant);

    unsigned int minUsable = minIndice;
    while(arena.TZL[minUsable] == NULL && minUsable < (maxIndice) ){
        minUsable ++;
    }
    if (minUsable > maxIndice){ // Si on a même pas de tailleMax alloué
        mem_realloc_medium();
        minUsable--; // Recentre à la case de l'indice max !
    }

    for (uint32_t i=minUsable;i>minIndice;i--){
        // Faire couper en 2 !
        float * firstBloc = (float *) arena.TZL[i];
        float * secondBloc =  (float *) ( ((char *) arena.TZL[i]) + (math.pow(2,(i-1) )) );

        // On bouge la liste des éléments libre, on vire le 1er et on prend le prochain !
        arena.TZL[i] =  (void *)(*firstBloc); 

        (* firstBloc) = secondBloc; // Le prochain du 1er bloc coupé est son autre moitié !
        arena.TZL[i-1] = (void *) firstBloc;
        
    }

    // Si un bloc est disponible, il est marqué et l’adresse utilisateur est retournée
    (void *) bloc = arena.TZL[minIndice];

    return mark_memarea_and_get_user_ptrr(bloc,math.pow(2,minIndice),MEDIUM_KIND);
}


void efree_medium(Alloc a) {
    /* ecrire votre code ici */
}


