
all: ign.c
	gcc -g -Wall ign.c -lpaho-mqtt3cs -lpthread  -o ign

list: test_list.c list.h task_queue.h
	gcc -g -Wall test_list.c  -o list

clean:
	rm ign list log_ign 

