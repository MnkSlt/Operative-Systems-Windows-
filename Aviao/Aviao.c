#include "FuncoesA.h"

DWORD WINAPI ThreadEnviaPedidoAviao(LPVOID param)
{
	RecebeEnviaPedidosAviao* dados = (RecebeEnviaPedidosAviao*)param;
	Avioes aviao = { NULL };
	int aux[MAPATAM][MAPATAM];

	int aviaox = 0;
	int aviaoy = 0;
	int aviaoAntx = 0;
	int aviaoAnty = 0;
	int flagUltimapos = 0;

	while (dados->terminar != 1)
	{
		if (dados->criaAviaoFlag == 1 && dados->escolhidoDestinoFlag == 1)	//aviao no aeroporto
		{
			aviao.id = dados->id;
			aviao.pos = dados->origem;
			aviao.posDestino = dados->destino;
			aviaox = aviao.pos.x;
			aviaoy = aviao.pos.y;
	
			WaitForSingleObject(dados->hSemVazios, INFINITE);
			WaitForSingleObject(dados->hMutex, INFINITE);

			CopyMemory(&dados->memPar->buffer[dados->memPar->posL], &aviao, sizeof(Avioes));
			dados->memPar->posL = (dados->memPar->posL + 1) % dados->memPar->tamMaxAvioes;
			dados->criaAviaoFlag = 0;
			dados->escolhidoDestinoFlag = 0;
			dados->memPar->flagPassageirosEmbarque = 1;

			ReleaseMutex(dados->hMutex);
			ReleaseSemaphore(dados->hSemItens, 1, NULL);
		}
		if (dados->viagemAviaoFlag == 1)	//aviao em viagem
		{
			do {
				WaitForSingleObject(dados->hSemVazios, INFINITE);
				WaitForSingleObject(dados->hMutex, INFINITE);

				_tprintf_s(TEXT("\n Aviao está a andar..."));
				_tprintf_s(TEXT("\n %d : %d"), aviaox, aviaoy);

				CopyMemory(&aux, &dados->memPar->m.map, sizeof(dados->memPar->m.map));
				for (int i = 0; i < MAPATAM; i++)
				{
					_tprintf_s(TEXT("\n"));

					for (int j = 0; j < MAPATAM; j++)
					{
						_tprintf_s(TEXT("%d"), aux[i][j]);

					}
				}
				

				for (int j = 0; j < dados->velocidade; j++) {
					if (move(aviao.pos.x, aviao.pos.y, aviao.posDestino.x, aviao.posDestino.y, &aviaox, &aviaoy) == 1 && dados->memPar->m.map[aviaox][aviaoy] <= 0 )
					{


							for (int i = 0; i < dados->memPar->tamMaxAvioes; i++)
							{
								if (aviao.id == dados->memPar->buffer[i].id)
								{
									

									CopyMemory(&dados->memPar->buffer[i], &aviao, sizeof(Avioes));
									dados->memPar->flagPassageirosEmViagem = 1;

							
									WaitForSingleObject(dados->hMutexMapa,INFINITE);
								 if (dados->memPar->m.map[aviao.pos.x][aviao.pos.y] != -1 && dados->memPar->m.map[aviaox][aviaoy] != -1)
									{
									 dados->memPar->m.map[aviao.pos.x][aviao.pos.y] = 0;
									 dados->memPar->m.map[aviaox][aviaoy] = aviao.id;
								 }
									ReleaseMutex(dados->hMutexMapa);
									aviao.pos.x = aviaox;
									aviao.pos.y = aviaoy;
									break;

								}
								
							}

					}						//se chegou a posicao final
					else if (move(aviao.pos.x, aviao.pos.y, aviao.posDestino.x, aviao.posDestino.y, &aviaox, &aviaoy) == 0)
					{
						_tprintf_s(TEXT("\n Aviao chegou ao destino."));
						for (int i = 0; i < dados->memPar->tamMaxAvioes; i++)
						{
							if (aviao.id == dados->memPar->buffer[i].id)
							{
								CopyMemory(&dados->memPar->buffer[i], &aviao, sizeof(Avioes));
								dados->memPar->flagPassageirosEmViagem = 1;

								WaitForSingleObject(dados->hMutexMapa, INFINITE);

								
								if (dados->memPar->m.map[aviao.pos.x][aviao.pos.y] != -1)
								{
									dados->memPar->m.map[aviao.pos.x][aviao.pos.y] = 0;
								}
								ReleaseMutex(dados->hMutexMapa);
								aviao.pos.x = aviaox;
								aviao.pos.y = aviaoy;
								break;
							}
						}

						//dados->map->map[aviao.pos.x][aviao.pos.y] = 0;

						flagUltimapos = 1;
					}
					else {
						aviao.pos.x += 2;
						aviao.pos.y += 2;
					}
				}
				Sleep(1000);
				ReleaseMutex(dados->hMutex);
				ReleaseSemaphore(dados->hSemItens, 1, NULL);
			} while (flagUltimapos != 1);
			//dados->memPar->flagPassageirosEmViagem = 0;
			flagUltimapos = 0;
			dados->viagemAviaoFlag = 0;
			dados->escolhidoDestinoFlag = 0;
		}
	}
	return 0;
}

int _tmain(int argc, TCHAR* argv[])
{
	RecebeEnviaPedidosAviao dadosAviao;
	RecebeInfoAero aeroporto;
	Aeroportos* aero;
	Avioes* aux;
	
	Info info;
	int TamanhoAero;

#ifdef UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
#endif 


	info.primeiroAcesso = FALSE;
	aero = malloc(sizeof(Aeroportos));

	//mutexes, semaforos e mem partilhada
	{
		info.Maximo_Avioes = accessNumMaxAvioes();
		//semaforo - escrita - buffer circular
		dadosAviao.hSemItens = CreateSemaphore(NULL, 0, info.Maximo_Avioes, TEXT("SO2_SEM_PRODUTOR"));
		//semaforo - leitura - buffer circular
		dadosAviao.hSemVazios = CreateSemaphore(NULL, info.Maximo_Avioes, info.Maximo_Avioes, TEXT("SO2_SEM_VAZIOS"));
		//mutex para accesso concorrente das instâncias avião
		dadosAviao.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_CONSUMIDOR"));
		dadosAviao.hMutexMapa=CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_MAPA"));

		//dadosAviao.map->hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_MAPA"));

		if (dadosAviao.hSemItens == NULL || dadosAviao.hSemVazios == NULL || dadosAviao.hMutex == NULL || dadosAviao.hMutexMapa == NULL )
		{
			_tprintf_s(TEXT("\n Erro no CreateSemaforo ou no CreateMutex ou no CreateEvent."));
			return 1;
		}

		info.hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));

		if (info.hFileMap == NULL)
		{
			_tprintf_s(TEXT("\n Erro no CreateFileMapping1."));
			return 1;
		}

		dadosAviao.memPar = (BufferCircular*)MapViewOfFile(info.hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (dadosAviao.memPar == NULL)
		{
			_tprintf_s(TEXT("\n Erro no MapViewOfFile, dadosAviao.mempar."));
			return 1;
		}

		info.hFileMap2 = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,
			FALSE,
			TEXT("SO2_MEM_PARTILHADA2"));

		if (info.hFileMap2 == NULL)
		{
			_tprintf_s(TEXT("\n Erro no OpenFileMapping2"));
			return 1;
		}

		info.hFileMap3 = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,
			FALSE,
			TEXT("SO2_MEM_PARTILHADA3"));

		if (info.hFileMap3 == NULL)
		{
			_tprintf_s(TEXT("\n Erro no OpenFileMapping3"));
			return 1;
		}
		aeroporto.tamanhoAero = MapViewOfFile(info.hFileMap3, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (aeroporto.tamanhoAero == NULL)
		{
			_tprintf_s(TEXT("\nErro no MapViewOfFile, aeroporto.tamanhoAereo."));
			return 1;
		}
		aeroporto.fileViewMap = (Aeroportos*)MapViewOfFile(info.hFileMap2, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (aeroporto.fileViewMap == NULL)
		{
			_tprintf_s(TEXT("\n Erro no MapViewOfFile, aeroporto.fileViewMap."));
			return 1;
		}


		dadosAviao.terminar = 0;
		WaitForSingleObject(dadosAviao.hMutex, INFINITE);
		dadosAviao.memPar->nAvioes++;
		dadosAviao.id = dadosAviao.memPar->nAvioes;
		ReleaseMutex(dadosAviao.hMutex);


		info.hThreads[0] = CreateThread(NULL, 0, ThreadEnviaPedidoAviao, &dadosAviao, 0, NULL);

		if (info.hThreads[0] == NULL || info.hThreads[1] == NULL)
		{
			_tprintf_s(TEXT("\n Erro no CreateThread."));
			return 1;
		}
		aeroporto.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_AERO"));
		if (aeroporto.hMutex == NULL)
		{
			_tprintf_s(TEXT("\n Erro  no CreateMutex."));
			return 1;
		}
		dadosAviao.velocidade = 1;
		dadosAviao.escolhidoDestinoFlag = 0;
	}

	if (dadosAviao.memPar->flagAceitaAvioes == 1)
	{
		do {

			WaitForSingleObject(aeroporto.hMutex, INFINITE);
			CopyMemory(&TamanhoAero, aeroporto.tamanhoAero, sizeof(TamanhoAero));
			aero = realloc(aero, sizeof(Aeroportos) * TamanhoAero);				
			CopyMemory(aero, aeroporto.fileViewMap, sizeof(Aeroportos) * TamanhoAero);
			ReleaseMutex(aeroporto.hMutex);

			
			_tprintf_s(TEXT("\n\n\n\n\t* inicio -> Indique o aeroporto em que vai começar *\n\t* destino -> Indique o seu aeroporto destino *\n\t   * viagem -> Iniciar viagem *\n\t       * velocidade -> Mudar velocidade *\n\t\t      * fim -> Desligar avião *\n\n\nInsira um comando: "));
			do {
				_fgetts(info.comando, TAM, stdin);
				info.comando[_tcslen(info.comando) - 1] = '\0';
			} while (_tcscmp(info.comando, TEXT("")) == 0);
			if (_tcscmp(info.comando, _T("inicio")) == 0)
			{
				_tprintf_s(TEXT("\n Aeroportos disponiveis:\n\n"));
				listaAero(aero, TamanhoAero);
				do {
					_fgetts(info.destino, TAM, stdin);
					info.destino[_tcslen(info.destino) - 1] = '\0';
					for (int i = 0; i < TamanhoAero; i++)
					{
						if (_tcscmp(info.destino, aero[i].nome) == 0)
						{
							_tprintf_s(TEXT("\n Vai começar no aeroporto : %s...\n"), aero[i].nome);
							CopyMemory(&dadosAviao.origem, &aero[i].pos, sizeof(Posicao));
							CopyMemory(&dadosAviao.aux.nome, &aero[i].nome, sizeof(aero[i].nome));
							dadosAviao.criaAviaoFlag = 1;
							break;
						}
					}
				} while (_tcscmp(info.destino, TEXT("")) == 0);	//andre - enquanto destino for igual a nada??
			}
			else if (_tcscmp(info.comando, _T("destino")) == 0)
			{
				_tprintf_s(TEXT("\n Escolha o seu próximo destino\n Aeroportos disponiveis:\n\n"));
				for (int i = 0; i < TamanhoAero; i++)
				{
					if (_tcscmp(aero[i].nome, dadosAviao.aux.nome) == 0)
					{
						_tprintf_s(TEXT(""));
					}
					else {

						_tprintf(TEXT("\n Nome: %s\n Posição: (%d:%d)\n"), aero[i].nome, aero[i].pos.x, aero[i].pos.y);
					}
				}
				do {
					_fgetts(info.destino, TAM, stdin);
					info.destino[_tcslen(info.destino) - 1] = '\0';
					for (int i = 0; i < TamanhoAero; i++)
					{
						if (_tcscmp(info.destino, aero[i].nome) == 0 && _tcscmp(info.destino, dadosAviao.aux.nome) != 0)
						{
							CopyMemory(&dadosAviao.destino, &aero[i].pos, sizeof(Posicao));
							//dadosAviao.memPar->buffer
							dadosAviao.escolhidoDestinoFlag = 1;
							break;
						}
					}
					if (dadosAviao.escolhidoDestinoFlag == 1)
					{
						_tprintf_s(TEXT("\n O seu destino foi escolhido..."));
						break;
					}
					else {
						_tprintf_s(TEXT("\n Destino indisponível ... "));
					}
				} while (dadosAviao.escolhidoDestinoFlag != 1);//while (_tcscmp(destino, TEXT("")) == 0);
			}
			else if (_tcscmp(info.comando, _T("viagem")) == 0)		//suspende a aceitação de novos avioes
			{
				_tprintf_s(TEXT("\n Viagem a iniciar..."));


				_tprintf_s(TEXT("\n Viagem a iniciar...\n"));
				dadosAviao.viagemAviaoFlag = 1;
				do {
					_tprintf_s(TEXT("\n Em viagem..."));
					_tprintf_s(TEXT("\n Insira um fim para terminar... "));
					_fgetts(info.comandoViagem, TAM, stdin);
					info.comandoViagem[_tcslen(info.comandoViagem) - 1] = '\0';
					if (dadosAviao.viagemAviaoFlag == 0)
					{
						_tprintf_s(TEXT("\n Chegou ao destino..."));
						break;

					}
					/*else if (_tcscmp(info.comandoViagem, TEXT("fim")) == 0)
					{
						WaitForSingleObject(dadosAviao.hMutex, INFINITE);
						dadosAviao.memPar->nAvioes--;
						dadosAviao.memPar->flagAceitaAvioes = 1;
						ReleaseMutex(dadosAviao.hMutex);

						UnmapViewOfFile(dadosAviao.memPar);
						UnmapViewOfFile(aeroporto.tamanhoAero);
						UnmapViewOfFile(aeroporto.fileViewMap);

						CloseHandle(info.hFileMap);
						CloseHandle(info.hFileMap2);
						CloseHandle(info.hFileMap3);
						CloseHandle(info.hThreads);
						CloseHandle(dadosAviao.hSemItens);
						CloseHandle(dadosAviao.hSemVazios);
						CloseHandle(dadosAviao.hMutex);
						CloseHandle(aeroporto.hMutex);

						free(aero);
						return 0;
					}*/


				} while (_tcscmp(info.comandoViagem, TEXT("fim")) != 0);
			}
			else if (_tcscmp(info.comando, _T("velocidade")) == 0)	//mudar velocidade
			{
				do {
					_tprintf_s(TEXT("\n Mudar a velocidade\n Indique uma velocidade de 1 a 5: "));
					_tscanf_s(TEXT("%d"), &dadosAviao.velocidade);
				} while (dadosAviao.velocidade < 1 || dadosAviao.velocidade >5);
			}

		} while (_tcscmp(info.comando, _T("fim")) != 0);
	}
	else
		_tprintf(TEXT("\n Controlador não está a aceitar pedidos.\n"));

	//WaitForSingleObject(dadosAviao.hMutex, INFINITE);
	dadosAviao.memPar->nAvioes--;
	//ReleaseMutex(dadosAviao.hMutex);

	UnmapViewOfFile(dadosAviao.memPar);
	UnmapViewOfFile(aeroporto.tamanhoAero);
	UnmapViewOfFile(aeroporto.fileViewMap);

	CloseHandle(info.hFileMap);
	CloseHandle(info.hFileMap2);
	CloseHandle(info.hFileMap3);
	CloseHandle(info.hThreads);

	CloseHandle(dadosAviao.hSemItens);
	CloseHandle(dadosAviao.hSemVazios);
	CloseHandle(dadosAviao.hMutex);
	CloseHandle(aeroporto.hMutex);

	free(aero);
	Sleep(5000);

	return 0;
}


