#include "varredura.h"

char *caracterAtual = NULL;     /// salvar o caracter que foi lido mas n�o processado - verifica��o a frente
FILE *leitorArquivo;            /// ponteiro para leitura do arquivo
pthread_t *desalocador = NULL;  /// ponteiro da thread respons�vel por desalocar a mem�ria

/********************************************************************
*                   IMPLEMENTA��O DAS FUN��ES                       *
********************************************************************/
/// apenas desaloca a memoria alocada neste ponteiro
/// como � usado por uma thread, tem que ser void *
void *desaloca(void *ptr){
    free(ptr);
}

/// verifica se um identificador � igual � uma palavra reservada.
/// se o for chegar at� o final, quer dizer que cada caracter s�o iguais.
/// vale tamb�m para identificadores acentuadpos: SEN�O, ENT�O e AT�
char iguais(char* identificador, char palavraReservada[]){
    char i;
    for(i = 0; (identificador[i] == palavraReservada[i]) && identificador[i] != '\0' && palavraReservada[i] != '\0'; i ++);

    return (identificador[i]==palavraReservada[i] && palavraReservada[i]=='\0')?'1':'0';
}

void palavrasReservadas(TokenRecord *token){

    /// descobre qual � o caracter inicial do identificador, pois elimina mais da metade das palavras reservadas
    switch(((char *) token->val)[0]){
        case 'a':   /// pode ser o 'at�'
            if(iguais(token->val, "at�\0") == '0')              /// se forem diferentes
                return;                                         /// n�o faz nada

            token->tokenval = ATE;
            break;

        case 'e':   /// pode ser o 'ent�o' ou 'escreva'
            if(iguais(token->val, "ent�o\0") == '1')            /// � o ent�o
                token->tokenval = ENTAO;                        /// atualiza o token
            else if (iguais(token->val, "escreva\0") == '1')
                token->tokenval = ESCREVA;
            else
                return;                                         /// retorna pra n�o desalocar o token->val

            break;

        case 'i':   /// pode ser o 'inteiro'
            if(iguais(token->val, "inteiro\0") == '0')          /// se forem diferentes
                return;                                         /// n�o faz nada

            token->tokenval = INTEIRO;
            break;

        case 'f':   /// pode ser 'fim' ou 'flutuante'
            if(iguais(token->val, "fim\0") == '1')              /// � o fim
                token->tokenval = FIM;                          /// atualiza o token
            else if (iguais(token->val, "flutuante\0") == '1')
                token->tokenval = FLUTUANTE;
            else
                return;                                         /// retorna pra n�o desalocar o token->val

            break;

        case 'l':   /// pode ser o 'leia'
            if(iguais(token->val, "leia\0") == '0')             /// se forem diferentes
                return;                                         /// n�o faz nada

            token->tokenval = LEIA;
            break;

        case 'r':   /// pode ser o 'repita' ou 'retorna'
            if(iguais(token->val, "repita\0") == '1')           /// � o repita
                token->tokenval = REPITA;                       /// atualiza o token
            else if (iguais(token->val, "retorna\0") == '1')
                token->tokenval = RETORNA;
            else
                return;                                         /// retorna pra n�o desalocar o token->val

            break;

        case 's':   /// pode ser o 'se' ou 'sen�o'
            if(iguais(token->val, "se\0") == '1')               /// � o se
                token->tokenval = SE;                           /// atualiza o token
            else if (iguais(token->val, "sen�o\0") == '1')
                token->tokenval = SENAO;
            else
                return;                                         /// retorna pra n�o desalocar o token->val

            break;

        default:    /// outra palavra que n�o come�a com estas letras
            return; /// ent�o retorna para n�o desalocar
    }

    if(!desalocador){                                               /// se ainda n�o tiver sido usado thread
        desalocador = (pthread_t*) malloc(sizeof(pthread_t));
    }

    pthread_create(desalocador, NULL, desaloca, token->val);        /// desaloca pois n�o precisa ter esta informa��o
}

/// fun��o usada para realocar mem�ria se a letra ou o n�mero tiver muitos caracteres.
/// para evitar realocar sempre, somente sera realocado
/// c � o ponteiro para o vetor a alocar; i � o tamanho total; size_max � o tamanho total deste dado: SIZE_NUM|SIZE_ID
char *realoca(char *c, char *i, char *size_max) {
    *size_max = (char) *i + *size_max-1;            /// novo tamanho sera o tamanho atual mais o tamanho m�ximo inicial menos 1
    c = (char *) realloc(c, *size_max*sizeof(char));/// realoca
    return c;
}

/// chamado diretamente da main, esta fun��o retorna 0 se existir o arquivo ou 1 se n�o existir
/// o ponteiro do arquivo � guardado para ser lido caracter por caracter na fun��o getCaracter
char openFile(char *filename) {
    leitorArquivo = fopen(filename, "r");

    if(leitorArquivo == NULL){
        fprintf(stderr, "ERR: arquivo '%s' nao existe.\n", filename);
        return '1';
    }

    return '0';
}

/// retorna um caracter que ainda n�o tenha sido processado
/// ou retorna um caracter direto do arquivo
char* getCaracter(){

    char *c = malloc(sizeof(char));

    if(caracterAtual != NULL){    /// se ja tiver um caracter que nao foi processado pela getToken(void)
        c = caracterAtual;
        caracterAtual = NULL;

        return c;
    }

    *c = getc(leitorArquivo);

    return c;
}

/// l� toda a sequ�ncia de digitos e guarda em 'resp'.
/// como pode ser chamada de outras fun��es, por exemplo, 'getFlutuante',
/// ir� come�ar a concatenar a partir de 'i'.
/// 'size_max' � o tamanho m�ximo que pode ser lido sem precisar de usar um 'realloc'
/// 'c' � o d�gito que ainda n�o foi processado
char getDecimal(char *resp, char i, char size_max, char *c) {

    while(TRUE){                            /// fica lendo at� parar de ler d�gitos

        if(*c < '0' || *c > '9'){           /// n�o � um digito
            caracterAtual = c;              /// guarda o caracter que n�o � n�mero
            return i;                       /// retorna a pr�xima posi��o vazia do array
        }

        if(i % size_max == size_max-1){     /// se precisar realocar
            resp = realoca(resp, &i, &size_max);
        }

        resp[i] = *c;                       /// adiciona o digito na resposta
        c = getCaracter();                  /// le o proximo

        i ++;
    }
}

/// l� toda a sequ�ncia de n�meros a e guarda em 'resp'.
/// 'c' deve ser o '.' que trousse a esta fun��o.
char getFlutuante(char *resp, char i, char size_max) {

    resp[i] = *getCaracter();                   /// consome o caractere
    char *c = getCaracter();                    /// pega o pr�ximo

    return getDecimal(resp, i+1, size_max, c);   /// l� todo o resto de n�mero
}

/// alguns tokens podem mudar dependendo do pr�ximo caracter: >, >=
/// esta fun��o verifica se o pr�ximo caracter � igual ao 'proximoCaracter'.
/// se for, troca o tipo atual do token para o 'tipo'
char verificaAFrente(TokenRecord *token, char proximoCaracter, TokenType tipo){
    char *caracter = getCaracter();

    if(*caracter != proximoCaracter){   /// se o caracter lido for diferente do esperado
        caracterAtual = caracter;       /// guarda o caracter n�o processado
        return '0';                     /// retorna que n�o foi trocado, pois o '<' pode ser '<=' e '<>'
    }

    token->tokenval = tipo;             /// troca o tipo;
    return '1';
}

/// estes s�o os poss�veis tokens para processar
/// eles s�o usados no estado inicial para saber qual ser� o token a processar
#define INICIAL 0
#define ESPACO 1        /// ' '
#define NUMERO 2        /// '123.213'
#define IDENTIFICADOR 3 /// 'string' pode ser uma variavel ou palavra reservada
#define UNICO 4         /// '+' '-' '*' '/' '[' ']' '<' '=' '>' '[' ']' '(' ')' ',' estes sao unicos e seus valores sao seus significados
#define LOGICO 5        /// && e ||
#define COMENTARIO 6    /// { }
#define NI 7            /// caracter n�o identificado

TokenRecord* getToken(void){

    char *c;                    /// cada caracter lido
    char *resp;                 /// o resultado do token - este ser� usado para guardar somente dos tokens id ou numero.
    char *nextCharacter;        /// alguns tokens precisam ver o pr�ximo para saber quem s�o: '>' '>='
    int finishToken = FALSE;    /// se terminou de ler o token
    int tokenAtual = INICIAL;   /// o token atual de processamento
    TokenRecord *token;         /// o token propriamente dito

    while(!finishToken){
        c = getCaracter();      /// l� o caracter

        if(*c == EOF){          /// se j� chegou no fim do arquivo
            tokenAtual = EOF;   /// retorna um token EOF
        }

        char i;                 /// usado nos cases de tokens de n�meros e identificadores
        recomputaSwitch:        /// label do goto para nao precisar ler caracter novamente
        switch(tokenAtual){

            case INICIAL:       /// primeira vez, entao ve qual sera o proximo a processar
                if (*c >= '0' && *c <= '9'){
                    tokenAtual = NUMERO;
                } else if ( (*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') ){
                    tokenAtual = IDENTIFICADOR;
                } else if (*c == '*' || *c == '+' || *c == '-' || *c == '/' || *c == ':' || *c == '<' || *c == '='
                        || *c == '>' || *c == '[' || *c == ']' || *c == '(' || *c == ')' || *c == ',' || *c == '!'){
                    tokenAtual = UNICO;
                } else if (*c == ' ' || *c == 13 || *c == 10 || *c == '\t') {
                            /// espa�o, nova linha, line feed ou tabula��o
                    break;  /// entao volta para o comeco do laco e le o proximo caracter
                } else if (*c == '{') {
                    tokenAtual = COMENTARIO;
                } else if (*c == '&' || *c == '|'){
                    tokenAtual = LOGICO;
                } else if (*c == EOF){
                    tokenAtual = EOF;
                } else {    /// algum caracter n�o v�lido
                    tokenAtual = NI;
                    printf("CARACTER INVALIDO: %d\n", *c);
                }

                /// quando sai do case inicial, volta para o 'switch(tokenAtual)' para processar o token identificado no case INICI
                goto recomputaSwitch;

            case NUMERO:    /// este estado le ate o final do numero

                resp = (char*) malloc(SIZE_NUM*sizeof(char));   /// v�riavel para guardar cada d�gito do n�mero
                char size_num = SIZE_NUM;                       /// se precisar realocar mais espaco, sera incrementado o size_num
                char isFloat = FALSE;                           /// se ira transformar em numero com o atof() ou atoi()

                i = getDecimal(resp, (char) 0, size_num, c);    /// l� todo o n�mero e retorna a �ltima posi��o do array

                if(*caracterAtual == '.'){                      /// se for igual h� um ponto, l� o ponto e o n�mero depois do ponto
                    i = getFlutuante(resp, i, size_num);        /// l� todo o restante de n�meros depois da v�rgula
                    isFloat = TRUE;
                }

                /// n�mero com nota��o cient�fica
                /*if(*characterAtual == 'e' || *characterAtual == 'E'){
                    /// chama a fun��o que pega o caractere
                }*/

                finishToken = TRUE;                                 /// termina de ler
                resp[i] = '\0';                                     /// finaliza a representacao do numero no resposta
                token = (TokenRecord*) malloc(sizeof(TokenRecord)); /// cria o token

                /// transformacao do numero
                token->tokenval = isFloat ? NUM_F:NUM_I;            /// marca se � numero inteiro ou float
                if(isFloat) {
                    float *stringval = (float *) malloc(sizeof(float));
                    *stringval = atol(resp);
                    token->val = (void *) stringval;
                } else {
                    int *stringval = (int *) malloc(sizeof(int));
                    *stringval = atoi(resp);                            /// recupera o valor inteiro
                    token->val = (void *) stringval;                    /// guarda no string val
                }

                if(!desalocador){                                       /// se ainda nao tiver sido usado thread
                    desalocador = (pthread_t*) malloc(sizeof(pthread_t));
                }

                pthread_create(desalocador, NULL, desaloca, (void *)resp);   /// inicia desalocar com a thread*/
                //free(resp);
                break;

            case IDENTIFICADOR:
                token = (TokenRecord*) malloc(sizeof(TokenRecord));             /// cria o token
                token->tokenval = ID;                                           /// diz que ele � identificador, POR�M PODE SER ALTERADO SE FOR UMA PALAVRA RESERVADA
                char size_ident = SIZE_IDENT;
                char *stringval = (char*) malloc(size_ident*sizeof(char));      /// letras de ate size_ident caracteres

                for(i = 0; TRUE; i ++){                                         /// l� enquanto tiver caracteres para ler

                    stringval[i] = *c;                                          /// adiciona o caracter na resposta
                    c = getCaracter();                                          /// le o proximo

                    if( ((*c < 'A' || *c > 'Z') && (*c < 'a' || *c > 'z')) &&   /// nao � um digito
                        (*c < '0' || *c > '9') &&                               /// n�o � n�mero
                        (*c != '_' && *c != '�' && *c != '�')){                 /// n�o � '_' nem '�' nem '�'
                        break;                                                  /// termina este for
                    }

                    if(i == size_ident-1){                                      /// se precisar realocar
                        stringval = realoca(stringval, &i, &size_ident);        /// realoca e devolve o novo ponteiro para stringval
                    }
                }

                finishToken = TRUE;                     /// termina de ler
                stringval[i+1] = '\0';                  /// finaliza a representa��o do identificador
                token->val = (void *) stringval;        /// guarda o ponteiro do identificador
                caracterAtual = c;                      /// n�o processa o caracter atual
                palavrasReservadas(token);              /// verifica se o valor do token n�o � uma palavra reservada e troca o seu tipo

                break;

            case UNICO:     /// estes s�o: * + - / : < = > [ ] , !
                token = (TokenRecord*) malloc(sizeof(TokenRecord));

                switch(*c){
                    case '+':
                        token->tokenval = SOMA;
                        break;
                    case '-':
                        token->tokenval = SUBTRACAO;
                        break;
                    case '*':
                        token->tokenval = MULTIPLICACAO;
                        break;
                    case '/':
                        token->tokenval = DIVISAO;
                        break;
                    case '=':
                        token->tokenval = IGUALDADE;
                        break;
                    case '[':
                        token->tokenval = ABRE_COLCHETES;
                        break;
                    case ']':
                        token->tokenval = FECHA_COLCHETES;
                        break;
                    case '(':
                        token->tokenval = ABRE_PARENTESES;
                        break;
                    case ')':
                        token->tokenval = FECHA_PARENTESES;
                        break;
                    case ',':
                        token->tokenval = VIRGULA;
                        break;
                    case '!':
                        token->tokenval = NEGACAO;
                        break;
                    case '>':   /// pode ser '>' ou '>='
                        token->tokenval = MAIOR;
                        verificaAFrente(token, '=', MAIOR_IGUAL);
                        break;
                    case '<':   /// pode ser '<' ou '<=' ou '<>'
                        token->tokenval = MENOR;
                        if(verificaAFrente(token, '=', MENOR_IGUAL) != '1') /// se n�o for '<='
                            verificaAFrente(token, '>', DIFERENTE);         /// teste se � '<>'
                        break;
                    case ':':   /// pode ser ': ou ':='
                        token->tokenval = DOIS_PONTOS;          /// a principio s�o dois pontos
                        verificaAFrente(token, '=', ATRIBUICAO);
                        break;
                }

                finishToken = TRUE;                     /// termina de ler
                break;

            case LOGICO:
                    tokenAtual = NI;                    /// o token ainda n�o foi identificado
                    char *nextCharacter = getCaracter();
                    if (*c != *nextCharacter){          /// n�o s�o os mesmos caracteres: '||' ou '&&'
                        caracterAtual = nextCharacter;  /// guarda o �ltimo caractere lido, o 'nextCharacter', pois o 'c' ser� processado pelo case 'NI'
                        tokenAtual = NI;                /// token n�o identificado
                        goto recomputaSwitch;           /// cria um token de NI
                    }

                    finishToken = TRUE;
                    token = (TokenRecord*) malloc(sizeof(TokenRecord));
                    token->tokenval = (*c == '&' ? E_LOGICO : OU_LOGICO);

                break;

            case COMENTARIO:
                tokenAtual = INICIAL;   /// volta para estado inicial quando sair do while

                char qtd = 1;           /// quantidade de fechas - '}' - que faltam
                while(qtd){             /// s� termina de processar o coment�rio quando tiver fechado todos os abre
                    c = getCaracter();
                    if (*c == '}')
                        qtd --;
                    else if (*c == '{')
                        qtd ++;
                }

                break;

            case EOF:    /// EOF
                finishToken = TRUE;
                token = (TokenRecord*) malloc(sizeof(TokenRecord));
                token->tokenval = EOF;
                break;

            case NI:    /// NAO_IDENTIFICADO
                finishToken = TRUE;
                token = (TokenRecord*) malloc(sizeof(TokenRecord));
                token->tokenval = NAO_IDENTIFICADO;
                token->val = (void *) c;
                break;
        }
    }

    return token;
}

void printToken(TokenRecord *token){
        if (token->tokenval == ID)
            printf("(ID, %s)\n", (char *) token->val);
        else if (token->tokenval == NUM_I)
            printf("(NUM, %d)\n", *((int *) token->val));
        else if (token->tokenval == NUM_F)
            printf("(NUM, %f)\n", *((float *) token->val));
        else if (token->tokenval == ATE)
            printf("ATE\n");
        else if (token->tokenval == ENTAO)
            printf("ENTAO\n");
        else if (token->tokenval == ESCREVA)
            printf("ESCREVA\n");
        else if (token->tokenval == FIM)
            printf("FIM\n");
        else if (token->tokenval == FLUTUANTE)
            printf("FLUTUANTE\n");
        else if (token->tokenval == INTEIRO)
            printf("INTEIRO\n");
        else if (token->tokenval == LEIA)
            printf("LEIA\n");
        else if (token->tokenval == REPITA)
            printf("REPITA\n");
        else if (token->tokenval == RETORNA)
            printf("RETORNA\n");
        else if (token->tokenval == SE)
            printf("SE\n");
        else if (token->tokenval == SENAO)
            printf("SENAO\n");
        else if (token->tokenval == SOMA)
            printf("( + )\n");
        else if (token->tokenval == SUBTRACAO)
            printf("( - )\n");
        else if (token->tokenval == MULTIPLICACAO)
            printf("( * )\n");
        else if (token->tokenval == DIVISAO)
            printf("( / )\n");
        else if (token->tokenval == MAIOR)
            printf("( > )\n");
        else if (token->tokenval == MENOR)
            printf("( < )\n");
        else if (token->tokenval == MAIOR_IGUAL)
            printf("( >= )\n");
        else if (token->tokenval == MENOR_IGUAL)
            printf("( <= )\n");
        else if (token->tokenval == IGUALDADE)
            printf("( = )\n");
        else if (token->tokenval == ABRE_COLCHETES)
            printf("( [ )\n");
        else if (token->tokenval == FECHA_COLCHETES)
            printf("( ] )\n");
        else if (token->tokenval == ABRE_PARENTESES)
            printf("( ( )\n");
        else if (token->tokenval == FECHA_PARENTESES)
            printf("( ) )\n");
        else if (token->tokenval == VIRGULA)
            printf("( , )\n");
        else if (token->tokenval == DOIS_PONTOS)
            printf("( : )\n");
        else if (token->tokenval == ATRIBUICAO)
            printf("( := )\n");
        else if (token->tokenval == E_LOGICO)
            printf("( && )\n");
        else if (token->tokenval == OU_LOGICO)
            printf("( || )\n");
        else if (token->tokenval == DIFERENTE)
            printf("( <> )\n");
        else if (token->tokenval == NAO_IDENTIFICADO)
            printf("(NAO_IDENTIFICADO, %s)\n", (char *) token->val);
}
