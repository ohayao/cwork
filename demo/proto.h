#ifndef _PROTO_H_
#define _PROTO_H_

#include <stdio.h>
#include <string.h>

#include "proto.pb-c.h"


/*
 * 打包Question
 * */
size_t proto_packQuestion(uint8_t* result,char* topic,char* tips);

/*
 * 解包Question
 * */
Proto__Question* proto_unPackQuestion(const uint8_t* data,size_t len);


/*
 * 打包Answer
 * */
size_t proto_packAnswer(uint8_t* resultBuf,char* result,int times);

/*
 * 解包Answer
 * */
Proto__Answer* proto_unPackAnswer(const uint8_t* data,size_t len);

/*
 * 测试打包解包（protobuf）
 * */
void proto_test();


/*
 *
 * 函数实现
 * */

size_t proto_packQuestion(uint8_t* result, char* topic,char* tips){
    Proto__Question question=PROTO__QUESTION__INIT;
    question.topic=topic;
    question.tips=tips;
    return proto__question__pack(&question,result);
}

Proto__Question* proto_unPackQuestion(const uint8_t* data,size_t len){
    return proto__question__unpack(NULL,len,data);
}

size_t proto_packAnswer(uint8_t* resultBuf,char* result, int times){
    Proto__Answer answer=PROTO__ANSWER__INIT;
    answer.result=result;
    answer.times=times;
    return proto__answer__pack(&answer,resultBuf);
}

Proto__Answer* proto_unPackAnswer(const uint8_t* data,size_t len){
    return proto__answer__unpack(NULL,len,data);
}

void proto_test(){
    uint8_t resTest[1024];
    memset(resTest,0,sizeof(resTest));
    proto_packQuestion(resTest, "Hello world，来点中文，特殊符号*&@#（）@￥","This is tips，能正确显示吗*(#@#% ^#%$$)");
    printf("----pack result：%s\n",(char*)resTest);


    Proto__Question* question=proto_unPackQuestion(resTest,strlen((char*)resTest));
    printf("----unpack result：\nTopic=%s\tTips=%s\n",question->topic,question->tips);
}

#endif
