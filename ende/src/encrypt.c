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

#include"encrypt.h"
#include"json_interface.h"

static int Rand_char(unsigned char *tmp);
static unsigned char *Encrypt_Password(const char *public_key, unsigned char *password);
static unsigned char *Encrypt_Password_Private(const char *private_key, unsigned char *password);
static unsigned char *Encrypt_Head(unsigned char *str, const char *private_key, int total_len);
static int Encrypt_Source(unsigned char *password, FILE *fpIn, FILE *fpOut);

static int Rand_char(unsigned char *tmp) {
	unsigned char pool[]=
	{
		'0','1','2','3','4','5','6','7','8','9',
		'a','b','c','d','e','f','g','h','i','j',
		'k','l','m','n','o','p','q','r','s','t',
		'u','v','w','x','y','z','A','B','C','D',
		'E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','U','V','W','X',
		'Y','Z'
	};
	unsigned char pwd[9] = { 0 };
	int i=0;
	for(i = 0; i < 8; i++)
	{
		pwd[i]=pool[rand()%sizeof(pool)];
	}
	pwd[8] = '\0';
	strcpy(tmp, pwd);
	return 0;
}

static unsigned char *Encrypt_Password(const char *public_key, unsigned char *password) {
	if(public_key == NULL || password == NULL) {
		LOGERROR("public_key is null");
		return NULL;
	}
	RSA *rsa_public = NULL;
	FILE *fp_public;
	int rsa_public_len = 0;
	if((fp_public = fopen(public_key, "rb")) == NULL) {
		LOGERROR("open public key fail");
		return NULL;
	}
	if((rsa_public = PEM_read_RSAPublicKey(fp_public, NULL, NULL, NULL)) == NULL) {
		LOGERROR("read public key fail");
		fclose(fp_public);
		return NULL;
	}
	rsa_public_len = RSA_size(rsa_public);
	unsigned char *pass_en = (unsigned char *)malloc(256);
	memset(pass_en, 0, 256);
	if(RSA_public_encrypt(rsa_public_len, password, pass_en, rsa_public, RSA_NO_PADDING) < 0) {
		fclose(fp_public);
		RSA_free(rsa_public);
		free(pass_en);
		LOGERROR("public key encrypt fail");
		return NULL;
	}
	RSA_free(rsa_public);
	fclose(fp_public);
	return pass_en;
}

static unsigned char *Encrypt_Password_Private(const char *private_key, unsigned char *password) {
	if(private_key == NULL || password == NULL) {
		LOGERROR("input is null");
		return NULL;
	}
	RSA *rsa_private = NULL;
	FILE *fp_private;
	int rsa_private_len = 0;
	if((fp_private = fopen(private_key, "rb")) == NULL) {
		LOGERROR("open private key fail");
		return NULL;
	}
	if((rsa_private = PEM_read_RSAPrivateKey(fp_private, NULL, NULL, NULL)) == NULL) {
		LOGERROR("read private key fail");
		fclose(fp_private);
		return NULL;
	}
	rsa_private_len = RSA_size(rsa_private);
	unsigned char *pass_en = (unsigned char *)malloc(256);
	memset(pass_en, 0, 256);
	if(RSA_private_encrypt(rsa_private_len, password, pass_en, rsa_private, RSA_NO_PADDING) < 0) {
		fclose(fp_private);
		RSA_free(rsa_private);
		free(pass_en);
		LOGERROR("private key encrypt fail");
		return NULL;
	}
	RSA_free(rsa_private);
	fclose(fp_private);
	return pass_en;
}

static unsigned char *Encrypt_Head(unsigned char *str, const char *private_key, int total_len) {
	//printf("str_len:%ld\n", strlen(str));
	if(str == NULL || private_key == NULL || total_len <= 0) {
		LOGERROR("str is null");
		return NULL;
	}
	RSA *rsa_private = NULL;
	FILE *fp_private;
	unsigned char *en = NULL;
	int rsa_private_len = 0;
	int i;
	if((fp_private = fopen(private_key, "rb")) == NULL) {
		LOGERROR("Encrypt_Head(): open private key fail");
		return NULL;
	}
	if((rsa_private = PEM_read_RSAPrivateKey(fp_private, NULL, NULL, NULL)) == NULL) {
		fclose(fp_private);
		LOGERROR("read privatekey fail");
		return NULL;
	}
	rsa_private_len = RSA_size(rsa_private);
	double times = ceil((double)total_len / 128);
	en = (unsigned char *)malloc(1024 * 10);
	memset(en, 0 ,1024 * 10);
	for(i = 0; i < times; i++) {
		if(RSA_private_encrypt(rsa_private_len, str+i*128, en+i*128, rsa_private, RSA_NO_PADDING) < 0) {
			fclose(fp_private);
			RSA_free(rsa_private);
			free(en);
			LOGERROR("private encrypt fail");
			return NULL;
		}
		LOGINFO("en//:%d:%ld", i, strlen(en));
	}
	LOGINFO("en_len:%ld", strlen(en));
	RSA_free(rsa_private);
	fclose(fp_private);
	return en;
}

static int Encrypt_Source(unsigned char *password, FILE *fpIn, FILE *fpOut) {
	OpenSSL_add_all_algorithms();
	if(password == NULL || fpIn == NULL || fpOut == NULL) {
		LOGERROR("input is null");
		return -1;
	}
	char iv[EVP_MAX_KEY_LENGTH]; //保存初始化向量的数组
	EVP_CIPHER_CTX ctx;   //EVP加密上下文环境
	char out[1024];  //保存密文的缓冲区
	int outl;
	char in[1024];   //保存原文的缓冲区
	int inl;
	int rv;
	int i;
	//设置iv
	for(i=0;i<8;i++)
	{
		iv[i]=i;
	}
	//初始化ctx
	EVP_CIPHER_CTX_init(&ctx);
	//设置密码算法和iv
	rv = EVP_EncryptInit_ex(&ctx,EVP_des_ede3_cbc(),NULL,password,iv);
	if(rv != 1)
	{
		return -1;
	}
	//循环读取原文，加密后后保存到密文文件
	for(;;)
	{
		inl = fread(in,1,1024,fpIn);
		if(inl <= 0)//读取原文结束
			break;
		rv = EVP_EncryptUpdate(&ctx,out,&outl,in,inl);//加密
		if(rv != 1)
		{
			EVP_CIPHER_CTX_cleanup(&ctx);
			return -1;
		}
		fwrite(out,1,outl,fpOut);//保存密文到文件
	}
	//加密结束
	rv = EVP_EncryptFinal_ex(&ctx,out,&outl);
	if(rv != 1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}
	fwrite(out,1,outl,fpOut);  //保存密文到文件
	EVP_CIPHER_CTX_cleanup(&ctx); //清除EVP加密上下文环境
	return 0;
}

int encrypt_file(const char *path_in, const char *path_out, char *message) {
	if(path_in == NULL || path_out == NULL || message == NULL) {
		LOGINFO("input is null");
		return -1;
	}
	if(access(path_out, F_OK) == 0) {
		LOGERROR("file already exists");
		return -1;
	}
	FILE *fpIn;
	FILE *fpOut;
	unsigned char password[20] = { 0 };//密码
	unsigned char *password_en = NULL;//公钥加密后的密码
	int password_en_len = 0;
	int len_last = 0;//第一次加密后头的长度
	unsigned char len_last_tmp[10] = { 0 };
	int head_en_len = 0;//第二次加密后头的长度
	unsigned char head_en_len_tmp[10] = { 0 };
	loop:	if(Rand_char(password) != 0) {
		LOGERROR("creat rand password fail");
		return -1;
	}
	LOGINFO("password:%s", password);
	fpIn = fopen(path_in,"rb");
	if(fpIn==NULL)
	{
		LOGERROR("open package fail");
		return -1;
	}
	fpOut = fopen(path_out,"ab+");
	if(fpOut==NULL)
	{
		fclose(fpIn);
		LOGERROR("open package_en fail");
		return -1;
	}
	if(fseek(fpOut, 0, SEEK_END) != 0) {
		LOGERROR("fseek package_en fail");
		fclose(fpIn);
		fclose(fpOut);
		return -1;
	}
	//解析message
	LOGINFO("message:%s", message);
	json_object *informatin_key = json_tokener_parse(message);
	json_object *private_obj = NULL;
	if(!json_object_object_get_ex(informatin_key, JSON_KEY_PRIVATE_KEY, &private_obj)) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("get parivate key fail");
		return -1;
	}
	const char *private_key = json_object_get_string(private_obj);
	json_object *serial_num_m_obj = NULL;
	if(!json_object_object_get_ex(informatin_key, JSON_KEY_SERIAL_NUM_M, &serial_num_m_obj)) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("get master serial fail");
		return -1;
	}
	const char *serial_num_m = json_object_get_string(serial_num_m_obj);
	json_object *header_list_obj = NULL;
	if(!json_object_object_get_ex(informatin_key, JSON_KEY_FILE_HEADER, &header_list_obj)) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("get head list fail");
		return -1;
	}
	int i = 0;
	int amount_flight = json_object_array_length(header_list_obj);
	if(amount_flight == 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("head is null");
		return -1;
	}
	//将飞机个数写入文件中,前4位表示飞机数
	unsigned char quantity_flight[10] = { 0 };
	sprintf(quantity_flight, "%04d", amount_flight);
	if(fwrite(quantity_flight, 1, strlen(quantity_flight), fpOut) <= 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("write flight num_tmp fail");
		return -1;
	}
	//将地面序列号写入文件中
	if(fwrite(serial_num_m, 1, strlen(serial_num_m), fpOut) <= 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("write serial_num_m fail");
		return -1;
	}
	unsigned char head_alone[256] = { 0 };
	unsigned char head_all[1024 * 10] = { 0 };
	unsigned char *head_en = NULL;
	//head_en = (unsigned char *)malloc(1024 * 10);
	for(i = 0; i < amount_flight; i++) {
		json_object *p_item = json_object_array_get_idx(header_list_obj, i);
		json_object *flight_num_obj = NULL;
		json_object *public_key_obj = NULL;
		json_object *serial_num_obj = NULL;
		if(!json_object_object_get_ex(p_item, JSON_KEY_FLIGHT_NUM, &flight_num_obj)) {
			LOGERROR("Encrypt_File(): get fligh num fail");
			fclose(fpIn);
			fclose(fpOut);
			//free(head_en);
			json_object_put(informatin_key);
			LOGERROR("parse message fail");
			return -1;
		}
		if(!json_object_object_get_ex(p_item, JSON_KEY_PUBLIC_KEY, &public_key_obj)) {
			LOGERROR("Encrypt_File(): get public key fail");
			fclose(fpIn);
			fclose(fpOut);
			//free(head_en);
			json_object_put(informatin_key);
			LOGERROR("parse message fail");			
			return -1;
		}
		if(!json_object_object_get_ex(p_item, JSON_KEY_SERIAL_NUM, &serial_num_obj)) {
			LOGERROR("Encrypt_File(): get fligh serial fail");
			fclose(fpIn);
			fclose(fpOut);
			//free(head_en);
			json_object_put(informatin_key);
			LOGERROR("parse message fail");			
			return -1;
		}
		const char *flight_num = json_object_get_string(flight_num_obj);
		const char *public_key = json_object_get_string(public_key_obj);
		const char *serial_num = json_object_get_string(serial_num_obj);
		//LOGINFO("flight_num:%s	public_key:%s	serial_num:%s", flight_num, public_key, serial_num);
		password_en = Encrypt_Password(public_key, password);
		password_en_len = strlen(password_en);
		if(password_en == NULL) {
			LOGERROR("Encrypt_File(): encrypt password fail");
			fclose(fpIn);
			fclose(fpOut);
			json_object_put(informatin_key);
			LOGERROR("encrypt password fail");
			return -1;
		}
		if(password_en_len != 128) {
			memset(password, 0, 20);
			fclose(fpIn);
			fclose(fpOut);
			remove(path_out);
			private_key = NULL;
			serial_num_m = NULL;
			json_object_put(informatin_key);
			free(password_en);
			len_last = 0;
			goto loop;
		}
		//unsigned char len[10] = { 0 };//密码加密以后的长度
		//sprintf(len, "%04d", password_en_len);
		snprintf(head_alone, sizeof(head_alone), "%20s%s%s", flight_num, serial_num, password_en);
		memcpy(head_all + len_last, head_alone, strlen(head_alone));
		len_last = strlen(head_alone) + len_last;
		memset(head_alone, 0, 256);
	}
	//LOGINFO("head_all:%s", head_all);
	//sprintf(len_last_tmp, "%08d", len_last);
	//fwrite(len_last_tmp, 1, strlen(len_last_tmp), fpOut);
	head_en = Encrypt_Head(head_all, private_key, len_last);
	if(head_en == NULL) {
		memset(password, 0, 20);
		fclose(fpIn);
		fclose(fpOut);
		remove(path_out);
		private_key = NULL;
		serial_num_m = NULL;
		json_object_put(informatin_key);
		free(password_en);
		memset(len_last_tmp, 0, 10);
		len_last = 0;
		goto loop;
	}
	if(strlen(head_en) < 164 * amount_flight || strlen(head_en) % 128 != 0) {
		memset(password, 0, 20);
		fclose(fpIn);
		fclose(fpOut);
		remove(path_out);
		private_key = NULL;
		serial_num_m = NULL;
		json_object_put(informatin_key);
		free(password_en);
		free(head_en);
		memset(len_last_tmp, 0, 10);
		len_last = 0;
		goto loop;
	}
	//将加密后头的总长度写入文件中
	head_en_len = strlen(head_en);
	sprintf(head_en_len_tmp, "%08d", head_en_len);
	LOGINFO("二次加密后头的长度：%d", head_en_len);
	fwrite(head_en_len_tmp, 1, strlen(head_en_len_tmp), fpOut);
	if(fwrite(head_en, 1, strlen(head_en), fpOut) < 0) {
		free(head_en);
		free(password_en);
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(informatin_key);
		LOGERROR("write head to file fail");
		return -1;
	}
	if(Encrypt_Source(password, fpIn, fpOut) != 0) {
		fclose(fpIn);
		fclose(fpOut);
		free(password_en);
		free(head_en);
		json_object_put(informatin_key);
		LOGERROR("encrypt_package fail");
		return -1;
	}
	LOGERROR("encrypt package sueecssed");
	json_object_put(informatin_key);
	free(password_en);
	free(head_en);
	fclose(fpIn);
	fclose(fpOut);
	return 0;
}

int encrypt_log(const char *path_in, const char *path_out, const char *message) {
	if(path_in == NULL || path_out == NULL || message == NULL) {
		LOGERROR("input is null");
		return -1;
	}
	if(access(path_out, F_OK) == 0) {
		LOGERROR("file already exists");
		return -1;
	}
	int password_len;
	FILE *fpIn;
	FILE *fpOut;
	unsigned char *password_en = NULL;
	unsigned char password_len_buff[10] = { 0 };
	int i;
	//生成随机码
	unsigned char password[20] = { 0 };
	label:	if(Rand_char(password) != 0) {
		LOGERROR("creat rand password fail");
		return -1;
	}
	LOGINFO("password:%s password_len:%ld", password, strlen(password));
	//打开待加密文件
	fpIn = fopen(path_in,"rb");
	if(fpIn==NULL)
	{
		LOGERROR("open log fail");
		return -1;
	}
	//打开保存密文的文件
	fpOut = fopen(path_out,"ab+");
	if(fpOut==NULL)
	{
		LOGERROR("open log_en fail");
		fclose(fpIn);
		return -1;
	}
	if(fseek(fpOut, 0, SEEK_END) != 0) {
		LOGERROR("fseek log fail");
		fclose(fpIn);
		fclose(fpOut);
		return -1;
	}
	json_object *log_key = json_tokener_parse(message);
	json_object *header_obj = NULL;
	if(!json_object_object_get_ex(log_key, JSON_KEY_FILE_HEADER, &header_obj)) {
		LOGERROR("get head fail");
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		return -1;
	}
	json_object *private_key_obj = NULL;
	json_object *serial_num_obj = NULL;
	if(!json_object_object_get_ex(header_obj, JSON_KEY_PRIVATE_KEY, &private_key_obj)) {
		LOGERROR("get public key fail");
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		return -1;
	}
	if(!json_object_object_get_ex(header_obj, JSON_KEY_SERIAL_NUM, &serial_num_obj)) {
		LOGERROR("get serial num fail");
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		return -1;
	}
	const char *private_key = json_object_get_string(private_key_obj);
	const char *serial_num = json_object_get_string(serial_num_obj);
	//将中心序列号写入文件中
	if(fwrite(serial_num, 1, strlen(serial_num), fpOut) <= 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		printf("Encrypt_Log(): write serial_num_c fail");
		return -1;
	}
	//加密密码
	password_en = Encrypt_Password_Private(private_key, password);
	if(password_en == NULL) {
		fclose(fpIn);
		fclose(fpOut);	
		json_object_put(log_key);
		LOGERROR("encrypt password fail");
		return -1;
	}
	if(strlen(password_en) != 128) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		memset(password, 0, 20);
		remove(path_out);
		free(password_en);
		goto label;
	}
	//将密码长度写入文件
	//int en_len = strlen(password_en);
	//sprintf(password_len_buff, "%04d", en_len);
	/*if(fwrite(password_len_buff, 1, strlen(password_len_buff), fpOut) < 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		free(password_en);
		LOGERROR("Encrypt_Log(): write password_len fail");
		return -1;
	}*/
	//将加密后的密码写入文件中
	if(fwrite(password_en, 1, strlen(password_en), fpOut) < 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		free(password_en);
		LOGERROR("Encrypt_Log(): write password_en fail");
		return -1;
	}
	if( Encrypt_Source(password, fpIn, fpOut) != 0) {
		fclose(fpIn);
		fclose(fpOut);
		json_object_put(log_key);
		free(password_en);
		LOGERROR("encrypt soure fail");
		return -1;
	}
	fclose(fpIn);
	fclose(fpOut);
	free(password_en);
	json_object_put(log_key);
	LOGINFO("encrypt log sueecssed");
	return 0;
}