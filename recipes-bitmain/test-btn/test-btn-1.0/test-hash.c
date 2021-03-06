#include <linux/input.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "test-hash.h"

#define devpath "/dev/bitmain-asic"
#define rsult_file "/tmp/test_result.data"

// --------------------------------------------------------------
//      CRC16 check table
// --------------------------------------------------------------
const uint8_t chCRCHTalbe[] =                                 // CRC high byte table
{
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
 0x00, 0xC1, 0x81, 0x40
};

const uint8_t chCRCLTalbe[] =                                 // CRC low byte table
{
 0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
 0x41, 0x81, 0x80, 0x40
};
uint16_t CRC16(const uint8_t* p_data, uint16_t w_len)
{
	uint8_t chCRCHi = 0xFF; // CRC high byte initialize
	uint8_t chCRCLo = 0xFF; // CRC low byte initialize
	uint16_t wIndex = 0;    // CRC cycling index

	while (w_len--) {
		wIndex = chCRCLo ^ *p_data++;
		chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
		chCRCHi = chCRCLTalbe[wIndex];
	}
	return ((chCRCHi << 8) | chCRCLo);
}
int parse_status_data(uint8_t *status_data, uint16_t datalen)
{
	struct BITMAIN_STATUS_DATA *p_status_data = (struct BITMAIN_STATUS_DATA *)status_data;
	struct BITMAIN_STATUS_DATA bm_status_data;
	struct BITMAIN_STATUS_DATA *bm = &bm_status_data;
	uint16_t dataindex;
	uint16_t crc;
	uint16_t i,j;
	int asic_num = 0;
	int rsult_fd = -1;
	if(p_status_data->data_type != BM_STATUS_DATA)
	{
		printf("don't match data type\n");
		return -1;
	}
	if (p_status_data->length+4 != datalen) {
		printf("parse_rxstatus length(%d) datalen(%d) error", p_status_data->length, datalen);
		return -1;
	}
	memcpy(bm, status_data, 28);
	crc = CRC16(status_data, datalen-2);
	memcpy(&(bm->crc), status_data+datalen-2, 2);
	if(crc != bm->crc) {
		printf("bitmain_parse_rxstatus check crc(%d) != bm crc(%d) datalen(%d)", crc, p_status_data->crc, datalen);
		return -1;
	}

	dataindex = 28;
	if(bm->chain_num > 0) {
		memcpy(bm->chain_asic_num, status_data+datalen-2-bm->chain_num-bm->temp_num-bm->fan_num, bm->chain_num);
	}
	for(i = 0; i < bm->chain_num; i++) {
		asic_num = bm->chain_asic_num[i];
		if(asic_num <= 0) {
			asic_num = 1;
		} else {
			if(asic_num % 32 == 0) {
				asic_num = asic_num / 32;
			} else {
				asic_num = asic_num / 32 + 1;
			}
		}
		memcpy((uint8_t *)&bm->chain_asic_exist[i][0], status_data+dataindex, asic_num*4);
		dataindex += asic_num*4;
	}
	for(i = 0; i < bm->chain_num; i++) {
		asic_num = bm->chain_asic_num[i];
		if(asic_num <= 0) {
			asic_num = 1;
		} else {
			if(asic_num % 32 == 0) {
				asic_num = asic_num / 32;
			} else {
				asic_num = asic_num / 32 + 1;
			}
		}
		memcpy((uint8_t *)&bm->chain_asic_exist[i][0], status_data+dataindex, asic_num*4);
		dataindex += asic_num*4;
	}
	dataindex += bm->chain_num;
	if(dataindex + bm->temp_num + bm->fan_num + 2 != datalen) {
		printf("bitmain_parse_rxstatus dataindex(%d) chain_num(%d) temp_num(%d) fan_num(%d) not match datalen(%d)",
				dataindex, bm->chain_num, bm->temp_num, bm->fan_num, datalen);
		return -1;
	}
	if(bm->temp_num > 0) {
		memcpy(bm->temp, status_data+dataindex, bm->temp_num);
		dataindex += bm->temp_num;
	}
	if(bm->fan_num > 0) {
		memcpy(bm->fan, status_data+dataindex, bm->fan_num);
		dataindex += bm->fan_num;
	}
	rsult_fd = open(rsult_file, O_RDWR|O_EXCL|O_NONBLOCK|O_APPEND);
	printf("BitMain RxStatusData: chipv_e(%d) chainnum(%d) fifos(%d) v(%#x) fann(%d) tempn(%d) fanet(%04x) tempet(%08x) ne(%d) regvalue(%d) crc(%04x)\n",
			bm->chip_value_eft, bm->chain_num, bm->fifo_space, bm->hw_version, bm->fan_num, bm->temp_num, bm->fan_exist, bm->temp_exist, bm->nonce_err, bm->reg_value, bm->crc);
	if(bm->chain_num == 0)
		write(rsult_fd, "H: no chain", sizeof("H: no chain"));
	else
	{
		printf("BitMain RxStatus Data chain info:");
		write(rsult_fd, "H:", 2);
		for(i = 0; i < bm->chain_num; i++) {
			printf("BitMain RxStatus Data chain(%d) asic num=%d asic_exist=%08x asic_status=%08x\n", i+1, bm->chain_asic_num[i], bm->chain_asic_exist[i][0], bm->chain_asic_status[i][0]);
			if(bm->chain_asic_exist[i][0] != 0)
				write(rsult_fd, " O", 2);
			else
				write(rsult_fd, " X", 2);
		}
	}
	if(bm->fan_num == 0)
		write(rsult_fd, "\nF: no fan", sizeof("\nF: no fan"));
	else
	{
		printf("BitMain RxStatus Data fan info:\n");
		write(rsult_fd, "\nF:", 3);
		for(i = 0; i < bm->fan_num; i++) {
			printf("BitMain RxStatus Data fan(%d) fan=%d\n", i+1, bm->fan[i]);
			if(bm->fan[i] != 00)
				write(rsult_fd, " O", 2);
			else
				write(rsult_fd, " X", 2);
		}
	}
	if(bm->temp_num == 0)
		write(rsult_fd, "\nT: no temp", sizeof("\nT: no temp"));
	else
	{
		printf("BitMain RxStatus Data temp info:\n");
		write(rsult_fd, "\nT:", 3);
		for(i = 0; i < bm->temp_num; i++) {
			printf("BitMain RxStatus Data temp(%d) temp=%d\n", i+1, bm->temp[i]);
			if((bm->temp[i] !=0) && (bm->temp[i] !=0xff))
				write(rsult_fd, " O", 2);
			else
				write(rsult_fd, " X", 2);
		}
	}
	write(rsult_fd, "\n", 1);
	close(rsult_fd);
	return 0;
}


int main(int argc, char **argv)
{
	struct BITMAIN_GET_STATUS get_status;
	struct BITMAIN_STATUS_DATA status_data;
	int read_len;
	int fd = -1;
	unsigned int i;
	printf("test-hash compile %s--%s\n", __DATE__,__TIME__);
	i = 0;
	while( i++ < 3)
	{
		fd = open(devpath, O_RDWR|O_EXCL|O_NONBLOCK);
		if(fd == -1) {
			printf("open %s error %d\n",devpath, errno);
			//return -1;
			system("/etc/init.d/bmminer.sh stop");
			sleep(1);
		}
		else
			break;
	}
	if(i == 3)
		return -1;
	memset(&get_status, 0x00, sizeof(get_status));
	//detect chain
	get_status.token_type = BM_GET_STATUS;
	get_status.length = sizeof(get_status) - 4;
	get_status.detect_get = 1;
	get_status.crc = CRC16((uint8_t *)&get_status, sizeof(get_status)-2);
	write(fd, &get_status, sizeof(get_status));
	//get hash data
	get_status.token_type = BM_GET_STATUS;
	get_status.length = sizeof(get_status) - 4;
	get_status.detect_get = 0;
	get_status.test_hash = 0xba;
	get_status.crc = CRC16((uint8_t *)&get_status, sizeof(get_status)-2);
	for(i=0; i < 10; i++)
	{
		write(fd, &get_status, sizeof(get_status));
		usleep(100*1000); //ms
	}
	read_len = read(fd, &status_data, sizeof(status_data));
	parse_status_data((uint8_t*)&status_data, read_len);
	close(fd);
	return 0;
}

