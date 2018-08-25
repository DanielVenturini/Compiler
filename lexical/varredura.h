#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

/// dados que ser�o usados para processar os tokens
#define FALSE 0
#define TRUE 1

#define SIZE_NUM 10				/// quantidade m�xima - inicialmente - de digitos em um numero
#define SIZE_IDENT 21			/// quantidade m�xima - inicialmente - de caracteres em uma string

/********************************************************************
*                   ESTRUTURAS NECESS�RIAS                          *
********************************************************************/

typedef enum {                  /// palavras reservadas e s�mbolos
    SE, ENTAO, SENAO, FIM, REPITA, FLUTUANTE, RETORNA, ATE, LEIA, ESCREVA, INTEIRO, SOMA, SUBTRACAO, MULTIPLICACAO, DIVISAO, IGUALDADE, VIRGULA, NUM_I, NUM_F, ID, ATRIBUICAO, MENOR, MAIOR, MAIOR_IGUAL, MENOR_IGUAL, DIFERENTE, ABRE_COLCHETES, FECHA_COLCHETES, ABRE_PARENTESES, FECHA_PARENTESES, DOIS_PONTOS, E_LOGICO, OU_LOGICO, NEGACAO, NAO_IDENTIFICADO
} TokenType;

typedef struct {                /// tokens propriamente ditos

    TokenType tokenval;			/// tipo do token
    unsigned short int numline;	/// n�mero da linha que este token est�. Permite arquivos de 0 at� 65,535 linhas
    void *val;					/// posi��o deste na memoria. Foi escolhido um ponteiro void para guardar o endere�o de qualquer valor na mem�ria

} TokenRecord;

/********************************************************************
*                   	DEFINICAO DAS FUN��ES                       *
********************************************************************/
/// estas s�o as que s�o usadas fora do arquivo varredura.c
void *desaloca(void *ptr);		/// desaloca a 'ptr' mem�ria usando uma thread
char openFile(char *filename);	/// realiza as opera��es de abertura do arquivo
TokenRecord* getToken(void);	/// retorna um token
void printToken(TokenRecord *token, char printLines);	/// imprime o valor de um token
