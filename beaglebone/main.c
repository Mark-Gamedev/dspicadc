#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static const char *device = "/dev/spidev2.0";
static uint8_t mode = 0;
static uint8_t bits = 16;
static uint32_t speed = 1000000;
static uint16_t delay = 0;
static int fd;

// max is 158
//#define SZ 150
#define SZ 2

#define SMPSZ 512

#define STARTWORD 0xFABC

extern int xcorr(int*, int*, int, double*);
#define THRESHOLD 530
extern int firstPeak(int*, int, int);
extern int maxIndex(double *, int , double *);
extern void plotWithGnuplot(char *, ...);
extern void saveBufferToFile(int *buf, int size, char* path);

extern void startServer();
extern void sendToServer(char *data, int len);

static void pabort(const char *s);
int saveBufferFromSpi(int**, int*);

int delayData[9][2] = {
	{8, 6}, {2, 5}, {-4, 3},
	{4, 1}, {-2, -2}, {-8, -1},
	{2, -1}, {-4, -8}, {-8, -6}};

int hexDumpCount;
void hexDump(int x){
	if(!x){
		return;
	}
	if(x==STARTWORD){
		printf("#### ");
	}else{
		printf("%04x ", x);
	}
	hexDumpCount++;
	if((hexDumpCount & 15) == 0){
		printf("\n");
	}
}

int spiGetInt16(){
	uint8_t tx[2] = {0x00, };
	uint8_t rx[2] = {0x00, };

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
	};

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1){
		pabort("can't send spi message");
	}
	ret = *((int *)(&rx[0])) & 0xFFFF;
	//hexDump(ret);
	return ret;
}

void spiInit(){
	int ret = 0;

	fd = open(device, O_RDWR);
	if (fd < 0){
		pabort("can't open device");
	}
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1){
		pabort("can't set mode");
	}

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1){
		pabort("can't get mode");
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1){
		pabort("can't set bit per word");
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1){
		pabort("can't get bit per word");
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		pabort("can't set max speed hz");
	}

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		pabort("can't get max speed hz");
	}
}

void spiCleanup(){
	close(fd);
}

static void pabort(const char *s){
	perror(s);
	abort();
}

void printDataPoint(short *data, int size){
	int i = data[0] & 0xFFFF;
	if(i&0x8000){
		if((i & 0xFFED) == 0xFFED){
			exit(0);
		}
		if((i & 0xFBC0) == 0xFBC0){
			printf("########\n");
		}else{
			i &= 0x7FFF;
			printf("%d\n", i);
		}
	}
}

int transfer(){
	uint8_t tx[SZ] = {0x00, };
	uint8_t rx[SZ] = {0x00, };

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = SZ,
		.delay_usecs = delay,
		//.cs_change = 1,
		//.speed_hz = 0,
		//.bits_per_word = 0,
	};

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1){
		pabort("can't send spi message");
	}

	//processDataTimeDelay((short*)rx);
	//printDataPoint((short*)rx, SZ>>1);

	return 0;
}

int saveBufferFromSpi(int** outPtr, int* outCount){
	int word, count, *buffer, i;

	if(!outPtr){
		return -1;
	}
	
	while(word != STARTWORD){
		word = spiGetInt16();
	}
	count = spiGetInt16();
	buffer = calloc(count, sizeof(int));

	for(i=0;i<count;i++){
		buffer[i] = spiGetInt16();
	}

	*outPtr = buffer;
	if(outCount){
		*outCount = count;
	}
	return count;
}

void performFirstPeak(){
	int *buf, *ch0, *ch1, *ch2, index0, index1, index2;
	int sz, chsz;
	printf("waiting for knock...\n");
	saveBufferFromSpi(&buf, &sz);
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];
	index0 = firstPeak(ch0, chsz, THRESHOLD);
	index1 = firstPeak(ch1, chsz, THRESHOLD);
	index2 = firstPeak(ch2, chsz, THRESHOLD);
	printf("i0: %d, i1: %d, i2: %d\n", index0, index1, index2);

	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");
}

void performXCorr(int *ch0, int *ch1, int *ch2, int chsz, int *d01, int *d02, int *d12){
	double *corr, maxVal;

	int sz = chsz*2;
	corr = calloc(sz, sizeof(double));

	xcorr(ch0, ch1, chsz, corr);
	*d01 = maxIndex(corr, sz, &maxVal);
	*d01 -= chsz;

	xcorr(ch0, ch2, chsz, corr);
	*d02 = maxIndex(corr, sz, &maxVal);
	*d02 -= chsz;

	xcorr(ch1, ch2, chsz, corr);
	*d12 = maxIndex(corr, sz, &maxVal);
	*d12 -= chsz;
}

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

#define MARGIN 2
int calculateCoord(int d0, int d1){
	int i;
	for(i=0;i<9;i++){
		if(d0 >= delayData[i][0]-MARGIN && d0 <= delayData[i][0]+MARGIN && 
				d1 >= delayData[i][1]-MARGIN && d1 <= delayData[i][1]+MARGIN){
			return i;
		}
	}
	return -1;
}

void printCoord(int x){
	int i;
	for(i=0;i<9;i++){
		if(i==x){
			printf("    X    ");
		}else{
			printf("    -    ");
		}
		if(i==2||i==5||i==8){
			printf("\n\n\n");
		}
	}
}

void runOnce(){
	int *buf, *ch0, *ch1, *ch2;
	int d0, d1, d2;
	int sz, chsz;
	int msg[3];

	printf("ready...");
	fflush(stdout);

	saveBufferFromSpi(&buf, &sz);
	printf("processing...\n");
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];

	performXCorr(ch0, ch1, ch2, chsz, &d0, &d1, &d2);
	performThreshold(ch0, ch1, ch2, chsz, &d0, &d1, &d2);

	msg[0] = d0;
	msg[1] = d1;
	msg[2] = d2;
	sendToServer((char*)msg, sizeof(msg));


	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");

	//plotWithGnuplot("sss", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2");

	int i;
	for(i=0;i<20;i++){
		fputs("\033[A\033[2K", stdout);
	}
	//rewind(stdout);
	printf("%d %d\n", d0-d2, d1-d2);
	printCoord(calculateCoord(d0-d2, d1-d2));
}

int main(int argc, char *argv[]){
	printf("connecting to SPI...");
	fflush(stdout);
	spiInit();
	printf("done\n");

	printf("waiting for TCP connection...");
	fflush(stdout);
	//startServer();
	printf("done\n");

	while(1){
		runOnce();
	}

	spiCleanup();
	return 0;
}

