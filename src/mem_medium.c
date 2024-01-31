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
  //  printf("====================== UWU : MEDIUM CALLED\n");
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    
    // Prend en compte le marquage
    unsigned long sizeFinal = size + 32;
    // calcule l’indice de la TZL contenant la bonne taille 
    unsigned int minIndice = puiss2(sizeFinal); 
    unsigned int maxIndice = (FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant);

    unsigned int minUsable = minIndice;
    while(arena.TZL[minUsable] == NULL && minUsable < (maxIndice+1) ){
        minUsable ++;
    }
    //printf("UWU : Taille %lu MinIndice:%u MaxIndice:%u MinUsable:%u\n",size,minIndice,maxIndice,minUsable);

    if (minUsable > maxIndice){ // Si on a même pas de tailleMax alloué
        mem_realloc_medium();
        minUsable--; // Recentre à la case de l'indice max !
    }
    
    for (uint32_t i=minUsable;i>minIndice;i--){ // 140388857741312 - 140388857675776 
        // Faire couper en 2 !
        unsigned long * firstBloc = (unsigned long *) arena.TZL[i];
        unsigned long * secondBloc =  (unsigned long *) ( ((char *) arena.TZL[i]) + ((int) pow(2,(i-1) )) );

       // printf("K:%u TaileBloc:%f Moitier:%f PointeurA:%p PointeurB:%p \n",i,pow(2,i), pow(2,(i-1) ),firstBloc,secondBloc);
        // On bouge la liste des éléments libre, on vire le 1er et on prend le prochain !
        arena.TZL[i] =  (void *)(*firstBloc);

        (*firstBloc) = (unsigned long) secondBloc; // Le prochain du 1er bloc coupé est son autre moitié !
        arena.TZL[i-1] = (void *) firstBloc;
        
    }
    
    // Si un bloc est disponible, il est marqué et l’adresse utilisateur est retournée
    void * bloc = arena.TZL[minIndice];

    // Le retire de la liste ! et met le prochain à la place
    arena.TZL[minIndice] = (void*)  (*( (unsigned long *) bloc));

    //printf("Utilise:%p Nouveau:%p \n",bloc,arena.TZL[minIndice]);

    return mark_memarea_and_get_user_ptr(bloc,pow(2,minIndice),MEDIUM_KIND);
}


void efree_medium(Alloc a) {
    unsigned int indice = puiss2(a.size);
    unsigned int maxIndice = (FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant);

    unsigned long * adresseBuddyA = (unsigned long *) ((unsigned long)a.ptr ^ (unsigned long)pow(2,indice));

    void * prev = NULL;
    void * addrBloc = arena.TZL[indice];
    while(addrBloc != NULL){
        printf("Check ListBloc ! \n");
        unsigned long * adresseBuddyB = (unsigned long *) ((unsigned long)addrBloc ^ (unsigned long)pow(2,indice));
        if (adresseBuddyA == addrBloc && adresseBuddyB == a.ptr){
            // On a trouver son poto ! on remonte d'un indice uwu
            // On le vire de la liste !
            if (prev == NULL){ // Il est directement utilisé par arena.TZL
                arena.TZL[indice] = (void *) (* ( (unsigned long *) addrBloc)); // Le prochain est stocké dans addrBloc
            } else { // Il est dans la liste, on utilise prev pour le retirer de la liste
                (*(unsigned long *) prev) = * ( (unsigned long *) addrBloc); // Le prochain est stocké dans addrBloc
            }

            // On l'ajoute au dessus
            // Si on a atteint, la fin on arrête !
            if (indice == maxIndice){
                return;
            }
            // Sinon, on ajoute le buddy le plus petit dessus!
            indice++;
            unsigned long * adrFirstBuddy = (adresseBuddyB < adresseBuddyA) ? adresseBuddyB : adresseBuddyA ;
            (*adrFirstBuddy) =  (unsigned long) arena.TZL[indice];
            arena.TZL[indice] = (void *)adrFirstBuddy;

            printf("FOUND BUDDY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            

            // Reset prev
            prev = NULL;
            addrBloc = arena.TZL[indice];
        }

        // va au prochain
        prev = addrBloc;
        addrBloc = (unsigned long *)*((unsigned long *)addrBloc);
    }

    // Il n'a pas été trouvé dans la liste ! on l'ajoute
    void * oldBloc = arena.TZL[indice];
    arena.TZL[indice] = a.ptr;

    (*(unsigned long *)a.ptr) = (unsigned long)oldBloc;

}


