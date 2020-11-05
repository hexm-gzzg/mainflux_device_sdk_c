#include "senml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>


#include "parson.h"

senml_pack_t *senml_create_pack(int record_num)
{
    senml_pack_t *pack = malloc(sizeof(senml_pack_t));

    pack->records = malloc(sizeof(senml_record_t) * record_num);
    memset((void *)pack->records, 0, sizeof(senml_record_t) * record_num);
    pack->num = record_num;
    for (unsigned int idx = 0; idx < pack->num; idx++)
    {
        pack->records[idx].base_info = malloc(sizeof(senml_base_info_t));
        memset((void *)pack->records[idx].base_info, 0, sizeof(senml_base_info_t));
    }
    return pack;
}
static void senml_destroy_record(senml_record_t *record)
{
    if (!record)
    {
        return;
    }
    if (record->name != NULL)
    {
        free(record->name);
    }
    if (record->unit)
    {
        free(record->unit);
    }
    if ((record->value_type == SENML_TYPE_STRING) && record->value.value_s)
    {
        free(record->value.value_s);
    }

    if (record->base_info)
    {
        if (record->base_info->base_name)
        {
            free(record->base_info->base_name);
        }
        if (record->base_info->base_unit)
        {
            free(record->base_info->base_unit);
        }
        if (record->base_info->base_value.base_value_s)
        {
            free(record->base_info->base_value.base_value_s);
        }
        free(record->base_info);
    }
    return;
}
void senml_destroy_pack(senml_pack_t *pack)
{
    for (unsigned int idx = 0; idx < pack->num; idx++)
    {
        senml_destroy_record(&pack->records[idx]);
    }

    free(pack->records);
    free(pack);
    return;
}

senml_pack_t *senml_normalize(senml_pack_t *pack)
{
    senml_record_t *record;
    double btime = 0;
    char *bname = NULL;
    char *base_unit = NULL;
    char *buf;
    if (!pack)
    {
        return NULL;
    }
    for (unsigned int idx = 0; idx < pack->num; idx++)
    {
        record = &pack->records[idx];
        if (record->base_info->base_time != 0)
        {
            btime = record->base_info->base_time;
        }

        if ((record->base_info->base_name != NULL) && (strlen(record->base_info->base_name) > 0))
        {
            bname = record->base_info->base_name;
        }
        if ((record->base_info->base_unit != NULL) && (strlen(record->base_info->base_unit) > 0))
        {
            base_unit = record->base_info->base_unit;
        }
        if ((bname != NULL) && (strlen(bname) != 0))
        {
            buf = malloc(strlen(bname) + strlen(record->name));
            sprintf(buf, "%s%s", bname, record->name);
            free(record->name);
            record->name = buf;
        }
        if ((base_unit != NULL) && (strlen(base_unit) != 0))
        {
            if ((record->unit != NULL) && (strlen(record->unit) != 0))
            {
                buf = malloc(strlen(base_unit) + strlen(record->unit));
                sprintf(buf, "%s%s", base_unit, record->unit);
                free(record->unit);
                record->unit = buf;
            }
            else
            {
                buf = malloc(strlen(base_unit));
                sprintf(buf, "%s", base_unit);
                record->unit = buf;
            }
        }
        record->time = btime + record->time;

        memset(record->base_info, 0, sizeof(senml_base_info_t));
    }
    return pack;
}
static char *get_string_dfl(const JSON_Object *obj, const char *name, const char *dfl)
{
    const char *str = json_object_get_string(obj, name);
    const char *p = str ? str : dfl;
    char *ret;
    ret = malloc(strlen(p) + 1);
    strcpy(ret, p);
    return ret;
}

static char *get_string(const JSON_Object *obj, const char *name)
{
    return get_string_dfl(obj, name, "");
}

static inline bool senml_is_base_info(const JSON_Object *record)
{
    char *str;
    str = get_string(record, SJ_VERSION);
    if (str)
    {
        free(str);
        return true;
    }
    free(str);
    str = get_string(record, SJ_BASE_NAME);
    if (str)
    {
        free(str);
        return true;
    }
    free(str);
    str = get_string(record, SJ_BASE_TIME);
    if (str)
    {
        free(str);
        return true;
    }
    free(str);

    str = get_string(record, SJ_BASE_UNIT);
    if (str)
    {
        free(str);
        return true;
    }
    free(str);

    str = get_string(record, SJ_BASE_VALUE);
    if (str)
    {
        free(str);
        return true;
    }
    free(str);

    return false;
}
senml_pack_t *senml_decode_json( char *input, size_t len)
{
    char *msg_str;
    JSON_Value *root_val;
    JSON_Value *val;
    size_t arr_cnt = 0;
    char *str;
    senml_pack_t *pack = malloc(sizeof(senml_pack_t));
    
    // 转换为字符串
    msg_str = malloc(len + 1);
    memcpy(msg_str, input, len);
    input[len + 1] = '\0';

    root_val = json_parse_string(input);
    JSON_Array *array = json_value_get_array(root_val);
    if (!array)
    {
        fprintf(stderr, "ERROR: not an array\n");
        //json_decref(json_root);
        goto error;
    }

    arr_cnt = json_array_get_count(array);
    pack->records = malloc(sizeof(senml_record_t) * arr_cnt);
    memset((void *)pack->records, 0, sizeof(senml_record_t) * arr_cnt);
    pack->num       = arr_cnt;

    unsigned int i = 0;

    JSON_Object *json_record = json_array_get_object(array, i);
    if (!json_record)
    {
        fprintf(stderr, "ERROR: not an object\n");
        goto error;
    }

    for (unsigned int index = 0; i < arr_cnt; i++, index++)
    {
        if (senml_is_base_info(json_record))
        {
            pack->records[index].base_info = malloc(sizeof(senml_base_info_t));
            memset(pack->records[index].base_info, 0, sizeof(senml_base_info_t));
            pack->records[index].base_info->base_value_type = SENML_TYPE_UNDEF;

            val = json_object_get_value(json_record, SJ_VERSION);

            if (val)
            {
                pack->records[index].base_info->version = (uint8_t)json_uint(val);
            }

            str = get_string(json_record, SJ_BASE_NAME);

            if (str != NULL && strlen(str) != 0)
            {
                pack->records[index].base_info->base_name = malloc(strlen(str) + 1);
                strcpy(pack->records[index].base_info->base_name, str);
            }
            free(str);

            val = json_object_get_value(json_record, SJ_BASE_TIME);
            if (val)
            {
                pack->records[index].base_info->base_time = json_number(val);
            }

            str = get_string(json_record, SJ_BASE_UNIT);
            if (str != NULL && strlen(str) != 0)
            {
                pack->records[index].base_info->base_unit = malloc(strlen(str) + 1);
                strcpy(pack->records[index].base_info->base_unit, str);
            }
            free(str);

            str = get_string(json_record, SJ_BASE_VALUE);

            if (str != NULL && strlen(str) != 0)
            {
                // FIXME how do we handle different data types here?
            }

            free(str);
        }

        json_record = json_array_get_object(array, i);
        if (!json_record)
        {
            fprintf(stderr, "ERROR: not an object\n");
            goto error;
        }

        str = get_string(json_record, SJ_NAME);
        if (str != NULL && strlen(str) != 0)
        {
            pack->records[index].name = malloc(strlen(str) + 1);
            strcpy(pack->records[index].name, str);
            free(str);
        }

        str = get_string(json_record, SJ_UNIT);
        if (str != NULL && strlen(str) != 0)
        {
            pack->records[index].unit = malloc(strlen(str) + 1);
            strcpy(pack->records[index].unit, str);
            free(str);
        }

        val = json_object_get_value(json_record, SJ_TIME);
        if (val)
        {
            pack->records[index].time = json_number(val);
        }

        val = json_object_get_value(json_record, SJ_UPDATE_TIME);
        if (val)
        {
            pack->records[index].update_time = (unsigned int)json_uint(val);
        }

        // TODO insert check for value sum here, not sure how yet

        val = json_object_get_value(json_record, SJ_VALUE);
        if (val)
        {
            pack->records[index].value_type    = SENML_TYPE_FLOAT;
            pack->records[index].value.value_f = json_number(val);
            continue;
        }

        val = json_object_get_value(json_record, SJ_BOOL_VALUE);
        if (val)
        {
            pack->records[index].value_type = SENML_TYPE_BOOL;
            pack->records[index].value.value_b = json_boolean(val);
            continue;
        }

        val = json_object_get_value(json_record, SJ_STRING_VALUE);
        if (val)
        {
            pack->records[index].value_type    = SENML_TYPE_STRING;
            pack->records[index].value.value_s = malloc(strlen(json_string(val)) + 1);
            strcpy(pack->records[index].value.value_s, json_string(val));
            continue;
        }
        // TODO what about binary values?
    }
    
    
    json_value_free(root_val);

    return pack;
error:
    free(pack->records);

    for (unsigned int index = 0; i < arr_cnt; i++, index++)
    {
        if (pack->records[index].base_info)
        {
            free(pack->records[index].base_info);
        }
        if (pack->records[index].base_info->base_name)
        {
            free(pack->records[index].base_info->base_name);
        }

        if (pack->records[index].base_info->base_unit)
        {
            free(pack->records[index].base_info->base_unit);
        }

        if (pack->records[index].name)
        {
            free(pack->records[index].name);
        }

    }

    free(pack);
    json_value_free(root_val);
    return NULL;

}
char *senml_encode_json(const senml_pack_t *pack)
{
    JSON_Value *val = json_value_init_array();
    JSON_Array *array = json_value_get_array(val);
    if (!pack)
    {
        json_value_free(val);
        return NULL;
    }

    for (unsigned int i = 0; i < pack->num; i++)
    {
        JSON_Value *pval = json_value_init_object();
        JSON_Object *pobj = json_value_get_object(pval);

        if (pack->records[i].base_info->version)
        {
            json_object_set_uint(pobj, SJ_VERSION, (uint64_t)pack->records[i].base_info->version);
        }

        if (pack->records[i].base_info->base_name)
        {
            json_object_set_string(pobj, SJ_BASE_NAME, pack->records[i].base_info->base_name);
        }
        if (pack->records[i].base_info->base_time > 0)
        {
            json_object_set_number(pobj, SJ_BASE_TIME,
                                   pack->records[i].base_info->base_time);
        }

        if (pack->records[i].base_info->base_unit)
        {
            json_object_set_string(pobj, SJ_BASE_UNIT,
                                   pack->records[i].base_info->base_unit);
        }
        json_array_append_value(array, pval);

        if (pack->records[i].name)
        {
            json_object_set_string(pobj, SJ_NAME, pack->records[i].name);
        }
        if (pack->records[i].unit)
        {
            json_object_set_string(pobj, SJ_UNIT, pack->records[i].unit);
        }

        if (pack->records[i].time != 0)
        {
            json_object_set_number(pobj, SJ_TIME, pack->records[i].time);
        }

        if (pack->records[i].update_time != 0)
        {
            json_object_set_uint(pobj, SJ_UPDATE_TIME, pack->records[i].update_time);
        }

        if (pack->records[i].value_type == SENML_TYPE_FLOAT)
        {
            json_object_set_number(pobj, SJ_VALUE, pack->records[i].value.value_f);
        }
        else if (pack->records[i].value_type == SENML_TYPE_STRING)

        {
            json_object_set_string(pobj, SJ_STRING_VALUE, pack->records[i].value.value_s);
        }

        else if (pack->records[i].value_type == SENML_TYPE_BOOL)
        {
            json_object_set_boolean(pobj, SJ_BOOL_VALUE, pack->records[i].value.value_b);
        }
        // 		else if (pack->records[i].value_type == SENML_TYPE_BINARY)
        // FIXME handle binary value
    }

    char *str = json_serialize_to_string(val);

    json_value_free(val);
    return str;
}

void senml_print_base_info(const senml_base_info_t *base_info)
{
    printf("Version:\t%u\n", base_info->version);
    printf("Base Name:\t%s\n", base_info->base_name ? base_info->base_name : "NULL");
    printf("Base Time:\t%f\n", base_info->base_time);
    printf("Base Unit:\t%s\n", base_info->base_unit ? base_info->base_unit : "NULL");

    if (base_info->base_value_type == SENML_TYPE_FLOAT)
    {
        printf("Base Value:\t%f\n", base_info->base_value.base_value_f);
    }
    else if (base_info->base_value_type == SENML_TYPE_STRING)
    {
        printf("Base Value:\t%s\n", base_info->base_value.base_value_s);
    }
    else if (base_info->base_value_type == SENML_TYPE_BOOL)
    {
        printf("Base Value:\t%s\n", base_info->base_value.base_value_b ? "true" : "false");
    }

    printf("\n");
}


void senml_print_record(const senml_record_t *record)
{
    printf("Name:\t\t%s\n", record->name ? record->name : "NULL");
    printf("Unit:\t\t%s\n", record->unit ? record->unit : "NULL");
    printf("Time:\t\t%f\n", record->time);
    printf("Update Time:\t%u\n", record->update_time);

    if (record->value_type == SENML_TYPE_FLOAT)
    {
        printf("Value:\t\t%f\n", record->value.value_f);
    }
    else if (record->value_type == SENML_TYPE_STRING)
    {
        printf("Value:\t\t%s\n", record->value.value_s);
    }
    else if (record->value_type == SENML_TYPE_BOOL)
    {
        printf("Value:\t\t%s\n", record->value.value_b ? "true" : "false");
    }

    printf("\n");
}


void senml_print_pack(const senml_pack_t *pack)
{
    if (!pack)
    {
        return;
    }
    for (size_t i = 0; i < pack->num; i++)
    {

        if (pack->records[i].base_info)
        {
            senml_print_base_info(pack->records[i].base_info);
        }
        senml_print_record(&(pack->records[i]));
    }
}

