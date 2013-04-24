#include "processData.h"

void performThreshold(int *ch0, int *ch1, int *ch2, int chsz, int *index0, int *index1, int *index2){
	int i;

	*index0 = *index1 = *index2 = 0;

	for(i=0;i<chsz;i++){
		if(ch0[i] > THRESHOLD){
			*index0 = i;
			break;
		}
	}
	
	for(i=0;i<chsz;i++){
		if(ch1[i] > THRESHOLD){
			*index1 = i;
			break;
		}
	}
	
	for(i=0;i<chsz;i++){
		if(ch2[i] > THRESHOLD){
			*index2 = i;
			break;
		}
	}
}
