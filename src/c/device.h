#include "rest.h"
#include "paho.mqtt.c/MQTTClient.h"

typedef struct device_cfg_u
{
    char *thing_id;
    char *thing_key;
    char *control_channel_id;
    char *data_channel_id;
    char *mqtt_host_url;
} device_cfg_t;
typedef struct bootstrap_cfg_u
{
    char *ex_id;
    char *ex_key;
    char *server_ip;
    int server_port;
} bootstrap_cfg_t;

typedef struct mqtt_cfg_u
{
    char *client_id;
    int sub_qos;
    MQTTClient_connectOptions *conn_opts;
    int (*msg_handle)(void *context, char *topicName, char *payload, size_t payload_len);
} mqtt_cfg_t;

typedef struct device_cb_u
{
    device_cfg_t dev_cfg;
    mqtt_cfg_t *mqtt_cfg;
    iot_logger_t *lc;
    MQTTClient client;
} device_cb_t;

extern device_cb_t *device_init(mqtt_cfg_t *pmqtt_cfg, bootstrap_cfg_t *pbs_cfg,iot_logger_t *lc );
extern void device_deinit(struct device_cb_u *pdev_cb);
extern int device_pub_message(device_cb_t *pdev_cb, char *topic, int qos, char *payload);

