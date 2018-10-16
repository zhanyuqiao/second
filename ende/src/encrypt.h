/*
 *encrypt.h
 *
 *Create on:2018.8.29
 *   Author:airport
 *
 */

#ifndef ENCRYPT_H_
#define ENCRYPT_H_

int encrypt_file(const char *path_in, const char *path_out, char *message);
int encrypt_log(const char *path_in, const char *path_out, const char *message);

#endif