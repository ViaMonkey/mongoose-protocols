/**
 * Mongoose Telnet.
 *
 */

#include <stdlib.h> // Required for libtelnet.h

#include "mg_telnet.h"





#if MG_ENABLE_TELNET

static const telnet_telopt_t opts_default[] = {
	{ TELNET_TELOPT_ECHO,      TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_TTYPE,     TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
	{ TELNET_TELOPT_NAWS,      TELNET_WILL, TELNET_DONT },
	{ -1, 0, 0 }
};


struct mg_telnet_proto_data {
	telnet_telopt_t *opts;
	telnet_t *handle;
	telnet_data_t *data;
};

/* Private Prototype */
static void mg_telnet_conn_destructor(void *proto_data);
static void mg_telnet_ev_handler(telnet_t *telnet, telnet_event_t *event, void *userData);



/* Private function */
static void mg_telnet_conn_destructor(void *proto_data) {

	MG_FREE(proto_data);
}


static struct mg_telnet_proto_data *mg_telnet_get_proto_data(struct mg_connection *nc) {

  if (nc->proto_data == NULL) {
	  nc->proto_data = MG_CALLOC(1, sizeof(struct mg_telnet_proto_data));
	  nc->proto_data_destructor = mg_telnet_conn_destructor;
  }

  return (struct mg_telnet_proto_data *) nc->proto_data;
}


static void mg_telnet_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data)) {

  struct mbuf *io = &nc->recv_mbuf;

  struct mg_telnet_proto_data *pd = mg_telnet_get_proto_data(nc);

  //Call handler
  nc->handler(nc, ev, ev_data MG_UD_ARG(user_data));

  switch (ev) {
    case MG_EV_ACCEPT: {

    	pd->opts = (telnet_telopt_t *)opts_default;
    	pd->data = (telnet_data_t *)MG_MALLOC(sizeof(telnet_data_t));
    	pd->data->nc = nc;
    	pd->data->event_type = TELNET_EV_DATA;
    	pd->data->buffer = NULL;
    	pd->data->size = 0;

    	pd->handle = telnet_init(pd->opts, mg_telnet_ev_handler, 0, pd->data);


    	nc->handler(nc, MG_EV_TELNET_ACCEPTED, pd->handle MG_UD_ARG(user_data));
      break;
    }
    case MG_EV_RECV: {

    	telnet_recv(pd->handle, (char *)io->buf, (size_t)io->len);

    	nc->handler(nc, MG_EV_TELNET, pd->data MG_UD_ARG(user_data));

    	mbuf_remove(io, io->len);
      	break;
    }
    case MG_EV_SEND: {

    	nc->handler(nc, MG_EV_TELNET, pd->data MG_UD_ARG(user_data));
		break;
	}
    case MG_EV_CLOSE: {

    	//Remove telenet memory connection close
    	MG_FREE(pd->data);
    	telnet_free(pd->handle);
    	pd->handle = NULL;

    	nc->handler(nc, MG_EV_TELNET_CLOSE, pd->handle MG_UD_ARG(user_data));
    	break;
    }
  }
}


/**
 * Telnet Event Handler.
 */
static void mg_telnet_ev_handler(telnet_t *telnet, telnet_event_t *event, void *userData) {

	telnet_data_t *telnetData = (telnet_data_t *)userData;

	switch(event->type) {
		case TELNET_EV_SEND:
			telnetData->event_type = event->type;

			mg_send(telnetData->nc, event->data.buffer, event->data.size);

			telnetData->buffer = NULL;
			telnetData->size = 0;
			break;
		case TELNET_EV_DATA:

			telnetData->event_type = event->type;
			telnetData->buffer = event->data.buffer;
			telnetData->size = event->data.size;
			break;
		default:
			break;
	}
}


/* Public function */
void mg_set_protocol_telnet(struct mg_connection *nc) {

  nc->proto_handler = mg_telnet_handler;

}



/**
 * Send data to the telnet connection.
 */
void mg_telnet_send(struct mg_connection *nc, uint8_t *buffer, size_t size) {

	struct mg_telnet_proto_data *pd = mg_telnet_get_proto_data(nc);

	if (pd->handle != NULL) {

		telnet_send(pd->handle, (char *)buffer, size);
	}
}



/**
 * Send a vprintf formatted output to the telnet connection.
 */
int mg_telnet_vprintf(struct mg_connection *nc, const char *fmt, va_list va) {

	struct mg_telnet_proto_data *pd = mg_telnet_get_proto_data(nc);


	if (pd->handle == NULL) {
		return 0;
	}

	return telnet_vprintf(pd->handle, fmt, va);
}


#endif
