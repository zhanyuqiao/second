/*
 *encrypt.h
 *
 *Create on:2018.8.29
 *   Author:airport
 *
 */

#ifndef DECRYPT_H_
#define DECRYPT_H_

typedef struct _data_t {
	FILE *fpIn;
	FILE *fpOut;
	int num;
	unsigned char num_tmp[20];
	unsigned char de_serial_num_m[25];
	unsigned char de_serial_num_f[25];
	unsigned char flight_no[10];
	unsigned char password_en[150];
	unsigned char head_en_len_buff[10];
	unsigned char head_de[1024 * 10];
	int head_en_len;
	char location[10];
	unsigned char head_en[1024 * 10];
	int fligh_no_len;
}Data;

typedef struct _data_log {
	int len;
	FILE *fpIn;
	FILE *fpOut;
	unsigned char serial_num_f[25];
	unsigned char password_en[256];
	unsigned char password_de[128];
	unsigned char password_len[10];
	char location[10];
}DataLog;

int decrypt_file_parse_message(const char *path_in, const char *path_out, char *serial_num_m);
int decrypt_password(const char *key, const char *type);
int decrypt_head(const char *public_key, char *serial_num_f);
char *decrypt_log_parse_message(const char *path_in, const char *path_out, char *flight_serial);
#endif
