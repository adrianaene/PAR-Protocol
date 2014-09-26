#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include <time.h>

#define HOST "127.0.0.1"
#define PORT 10000

//functie care imi transforma un integer in format binar;
char* toBin(int x){
	char* rez = malloc(9 * sizeof(char));
	int rest;
	int i;
	for(i = 0; i < 8; i++)
		rez[i] = '0';
	int k = 7;
	//impart succesiv la 2 pentru pentru a obtine numarul in format binar;
	while(x){
		rest = x % 2;
		x = x/2;
		rez[k] += rest;
		k--;
	}
	rez[8] = '\0';
	return rez;
}

//functie care imi calculeaza CheckSUm facand xor pe toate caracterele payload-ului;
char getXor(char* date){
	char xor = date[0];
	int i;
	for(i = 1; i < strlen(date); i++){
		xor ^= date[i];
	}
	return xor;
}

int main(int argc,char** argv){
	init(HOST,PORT);
	msg t, *r;
	char nr_secv_sender = 0;
  	
	FILE *fd = fopen("date_intrare", "r");	
	
	//variabile folosite pentru a scrie data si ora in fisierul log.txt;    
	time_t timp = time(NULL);
	struct tm timp_str = *(localtime(&timp));

	//alegem o dimensiune de pachet aleatoare, care imi intoarca un numar intre 1 si 60;
	int rnd = rand() % 61;
	
	//cat timp mai citesc din fisier;
	while(fgets(t.payload, rnd, fd) != NULL){
		//pun textul in payload;
		t.len = strlen(t.payload);
		//calculez CheckSum
		char CheckSum = getXor(t.payload);	
		
		//printez data, ora, numarul secventei, payloadul si CheckSumul cadrului trimis;		
		printf("[sender] %d-%02d-", timp_str.tm_year + 1900, timp_str.tm_mon + 1);
		printf("%d %2d:%2d:%02d\n", timp_str.tm_mday, timp_str.tm_hour, timp_str.tm_min, timp_str.tm_sec);
		printf("Am trimis urmatorul pachet:\n");
		printf("Seq_No: %d\n", nr_secv_sender);
		printf("Payload: %s\n", t.payload);
		printf("CheckSum: %s\n", toBin(CheckSum));
		printf("--------------------------------------------------------------------\n");
		
		//pun paritatea pe penultima pozitie
		t.payload[t.len + 2] = CheckSum;
		//pun seq_nr pe ultima pozitie
		t.payload[t.len + 3] = nr_secv_sender;

		//trimit mesajul
		send_message(&t);

		//resetez dimensiunea pachetului de trimis  	
		rnd = rand() % 61;

		//primesc ack; setez valoarea de timeout la 200 ms
		r = receive_message_timeout(200);
		
		//cat timp nu am trimis pachetul in timp util;
		while(r == NULL){
			//printez data, ora, numarul secventei, payloadul si CheckSumul cadrului trimis;	
			printf("[sender] %d-%02d-", timp_str.tm_year + 1900, timp_str.tm_mon + 1);
			printf("%d %2d:%2d:%02d\n", timp_str.tm_mday, timp_str.tm_hour, timp_str.tm_min, timp_str.tm_sec);
			printf("Am trimis urmatorul pachet:\n");
			printf("Seq_No: %d\n", nr_secv_sender);
			printf("Payload: %s\n", t.payload);
			printf("CheckSum: %s\n", toBin(CheckSum));
			printf("--------------------------------------------------------------------\n");
			send_message(&t);
			r = receive_message_timeout(200);
		}	
		
		char nr_secv_receiver = r->payload[0];
		
		//cat timp primesc ack pentru acelasi pachet, il retrimit
		//si reprimesc ack pentru a ma asigura ca a fost trimis bine
		while(nr_secv_receiver == nr_secv_sender || r == NULL){
			//printez data, ora, numarul secventei, payloadul si CheckSumul cadrului trimis;
			printf("[sender] %d-%02d-", timp_str.tm_year + 1900, timp_str.tm_mon + 1);
			printf("%d %2d:%2d:%02d\n", timp_str.tm_mday, timp_str.tm_hour, timp_str.tm_min, timp_str.tm_sec);
			printf("Am trimis urmatorul pachet:\n");
			printf("Seq_No: %d\n", nr_secv_sender);
			printf("Payload: %s\n", t.payload);
			printf("CheckSum: %s\n", toBin(CheckSum));
			printf("--------------------------------------------------------------------\n");
			send_message(&t);
			r = receive_message_timeout(200);
			nr_secv_receiver = r->payload[0];
		}
		//incrementez numarul secventei;
		nr_secv_sender++;
	}

	return 0;
}
