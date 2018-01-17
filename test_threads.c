#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h> 


//DEFINES
#define MODO 0 //o modo indica qual tipo de varredura sera feito 0 serial e 1 para o uso de thread(mesmo que seja 1 thread existe diferenca entre o modo serial)
#define FALSE 0 //usado apenas para praticidade
#define TRUE 1 //usado apenas para praticidade
#define NUM_THREADS 2
#define LINHA_M 15000 //linhas para a matrix
#define COL_M 15000 //colunas para matrix
#define LINHA_MB 150 //linhas para macrobloco
#define COL_MB 150 //colunas pra macrobloco
#define VMAX 29999 //valor maximo do intervalo de inteiros que podem compor a matriz
#define VMIN 0 //valor minimo do intervalo de inteiros que podem compor a matriz
//----FIM DEFINES

//TYPEDEF
struct mtx {
	int linhas;
	int colunas;
	int **mat;
};
typedef struct mtx matrix, *pmatrix; // pmatrix == ponteiro para matrix

struct area { //struct usada para guardar dados do macrobloco, tanto para trabalhar quanto para saber quais ja foram varridos
	int linhas;
	int colunas;
};
typedef struct area tArea, *pArea;

//----FIM TYPEDEF

//VARIÁVEIS GLOBAIS
tArea varreduraStatus;
pthread_mutex_t mutex_1; //mutex para controle das áreas varridas pelas threads
pthread_mutex_t mutex_2; //mutex para o contador de primos
int countPrimo = 0;  //contador de nºs primos
int maxBlocos = (LINHA_M * COL_M) / (LINHA_MB*COL_MB); //quantidade de blocos que serão processados com base nas linhas passadas
int contBlocos = 0; //usado para controlar a quantidade verificações por blocos
pmatrix gMatrix; //matriz global
//----FIM GLOBAL

//ASSINATURA FUNÇÕES
void startMtx(); //usando pmatrix para otimizar a escrita, apenas.
void fillMtx(); //adiciona os números da seed na matriz
void freeMtx();  //libera a memória alocada para a matriz
int isPrimo(int num); //verifica se o numero eh primo
void* usoThread(void* numThread);
void buscaSerial();
//----FIM ASSINATURAS

//PROGRAMA PRINCIPAL
int main() {
	clock_t tInicio=0, tFinal=0; //usados para a marcação de tempo
	double tTotal=0, tExecucao=0;
	if (MODO==0){	
		//prints iniciais
		printf("MODO SERIAL");
		printf("\n\nDimensao da matriz: %d x %d",LINHA_M,COL_M);
		printf("\n\nIntervalo de numeros da matriz [%d,%d]", VMIN, VMAX);
		//alocacao de memoria para a matriz
		tInicio = clock();
		startMtx();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		printf("\n\n\nTEMPO ALOCACAO DE MEMORIA: %.20lf segundos.", tTotal);
		tExecucao += tTotal;

		//enchendo a matriz com inteiros
		tInicio = clock();
		fillMtx();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		printf("\n\nTEMPO PREENCHIMENTO MATRIZ: %.20lf segundos.\n", tTotal);
		tExecucao += tTotal;
		

		//verificacao serial
		tInicio = clock();
		buscaSerial();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		tExecucao += tTotal;
		printf("\nTEMPO VARREDURA SERIAL: %.20lf segundos.",tTotal);
		printf("\n\nTOTAL PRIMOS: %d  (SERIAL).\n",countPrimo);

		//liberacao da matriz
		tInicio = clock();
		freeMtx();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		tExecucao += tTotal;
		printf("\n\nTEMPO LIBERACAO: %.20lf segundos.", tTotal);

		printf("\n\nTEMPO EXECUCAO: %.20lf segundos",tExecucao);
		system("pause");
		return 0;
	}
	else{
		pthread_t threads[NUM_THREADS];

		//inicializacao dos blocos varridos
		varreduraStatus.linhas = 0;
		varreduraStatus.colunas = 0;

		//inicializacao dos mutex
		pthread_mutex_init(&mutex_1, NULL);
		pthread_mutex_init(&mutex_2, NULL);
		
		//prints iniciais
		printf("Numero de threads de threads declaradas: %d.", NUM_THREADS);
		printf("\n\nDimensao matriz: %d x %d.", LINHA_M, COL_M);
		printf("\n\nIntervalo de numeros da matriz [%d,%d]", VMIN, VMAX);
		printf("\n\nVarredura em macroblocos de %d x %d.", LINHA_MB, COL_MB);


		//alocação de memória para a matriz
		tInicio = clock();
		startMtx();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		printf("\n\n\nTEMPO ALOCACAO DE MEMORIA: %.20lf segundos.", tTotal);
		tExecucao += tTotal;
		

		//preenchimento a matriz com inteiros
		tInicio = clock();
		fillMtx();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		printf("\n\nTEMPO PREENCHIMENTO MATRIZ: %.20lf segundos.\n", tTotal);
		tExecucao += tTotal;
		

		//verifica primos na matriz
		tInicio = clock();
		for (short i = 0; i < NUM_THREADS; i++) {
			short* threadNum = (short*)malloc(sizeof(short));
			*threadNum = i;
			if (pthread_create(&threads[i], NULL, usoThread, threadNum) != 0) {
				free(threadNum);
				printf("thread numero %hi falhou ao ser criada.\n", i);
				exit(1);
			}
		}
		for (short i = 0; i < NUM_THREADS; i++) {
			if (pthread_join(threads[i], NULL) != 0) {
				printf("erro ao retornar a thread %hi.\n", i);
				exit(1);
			}
		}
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		tExecucao += tTotal;
		printf("\n\nTEMPO VARREDURA PARALELA: %.20lf segundos.", tTotal);
		printf("\n\nTOTAL PRIMOS: %d", countPrimo);

		//liberacao da matriz
		tInicio = clock();
		freeMtx();
		tFinal = clock();
		tTotal = (double)(tFinal - tInicio) / CLOCKS_PER_SEC;
		tExecucao += tTotal;
		printf("\n\nTEMPO LIBERACAO: %.20lf segundos.", tTotal);



		printf("\n\nTEMPO EXECUCAO: %.20lf segundos",tExecucao);
		system("pause");
		return 0;
	}
}
//----FIM PRINCIPAL

//FUNCOES

void startMtx() {
	gMatrix = (pmatrix)malloc(sizeof(matrix)); //aloca a memória para a estrutura definida
	int **mtx; //vetor de vetor (matriz) dinâmico
	gMatrix->linhas = LINHA_M;
	gMatrix->colunas = COL_M;
	mtx = (int**)malloc(LINHA_M * sizeof(int*)); //aloca espaço para um vetor de vetores nao declarados
	for (int i = 0; i < LINHA_M; i++) {
		mtx[i] = (int*)malloc(COL_M * sizeof(int)); //aloca espaço para os vetores da matriz
	}
	gMatrix->mat = mtx;
}


void fillMtx() {
	srand(7); //usando a semente 7 pre estabelecida pelo professor na aula
	for (int i = 0; i < LINHA_M; i++) {
		for (int j = 0; j < COL_M; j++) {
			gMatrix->mat[i][j] = (rand() % (VMAX+1) ); //usado para apenas aceitar os numeros no intervalo informado (expressao tirada de http://c-faq.com/lib/randrange.html)
		}
	}
}

void freeMtx() {
	for (int i = 0; i < LINHA_M; i++) { //libera a memoria alocada para cada linha da matriz
		free(gMatrix->mat[i]);
	}
	free(gMatrix->mat); //libera a memoria da matriz

	free(gMatrix); //libera a memoria da estrutura na qual a matriz estava

}


int isPrimo(int num) {
	if(num <2){
		return FALSE;
	}
	else if (num == 2 || num == 3 || num == 5) {
		return TRUE;
	}
	else if (num % 2 == 0 || num % 3 == 0) {
		return FALSE;
	}
	else {
		int numVerificador = 5;
		while (numVerificador * numVerificador <= num) { // vai ate a raiz do numero sem fazer o uso da função sqrt, que tem custo maior
			if ((num % numVerificador) == 0) {
				return FALSE;
			}
			numVerificador++ ;
		}
		return TRUE;
	}
}

void* usoThread(void* numThread) {
	register int countprimoLocal =0;
	short incrementoLinha; //variável de incremento do numero de linhas para a varredura da matriz
	tArea workSpace;
	short idThread = *((short*)numThread);
	free(numThread);
	printf("Thread #%hi iniciada.\n", idThread);

	while (contBlocos < maxBlocos) { //loop usado para manter as threads na função ate que todos os blocos sejam trabalhados
		//regiao critica dos blocos de movimentação
		pthread_mutex_lock(&mutex_1);
		workSpace.linhas = varreduraStatus.linhas;
		workSpace.colunas = varreduraStatus.colunas;
		//após uma thread receber a area de serviço é necessário continuar na regiao critica para atualizar os valores.
		varreduraStatus.colunas += LINHA_MB * COL_MB;
		incrementoLinha = varreduraStatus.colunas / COL_M;
		if ( incrementoLinha > 0 ){
			varreduraStatus.linhas += incrementoLinha;
			varreduraStatus.colunas = varreduraStatus.colunas % COL_M;
		}
		contBlocos++; //incremento da quantidade de blocos varridos
		pthread_mutex_unlock(&mutex_1);
		//---fim RC

		for (int i = 0; i < (COL_MB*LINHA_MB); i++) {
			if (workSpace.linhas >= LINHA_M) {
				break;
			}
			if ( isPrimo(gMatrix->mat[workSpace.linhas][workSpace.colunas]) == TRUE) {
				countprimoLocal++;
			}
			workSpace.colunas++;
			if (workSpace.colunas >= COL_M ) { // caso o numero da coluna do workSpace tenha chegado ao tamanho maximo pula-se para a proxima linha
				workSpace.linhas++;
				workSpace.colunas = 0;
			}
		}
		if (countprimoLocal>=20){// metodo usado para tratar quando o numero do macro bloco é muito pequeno
		//rc de contagem de numeros primos
			pthread_mutex_lock(&mutex_2);
			countPrimo += countprimoLocal ; //incremento no contador de números primos
			pthread_mutex_unlock(&mutex_2);
			countprimoLocal = 0;
				//fim rc de contagem
		}
	}
	if (countprimoLocal>0){//apos verificar toda a matriz, verifica-se se o contador local ainda tem informações
		pthread_mutex_lock(&mutex_2);
		countPrimo+=countprimoLocal;
		pthread_mutex_unlock(&mutex_2);
	}
	return NULL;

}

void buscaSerial(){
	for (short i=0; i<LINHA_M;i++){
		for (short j=0; j<COL_M;j++){
			if ( isPrimo(gMatrix->mat[i][j]) == TRUE ){
				countPrimo++;
			}
		}
	}
}
//----FIM FUNCOES