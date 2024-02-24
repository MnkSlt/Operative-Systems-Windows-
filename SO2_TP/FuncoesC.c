#include "funcoesC.h"

//registry
void createRegistryKeys()
{
	HKEY chave;
	TCHAR SUBCHAVE_AV[TAM] = { _T("Maximo_Avioes") };
	TCHAR SUBCHAVE_AE[TAM] = { _T("Maximo_Aeroportos") };
	DWORD res;
	TCHAR numMaxAvioes[TAM] = _T("");
	TCHAR numMaxAeroportos[TAM] = _T("");

	if (RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &res) != ERROR_SUCCESS)
		_tprintf_s(TEXT("\n Erro a criar ou abrir chave (%d)."), GetLastError());

	if (res == REG_CREATED_NEW_KEY) {	//caso nao exista cria, com input do utilizador
		_tprintf_s(TEXT("\n [Registry] Chave \"%s\" foi criada com sucesso."), CHAVE);
		_tprintf_s(_T("\n\n Insira o número máximo de avioes: "));
		do {
			_fgetts(numMaxAvioes, TAM, stdin);
			numMaxAvioes[_tcslen(numMaxAvioes) - 1] = '\0';
		} while (_tcscmp(numMaxAvioes, TEXT("")) == 0);
		_tprintf(_T("\n\n Insira o número máximo de aeroportos: "));
		do {
			_fgetts(numMaxAeroportos, TAM, stdin);
			numMaxAeroportos[_tcslen(numMaxAeroportos) - 1] = '\0';
		} while (_tcscmp(numMaxAeroportos, TEXT("")) == 0);

		if (RegSetValueEx(chave, SUBCHAVE_AV, 0, REG_SZ, (LPBYTE)numMaxAvioes, _tcslen(numMaxAvioes) * sizeof(TCHAR)) == ERROR_SUCCESS)
			_tprintf_s(_T("\n [Registry] Par chave-valor '%s' foi criado com sucesso."), SUBCHAVE_AV);
		if (RegSetValueEx(chave, SUBCHAVE_AE, 0, REG_SZ, (LPBYTE)numMaxAeroportos, _tcslen(numMaxAeroportos) * sizeof(TCHAR)) == ERROR_SUCCESS)
			_tprintf_s(_T("\n [Registry] Par chave-valor '%s' foi criado com sucesso."), SUBCHAVE_AE);
	}
	else
		_tprintf_s(TEXT("\n [Registry] Chave \"%s\" foi aberta com sucesso."), CHAVE);


	RegCloseKey(chave);
}
int accessNumMaxAvioes()
{
	HKEY chave;
	DWORD res;
	TCHAR par_nome[TAM], par_valor[TAM], SUBCHAVE_AV[TAM] = { TEXT("Maximo_Avioes") };
	int i = 0;
	int numMaxAvioes = 0;
	DWORD tam1 = TAM, tipo;
	DWORD tam2 = TAM * sizeof(TCHAR);
	if (RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &res) != ERROR_SUCCESS)
	{
		_tprintf_s(TEXT("\n Erro a criar ou abrir chave(%d)"), GetLastError());
	}
	while (RegEnumValue(chave, i, par_nome, &tam1, NULL, &tipo, (LPBYTE)par_valor, &tam2) == ERROR_SUCCESS)
	{
		if ((_tcscmp(par_nome, SUBCHAVE_AV) == 0))
			numMaxAvioes = _tcstod(par_valor, _T('\0'));
		i++;
	}
	RegCloseKey(chave);
	return numMaxAvioes;
}
int accessNumMaxAeroportos()
{
	HKEY chave;
	DWORD res;
	PVOID pvData = NULL;
	LPDWORD lpcbData = sizeof(pvData);
	TCHAR par_nome[TAM], par_valor[TAM], SUBCHAVE_AE[TAM] = { _T("Maximo_Aeroportos") };
	int i = 0;
	int numMaxAeroportos = 0;
	DWORD tam1 = TAM, tipo = 0;
	DWORD tam2 = TAM * sizeof(TCHAR);
	if (RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &res) != ERROR_SUCCESS)
	{
		_tprintf_s(TEXT("\n Erro a criar ou abrir chave(%d)"), GetLastError());
	}

	do {
		if ((_tcscmp(par_nome, SUBCHAVE_AE) == 0))
			numMaxAeroportos = _tcstod(par_valor, _T('\0'));
		i++;
	} while (RegEnumValue(chave, i, par_nome, &tam1, NULL, &tipo, (LPBYTE)par_valor, &tam2) == ERROR_SUCCESS);
	//_tprintf_s(TEXT("\numMaxAeroportos: %d"), numMaxAeroportos);
	RegCloseKey(chave);
	return numMaxAeroportos;
}

//aeroportos
int foraDoLimite(int aero_x, int aero_y, int TamanhoAero) {

	if (aero_x < 1 || aero_x > 1000 || aero_y < 1 || aero_y > 1000) {
		_tprintf(_T("\n Coordenadas fora do limite (1 - 1000).\n"));
		return 1;
	}
	return 0;
}
int existeCoordenadasAeroportos(Aeroportos* aero, int aero_x, int aero_y, int TamanhoAero)
{

	for (int i = 0; i < TamanhoAero; i++)
	{
		if (aero_x >= aero[i].pos.x && aero_x < aero[i].pos.x + 10
			|| aero_x <= aero[i].pos.x && aero_x > aero[i].pos.x - 10
			|| aero_y >= aero[i].pos.y && aero_y < aero[i].pos.y + 10
			|| aero_y <= aero[i].pos.y && aero_y > aero[i].pos.y - 10)
		{
			_tprintf_s(_T("\n Existe um Aeroporto nas proximidades."));
			return 1;
		}

	}
	return 0;
}
int coordenadasAeroportos(Aeroportos* aero, int* aero_x, int* aero_y, int TamanhoAero)
{
	_tscanf_s(_T("%d %d"), aero_x, aero_y);
	//if (isdigit(aero_x) && isdigit(aero_y))
	if (foraDoLimite(*aero_x, *aero_y, TamanhoAero) || existeCoordenadasAeroportos(aero, *aero_x, *aero_y, TamanhoAero))
		return 0;
	else
		return 1;

}
int existeNomeAeroportos(Aeroportos* aero, TCHAR nomeAero[TAM], int TamanhoAero)
{

	_fgetts(nomeAero, TAM, stdin);
	nomeAero[_tcslen(nomeAero) - 1] = '\0';
	for (int j = 0; j < TamanhoAero; j++)
	{
		if (_tcscmp(nomeAero, aero[j].nome) == 0)
		{
			_tprintf_s(TEXT("\n Já existe um Aeroporto com esse nome."));
			return 1;
		}
	}
	return 0;
}
void listaTudo(Aeroportos* aero, BufferCircular* dadosAviao, Info info, int TamanhoAero,int m[MAPATAM][MAPATAM])
{
	int i;
	_tprintf_s(TEXT("\n ***** Lista de todos os elementos *****\n\n"));
	_tprintf_s(TEXT("\n * Aeroportos *\n"));
	for (i = 0; i < TamanhoAero; i++)
	{
		_tprintf_s(_T("\n Aeroporto %d\n Nome: %s\n Coordenadas: %d-%d\n"), i, aero[i].nome, aero[i].pos.x, aero[i].pos.y);
	}
	_tprintf_s(TEXT("\n * Aviões *\n"));
	for (i = 0; i < info.Maximo_Avioes; i++)
	{
		if (dadosAviao->buffer[i].id != 0)
			_tprintf_s(TEXT("\n Aviao %d \n Posicao: (%d,%d)\n"), dadosAviao->buffer[i].id, dadosAviao->buffer[i].pos.x, dadosAviao->buffer[i].pos.y);
	}
	_tprintf_s(TEXT("\n * Mapa *\n"));
	
	for (int i = 0; i < MAPATAM; i++)
	{
		_tprintf_s(TEXT("\n"));
		for (int j = 0; j < MAPATAM; j++)
		{
			_tprintf_s(TEXT("%d"), m[i][j]);
		}

	}



}


void iniciaPassageiros()
{
	int i;
	for (i = 0; i < MAXPASSAGEIROS; i++)
		passageiros[i] = NULL;

	
}
void adicionaPassageiro(HANDLE cli)
{
	int i;
	for (i = 0; i < MAXPASSAGEIROS; i++)
	{
		if (passageiros[i] == NULL)
		{
			passageiros[i] = cli;
			return;
		}
	}
}
void removePassageiro(HANDLE cli)
{
	int i;
	for (i = 0; i < MAXPASSAGEIROS; i++) {
		if (passageiros == cli)
		{
			passageiros[i] = NULL;
			return;
		}
	}
}
int writePassageiroASINC(HANDLE hPipe, InfoPassageiro msg) {

	DWORD cbWritten = 0;
	BOOL fSuccess = FALSE;
	OVERLAPPED OverlWr = { 0 };
	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(writeReady);	//   nao  assinalado
	OverlWr.hEvent = writeReady;

	fSuccess = WriteFile(
		hPipe,
		&msg,
		Msg_Sz,
		&cbWritten,
		&OverlWr);
	if (fSuccess == NULL)
	{
		_tprintf(TEXT("Passageiro %s atirou-se do avião"), msg.passageiro);
	}

	WaitForSingleObject(writeReady, INFINITE);
	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE); // sem WAIT
	if (cbWritten < Msg_Sz)
	{
		_tprintf(TEXT("\nWriteFile nao escreveu toda a informaçao. Erro	%d"), GetLastError());
	}
	return 1;

}
int broadcastPassageiros(InfoPassageiro msg)
{
	int i, numwrites = 0;
	for (int i = 0; i < MAXPASSAGEIROS; i++)
	{
		if (passageiros[i] != 0)
		{
			numwrites += writePassageiroASINC(passageiros[i], msg);
		}
	}
	return numwrites;
}







