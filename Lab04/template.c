// Initialize module:
else if(!strcmp(name, "mqtt"))
    ncrack_mqtt(nsp, con);

// Function register:
void ncrack_mqtt(nsock_pool nsp, Connection *con);

// File begin:
#include "ncrack.h"
#include "nsock.h"
#include "Service.h"
#include "modules.h"
#define MQTT_TIMEOUT 20000
extern void ncrack_read_handler(nsock_pool nsp, nsock_event nse, void *mydata); 
extern void ncrack_write_handler(nsock_pool nsp, nsock_event nse, void *mydata);
extern void ncrack_module_end(nsock_pool nsp, void *mydata);
static int mqtt_loop_read(nsock_pool nsp, Connection *con);
enum states { MQTT_INIT, MQTT_FINI };

// Struct
struct connect_cmd {
    uint8_t message_type; /* 1 for CONNECT packet */
    uint8_t msg_len; /* length of remaining packet */
    uint16_t prot_name_len; /* should be 4 for "MQTT" */
    u_char protocol[4]; /* it's always "MQTT" */
    uint8_t version; /* 4 for version MQTT version 3.1.1 */
    uint8_t flags; /* 0xc2 for flags: username, password, clean session */
    uint16_t keep_alive; /* 60 seconds */
    uint16_t client_id_len; /* should be 6 with "Ncrack" as id */
    u_char client_id[6]; /* let's keep it short - Ncrack */
    uint16_t username_len; /* length of username string */
    /* the rest of the packet,  add dynamically in buffer:
    * username (dynamic length),
    * password_length (uint16_t)
    * password (dynamic length)
    */
    connect_cmd() { /* constructor - fill code based on above comments, remeber to use htons when it is reqired */ 
        message_type = ...;
        prot_name_len = ...;
        protocol = ...;
        version = ...;
        flags = ...;
        keep_alive = ...;
        client_id_len = ...;
        client_id = ..;
    }
} __attribute__((__packed__)) connect_cmd;

struct ack {
    uint8_t message_type;
    uint8_t msg_len;
    uint8_t flags;
    uint8_t ret_code;
} __attribute__((__packed__)) ack;

// Read incomming
static int
mqtt_loop_read(nsock_pool nsp, Connection *con)
{
    struct ack *p; 
    if (con->inbuf == NULL || con->inbuf->get_len() < 4) {
        nsock_read(nsp, con->niod, ncrack_read_handler, MQTT_TIMEOUT, con);
        return -1;
    }
    p = (struct ack *)((char *)con->inbuf->get_dataptr()); 
    ...
    ...
    ...
    ...
    ...
    ...
}

// Logic:
void
ncrack_mqtt(nsock_pool nsp, Connection *con) {
    nsock_iod nsi = con->niod; 
    struct connect_cmd cmd;
    uint16_t pass_len;
    switch (con->state) 
    {
        case MQTT_INIT:
            con->state = MQTT_FINI;
            delete con->inbuf; 
            con->inbuf = NULL;
            if (con->outbuf)
                delete con->outbuf;
            con->outbuf = new Buf();
            /* the message len is the size of the struct plus the length of the usernames
            * and password minus 2 for the first 2 bytes (message type and message length) that
            * are not counted in
            */
            cmd.msg_len = ...;
            cmd.username_len = htons(strlen(con->user));
            pass_len = htons(strlen(con->pass));
            con->outbuf->append(&cmd, sizeof(cmd)); 
            con->outbuf->snprintf(strlen(con->user), "%s", con->user);
            con->outbuf->append(&pass_len, sizeof(pass_len));
            con->outbuf->snprintf(strlen(con->pass), "%s", con->pass);
            nsock_write(nsp, nsi, ncrack_write_handler, MQTT_TIMEOUT, con,
            (const char *)con->outbuf->get_dataptr(), con->outbuf->get_len());
            break;
        case MQTT_FINI:
            if (mqtt_loop_read(nsp, con) == -1)
                break;
            else if (mqtt_loop_read(nsp, con) == 0)
                con->auth_success = true;
            con->state = MQTT_INIT;
            delete con->inbuf;
            con->inbuf = NULL;
        return ncrack_module_end(nsp, con);
    }
}
