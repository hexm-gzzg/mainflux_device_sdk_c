#include <curl/curl.h>

#include "device.h"
#include "parson.h"
#include "senml.h"

void *device_read_cfg(iot_logger_t *lc, const char *json, device_cfg_t *pdev_cfg);

static char *get_string_dfl(const JSON_Object *obj, const char *name, const char *dfl)
{
    const char *str = json_object_get_string(obj, name);
    return strdup(str ? str : dfl);
}

static char *get_string(const JSON_Object *obj, const char *name)
{
    return get_string_dfl(obj, name, "");
}

#define CONTROL_CHANNEL_TYPE "data"
#define EXPORT_CHANNEL_TYPE "export"

static int __device_read_cfg(iot_logger_t *lc, const JSON_Object *obj, device_cfg_t *pdev_cfg)
{
    JSON_Object *temp_obj;
    char *ctrl_chan_id;
    char *data_chan_id;
    char *mem;
    memset(pdev_cfg, 0, sizeof(device_cfg_t));
    char *mainflux_id = get_string(obj, "mainflux_id");
    char *mainflux_key = get_string(obj, "mainflux_key");
    JSON_Array *array = json_object_get_array(obj, "mainflux_channels");
    if (array)
    {
        size_t count = json_array_get_count(array);
        for (size_t i = 0; i < count; i++)
        {
            temp_obj = json_array_get_object(array, i);
            char *id = get_string(temp_obj, "id");
            const char *type = json_object_dotget_string(temp_obj, "metadata.type");

            if (!strcmp(type, CONTROL_CHANNEL_TYPE))
            {
                ctrl_chan_id = id;
            }
            if (!strcmp(type, EXPORT_CHANNEL_TYPE))
            {
                data_chan_id = id;
            }
        }
    }
    char *content = get_string(obj, "content");
    JSON_Value *val = json_parse_string(content);
    JSON_Object *obj_t;
    obj_t = json_value_get_object(val);
    const char *agent = json_object_dotget_string(obj_t, "agent.mqtt.url");

    mem = (char *)malloc(strlen(mainflux_id) + 1);
    memcpy(mem, mainflux_id, strlen(mainflux_id) + 1);
    pdev_cfg->thing_id = mem;

    mem = (char *)malloc(strlen(mainflux_key) + 1);
    memcpy(mem, mainflux_key, strlen(mainflux_key) + 1);
    pdev_cfg->thing_key = mem;

    mem = (char *)malloc(strlen(ctrl_chan_id) + 1);
    memcpy(mem, ctrl_chan_id, strlen(ctrl_chan_id) + 1);
    pdev_cfg->control_channel_id = mem;

    mem = (char *)malloc(strlen(data_chan_id) + 1);
    memcpy(mem, data_chan_id, strlen(data_chan_id) + 1);
    pdev_cfg->data_channel_id = mem;

    mem = (char *)malloc(strlen(agent) + 1);
    memcpy(mem, agent, strlen(agent) + 1);
    pdev_cfg->mqtt_host_url = mem;

    free(mainflux_id);

    free(mainflux_key);

    free(content);

    json_value_free(val);
    return 0;
}

void *device_read_cfg(iot_logger_t *lc, const char *json, device_cfg_t *pdev_cfg)
{
    JSON_Value *val = json_parse_string(json);
    JSON_Object *obj;

    obj = json_value_get_object(val);
    if (obj)
    {
        __device_read_cfg(lc, obj, pdev_cfg);
    }

    json_value_free(val);

    return NULL;
}

static void *device_metadata_client_get_device
(
    iot_logger_t *lc,
    bootstrap_cfg_t *pbs_cfg,
    devsdk_error *err,
    device_cfg_t *pdev_cfg
)
{
    device_ctx ctx;
    char url[URL_BUF_SIZE];

    memset(&ctx, 0, sizeof(device_ctx));
    snprintf
    (
        url,
        URL_BUF_SIZE - 1,
        "http://%s:%u/things/bootstrap/%s",
        pbs_cfg->server_ip,
        pbs_cfg->server_port,
        pbs_cfg->ex_id
    );
    ctx.jwt_token = pbs_cfg->ex_key;

    iot_log__info(lc, "bootstrap url %s\n", url);

    device_http_get(lc, &ctx, url, device_http_write_cb, err);

    if (err->code)
    {
        free(ctx.buff);
        iot_log__error(lc, "device_http_get failed err reason%s\n", err->reason);
        return 0;
    }

    device_read_cfg(lc, ctx.buff, pdev_cfg);
    free(ctx.buff);
    return NULL;
}

static void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
}

static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    device_cb_t *pdev_cb = context;

    iot_log__debug(pdev_cb->lc, "Message arrived topic :%s\n", topicName);

    if (!pdev_cb)
    {

        printf("pdev_cb is NULL\n");
        MQTTClient_freeMessage(&message);
        MQTTClient_free(topicName);
        return 1;
    }
    if (!pdev_cb->mqtt_cfg->msg_handle)
    {
        printf("pdev_cb->mqtt_cfg->msg_handle  is NULL\n");
        MQTTClient_freeMessage(&message);
        MQTTClient_free(topicName);
        return 1;
    }
    pdev_cb->mqtt_cfg->msg_handle(pdev_cb, topicName, message->payload, message->payloadlen);


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

static void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
static void dump_device_config(device_cb_t *pdev_cb)
{
    if (!pdev_cb)
    {
        return;
    }
    iot_log__debug(pdev_cb->lc, "%s config infomation : %s\n", pdev_cb->mqtt_cfg ->client_id);
    iot_log__debug(pdev_cb->lc, "  thing_id %s\n", pdev_cb->dev_cfg.thing_id);
    iot_log__debug(pdev_cb->lc, "  thing_key %s\n", pdev_cb->dev_cfg.thing_key);
    iot_log__debug(pdev_cb->lc, "  mqtt_host_url %s\n", pdev_cb->dev_cfg.mqtt_host_url);
    iot_log__debug(pdev_cb->lc, "  control_channel_id %s\n", pdev_cb->dev_cfg.control_channel_id);
    iot_log__debug(pdev_cb->lc, "  data_channel_id %s\n", pdev_cb->dev_cfg.data_channel_id);
    return;
}
device_cb_t *device_init(mqtt_cfg_t *pmqtt_cfg, bootstrap_cfg_t *pbs_cfg, iot_logger_t *lc)
{
    device_cb_t *pdev_cb;
    devsdk_error err;
    char topic[128];
    int rc;

    pdev_cb = (device_cb_t *)malloc(sizeof(device_cb_t));
    if (!pdev_cb)
    {
        return NULL;
    }

    pdev_cb->lc = lc;    
    pdev_cb->mqtt_cfg = pmqtt_cfg;
    curl_global_init(CURL_GLOBAL_NOTHING);
    device_metadata_client_get_device(lc, pbs_cfg, &err, &pdev_cb->dev_cfg);
    dump_device_config(pdev_cb);
    MQTTClient_create(&pdev_cb->client, pdev_cb->dev_cfg.mqtt_host_url, pdev_cb->mqtt_cfg ->client_id,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    pdev_cb->mqtt_cfg->conn_opts->username = pdev_cb->dev_cfg.thing_id;
    pdev_cb->mqtt_cfg->conn_opts->password = pdev_cb->dev_cfg.thing_key;

    MQTTClient_setCallbacks(pdev_cb->client, pdev_cb, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(pdev_cb->client, pmqtt_cfg->conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        iot_log__error(pdev_cb->lc, "Failed to connect, return code %d\n", rc);
        free(pdev_cb);
        return NULL;
    }

    sprintf(topic, "channels/%s/messages/#", pdev_cb->dev_cfg.data_channel_id);
    MQTTClient_subscribe(pdev_cb->client, topic, pmqtt_cfg->sub_qos);
    return pdev_cb;
}

void device_deinit(device_cb_t *pdev_cb)
{
    if (!pdev_cb)
    {
        return;
    }
    iot_logger_free(pdev_cb->lc);
    MQTTClient_disconnect(pdev_cb->client, 10000);
    MQTTClient_destroy(&pdev_cb->client);
    if (pdev_cb->dev_cfg.control_channel_id)
    {
        free(pdev_cb->dev_cfg.control_channel_id);
    }
    if (pdev_cb->dev_cfg.data_channel_id)
    {
        free(pdev_cb->dev_cfg.data_channel_id);
    }
    if (pdev_cb->dev_cfg.thing_id)
    {
        free(pdev_cb->dev_cfg.thing_id);
    }
    if (pdev_cb->dev_cfg.thing_key)
    {
        free(pdev_cb->dev_cfg.thing_key);
    }
    if (pdev_cb->dev_cfg.mqtt_host_url)
    {
        free(pdev_cb->dev_cfg.mqtt_host_url);
    }

    free(pdev_cb);
}

int device_pub_message(device_cb_t *pdev_cb, char *topic, int qos, char *payload)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = qos;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(pdev_cb->client, topic, &pubmsg, NULL);

    iot_log__info(pdev_cb->lc, "Waiting for publication of %s.on topic %s for client with ClientID: %s\n",
                  payload, topic, pdev_cb->mqtt_cfg->client_id);
    return 0;
}
