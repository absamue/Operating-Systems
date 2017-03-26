#include "slab.h"

unsigned char *slab_allocate(){
	int i;
	int pos = 0;
	int partial;
	int empty;

	if(full_mask == 0xffff)
		return NULL;


	pos = -1;
	if(partial_mask != 0x0000){
		//check partial 
		for(i = 0; i<16; i++){
			if((partial_mask >> 15-i) & 1 == 1){
				pos = i;
				partial = 1;
				break;
			}
		}
	}
	//no partial, find first empty
	else{
		for(i = 0; i<16; i++){
			if((empty_mask >> 15-i) & 1 == 1){
				pos = i;
				empty = 1;
				break;
			}
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
	s[pos].free_mask &= ~(1 << (s[pos].free_count));

	//set free count bits
	for(i=0; i < 16; i++){
		s[pos].free_space[15-i] = (s[pos].free_count >> i) & 1;
	}
	
	//set free mask
	for(i=16; i<32; i++){
		s[pos].free_space[31-i] = (s[pos].free_mask >> i) & 1;
	}

	//set signature
	for(i=32; i<64; i++){
		s[pos].free_space[63-i] = (s[pos].free_mask >> i) & 1;
	}

	//return the slab at correct position
	return s[pos].free_space + ((15 - s[pos].free_count) * 256);
}

int slab_release(unsigned char *addr){
	
	
	return 0;

}
