#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<openssl/evp.h>
#include<openssl/x509.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#include<json-c/json.h>
#include<time.h>
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#include<sys/utsname.h>
#include<math.h>

#include"decrypt.h"
#include"json_interface.h"

static DataLog cell_log={0};
static Data cell = {0};

static int decrypt_source(unsigned char *password_de, FILE *fpIn, FILE *fpOut, int total_en_len, int base);
static int decrypt_file_get_password_en(Data *cell);

int decrypt_file_parse_message(const char *path_in, const char *path_out, char *serial_num_m) {
	if(path_in == NULL || path_out == NULL) {
		LOGERROR("input is null");
		return -1;
	}
	//打开待解密的密文文件
	cell.fpIn = fopen(path_in, "rb");
	if(cell.fpIn == NULL) {
		LOGERROR("open package_en fail");
		return -1;
	}
	fseek(cell.fpIn, 0, SEEK_SET);
	//打开保存明文的文件
	cell.fpOut = fopen(path_out, "ab+");
	if(cell.fpOut == NULL) {
		LOGERROR("open package_de fail");
		fclose(cell.fpIn);
		return -1;
	}
	fseek(cell.fpOut, 0, SEEK_SET);
	//获取本机飞机号
	if(gethostname(cell.flight_no, 10) != 0) {
		LOGERROR("get fligh no. fail\n");
		return -1;
	}
	cell.fligh_no_len = strlen(cell.flight_no);
	LOGINFO("flight_no:%s", cell.flight_no);
	//memcpy(cell.flight_no, "BC-4001", 7);
	//cell.fligh_no_len = strlen(cell.flight_no);
	//判断文件的位置
	if(strstr(path_in, ".ihm") != NULL) {
		strcpy(cell.location, LOCATION_FLIGHT);
	} else if(strstr(path_in, ".zip") != NULL) {
		strcpy(cell.location, LOCATION_CENTER);
	} else {
		LOGERROR("file miss");
		return -1;
	}
	//获取飞机的个数
	fread(cell.num_tmp, 1, 4, cell.fpIn);
	cell.num = atoi(cell.num_tmp);
	LOGINFO("飞机数量：%d", cell.num);
	//LOGINFO("the amount of aircarft:%d", cell.num);
	//获取地面序列号
	fseek(cell.fpIn, 4, SEEK_SET);
	fread(cell.de_serial_num_m, 1, 16, cell.fpIn);
	LOGINFO("serial_num_m:%s", cell.de_serial_num_m);
	memcpy(serial_num_m, cell.de_serial_num_m, 16);
	fseek(cell.fpIn, 20, SEEK_SET);

	if(fread(cell.head_en_len_buff, 1, 8, cell.fpIn) < 0) {
		fclose(cell.fpIn);
		fclose(cell.fpOut);
		LOGERROR("get head_en len fail");
		return -1;		
	}
	cell.head_en_len = atoi(cell.head_en_len_buff);
	LOGINFO("获取二次加密后头的长度：%d", cell.head_en_len);
	fseek(cell.fpIn, 28, SEEK_SET);
	if(fread(cell.head_en, 1, cell.head_en_len, cell.fpIn) < 0) {
		fclose(cell.fpIn);
		fclose(cell.fpOut);
		LOGERROR("read head_en fail");
		return -1;	
	}
    return 0;
}

int decrypt_head(const char *public_key, char *serial_num_f) {
	//LOGINFO("start decrypt head");
	if(public_key == NULL) {
		LOGERROR("input is null");
		return -1;
	}
	RSA *rsa_public = NULL;
	FILE *fp_public;
	unsigned char *de = NULL;
	int rsa_public_len;
	int i;
	if((fp_public = fopen(public_key, "rb")) == NULL) {
		LOGERROR("open public key fail");
		return -1;
	}
	if((rsa_public = PEM_read_RSAPublicKey(fp_public, NULL, NULL, NULL)) == NULL) {
		fclose(fp_public);
		LOGERROR("read public key fail");
		return -1;
	}
	rsa_public_len = RSA_size(rsa_public);
	double times = ceil((double)cell.head_en_len / 128);
	de = (unsigned char *)malloc( 1024 * 10 );
	memset(de, 0, 1024 * 10);
	for(i = 0; i < times; i++) {
		if(RSA_public_decrypt(rsa_public_len, cell.head_en+i*128, de+i*128, rsa_public, RSA_NO_PADDING) < 0) {
			LOGERROR("public key decrypt fail");
			fclose(fp_public);
			RSA_free(rsa_public);
			free(de);
			return -1;
		}
	}
	RSA_free(rsa_public);
	fclose(fp_public);
	memcpy(cell.head_de, de, strlen(de));
	//LOGINFO("head_de:%s", cell.head_de);
	if(decrypt_file_get_password_en(&cell) != 0) {
		LOGERROR("decrypt file get password_en fail");
		free(de);
		return -1;
	}
	memcpy(serial_num_f, cell.de_serial_num_f, strlen(cell.de_serial_num_f));
	free(de);
	LOGINFO("decrypt head successed");
	return 0;
}

static int decrypt_file_get_password_en(Data *cell) {
	unsigned char head_alone[256] = { 0 };
	char *final = strstr(cell->head_de, cell->flight_no);
	if(final == NULL) {
		fclose(cell->fpIn);
		fclose(cell->fpOut);
		LOGERROR("there is no corresponding");
		return -1;
	}
	memcpy(cell->de_serial_num_f, final+cell->fligh_no_len, 16);
	LOGINFO("de_serial_num_f:%s", cell->de_serial_num_f);
	memcpy(cell->password_en, final+cell->fligh_no_len+16, 128);
	return 0;
}

int decrypt_password(const char *key, const char *type) {
	if(strcmp(type, "pkg") == 0) {
		if(key == NULL || type == NULL) {
			LOGERROR("input is null");
			return -1;
		}
		RSA *rsa_private = NULL;
		FILE *fp_private;
		unsigned char *de = NULL;
		int rsa_private_len;
		if((fp_private = fopen(key, "rb")) == NULL) {
			LOGERROR("open private key fail");
			return -1;
		}
		if((rsa_private = PEM_read_RSAPrivateKey(fp_private, NULL, NULL, NULL)) == NULL) {
			fclose(fp_private);
			LOGERROR("read PrivateKey fail");
			return -1;
		}
		rsa_private_len = RSA_size(rsa_private);
		de = (unsigned char *)malloc(rsa_private_len + 1);
		memset(de, 0, rsa_private_len + 1);
		if(RSA_private_decrypt(rsa_private_len, cell.password_en, de, rsa_private, RSA_NO_PADDING) < 0) {
			free(de);
			fclose(fp_private);
			RSA_free(rsa_private);
			return -1;
		}
		LOGINFO("password_de:%s", de);
		RSA_free(rsa_private);
		fclose(fp_private);
		if(decrypt_source(de, cell.fpIn, cell.fpOut, cell.head_en_len, 28) != 0) {
			free(de);
			LOGERROR("decrypt package fail");
			return -1;
		}
		free(de);
		fclose(cell.fpIn);
		fclose(cell.fpOut);
	} else if(strcmp(type, "log") == 0) {
		if(key == NULL || type == NULL) {
			LOGERROR("input is null");
			return -1;
		}
		RSA *rsa_public = NULL;
		FILE *fp_public;
		unsigned char *de = NULL;
		int rsa_public_len;
		if((fp_public = fopen(key, "rb")) == NULL) {
			LOGERROR("open private key fail");
			return -1;
		}
		if((rsa_public = PEM_read_RSAPublicKey(fp_public, NULL, NULL, NULL)) == NULL) {
			fclose(fp_public);
			LOGERROR("read PublicKey fail");
			return -1;
		}
		rsa_public_len = RSA_size(rsa_public);
		de = (unsigned char *)malloc(rsa_public_len + 1);
		memset(de, 0, rsa_public_len + 1);

		if(RSA_public_decrypt(rsa_public_len, cell_log.password_en, de, rsa_public, RSA_NO_PADDING) < 0) {
			free(de);
			fclose(fp_public);
			RSA_free(rsa_public);
			return -1;
		}
		RSA_free(rsa_public);
		fclose(fp_public);
		if(decrypt_source(de, cell_log.fpIn, cell_log.fpOut, 128, 16) != 0) {
			free(de);
			LOGERROR("decrypt log fail");
			return -1;
		}
		free(de);
		fclose(cell_log.fpIn);
		fclose(cell_log.fpOut);
	}
	return 0;
}

static int decrypt_source(unsigned char *password_de, FILE *fpIn, FILE *fpOut, int total_en_len, int base) {
	OpenSSL_add_all_algorithms();
	LOGINFO("start decrypt source");
	if(password_de == NULL || fpIn == NULL || fpOut == NULL || total_en_len == 0 || base == 0) {
		LOGINFO("input is null");
		return -1;
	}
	char iv[EVP_MAX_KEY_LENGTH];  //保存初始化向量的数组
	EVP_CIPHER_CTX ctx;    //EVP加密上下文环境
	char out[1024+EVP_MAX_KEY_LENGTH]; //保存解密后明文的缓冲区数组
	int outl;
	char in[1024];    //保存密文数据的数组
	int inl;
	int rv;
	int i;
	//设置iv
	for(i = 0; i < 8; i++) {
		iv[i]=i;
	}
	//初始化ctx
	EVP_CIPHER_CTX_init(&ctx);
	//设置解密的算法、key和iv
	rv = EVP_DecryptInit_ex(&ctx,EVP_des_ede3_cbc(), NULL, password_de, iv);
	if(rv != 1) {
		LOGERROR("decrypt init fail");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}
	fseek(fpIn, base+total_en_len, SEEK_SET);
	//循环读取原文，解密后后保存到明文文件。
	for(;;)
	{
		inl = fread(in,1,1024,fpIn);
		if(inl <= 0)
			break;
		rv = EVP_DecryptUpdate(&ctx,out,&outl,in,inl);//解密
		if(rv != 1)
		{
			LOGERROR("decrypt update fail");
			EVP_CIPHER_CTX_cleanup(&ctx);
			return -1;
		}
		fwrite(out,1,outl,fpOut);//保存明文到文件
	}
	//解密结束
	rv = EVP_DecryptFinal_ex(&ctx,out,&outl);
	if(rv != 1)
	{
		LOGERROR("decrypt final fail");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}
	fwrite(out,1,outl,fpOut);//保存明文到文件
	EVP_CIPHER_CTX_cleanup(&ctx);//清除EVP加密上下文环境
	LOGINFO("decrypt source successed");
	return 0;
}

char *decrypt_log_parse_message(const char *path_in, const char *path_out, char *flight_serial) {
	if(path_in == NULL || path_out == NULL || flight_serial == NULL) {
		LOGERROR("input is null");
		return NULL;
	}
	//打开待解密的密文文件
	cell_log.fpIn = fopen(path_in, "rb");
	if(cell_log.fpIn == NULL) {
		LOGERROR("open log_en fail");
		return NULL;
	}
	fseek(cell_log.fpIn, 0, SEEK_SET);
	//打开保存明文的文件
	cell_log.fpOut = fopen(path_out, "ab+");
	if(cell_log.fpOut == NULL) {
		LOGERROR("open log_de fail");
		fclose(cell.fpIn);
		return NULL;
	}
	fseek(cell_log.fpOut, 0, SEEK_SET);
	//判断文件的位置
	if(strstr(path_in, ".ihm") != NULL) {
		strcpy(cell_log.location, LOCATION_FLIGHT);
	} else if(strstr(path_in, ".zip") != NULL) {
		strcpy(cell_log.location, LOCATION_CENTER);
	} else {
		LOGERROR("file miss");
		return NULL;
	}
	//获取飞机序列号
	fread(cell_log.serial_num_f, 1, 16, cell_log.fpIn);
	LOGINFO("serial_num_f:%s", cell_log.serial_num_f);
	strcpy(flight_serial, cell_log.serial_num_f);
	//读取待解密密码
	fseek(cell_log.fpIn, 16, SEEK_SET);
	if(fread(cell_log.password_en, 1, 128, cell_log.fpIn) < 0) {
		fclose(cell_log.fpIn);
		fclose(cell_log.fpOut);
		LOGERROR("read log_password_en fail");
		return NULL;	
	}
    return flight_serial;
}
