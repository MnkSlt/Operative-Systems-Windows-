#ifndef FUNCOESA_H
#define FUNCOESA_H

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <tchar.h>
#include <stdbool.h>
#include <windows.h>

#define TAM 30
#define TAM_BUFFER 20
#define CHAVE TEXT("Projecto")
#define MAPATAM 100


typedef struct {
	int x;
	int y;
}Posicao;

typedef struct {
	int id;
	Posicao pos;
	Posicao posDestino;
}Avioes;

typedef struct {
	int map[MAPATAM][MAPATAM];
}Mapa;

typedef struct {
	int nAvioes, posE, posL, tamMaxAvioes, flagAceitaAvioes, flagPassageirosEmbarque, flagPassageirosEmViagem;
	Mapa m;
	Avioes buffer[TAM_BUFFER];
}BufferCircular;

typedef struct {
	TCHAR nome[TAM];
	Posicao pos;
}Aeroportos;

typedef struct {
	HANDLE hMutex,hMutexMapa, semMutex, hSemItens, hSemVazios;
	BufferCircular* memPar;
	Posicao destino;
	Posicao origem;
	Aeroportos aux;
	int id, terminar, criaAviaoFlag, viagemAviaoFlag, velocidade, escolhidoDestinoFlag;
}RecebeEnviaPedidosAviao;

typedef struct {
	HANDLE hMutex;
	Aeroportos* fileViewMap;
	LPVOID tamanhoAero;
	Aeroportos* aero;
}RecebeInfoAero;

typedef struct
{
	HANDLE hThreads[2], hFileMap, hFileMap2, hFileMap3;
	TCHAR destino[TAM], comando[TAM], comandoViagem[TAM];
	BOOL primeiroAcesso;
	DWORD Maximo_Avioes, Maximo_Aeroportos;
}Info;

int accessNumMaxAvioes();
void listaAero(Aeroportos* aero, int TamanhoAero);

#endif