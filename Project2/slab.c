#include "slab.h"
#include "stdlib.h"

/* CPSC 3220 - Spring 2017
 * Project 3 - Slab memory allocator
 * Andrew Samuels - absamue
 *
 * This project involved writing an allocate and release function for a
 * slab memory allocator. In our implementation, slab allocation uses bit
 * masks to determine the location of the next available object to be
 * allocated. This allows for allocated objects to be packed together,
 * effectively eliminating fragmentation issues, and simplifying finding
 * free space.
 */


/* slab_allocate() will determine the location of the next available object
 * if there is one. If it successfully finds an object, it will set the bit
 * masks to indicate an object has become unavailable. The function will
 * also se the information object of the current slab to hold the correct
 * information.
 *
 * The function will then return a pointer to the location of the allocated
 * object that was selected. If there were no available objects, the
 * function will  return NULL
 */
unsigned char *slab_allocate(){
	int i;
	int pos;
	int object;

	if(full_mask == 0xffff)
		return NULL;


	//choose first available slab
	pos = -1;
	if(partial_mask != 0x0000){
		//check partial 
		for(i = 0; i<16; i++){
			if(((partial_mask >> (15-i)) & 1) == 1){
				pos = i;
				break;
			}
		}
	}
	//no partial, find first empty
	else{
		for(i = 0; i<16; i++){
			if(((empty_mask >> (15-i)) & 1) == 1){
				pos = i;
				break;
			}
		}			
	}

	//find first free object
	for(i=1; i<16; i++){
		if(((s[pos].free_mask >> (15-i)) & 1) == 1){
			object = i;
			break;
		}
	}

	//reduce count
	s[pos].free_count--;

	//if slab is full mark it in masks
	if(s[pos].free_count == 0){
		full_mask |= (1 << (15 - pos));
		partial_mask &= ~(1 << (15 - pos));
		empty_mask &= ~(1 << (15 - pos));
	}
	//otherwise update partial mask
	else{
		//set partial mask
		partial_mask |= (1 << (15 - pos));
		//clear empty and full mask
		empty_mask &= ~(1 << (15 - pos));
		full_mask &= ~(1 << (15 - pos));
	}

	//set slab info
	//set free mask to 0 at current position
	s[pos].free_mask &= ~(1 << (15 - object));

	//set free count bits
	for(i=0; i < 16; i++){
		s[pos].free_space[i] = (s[pos].free_count >> 15-i) & 1;
	}

	//set free mask bisk
	for(i=16; i<32; i++){
		s[pos].free_space[i] = (s[pos].free_mask >> 31-i) & 1;
	}

	//set signature bits
	for(i=32; i<64; i++){
		s[pos].free_space[i] = (s[pos].signature >> 63-i) & 1;
	}

	//return the slab at start + slab # + object #
	return start + (pos * 4096) + ((15 - s[pos].free_count) * 256);
}

/* slab_release(unsigned char *addr) will first check whether or not the
 * object requested to be released is a valid object. If the given address
 * is outside of the memory pool, or is not alinged to an object, the
 * function will return error code 1. If the address is that of an object
 * that has already been freed, the function will return error code 2.
 *
 * After completion of sanity checking, the function will set the bit masks
 * to their correct values, and then return 0 indicating a successful
 * release.
 */

int slab_release(unsigned char *addr){

	//check for slab outside of range
	if(addr < start || addr > (start + 61440))
		return 1;

	//get slab position from s[]
	int pos = addr - start;
	pos /= 4096;
	if(pos < 0 || pos > 15)
		return 1;

	//check signature
	if(s[pos].signature != 0x51ab51ab)
		return 1;
	//manually check signature bits as well
	int i;
	for(i=32; i<64; i++){
		if(s[pos].free_space[i] != ((0x51ab51ab >> 63-i) & 1))
			return 1;
	}

	//check free mask
	int object = addr - start;
	object %= 4096;
	object /= 256;
	if(object < 1 || object > 15)
		return 1;
	if(((s[pos].free_mask >> (15-object)) & 1) == 1){
		return 2;
	}
	
	//update count
	s[pos].free_count++;

	//if slab is empty mark it in masks
	if(s[pos].free_count == 15){
		full_mask &= ~(1 << (15 - pos));
		partial_mask &= ~(1 << (15 - pos));
		empty_mask |= (1 << (15 - pos));
	}
	//otherwise update partial mask
	else{
		//set partial mask
		partial_mask |= (1 << (15 - pos));
		//clear empty and full mask
		empty_mask &= ~(1 << (15 - pos));
		full_mask &= ~(1 << (15 - pos));
	}

	//update free mask to have a 1 at object position
	s[pos].free_mask |= (1 << (15 - object));

	//successfull release
	return 0;

}
