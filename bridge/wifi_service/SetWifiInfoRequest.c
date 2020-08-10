#include "bridge/wifi_service/SetWifiInfoRequest.h"
#include <string.h>
#include "bridge/lock/external/cbor/cbor.h"


int getWifiInfoRequest(SetWIFIInfoRequest **pp_wifi_info)
{
  if(!pp_wifi_info) return 1;
  if (*pp_wifi_info) free(*pp_wifi_info);
  *pp_wifi_info = NULL;
  *pp_wifi_info = malloc(sizeof(SetWIFIInfoRequest));
  if (!*pp_wifi_info) return 1;
  return 0;
}

void initWifiInfoRequest(SetWIFIInfoRequest * wifi_info)
{
  memset(wifi_info, 0, sizeof(SetWIFIInfoRequest));
}

void deinitWifiInfoRequest(SetWIFIInfoRequest * wifi_info)
{
  if (wifi_info->has_ssid)
  {
    wifi_info->has_ssid = false;
    free(wifi_info->ssid);
    wifi_info->ssid = NULL;
    wifi_info->ssid_len = 0;
  }

  if (wifi_info->has_token)
  {
    wifi_info->has_token = false;
    free(wifi_info->token);
    wifi_info->token = NULL;
    wifi_info->token_len = 0;
  }

  if (wifi_info->has_password)
  {
    wifi_info->has_password = false;
    free(wifi_info->password);
    wifi_info->password = NULL;
    wifi_info->password_len = 0;
  }
}

int setWifiInfoRequestSSID(SetWIFIInfoRequest * wifi_info_request, char *ssid, size_t ssid_len)
{
  if (!wifi_info_request) return 1;
  wifi_info_request->ssid = malloc(ssid_len);
  memset(wifi_info_request->ssid,0, sizeof(wifi_info_request->ssid));
  memcpy(wifi_info_request->ssid, ssid, ssid_len);
  wifi_info_request->has_ssid = true;
  wifi_info_request->ssid_len = ssid_len;
  return 0;
}

int setWifiInfoRequestToken(SetWIFIInfoRequest * wifi_info_request, char *token, size_t token_len)
{
  if (!wifi_info_request) return 1;
  wifi_info_request->token = malloc(token_len);
  memset(wifi_info_request->token,0, sizeof(wifi_info_request->token));
  memcpy(wifi_info_request->token, token, token_len);
  wifi_info_request->has_token = true;
  wifi_info_request->token_len = token_len;
  return 0;
}

int setWifiInfoRequestPassword(SetWIFIInfoRequest * wifi_info_request, char *password, size_t password_len)
{
  if (!wifi_info_request) return 1;
  wifi_info_request->password = malloc(password_len);
  memset(wifi_info_request->password,0, sizeof(wifi_info_request->password));
  memcpy(wifi_info_request->password, password, password_len);
  wifi_info_request->has_password = true;
  wifi_info_request->password_len = password_len;
  return 0;
}


int isWifiInfoRequestInvalid(SetWIFIInfoRequest * obj)
{
  // obj is null, invalid
  if (!obj) return 1;
  return !obj->has_password && !obj->has_ssid;
}

int encodeWifiInfoRequest(SetWIFIInfoRequest * obj, uint8_t *retval, size_t length,size_t *written_length)
{
  if (isWifiInfoRequestInvalid(obj)) return 1;

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

  // add ssid
  err = cbor_encode_uint(&map, 11);
  if(err) return err;
  err = cbor_encode_byte_string(&map, obj->ssid, obj->ssid_len);
  if(err) return err;

  // add password
  err = cbor_encode_uint(&map, 12);
  if(err) return err;
  err = cbor_encode_byte_string(&map, obj->password, obj->password_len);
  if(err) return err;

  // add token
  err = cbor_encode_uint(&map, 13);
  if(err) return err;
  err = cbor_encode_byte_string(&map, obj->token, obj->token_len);
  if(err) return err;

  // close
  err = cbor_encoder_close_container(&encoder, &map);
  if(err) return err;

  // return size
  *written_length = cbor_encoder_get_buffer_size(&map, retval);

  return 0;
}

int decodeWifiInfoRequest(uint8_t *buf,size_t buf_size, SetWIFIInfoRequest *retval,size_t index)
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
                setWifiInfoRequestSSID(retval, data_arr, size);
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
                setWifiInfoRequestPassword(retval, data_arr, size);
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
                setWifiInfoRequestToken(retval, data_arr, size);
                break;
            }
          default:
            return 2006;
        }
  
      }
  
    }
    return err;
}