#include "Passageiro.h"



DWORD WINAPI ThreadPassageiroReader(LPVOID lpvParam) {

	ThreadPassageiroDados* dados = (ThreadPassageiroDados*)lpvParam;
	InfoPassageiro FromServer;
	BOOL fSuccess = FALSE;
	int cbBytesRead = 0;

	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	if (dados->hPipe == NULL) {
		_tprintf(TEXT("\nThread Reader - o handle recebido no param da thread e nulo\n"));
		return -1;
	}

	ReadReady = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);

	if (ReadReady == NULL) {

		_tprintf(TEXT("\nCLiente: nao foi possivel criar o Evento Read. Mais vale parar ja"));
		return 1;
	}

	ReaderAlive = 1;
	//_tprintf(TEXT("Thread Reader - a receber mensagens\n"));
	FromServer.entrou = 0;
	while (DeveContinuar) {
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(
			dados->hPipe,
			&FromServer,
			Msg_Sz,
			&cbBytesRead,
			&OverlRd);

		WaitForSingleObject(ReadReady, INFINITE);
		//_tprintf(TEXT("\nRead concluido"));

		if (FromServer.termina == 1)
		{
			dados->termina = 1;
			return -1;
		}
		if (FromServer.entrou == 2)
		{
			_tprintf(TEXT("\n\tPassageiro está no aeroporto %s à espera de avião com destino a %s  [%d,%d]"), FromServer.aOrigem, FromServer.aDestino, FromServer.posx, FromServer.posy);
		}
		if (FromServer.embarca == 1)
		{
			_tprintf(TEXT("\n\tPassageiro já vai embarcar no aviao %d com destino a %s [%d,%d]"), FromServer.AviaoId, FromServer.aDestino, FromServer.posx, FromServer.posy);
		}
		if (FromServer.emViagem == 1)
		{
			_tprintf(TEXT("\n\tPassageiro está no aviao %d com destino a %s [%d,%d]"), FromServer.AviaoId, FromServer.aDestino, FromServer.posx, FromServer.posy);
			//FromServer.emViagem = 0;
			Sleep(1000);
			if (FromServer.posDx == FromServer.posx && FromServer.posDy == FromServer.posy)
			{
				dados->termina = 1;
				_tprintf(TEXT("\nPassageiro chegou ao destino\n"));
				return 1;
			}

		}



		GetOverlappedResult(dados->hPipe, &OverlRd, &cbBytesRead, FALSE);
		if (cbBytesRead < Msg_Sz)
		{
			_tprintf(TEXT("\nReadFile falhou. Erro = %d"), GetLastError());
			return -1;
		}
		//	Sleep(1000);
			//_tprintf(TEXT("\nServidor disse: [%s]"), FromServer.passageiro);

	}
	ReaderAlive = 0;

	_tprintf(TEXT("Thread Reader a terminar. \n"));
	return 1;
}




int _tmain(int argc, TCHAR* argv[])
{

	Aeroportos* aero;
	ThreadPassageiroDados dados;
	BOOL fSuccess = FALSE;
	DWORD cbWritten, dwMode;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeexemplo");
	TCHAR* aeroONome;
	TCHAR* aeroDNome;
	TCHAR* nomePass;
	TCHAR comando[TAM];


	InfoPassageiro MsgToSend;
	dados.termina = 0;

	HANDLE hThread;
	DWORD dwThreadId = 0;
	int count;

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	if (argc < 3)
		return -1;

	aeroONome = argv[1];
	aeroDNome = argv[2];
	nomePass = argv[3];
	_tprintf_s(TEXT("\nAero origem : %s\nAero destino:%s\n Nome do passageiro:%s\n"), aeroONome, aeroDNome, nomePass);


	while (dados.termina == 0) {

		dados.hPipe = CreateFile(
			lpszPipename,
			GENERIC_READ | GENERIC_WRITE,
			0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0 | FILE_FLAG_OVERLAPPED,
			NULL);

		if (dados.hPipe != INVALID_HANDLE_VALUE)
			break;

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("\nCreate file deu erro e nao foi BUSY. Erro %d\n"), GetLastError());
			//pressEnter();
			return -1;
		}

		if (!WaitNamedPipe(lpszPipename, 30000)) {
			_tprintf(TEXT("Esperei por uma instancia durante 30 segundos. Desisto. Sair"));
			return -1;
		}

	}

	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(
		dados.hPipe,
		&dwMode,
		NULL,
		NULL);

	if (!fSuccess) {
		_tprintf(TEXT("SetNamedPipeHandleState falhou. Erro %d\n"), GetLastError());
		return -1;
	}

	hThread = CreateThread(
		NULL,
		0,
		ThreadPassageiroReader,
		&dados,
		0,
		&dwThreadId);

	if (hThread == NULL) {

		_tprintf(TEXT("\nErro na cria ao da thread. Erro = %d"), GetLastError());
		return -1;
	}

	HANDLE WriteReady;
	OVERLAPPED OverlWr = { 0 };

	WriteReady = CreateEvent(
		NULL, // default security
		TRUE,
		FALSE,
		NULL);


	if (WriteReady == NULL) {
		_tprintf_s(TEXT("\nCLiente: nao foi possivel criar o Evento. Mais vale parar ja"));
		return 1;
	}

	_tprintf_s(TEXT("\nliga ao estabelecida. exit para sair"));



	_tcscpy_s(MsgToSend.passageiro, QUEMSZ, nomePass);
	_tcscpy_s(MsgToSend.aOrigem, QUEMSZ, aeroONome);
	_tcscpy_s(MsgToSend.aDestino, QUEMSZ, aeroDNome);

	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	fSuccess = WriteFile(
		dados.hPipe,
		&MsgToSend,
		Msg_Sz,
		&cbWritten,
		&OverlWr);

	WaitForSingleObject(WriteReady, INFINITE);
	_tprintf(TEXT("\nWrite concluido"));

	GetOverlappedResult(dados.hPipe, &OverlWr, &cbWritten, FALSE);
	if (cbWritten < Msg_Sz)
		_tprintf(TEXT("\nWriteFile TALVEZ falhou. Erro = %d"), GetLastError());

	//_tprintf(TEXT("\nMessagem enviada"));

	//_tprintf_s(TEXT("\nA enviar %d bytes: %s\n"), Msg_Sz, MsgToSend.msg);
	while (dados.termina == 0)
	{


		_tprintf(TEXT("\n\t* Selecione a opção *\n\t* fim para terminar *\t\n"));

		do {
			_fgetts(comando, TAM, stdin);
			comando[_tcslen(comando) - 1] = '\0';
		} while (_tcscmp(comando, TEXT("")) == 0);
		if (_tcscmp(comando, _T("fim")) == 0)
		{
			dados.termina = 1;
		}
	}

	DeveContinuar = 0;
	if (ReaderAlive) {
		WaitForSingleObject(hThread, 3000);
		_tprintf(TEXT("\nThread reader encerrada ou timeout"));
	}
	_tprintf(TEXT("\Passageiro vai terminar ligação e sair"));
	CloseHandle(WriteReady);
	CloseHandle(dados.hPipe);
	return 0;



}
