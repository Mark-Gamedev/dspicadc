
int firstPeak(int *x, int size, int threshold){
	int i, currentVal;
	if(size<1){
		return -1;
	}
	currentVal = x[0];
	for(i=1;i<size;i++){
		if(currentVal < x[i]){
			currentVal = x[i];
		}else if(currentVal > threshold){
			return i;
		}
	}
	return -1;
}
