#include "slab.h"
#include "stdlib.h"

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
		s[pos].free_space[15-i] = (s[pos].free_count >> i) & 1;
	}
	
	//set free mask
	for(i=16; i<32; i++){
		s[pos].free_space[31-i] = (s[pos].free_mask >> i) & 1;
	}

	//set signature
	for(i=32; i<64; i++){
		s[pos].free_space[63-i] = (s[pos].signature >> i) & 1;
	}

	//return the slab at start + slab # + object #
	return start + (pos * 4096) + ((15 - s[pos].free_count) * 256);
}

int slab_release(unsigned char *addr){

	//check for slab outside of range
	if(addr < start || addr > (start + 61440))
		return 1;

	//get slab position from s[]
	int pos = addr - start;
	pos /= 4096;

	//check signature
	if(s[pos].signature != 0x51ab51ab)
		return 1;

	//check free mask
	int object = addr - start;
	object %= 4096;
	object /= 256;
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

	//update free mask
	s[pos].free_mask |= (1 << (15 - object));

	return 0;

}
