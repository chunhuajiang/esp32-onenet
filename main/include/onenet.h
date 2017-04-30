#ifndef __ONENET_H__
#define __ONENET_H__

/* Supported format when publish data point to oneNET */
enum mqtt_save_data_type {
    data_type_full_json = 0x01,
    data_type_bin = 0x02,
    data_type_simple_json_without_time = 0x03,
    data_type_simple_json_with_time = 0x04,
    data_type_string = 0x05,
    data_type_string_with_time = 0x06,
    data_type_float  = 0x07
};
    

#endif /* __ONENET_H__ */
