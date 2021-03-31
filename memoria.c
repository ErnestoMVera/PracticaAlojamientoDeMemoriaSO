#include<stdio.h>
#include<unistd.h>
#include<ncurses.h>
#include<stdlib.h>
#include<string.h>
#define MAX 1000
#define SIZE 100
// La estructura espacio representa un proceso o un agujero en memoria.
struct Espacio {
	char nombre[140];
	// Si tipo = 0 agujero, tipo = 1 es un proceso y nombre != null
	int tipo;
	long int tamanio, inicio, fin;
};
void redibujar(struct Espacio* memoria, int cuenta);
void asignar(struct Espacio* memoria, char* nombre, int bytes, int* cuentaEspacios, int indice);
void release(struct Espacio *memoria,char* nombre, int cuenta);
int worstFit(struct Espacio* memoria,int bytes,int cuentaEspacios);
void imprimirBloque(int bytesTotales);
void ejecutar(struct Espacio*,int*,char*,char*,int);
void compactar(struct Espacio* memoria, int* cuenta, int bytesTotales);

int main(int argc, char *argv[]) {
	struct Espacio memoria[MAX];
	int cuenta=1;
	int bytesTotales = 1024;
	// Variables necesarias para leer el archivo.
	FILE *in;
	char *temp;
	char linea[SIZE];
	char *comando;
	char *nombre;
	int tamanio;
	memoria[0].tipo = 0;
	memoria[0].tamanio = bytesTotales;
	memoria[0].inicio = 0;
	memoria[0].fin = bytesTotales;
	initscr();
	raw();
	imprimirBloque(bytesTotales);
	// Ciclo que lee las lineas del archivo.
	in = fopen(argv[1],"r");
	while (fgets(linea,SIZE,in) != NULL) {
		temp = strdup(linea);
		// Leer el comando y sus argumentos.
        	comando = strsep(&temp," ");
        	nombre = strsep(&temp," ");
		tamanio = atoi(strsep(&temp," "));
		//tamanio = strcmp(comando,"RQ") ? atoi(strsep(&temp," ")) : 0;
        	// ejecutarlos uno por uno.
        	ejecutar(memoria,&cuenta,comando,nombre,tamanio);
        	free(temp);
		refresh();
    	}
	fclose(in);
	sleep(4);
	endwin();
	return 0;
}
/**
  * Imprime el bloque de memoria graficamente.
**/
void imprimirBloque(int bytesTotales) {
	int i,max = 100;
	mvprintw(10,0, "|");
	for(i = 1; i <= 100;i++) 
		mvprintw(9,i,"_");
	mvprintw(8,0,"0");
	mvprintw(10,101,"|");
	mvprintw(8,101,"%d",bytesTotales);
	refresh();
}
/**
* 	Ejecuta los comandos que vienen del archivo de texto.
**/
void ejecutar(struct Espacio* memoria,int* cuenta,char* comando,char* nombre,int tamanio) {
	int indiceAsignacion;
	if(strcmp(comando, "RQ") == 0) {
		indiceAsignacion = worstFit(memoria,tamanio,*cuenta);
		if(indiceAsignacion != -1) {
			asignar(memoria,nombre,tamanio,cuenta,indiceAsignacion);		
		}
	}
	else if(strcmp(comando, "RL") == 0) {
		release(memoria, nombre, *cuenta);	
	}
	else if(strcmp(comando,"C") == 0) {
		compactar(memoria, cuenta, 1024);
		redibujar(memoria,*cuenta);
	}
}
/**
* Algoritmo worst fit para encontrar el indice de memoria donde se colocara el proceso.
* Regresa el indice en el que se debe alojar la memoria.
**/
int worstFit(struct Espacio* memoria,int bytes,int cuentaEspacios) {
	int mayor = memoria->tamanio;
	int indiceHoyo=0,i;
	int head; // posicion donde se dibujara el asterisco.
	if(mayor < bytes || memoria->tipo == 1) {
		mayor = -1;
		head = 120;
		mvprintw(11,head,"*");
		refresh();
		sleep(1);
	}
	for(i = 0; i < cuentaEspacios; i++) {
	    	if(mayor < (memoria->tamanio) && bytes <= (memoria->tamanio) && (memoria->tipo) != 1) {
        		mayor = (memoria->tamanio);
        		indiceHoyo = i;
			refresh();
			sleep(1);
    		}
		mvprintw(11,head," ");
		head = memoria->tamanio/20+memoria->inicio/10;	
		mvprintw(11,head,"*");
    		memoria++;
	}
	refresh();
	sleep(1);
	mvprintw(11,head," ");
	refresh();
	if(mayor != -1)
    	return indiceHoyo;
	else
    	return -1;
}
/**
* Asigna un bloque de memoria en un indice en particular.
**/
void asignar(struct Espacio* memoria, char* nombre, int bytes, int* cuentaEspacios, int indice) {
	int i;
	if(bytes == memoria[indice].tamanio) {
    		memoria[indice].tamanio = bytes;
    		memoria[indice].tipo = 1;
    		strcpy(memoria[indice].nombre,nombre);
		mvprintw(10,memoria[indice].tamanio/20+memoria[indice].inicio/10,"%s",nombre);
	}
	else {
    		(*cuentaEspacios)++;
    		// Creacion del hoyo si aplica.
		struct Espacio hoyo;
		hoyo.fin = memoria[indice].fin;
		hoyo.inicio = memoria[indice].inicio+bytes;
		hoyo.tamanio = hoyo.fin - hoyo.inicio;
		hoyo.tipo = 0;
		// Asignacion del proceso.
		memoria[indice].fin = hoyo.inicio;
		memoria[indice].tamanio = bytes;
		memoria[indice].tipo = 1;
		strcpy(memoria[indice].nombre,nombre);
		for(i = (*cuentaEspacios-1); i > indice;i--) {
			memoria[i] = memoria[i-1];
		}
		memoria[indice+1] = hoyo;
		mvprintw(10,memoria[indice].tamanio/20+memoria[indice].inicio/10,"%s",nombre);
		mvprintw(10,hoyo.inicio/10,"|");
		refresh();
		sleep(1);
	}
}
/**
* Libera la memoria de un proceso.
**/
void release(struct Espacio *memoria,char* nombre, int cuenta) {
	int i, encontrado=-1,head=120;
	for(i = 0; i < cuenta;i++) {
		if(strcmp(nombre,memoria[i].nombre) == 0 && memoria[i].tipo == 1) {
			encontrado = i;
			break;
		}
	}
	if(encontrado != -1) {
		memoria[encontrado].tipo = 0;
		mvprintw(10,memoria[encontrado].tamanio/20 + memoria[encontrado].inicio/10,"  ");
	}
	refresh();
}
/**
* Se borran los procesos antiguos y se dibujan los nuevos despues de la compactacion.
**/
void redibujar(struct Espacio* memoria, int cuenta) {
	int i;
	char* nombre = "";
	for(i = 1; i <= 101;i++) 
		mvprintw(10,i," ");
	for(i = 0; i < cuenta;i++) {
		mvprintw(10,memoria[i].tamanio/20+memoria[i].inicio/10,"%s",memoria[i].tipo == 1 ? memoria[i].nombre:nombre);
		if(memoria[i].tipo == 1)
			mvprintw(10,memoria[i].fin/10,"|");
	}
	refresh();
	sleep(1);
}
/**
* Compacta la memoria principal.
**/
void compactar(struct Espacio* memoria, int* cuenta, int bytesTotales) {
	int encontrado = 0;
	//Los procesos se mueven a la izquierda
	for(int i = 0; i < *cuenta; i ++){
    		if(memoria[i].tipo == 1){ // Si es un proceso se movera a la derecha hasta que ya no pueda mas.
        		encontrado = i;
        		if((encontrado - 1) >= 0) {
            			while(memoria[encontrado - 1].tipo == 0) {
					// Logica para que los procesos se muevan a la izquierda y los hoyos a la derecha.
					strcpy(memoria[encontrado - 1].nombre, memoria[encontrado].nombre);
					memoria[encontrado - 1].tipo = 1;
					memoria[encontrado - 1].tamanio = memoria[encontrado].tamanio;
					memoria[encontrado - 1].fin = memoria[encontrado - 1].inicio + memoria[encontrado - 1].tamanio;
					memoria[encontrado].tipo = 0;
					memoria[encontrado].inicio = memoria[encontrado - 1].fin;
					memoria[encontrado].tamanio = (memoria[encontrado].fin - memoria[encontrado].inicio);
					encontrado --;
					if(encontrado == 0){
						break;
					}
				}
			}
    		}
	}
	//Los huecos de la derecha se juntan
	encontrado = 0;
	for(int i = 0; i < *cuenta; i ++) {
		if(memoria[i].tipo == 0){
			*cuenta = i + 1;
			memoria[i].fin = bytesTotales;
			memoria[i].tamanio = (memoria[i].fin - memoria[i].inicio);
		}
	}
}
