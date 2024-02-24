#ifndef FUNCOESC_H
#define FUNCOESC_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <strsafe.h>


#define TAM 30
#define BUFSIZE 2048
#define TAM_BUFFER 20
#define CHAVE TEXT("Projecto")
#define PIPE_NAME TEXT("\\\\.\\pipe\\tp")
#define MAPATAM 100
#define MAXPASSAGEIROS 10
#define QUEMSZ 60

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
	int nAvioes, posE, posL, tamMaxAvioes, flagAceitaAvioes,flagPassageirosEmbarque,flagPassageirosEmViagem;
	Mapa m;
	Avioes buffer[TAM_BUFFER];

}BufferCircular;

typedef struct {
	TCHAR nome[TAM];
	Posicao pos;
}Aeroportos;

typedef struct {
	TCHAR passageiro[QUEMSZ];
	TCHAR aOrigem[QUEMSZ];
	TCHAR aDestino[QUEMSZ];
	int termina;
	int embarca;
	int emViagem;
	int posx;
	int entrou;
	int posy;
	int AviaoId;
	int posDx;
	int posDy;
} InfoPassageiro;

#define Msg_Sz sizeof(InfoPassageiro)



typedef struct {
	HANDLE hMutex;
	Aeroportos* fileViewMap, *aero;
	LPVOID tamanhoAero;
	boolean atualiza;
	int terminar, TamanhoAero;	
}EnviaInfoAero;

typedef struct
{
	HANDLE hThreads[3], hFileMap, hFileMap2, hFileMap3;
	DWORD Maximo_Avioes, Maximo_Aeroportos;
	TCHAR nomeAero[TAM], comando[TAM];
	BOOL primeiroAcesso;
	int aero_x, aero_y;
}Info;

typedef struct {
	BufferCircular *auxAviao;
	HANDLE hPipe;
	Aeroportos* aux;
}DadosP;

typedef struct {
	HANDLE hThread;
	DWORD threadID;
	BOOL fconnected;
	DadosP pass;
}ThreadDadosPassageiro;

typedef struct {
	HANDLE hMutex, hSemItens, hSemVazios;
	BufferCircular* memPar;
	int terminar;
	ThreadDadosPassageiro tP;
}RecebeEnviaPedidosAviao;




HANDLE writeReady;
HANDLE passageiros[MAXPASSAGEIROS];

//registry
void createRegistryKeys();
int accessNumMaxAvioes();
int accessNumMaxAeroportos();

//aeroportos
int foraDoLimite(int aero_x, int aero_y, int TamanhoAero);
int existeCoordenadasAeroportos(Aeroportos* aero, int aero_x, int aero_y, int TamanhoAero);
int coordenadasAeroportos(Aeroportos* aero, int* aero_x, int* aero_y, int TamanhoAero);
int existeNomeAeroportos(Aeroportos* aero, TCHAR nomeAero[TAM], int TamanhoAero);

//geral
void listaTudo(Aeroportos* aero, BufferCircular* dadosAviao, Info info, int TamanhoAero);
void removePassageiro(HANDLE cli);
void iniciaPassageiros();
int writePassageiroASINC(HANDLE hPipe, InfoPassageiro msg);
int broadcastPassageiros(InfoPassageiro msg);

#endif