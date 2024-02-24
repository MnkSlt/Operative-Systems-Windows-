#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <tchar.h>
#include <stdbool.h>
#include <windows.h>
#include <stdbool.h>

#define TAM 100
#define PIPE_NAME TEXT("\\\\.\\pipe\\tp")
#define QUEMSZ 60
#define MSGTXTSZ 60

typedef struct {
	int x;
	int y;
}Posicao;

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

typedef struct {
	int termina;
	HANDLE hPipe;

}ThreadPassageiroDados;

#define Msg_Sz sizeof(InfoPassageiro)

typedef struct {
	TCHAR nome[TAM];
}Passageiro;

DWORD WINAPI ThreadPassageiroReader(LPVOID lpvParam); 

int DeveContinuar = 1;
int ReaderAlive = 0;
