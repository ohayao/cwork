
all: ign.c 
	gcc -g -Wall ign.c  pb_decode.c  pb_encode.c pb_common.c ign.pb.c cJSON.c -lpaho-mqtt3cs -lpthread  -o ign

list: test_list.c list.h task_queue.h
	gcc -g -Wall test_list.c  -o test_list

msgid:
	gcc -g -Wall test_msg_id.c -lpthread -o test_msg_id

clean:
	rm ign test_list  test_msg_id log_ign 

