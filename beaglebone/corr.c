int xcorr(int* waveA, int* waveB, int wavesize){
	int j,k;
	int corr=0,maxcorr=0,delay=0;

	for(j=0;j<2*wavesize;j++){
		corr=0;
		for(k=0;k<wavesize;k++){
			corr+=waveA[wavesize+k]*waveB[j+k];
		}
		if(corr>=maxcorr){
			maxcorr=corr;
			delay=j-wavesize;
		}
	}
	return delay;
}

