#ifndef _MG_TELNET_H_
#define _MG_TELNET_H_

#ifndef MG_ENABLE_TELNET
#define MG_ENABLE_TELNET 1
#endif


#if MG_ENABLE_TELNET

#include "mongoose.h"
#include "libtelnet.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef MG_MALLOC
#define MG_MALLOC malloc
#endif

#ifndef MG_CALLOC
#define MG_CALLOC calloc
#endif

#ifndef MG_REALLOC
#define MG_REALLOC realloc
#endif

#ifndef MG_FREE
#define MG_FREE free
#endif


/* Events */
#define MG_EV_TELNET_ACCEPTED 	100
#define MG_EV_TELNET 			101
#define MG_EV_TELNET_CLOSE		102


typedef struct {
	struct mg_connection *nc;

	enum telnet_event_type_t event_type; /*!< event type */
	const char *buffer;             /*!< byte buffer */
	size_t size;                    /*!< number of bytes in buffer */
} telnet_data_t;


/* Turn the connection into the TELNET server */
void mg_set_protocol_telnet(struct mg_connection *c);


void mg_telnet_send(struct mg_connection *nc, uint8_t *buffer, size_t size);
int mg_telnet_vprintf(struct mg_connection *nc, const char *fmt, va_list va);


#ifdef __cplusplus
}
#endif

#endif

#endif /*_MG_TELNET_H_*/
