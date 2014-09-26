#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"
#include <time.h>
#define HOST "127.0.0.1"
#define PORT 10001

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
	msg r,t;
	init(HOST,PORT);
	//date primite vor fi scrise in fisierul date_iesire;
	FILE *fo = fopen("date_iesire", "w");	
	char nr_secv_receiver = 0;
	
	//variabile folosite pentru a scrie data si ora in fisierul log.txt; 	
	time_t timp = time(NULL);
        struct tm timp_str = *(localtime(&timp));
	
	//cat timp primesc mesaje de la sender;
	while(recv_message(&r) > 0){
		//extrag CheckSum primit;
		char CheckSum_sender = r.payload[r.len + 2];
		char nr_secv_sender = r.payload[r.len + 3];	
	
		//extrag mesajul pentru a calcula CheckSum;
		char mesaj[60];
		strncpy(mesaj, r.payload, r.len + 1);

		//verific daca paritatea primita e egala cu cea calculata;
		if(CheckSum_sender == getXor(mesaj)){
			//printez data, ora, numarul secventei si CheckSumul cadrului trimis;
			printf("[receiver] %d-%02d-", timp_str.tm_year + 1900, timp_str.tm_mon + 1);
			printf("%d %2d:%2d:%02d\n", timp_str.tm_mday, timp_str.tm_hour, timp_str.tm_min, timp_str.tm_sec);
			printf("Trimit ACK:\n");
			printf("Seq_No: %d\n", nr_secv_sender);
			printf("CheckSum: %s\n", toBin(CheckSum_sender));

			printf("mesaj primit corect: %s\n", r.payload);
			printf("--------------------------------------------------------------------\n");
			//scriu in fisier mesajul primit	
			fprintf(fo, "%s", r.payload);
			fflush(fo);
			//trimit ack pentru urmatorul nr_secv
			t.payload[0] = ++nr_secv_receiver;
			t.payload[1] = t.payload[0];
			send_message(&t);
		} else {
			//daca nu s-a trimis bine, cer sa retrimita cadrul curent
			//printez data, ora, numarul secventei si CheckSumul cadrului trimis;
			printf("[receiver] %d-%02d-", timp_str.tm_year + 1900, timp_str.tm_mon + 1);
                        printf("%d %2d:%2d:%02d\n", timp_str.tm_mday, timp_str.tm_hour, timp_str.tm_min, timp_str.tm_sec);
                        printf("Am primit urmatorul pachet:\n");
                        printf("Seq_No: %d\n", nr_secv_sender);
                        printf("Payload: %s\n", mesaj);
			printf("Am calculat checksum si am detectat eroare. Voi trimite ACK pentru Seq No %d", nr_secv_receiver);
			printf("(ultimul cadru corect pe care l-am primit)\n");
			printf("--------------------------------------------------------------------\n");

			t.payload[0] = nr_secv_receiver;
			t.payload[1] = t.payload[0];
			send_message(&t);
		}
	}
	
	fclose(fo);
	return 0;
}
