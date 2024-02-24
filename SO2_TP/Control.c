#include "FuncoesC.h"

DWORD WINAPI ThreadEnviaDadosPassageiro(LPVOID lpvParam)
{
	DadosP* dados = (DadosP*)lpvParam;
	InfoPassageiro dadosPass;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = (HANDLE)dados->hPipe; // a informação enviada a thread e o handle do pipe
	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	int cbBytesRead = 0, cbReplyBytes = 0;
	int numresp = 0;
	int flag2 = 1;
	int flagAeroO = 0;
	int flagAeroD = 0;
	int flagRead = 0;

	if (hPipe == NULL) {
		_tprintf(TEXT("\nErro - o handle enviado no param da thread e nulo"));
		return -1;
	}

	ReadReady = CreateEvent(
		NULL, // default security+ non inheritable
		TRUE,
		FALSE, // Reset manual, por requisite do overlapped IO=> uso de ResetEvent FALSE, // estado inicial = not signoled
		NULL); // nao precisa de nome: uso interno ao processo

	if (ReadReady == NULL) {
		_tprintf(TEXT("\nServidor: nao foi possivel criar o evento Read. Mais vale parar ja"));
		return 1;
	}

	adicionaPassageiro(hPipe); // Regista cliente. Se ja for conhecido nao faz nada

	while (flag2 == 1) { // termina mediante uma condi ao qualquer // Ciclo de dialogo com o cliente

		if (flagRead == 0)
		{
			ZeroMemory(&OverlRd, sizeof(OverlRd));
			ResetEvent(ReadReady);
			OverlRd.hEvent = ReadReady;
			// Obtem mensagem do passageiro

			fSuccess = ReadFile(
				hPipe,
				&dadosPass,
				Msg_Sz,
				&cbBytesRead,
				&OverlRd);

			WaitForSingleObject(ReadReady, INFINITE);
			GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE); // sem WAIT 

			if (cbBytesRead < Msg_Sz)
				_tprintf(TEXT("\nReadFile nao leu os dados todos. Erro = %d"), GetLastError());

			dadosPass.entrou = 0;
			_tprintf(TEXT("\nServidor: Recebi de: [%s] msg: [%s][%s]"), dadosPass.passageiro, dadosPass.aOrigem, dadosPass.aDestino);
			flagRead = 1;
		}
		if (flagAeroO == 0 && flagAeroD == 0)
		{

			for (int i = 0; i < sizeof(dados->aux); i++)
			{ // verifica se tem o aeroporto origem e destino
				if (_tcscmp(dados->aux[i].nome, dadosPass.aOrigem) == 0)
				{
					flagAeroD = 1;
					dadosPass.entrou++;
					dadosPass.posx = dados->aux[i].pos.x;
					dadosPass.posy = dados->aux[i].pos.y;

				}
				if (_tcscmp(dados->aux[i].nome, dadosPass.aDestino) == 0)
				{
					dadosPass.posDx = dados->aux[i].pos.x;
					dadosPass.posDy = dados->aux[i].pos.y;
					flagAeroO = 1;
					dadosPass.entrou++;

				}
				//dadosPass.entrou = 0;

			}
			if (dadosPass.entrou == 2)
			{
				writePassageiroASINC(hPipe, dadosPass);
				dadosPass.entrou = 0;
			}

		}
		if (dados->auxAviao->flagPassageirosEmbarque == 1)
		{
			for (int i = 0; i < dados->auxAviao->nAvioes; i++) {
				if (dados->auxAviao->buffer[i].pos.x == dadosPass.posx &&
					dados->auxAviao->buffer[i].pos.y == dadosPass.posy &&
					dados->auxAviao->buffer[i].posDestino.x == dadosPass.posDx &&
					dados->auxAviao->buffer[i].posDestino.y == dadosPass.posDy)
				{
					//verifica se aviao vai para mesmo aeroporto que passageiro
					dadosPass.embarca = 1;
					dadosPass.AviaoId = dados->auxAviao->buffer[i].id;
					writePassageiroASINC(hPipe, dadosPass);
					dados->auxAviao->flagPassageirosEmbarque = 0;
					dadosPass.embarca = 0;
					break;
				}
			}

		}
		if (dados->auxAviao->flagPassageirosEmViagem == 1)
		{
			for (int i = 0; i < dados->auxAviao->nAvioes; i++) {
				if (dados->auxAviao->buffer[i].id == dadosPass.AviaoId)
				{

					dadosPass.posx = dados->auxAviao->buffer[i].pos.x;
					dadosPass.posy = dados->auxAviao->buffer[i].pos.y;
					dadosPass.emViagem = 1;
					writePassageiroASINC(hPipe, dadosPass);
					if (dados->auxAviao->buffer[i].posDestino.x == dados->auxAviao->buffer[i].pos.x &&
						dados->auxAviao->buffer[i].posDestino.y == dados->auxAviao->buffer[i].pos.y)
					{
						writePassageiroASINC(hPipe, dadosPass);
						dadosPass.termina = 1;
					}
					dadosPass.emViagem = 0;
					dados->auxAviao->flagPassageirosEmViagem = 0;
					break;
				}
			}
			//dados->auxAviao.flagPassageirosEmbarque = 0;
			//break;

		}
		if (dadosPass.termina == 1)
		{

			flag2 = 0;


		}

	}
	removePassageiro(hPipe);


	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	_tprintf(TEXT("\nPassageiro %s saiu do controlador"), dadosPass.passageiro);

	return 1;

}

DWORD WINAPI ThreadRecebePassageiro(LPVOID lpvParam)
{
	ThreadDadosPassageiro* dados = (ThreadDadosPassageiro*)lpvParam;

	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeexemplo");
	while (1)
	{
		dados->pass.hPipe = CreateNamedPipe(
			lpszPipename,	//  name do pipe
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			5000,
			NULL);

		if (dados->pass.hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("\nCreateNamedPipe falhou, erro  %d"), GetLastError());
			return -1;
		}


		dados->fconnected = ConnectNamedPipe(dados->pass.hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (dados->fconnected)
		{
			dados->hThread = CreateThread(NULL, 0, ThreadEnviaDadosPassageiro, &dados->pass, 0, &dados->threadID);
			if (dados->hThread == NULL) {
				_tprintf_s(TEXT("Erro no CreateThread pipes,Erro = %d"), GetLastError());
				return -1;
			}
			else {
				CloseHandle(dados->hThread);
			}
		}
		else
			CloseHandle(dados->pass.hPipe);
	}
	return 0;
}


DWORD WINAPI ThreadEnviaInfoAeroporto(LPVOID param)
{
	EnviaInfoAero* dados = (EnviaInfoAero*)param;

	while (dados->terminar != 1)
	{
		if (dados->atualiza == TRUE)
		{
			CopyMemory(dados->tamanhoAero, &dados->TamanhoAero, sizeof(dados->TamanhoAero));
			CopyMemory(dados->fileViewMap, dados->aero, sizeof(Aeroportos) * dados->TamanhoAero);
			dados->atualiza = FALSE;
		}

	}
}
DWORD WINAPI ThreadRecebeAvioes(LPVOID param)
{
	RecebeEnviaPedidosAviao* dados = (RecebeEnviaPedidosAviao*)param;
	Avioes aviao;
	int aux[MAPATAM][MAPATAM];


	for (int i = 0; i < MAPATAM; i++)
		for (int j = 0; j < MAPATAM; j++)
			aux[i][j] = 0;


	while (dados->terminar != 1)
	{


		WaitForSingleObject(dados->hSemItens, INFINITE);
		WaitForSingleObject(dados->hMutex, INFINITE);

		if (dados->memPar->nAvioes > dados->memPar->tamMaxAvioes)
		{
			dados->memPar->flagAceitaAvioes = 0;
			_tprintf_s(TEXT("\n Controlador não está a aceitar novos aviões. Máximo excedido.\n"));
			Sleep(2000);
		}
		//else {
		if (dados->memPar->flagPassageirosEmbarque == 1) {

			dados->memPar->flagAceitaAvioes = 1;
			CopyMemory(&aviao, &dados->memPar->buffer[dados->memPar->posE], sizeof(Avioes));
			CopyMemory(&dados->tP.pass.auxAviao, &dados->memPar, sizeof(dados->memPar));
			//dados->memPar->flagPassageirosEmbarque = 0;

		}
		if (dados->memPar->flagPassageirosEmViagem == 1)
		{
			//dados->memPar->flagAceitaAvioes = 1;
			CopyMemory(&aviao, &dados->memPar->buffer[dados->memPar->posE], sizeof(Avioes));
			CopyMemory(&dados->tP.pass.auxAviao, &dados->memPar, sizeof(dados->memPar));
			//dados->memPar->flagPassageirosEmViagem = 0;
		}
		dados->memPar->posE = (dados->memPar->posE + 1) % dados->memPar->tamMaxAvioes;

		ReleaseMutex(dados->hMutex, INFINITE);
		ReleaseSemaphore(dados->hSemVazios, 1, NULL);

	}
	for (int i = 0; i < MAPATAM; i++)
		free(aux[i]);
	free(aux);

	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {

	Info info;
	RecebeEnviaPedidosAviao dadosAviao;
	EnviaInfoAero aeroporto;
	//ThreadDadosPassageiro dadosPass;
	Mapa mapa;
	InfoPassageiro pass;


	int m[MAPATAM][MAPATAM];

	for (int i = 0; i < MAPATAM; i++)
		for (int j = 0; j < MAPATAM; j++)
			m[i][j] = 0;


	Avioes* aux;
	int i = 0;
	dadosAviao.tP.fconnected = FALSE;
	dadosAviao.tP.threadID = 0;


#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	info.aero_x = 0;
	info.aero_y = 0;
	aeroporto.atualiza = FALSE;
	aeroporto.aero = malloc(sizeof(Aeroportos));
	dadosAviao.tP.pass.aux = malloc(sizeof(Aeroportos));

	info.primeiroAcesso = FALSE;
	dadosAviao.tP.pass.hPipe = INVALID_HANDLE_VALUE;

	//registry
	{
		createRegistryKeys();
		info.Maximo_Aeroportos = accessNumMaxAeroportos();
		info.Maximo_Avioes = accessNumMaxAvioes();
	}

	//mutex, semáforos e mem partilhada
	{
		//semaforo - escrita - buffer circular
		dadosAviao.hSemItens = CreateSemaphore(NULL, info.Maximo_Avioes, info.Maximo_Avioes, TEXT("SO2_SEM_PRODUTOR"));
		//semaforo - leitura - buffer circular
		dadosAviao.hSemVazios = CreateSemaphore(NULL, 0, info.Maximo_Avioes, TEXT("SO2_SEM_VAZIOS"));
		//mutex para acesso concorrente das instâncias avião
		dadosAviao.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_CONSUMIDOR"));


		if (dadosAviao.hSemItens == NULL || dadosAviao.hSemVazios == NULL || dadosAviao.hMutex == NULL)
		{
			_tprintf_s(TEXT("\n Erro no CreateSemaforo ou no CreateMutex ou no CreateEvent."));
			return 1;
		}

		writeReady = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (writeReady == NULL)
		{
			_tprintf(TEXT("\nServidor: nao foi possivel criar o evento Write.  Mais  vale  parar  ja"));
			return 1;
		}
		iniciaPassageiros();



		//mem partilhada, para receber avioes, paradigma buffer circular
		info.hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));
		if (info.hFileMap == NULL)
		{
			info.primeiroAcesso = TRUE;
			info.hFileMap = CreateFileMapping(
				INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0,
				info.Maximo_Avioes * sizeof(BufferCircular),
				TEXT("SO2_MEM_PARTILHADA"));
			if (info.hFileMap == NULL)
			{
				_tprintf_s(TEXT("\n Erro no CreateFileMapping1\n"));
				return 1;
			}
		}
		//mem partilhada, para enviar aeroportos
		info.hFileMap2 = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			info.Maximo_Aeroportos * sizeof(Aeroportos),
			TEXT("SO2_MEM_PARTILHADA2"));

		if (info.hFileMap2 == NULL)
		{
			_tprintf_s(TEXT("\n Erro no CreateFileMapping2\n"));
			return 1;
		}

		//mem partilhada, para enviar tamanho dos aeroportos
		info.hFileMap3 = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			1024,
			TEXT("SO2_MEM_PARTILHADA3"));

		if (info.hFileMap3 == NULL)
		{
			_tprintf_s(TEXT("\n Erro no CreateFileMapping3\n"));
			return 1;
		}

		aeroporto.fileViewMap = (Aeroportos*)MapViewOfFile(info.hFileMap2, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (aeroporto.fileViewMap == NULL)
		{
			_tprintf_s(TEXT("\n Erro no MapViewOfFile, aeroporto.fileViewMap.\n"));
			return 1;
		}

		aeroporto.tamanhoAero = MapViewOfFile(info.hFileMap3, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (aeroporto.tamanhoAero == NULL)
		{
			_tprintf_s(TEXT("\n Erro no MapViewOfFile, aeroporto.tamanhoAero.\n"));
			return 1;
		}

		dadosAviao.memPar = (BufferCircular*)MapViewOfFile(info.hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (dadosAviao.memPar == NULL)
		{
			_tprintf_s(TEXT("\n Erro no MapViewOfFile, dadosAviao.memPar.\n"));
			return 1;
		}

		if (info.primeiroAcesso == TRUE)
		{
			dadosAviao.memPar->nAvioes = 0;
			dadosAviao.memPar->posE = 0;
			dadosAviao.memPar->posL = 0;
			dadosAviao.memPar->tamMaxAvioes = info.Maximo_Avioes;
			dadosAviao.memPar->flagAceitaAvioes = 1;
			dadosAviao.memPar->flagPassageirosEmbarque = 0;
			dadosAviao.memPar->flagPassageirosEmViagem = 0;
			dadosAviao.tP.pass.auxAviao = dadosAviao.memPar;

		}
		else
			return;

		dadosAviao.terminar = 0;

		info.hThreads[1] = CreateThread(NULL, 0, ThreadRecebeAvioes, &dadosAviao, 0, NULL);
		info.hThreads[0] = CreateThread(NULL, 0, ThreadEnviaInfoAeroporto, &aeroporto, 0, NULL);
		info.hThreads[2] = CreateThread(NULL, 0, ThreadRecebePassageiro, &dadosAviao.tP, 0, NULL);
		_tprintf(TEXT("\t\n * Controlador à espera de passageiros *\n\t"));



		if (info.hThreads[0] == NULL || info.hThreads[1] == NULL || info.hThreads[2] == NULL)
		{
			_tprintf_s(TEXT("\n Erro no CreateThread"));
			return 1;

		}
		aeroporto.TamanhoAero = 0;
	}

	do {


		_tprintf_s(TEXT("\n\n\n\n\t  * add -> Adicione um Aeroporto *\n\t* lista -> Listagem de todos os Aeroportos e Aviões *\n     * suspende -> Suspende a aceitação de novos aviões *\n        * ativa -> Ativa a aceitação de novos aviões *\n\t  * fim -> Desligar control *\n\n\n Insira um comando: "));
		do {
			_fgetts(info.comando, TAM, stdin);
			info.comando[_tcslen(info.comando) - 1] = '\0';
		} while (_tcscmp(info.comando, TEXT("")) == 0);

		if (_tcscmp(info.comando, _T("add")) == 0)
		{
			if ((aeroporto.TamanhoAero) >= info.Maximo_Aeroportos)
			{
				_tprintf_s(TEXT("\n Atingido o numero máximo de aeroportos."));

			}
			else {
				do {
					_tprintf_s(_T("\n Indique o nome do aeroporto: "));
				} while ((existeNomeAeroportos(aeroporto.aero, info.nomeAero, aeroporto.TamanhoAero)));						//verifica nome
				do {
					_tprintf_s(_T("\n Indique a posicao(x-y) do aeroporto: "));
				} while (!(coordenadasAeroportos(aeroporto.aero, &info.aero_x, &info.aero_y, aeroporto.TamanhoAero)));		//verifica coordenadas
				if (aeroporto.TamanhoAero == 0)//	caso nao exista nenhum aeroporto, adiciona 
				{
					_tcscpy_s(aeroporto.aero[aeroporto.TamanhoAero].nome, TAM, info.nomeAero);
					aeroporto.aero[aeroporto.TamanhoAero].pos.x = info.aero_x;
					aeroporto.aero[aeroporto.TamanhoAero].pos.y = info.aero_y;
					CopyMemory(&dadosAviao.tP.pass.aux, &aeroporto.aero, sizeof(aeroporto.aero));
					//	_tprintf(TEXT("\n%s\n%d\n%d"), dadosAviao.tP.pass.aux[aeroporto.TamanhoAero].nome, dadosAviao.tP.pass.aux[aeroporto.TamanhoAero].pos.x, dadosAviao.tP.pass.aux[aeroporto.TamanhoAero].pos.y);
						//dadosPass.pass.aux[aeroporto.TamanhoAero].pos.x = info.aero_x;
						//dadosPass.pass.aux[aeroporto.TamanhoAero].pos.y = info.aero_y;
					m[info.aero_x][info.aero_y] = -1;
					CopyMemory(&dadosAviao.memPar->m.map, &m, sizeof(m));
					aeroporto.TamanhoAero += 1;

				}
				else
				{
					aeroporto.aero = (Aeroportos*)realloc(aeroporto.aero, sizeof(Aeroportos) * (aeroporto.TamanhoAero + 1));
					dadosAviao.tP.pass.aux = (Aeroportos*)realloc(aeroporto.aero, sizeof(Aeroportos) * (aeroporto.TamanhoAero + 1));
					_tcscpy_s(aeroporto.aero[aeroporto.TamanhoAero].nome, TAM, info.nomeAero);
					aeroporto.aero[aeroporto.TamanhoAero].pos.y = info.aero_y;
					aeroporto.aero[aeroporto.TamanhoAero].pos.x = info.aero_x;
					CopyMemory(&dadosAviao.tP.pass.aux, &aeroporto.aero, sizeof(aeroporto.aero));
					//_tprintf(TEXT("\n%s\n%d\n%d"), dadosAviao.tP.pass.aux[aeroporto.TamanhoAero].nome, dadosAviao.tP.pass.aux[aeroporto.TamanhoAero].pos.x, dadosAviao.tP.pass.aux[aeroporto.TamanhoAero].pos.y);


					//dadosPass.pass.aux[aeroporto.TamanhoAero].pos.x = info.aero_x;
					//dadosPass.pass.aux[aeroporto.TamanhoAero].pos.y = info.aero_y;
					m[info.aero_x][info.aero_y] = -1;
					CopyMemory(&dadosAviao.memPar->m.map, &m, sizeof(m));
					aeroporto.TamanhoAero++;
				}

			}
			aeroporto.atualiza = TRUE;
		}
		else if (_tcscmp(info.comando, _T("lista")) == 0)			//Mostrar todos os aeroportos , avioes e passageiros bem como as suas informaçoes
		{
			listaTudo(aeroporto.aero, dadosAviao.memPar, info, aeroporto.TamanhoAero, dadosAviao.memPar->m.map);
		}
		else if (_tcscmp(info.comando, _T("suspende")) == 0)		//suspende a aceitação de novos avioes
		{
			_tprintf_s(TEXT("\n O controlador suspendeu a aceitação de novos aviões\n"));
			dadosAviao.memPar->flagAceitaAvioes = 0;
		}
		else if (_tcscmp(info.comando, _T("ativa")) == 0)			//suspende a aceitação de novos avioes
		{
			_tprintf_s(TEXT("\n O controlador está a permitir aceitar aviões de novo\n"));
			dadosAviao.memPar->flagAceitaAvioes = 1;
		}
		else if (_tcscmp(info.comando, _T("pass")) == 0)
		{
			_tprintf_s(TEXT("\n O controlador está a permitir aceitar aviões de novo\n"));

		}
	} while (_tcscmp(info.comando, _T("fim")) != 0);
	dadosAviao.terminar = 1;

	UnmapViewOfFile(dadosAviao.memPar);
	UnmapViewOfFile(aeroporto.tamanhoAero);
	UnmapViewOfFile(aeroporto.fileViewMap);

	CloseHandle(info.hFileMap);
	CloseHandle(info.hFileMap2);
	CloseHandle(info.hFileMap3);
	CloseHandle(info.hThreads[0]);
	CloseHandle(info.hThreads[1]);
	CloseHandle(dadosAviao.hSemItens);
	CloseHandle(dadosAviao.hSemVazios);
	CloseHandle(dadosAviao.hMutex);

	for (int i = 0; i < MAPATAM; i++)
		free(m[i]);
	free(m);

	free(aeroporto.aero);

	return 0;
}


