#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <curl/curl.h>

#include "parson.h"
#include "paho.mqtt.c/MQTTClient.h"
#include "device.h"
#include "senml.h"

#define ERRBUFSZ 1024

#define LOG_FILE "/var/log/device_pi.log"


#define EX_ID        "pi"
#define EX_KEY     "raspberry"
#define BOOTSTRP_SERVER_IP "139.9.104.96"
#define BOOTSTRP_PORT 8202
#define CLIENTID    "ExampleClientPub"
#define QOS         1
#define TIMEOUT     10000L

#define VOLTAGE_VALUE  (3.5)
#define CURRENT_VALUE  (4.2)

device_cb_t *g_device_cb = NULL;

static int build_voltage_record(senml_record_t *record, float voltage)
{
    char *name = "voltage";
    char *unit = "V";
    record->name = malloc(strlen(name) + 1);
    strcpy(record->name, name);
    record->unit = malloc(strlen(unit) + 1);
    strcpy(record->unit, unit);
    record->value.value_f = voltage;
    record->value_type = SENML_TYPE_FLOAT;

    return 0;
}
static int build_current_record(senml_record_t *record, float current)
{
    char *name = "current";
    char *unit = "A";
    record->name = malloc(strlen(name) + 1);
    strcpy(record->name, name);
    record->unit = malloc(strlen(unit) + 1);
    strcpy(record->unit, unit);
    record->value.value_f = current;
    record->value_type = SENML_TYPE_FLOAT;

    return 0;
}
static int mqtt_msg_handle(void *context, char *topicName, char *payload, size_t payload_len)
{
    senml_pack_t *pack;
    device_cb_t *pdev_cb;
    
    pdev_cb = (device_cb_t *)context;
    pack = senml_decode_json(payload, payload_len);
    senml_print_pack(pack);
    pack = senml_normalize(pack);
    senml_print_pack(pack);
    senml_destroy_pack(pack);

    iot_log__debug(pdev_cb->lc, "Message handled topic :%s\n", topicName);
    return 0;
}
int main(void)
{
    device_cb_t *pdev_cb;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    char topic[128];
    mqtt_cfg_t mqtt_cfg;
    bootstrap_cfg_t bs_cfg;
    senml_pack_t *pack;
    char *payload;
    iot_logger_t *lc = NULL;

    mqtt_cfg.client_id = EX_ID ;
    mqtt_cfg.sub_qos = QOS;
    mqtt_cfg.msg_handle = mqtt_msg_handle;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    mqtt_cfg.conn_opts = &conn_opts;
    bs_cfg.ex_id = EX_ID;
    bs_cfg.ex_key = EX_KEY;
    bs_cfg.server_ip = BOOTSTRP_SERVER_IP;
    bs_cfg.server_port = BOOTSTRP_PORT;
    
    lc = iot_logger_alloc_custom(bs_cfg.ex_id,IOT_LOG_DEBUG,LOG_FILE,iot_log_console, NULL, true);
    pdev_cb = device_init(&mqtt_cfg, &bs_cfg, lc);

    pack = senml_create_pack(2);
    build_voltage_record(&pack->records[0], VOLTAGE_VALUE);
    build_current_record(&pack->records[1], CURRENT_VALUE);
    payload = senml_encode_json(pack);
    senml_destroy_pack(pack);
    sprintf(topic, "channels/%s/messages/req", pdev_cb->dev_cfg.data_channel_id);
    int i = 5;
    while (i--)
    {
        device_pub_message(pdev_cb, topic, QOS, payload);
        sleep(2);
    }
    json_free_serialized_string(payload);
    device_deinit(pdev_cb);
    return 0;    
}

