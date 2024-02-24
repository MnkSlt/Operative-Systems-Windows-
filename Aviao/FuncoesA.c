#include "FuncoesA.h"

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
		_tprintf_s(TEXT(" [Registry] Erro a criar ou abrir chave ( %d )."), GetLastError());
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
void listaAero(Aeroportos* aero, int TamanhoAero)
{
	for (int i = 0; i < TamanhoAero; i++)
	{
		_tprintf_s(TEXT("\n Nome aero: %s\n Posição: (%d;%d)\n\n"), aero[i].nome, aero[i].pos.x, aero[i].pos.y);
	}
}