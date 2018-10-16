/*
 *encrypt.h
 *
 *Create on:2018.8.29
 *   Author:airport
 *
 */
#ifndef JSON_INTERFACE_H_
#define JSON_INTERFACE_H_

#include <syslog.h>

#define LOGDEBUG(format, args...) \
		syslog(LOG_DEBUG, "%s %d %s " format, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOGINFO(format, args...) \
		syslog(LOG_INFO, "%s %d %s " format, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOGWARN(format, args...) \
		syslog(LOG_WARNING, "%s %d %s " format, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOGERROR(format, args...) \
		syslog(LOG_ERR, "%s %d %s " format, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOGCRIT(format, args...) \
		{ \
			syslog(LOG_CRIT, "%s %d %s " format, __FILE__, __LINE__, __FUNCTION__, ##args); \
			exit(1); \
		}

#define JSON_KEY_FILE_HEADER "header"
#define	JSON_KEY_PRIVATE_KEY "private key"
#define JSON_KEY_FLIGHT_NUM "flight"
#define JSON_KEY_PUBLIC_KEY "public key"
#define JSON_KEY_SERIAL_NUM "serial number"
#define JSON_KEY_SERIAL_NUM_M "m serial number"

#define LOCATION_FLIGHT "fligh"
#define LOCATION_CENTER "center"
#define LOCATION_MASTER "master"

#define KEY_TYPE_PUBLIC "publicKey"
#define KEY_TYPE_PRIVATE "privateKey"

#define FLIGHT_SERIAL_NUM 16
#define MASTER_SERIAL_NUM 16
#define FLIGHT_NUM 6
#define PASSWORD 8
#define SPACE 1024

#endif