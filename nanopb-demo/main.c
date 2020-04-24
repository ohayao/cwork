#include <stdio.h>
#include <string.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "proto.pb.h"

int main()
{
    uint8_t buf[1024];

    //encode
    proto_question qst={};
    strcpy(qst.topic,"Hello, this is 主题!@#$￥");
    strcpy(qst.tips,"nanopb ...");

    memset(buf,0,sizeof(buf));
    pb_ostream_t out=pb_ostream_from_buffer(buf,sizeof(buf));
    if(pb_encode(&out,proto_question_fields,&qst)){
        size_t ol = out.bytes_written;
        printf("encode OK! Len=%ld,[%s]\n",ol,buf);

    }else printf("encode FAILED !!!\n");
    
    //decode
    proto_question res={};
    pb_istream_t in=pb_istream_from_buffer(buf,strlen((char*)buf));
    if(pb_decode(&in,proto_question_fields,&res)){
        printf("decode result [Topic:%s][Tips:%s]\n",res.topic,res.tips);
    }else printf("decode FAILED !!!\n");

    return 0;
}

