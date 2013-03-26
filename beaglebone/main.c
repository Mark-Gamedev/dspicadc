#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <pthread.h>

static const char *device = "/dev/spidev2.0";
static uint8_t mode = 0;
static uint8_t bits = 16;
static uint32_t speed = 1000000;
static uint16_t delay = 0;
static int fd;

// max is 158
#define SZ 150

#define SMPSZ 500

extern int xcorr(int*, int*, int);
static int ch0b0[SMPSZ];
static int ch1b0[SMPSZ];
static int ch0b1[SMPSZ];
static int ch1b1[SMPSZ];
static int fillingB1;
static int fillCounter;

pthread_t thread;

void *performXCorr(){
	int d;
	if(fillingB1){
		d = xcorr(ch1b0, ch0b0, SMPSZ);
	}else{
		d = xcorr(ch1b1, ch0b1, SMPSZ);
	}
	printf("%d\n", d);
	return 0;
}

static void pabort(const char *s){
	perror(s);
	abort();
}

void processData6(uint8_t *data){
	int val2 = data[5] << 8 | data[4];
	int val1 = data[3] << 8 | data[2];
	int val0 = data[1] << 8 | data[0];
	printf("%04x %04x %04x\n", val0, val1, val2);
}

void processData2(uint8_t *data){
	int raw = data[1] << 8 | data[0];
	int scanCounter = (raw >> 10) & 0b11;
	int sampleCounter = (raw >> 12) & 0xF;
	int value = raw & 0x3FF;
	//if(raw & 0x8000){
	//	printf("samples in buffer: %d  %04x\n", raw & 0x7FFF, raw & 0x7FFF);
	//}else{
		printf("%04d %01d %04d    %04x\n", sampleCounter, scanCounter, value, raw);
	//}
}

void processDataTestAcc(uint8_t *data){
	int raw = data[1] << 8 | data[0];
	int scanCounter = (raw >> 10) & 0b11;
	int value = raw & 0x3FF;
	if(scanCounter == 0){
		if(value > 400){
			printf("KNOCK!!!\n");
		}
	}
}

void processDataForXcorr(uint8_t *data){
	int raw = data[1] << 8 | data[0];
	int scanCounter = (raw >> 10) & 0b11;
	int value = raw & 0x3FF;

	int *ch0, *ch1;
	if(fillingB1){
		ch0 = (int*)ch0b1;
		ch1 = (int*)ch1b1;
	}else{
		ch0 = ch0b0;
		ch1 = ch1b0;
	}

	if(scanCounter==0){
		ch0[fillCounter >> 1] = value;
	}
	if(scanCounter==1){
		ch1[fillCounter >> 1] = value;
	}
	fillCounter++;
	if((fillCounter >> 1) == SMPSZ){
		//int d = xcorr(ch1, ch0, SMPSZ);
		//printf("%d\n", d);
		fillingB1 ^= 1;
		pthread_create(&thread, 0, performXCorr, 0);
		fillCounter = 0;
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

	int i;
	for(i=0;i<SZ;i+=2){
		//processData2(&rx[i]);
		processDataForXcorr(&rx[i]);
	}

	return 0;
}

int main(int argc, char *argv[]){

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

	//printf("spi mode: %d\n", mode);
	//printf("bits per word: %d\n", bits);
	//printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	while(1){
		transfer();
		//usleep(1000);
	}

	close(fd);

	return 0;
}
