#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
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


#define SZ 2
#define STARTWORD 0xFABC

static const char *device = "/dev/spidev2.0";
static uint8_t mode = 0;
static uint8_t bits = 16;
static uint32_t speed = 1000000;
static uint16_t delay = 0;
static int fd;

static void pabort(const char *s){
	perror(s);
	abort();
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


void spiCleanup(){
	close(fd);
}

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

