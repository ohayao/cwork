#include "bridge/wifi_service/crypt.h"
#include <stdio.h>
#include <stdlib.h>
#include "bridge/lock/external/cbor/cbor.h"

int initCrypt(Crypt *obj)
{
  if (!obj) return 1;
  memset(obj, 0, sizeof(Crypt));
  return 0;
} 

int getCrypt(Crypt **pobj)
{
  if (*pobj) return 1;
  *pobj = (Crypt *)malloc(sizeof(Crypt));
  Crypt *obj = *pobj;
  if (!obj) return 1;
  return 0;
}

int setCryptAdminKey(Crypt *data, uint8_t *admin_key, size_t admin_key_len)
{
  if (!data || !admin_key) return 1;
  data->server_pairing_admin_key = (uint8_t *)malloc(admin_key_len);
  memset(data->server_pairing_admin_key,0, admin_key_len);
  memcpy(data->server_pairing_admin_key, admin_key, admin_key_len);
  data->has_server_pairing_admin_key = true;
  data->server_pairing_admin_key_len = admin_key_len;
  return 0;
}

int setCryptClientNonce(Crypt *data, uint8_t *client_nonce, size_t client_nonce_len)
{
  if (!data || !client_nonce) return 1;
  data->client_nonce = (uint8_t *)malloc(client_nonce_len);
  memset(data->client_nonce, 0, client_nonce_len);
  memcpy(data->client_nonce, client_nonce, client_nonce_len);
  data->has_client_nonce = true;
  data->client_nonce_len = client_nonce_len;
  return 0;
}

int setCryptServerNonce(Crypt *data, uint8_t *server_nonce, size_t server_nonce_len)
{
  if (!data || !server_nonce) return 1;
  data->server_nonce = (uint8_t *)malloc(server_nonce_len);
  memset(data->server_nonce, 0, server_nonce_len);
  memcpy(data->server_nonce, server_nonce, server_nonce_len);
  data->has_server_nonce = true;
  data->server_nonce_len = server_nonce_len;
  return 0;
}

int isCryptInvalid(Crypt * obj)
{
  if (!obj) return 1;
  // 必须这三个都有, 才合法
  printf("isCryptInvalid %d %d %d\n", (!obj->has_client_nonce), (!obj->has_server_nonce), (!obj->has_server_pairing_admin_key));
  return !obj->has_client_nonce || !obj->has_server_nonce || !obj->has_server_pairing_admin_key;
}

int encodeCrypt(Crypt * obj, uint8_t *retval, size_t length,size_t *written_length)
{
  if (isCryptInvalid(obj)) return 1;

  CborEncoder encoder;
  CborEncoder map;
  CborError err;

  //msg_id + ssid  + password
  size_t fields_size = 1 + 1 + 1 + 1;

  cbor_encoder_init(&encoder, retval, length, 0);
  err = cbor_encoder_create_map(&encoder, &map, fields_size);
  if(err) return err;

  //add msg_id
  err = cbor_encode_uint(&map, 0);
  if(err) return err;
  err = cbor_encode_uint(&map, 51);
  if(err) return err;

  // add admin key, 11
  err = cbor_encode_uint(&map, 11);
  if(err) return err;
  err = cbor_encode_byte_string(&map, obj->server_pairing_admin_key, obj->server_pairing_admin_key_len);
  if(err) return err;

  // add server nonce 
  err = cbor_encode_uint(&map, 12);
  if(err) return err;
  err = cbor_encode_byte_string(&map, obj->server_nonce, obj->server_nonce_len);
  if(err) return err;

  // add client nonce 
  err = cbor_encode_uint(&map, 13);
  if(err) return err;
  err = cbor_encode_byte_string(&map, obj->client_nonce, obj->client_nonce_len);
  if(err) return err;

  // close
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return err;

  // return size
  *written_length = cbor_encoder_get_buffer_size(&map, retval);

  return 0;
}

int decodeCrypt(uint8_t *buf,size_t buf_size, Crypt *retval, size_t index)
{
  CborParser parser;
  CborValue it;
  CborValue content;
  CborError err;

  err = cbor_parser_init(buf, buf_size, 0, &parser, &it);
  if(err) return err;

  CborType type = cbor_value_get_type(&it);

  if(type == CborMapType){
    err = cbor_value_enter_container(&it, &content);
    if(err) return err;
    //TODO: index has to be 0
  }else if(type == CborArrayType){
    err = cbor_value_enter_container(&it, &it);
    if(err) return  err;
  
    for(int i=0; i<index+1; i++){
      err = cbor_value_enter_container(&it, &content);
      if(err) return  err;
      err = cbor_value_advance(&it);
      if(err) return err;
    }
  }else{
    //error
  }

  int64_t tag_id;
  while (!cbor_value_at_end(&content)) {
  
    CborType tag_id_type = cbor_value_get_type(&content);
    if(tag_id_type != CborIntegerType) return 2002;
  
    cbor_value_get_int64(&content, &tag_id);
    err = cbor_value_advance_fixed(&content);
    if(err) return  err;
  
    if(!err){
      //handle value
      CborType value_type = cbor_value_get_type(&content);
      switch (tag_id) {
        case 0:
          if(value_type == CborIntegerType){
            int64_t val;
            cbor_value_get_int64(&content, &val);
            //msgId value
            if(val != 51) return 2003;
            err = cbor_value_advance_fixed(&content);
            break;
          }
        // ssid
        case 11:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return err;
                uint8_t data_arr[size];
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return  err;
                setCryptAdminKey(retval, data_arr, size);
                break;
            }

        // password
        case 12:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return err;
                uint8_t data_arr[size];
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return  err;
                setCryptServerNonce(retval, data_arr, size);
                break;
            }
          case 13:
            if(value_type == CborByteStringType){
                size_t size;
                err = cbor_value_get_string_length(&content, &size);
                if(err) return err;
                uint8_t data_arr[size];
                err = cbor_value_copy_byte_string(&content, data_arr, &size, &content);
                if(err) return  err;
                setCryptClientNonce(retval, data_arr, size);
                break;
            }
            break;
          default:
            return 2006;
        }
  
      }
  
    }
    return err;
}