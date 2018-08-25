#include "varredura.h"

char respVal;               /// se respVal for 0, resp n�o est� apontando para um endere�o alocado, ent�o precisa alocar. Se for 1, n�o precisa alocar em resp
char *resp = NULL;          /// o resultado do token - este ser� usado para guardar somente dos tokens id ou numero.
unsigned short int posFile; /// posi��o do ponteiro do arquivo. Usamos quando precisamos 'voltar' um ou dois caracteres no arquivo, ent�o mudamos o ponteiro para tr�s. Permite arquivos com at� 65.535 caracteres
FILE *leitorArquivo;        /// ponteiro para leitura do arquivo
unsigned char *c = NULL;    /// vari�vel onde todos os valores dos caracteres ser�o colocados
unsigned short int numlines;/// vari�vel para contabilizar o n�mero de linhas
unsigned char numcaracters; /// vari�vel para contabilizar o n�mero do caracter na linha

/********************************************************************
*                   IMPLEMENTA��O DAS FUN��ES                       *
********************************************************************/
/// inicializa algumas v�riaveis necess�rias
void inicializaVariaveis(){
    posFile = 0;
    respVal = 0;
    c = (char *) malloc(sizeof(char));
    numlines = 1;
    numcaracters = 0;
}

/// chamado diretamente da main, esta fun��o retorna 0 se existir o arquivo ou 1 se n�o existir
/// o ponteiro do arquivo � guardado para ser lido caracter por caracter na fun��o getCaracter
char openFile(char *filename) {
    leitorArquivo = fopen(filename, "r");

    if(leitorArquivo == NULL){
        fprintf(stderr, "ERR: arquivo '%s' nao existe.\n", filename);
        return '1';
    }

    inicializaVariaveis();
    return '0';
}

/// retorna um caracter que ainda n�o tenha sido processado
/// ou retorna um caracter direto do arquivo
char* getCaracter(){

    *c = getc(leitorArquivo);   /// l� do arquivo
    posFile ++;                 /// incrementa a posi��o do arquivo
    numcaracters ++;            /// incrementa o n�mero de caracters da linha

    return c;
}

/// volta um posi��o no ponteiro do arquivo
/// assim n�o precisamos utilizar vari�veis auxiliares para guardar o caracter n�o processado
/// principalmente na fun��o onde � obtido o n�mero com nota��o cientifica, que precisa voltar at� tr�s caracteres
void voltaCaracter(){
    posFile --;                                     /// decrementa a quantidade de caracteres lido
    numcaracters --;                                /// decrementa a contagem de caracteres da linha
    char status = fseek(leitorArquivo, posFile, 0); /// desloca o ponteiro para, a partir do zero, uma posi��o atr�s da atual

    if(status == -1) {                              /// se deu erro
        printf("ERRO AO VOLTAR CARACTER NO ARQUIVO\n");
    }
}

/// apenas desaloca a memoria alocada neste ponteiro
/// como � usado por uma thread, tem que ser void *
void *desaloca(void *ptr){
    free(ptr);
}

/// fun��o usada para realocar mem�ria se a letra ou o n�mero tiver muitos caracteres.
/// para evitar realocar sempre, somente sera realocado
/// c � o ponteiro para o vetor a alocar; i � o tamanho total; size_max � o tamanho total deste dado: SIZE_NUM|SIZE_ID
char *realoca(char *c, char *i, char *size_max) {
    *size_max = (char) *i + *size_max-1;            /// novo tamanho sera o tamanho atual mais o tamanho m�ximo inicial menos 1
    c = (char *) realloc(c, *size_max*sizeof(char));/// realoca
    return c;
}

/// verifica se um identificador � igual � uma palavra reservada.
/// se o for chegar at� o final, quer dizer que cada caracter s�o iguais.
/// vale tamb�m para identificadores acentuadpos: SEN�O, ENT�O e AT�
/// retorna '1' caso forem iguais ou '0' caso o contr�rio
char iguais(char* identificador, char palavraReservada[]){
    char i;
    for(i = 0; (identificador[i] == palavraReservada[i]) && identificador[i] != '\0' && palavraReservada[i] != '\0'; i ++);

    return (identificador[i]==palavraReservada[i] && palavraReservada[i]=='\0')?'1':'0';
}

/// verifica se o identificador � uma palavra reservada
/// se for, n�o precisa ter seu valor alocado
void palavrasReservadas(TokenRecord *token, char temAcento){

    /// descobre qual � o caracter inicial do identificador, pois elimina mais da metade das palavras reservadas
    switch(((char *) token->val)[0]){
        case 'a':   /// pode ser o 'at�'
            if(!temAcento || iguais(token->val, "at�\0") == '0') {  /// se n�o tiver acento, nem verifica se s�o iguais
                respVal = 0;                                        /// indica que na pr�xima vez precisa alocar novamente
                return;                                             /// n�o faz nada
            }

            token->tokenval = ATE;
            break;

        case 'e':   /// pode ser o 'ent�o' ou 'escreva'
            if(temAcento && iguais(token->val, "ent�o\0") == '1')           /// se n�o tiver acento, nem verifica se s�o iguais; se forem iguais, j� troca o valor de
                token->tokenval = ENTAO;                                    /// atualiza o token
            else if (!temAcento && iguais(token->val, "escreva\0") == '1')  /// se tiver acento, nem verifica se � igual
                token->tokenval = ESCREVA;
            else {
                respVal = 0;                                    /// indica que na pr�xima vez precisa alocar novamente
                return;                                         /// retorna pra n�o desalocar o token->val
            }

            break;

        case 'i':   /// pode ser o 'inteiro'
            if(temAcento || iguais(token->val, "inteiro\0") == '0'){/// se tiver acento, nem verifica se s�o iguais
                respVal = 0;                                        /// indica que na pr�xima vez precisa alocar novamente
                return;                                             /// n�o faz nada
            }

            token->tokenval = INTEIRO;
            break;

        case 'f':   /// pode ser 'fim' ou 'flutuante'
            if(!temAcento && iguais(token->val, "fim\0") == '1')    /// se tiver acento nem verifica; se n�o, verifica com o fim
                token->tokenval = FIM;                          /// atualiza o token
            else if (!temAcento && iguais(token->val, "flutuante\0") == '1')
                token->tokenval = FLUTUANTE;
            else {
                respVal = 0;                                    /// indica que na pr�xima vez precisa alocar novamente
                return;                                         /// retorna pra n�o desalocar o token->val
            }

            break;

        case 'l':   /// pode ser o 'leia'
            if(temAcento || iguais(token->val, "leia\0") == '0') {  /// se tiver acento ou se forem diferentes
                respVal = 0;                                        /// indica que na pr�xima vez precisa alocar novamente
                return;                                             /// n�o faz nada
            }

            token->tokenval = LEIA;
            break;

        case 'r':   /// pode ser o 'repita' ou 'retorna'
            if(!temAcento && iguais(token->val, "repita\0") == '1') /// se tiver acento nem verifica se � igual
                token->tokenval = REPITA;                           /// atualiza o token
            else if (!temAcento && iguais(token->val, "retorna\0") == '1')
                token->tokenval = RETORNA;
            else {
                respVal = 0;                                    /// indica que na pr�xima vez precisa alocar novamente
                return;                                         /// retorna pra n�o desalocar o token->val
            }

            break;

        case 's':   /// pode ser o 'se' ou 'sen�o'
            if(!temAcento && iguais(token->val, "se\0") == '1') /// se n�o tiver acento e for igual a 'se'
                token->tokenval = SE;                           /// atualiza o token
            else if (temAcento && iguais(token->val, "sen�o\0") == '1') /// se tiver acento e for igual a 'sen�o'
                token->tokenval = SENAO;
            else {
                respVal = 0;                                    /// indica que na pr�xima vez precisa alocar novamente
                return;                                         /// retorna pra n�o desalocar o token->val
            }

            break;

        default:            /// outra palavra que n�o come�a com estas letras
                            /// se for um identificador, continuar� como est�
                            /// se for um n�o identificado, tamb�m continuar� como est�
            respVal = 0;    /// indica que na pr�xima vez precisa alocar novamente
            return;         /// ent�o retorna para n�o desalocar
    }

    /// n�o precisa desalocar, apenas precisa setar a variavel 'respVal' como 1
    /// pois indicamos que ela tem espa�o que pode ser sobrescrito
    respVal = 1;
}

/// l� toda a sequ�ncia de digitos e guarda em 'resp'.
/// como pode ser chamada de outras fun��es, por exemplo, 'getFlutuante',
/// ir� come�ar a concatenar a partir de 'i'.
/// 'size_max' � o tamanho m�ximo que pode ser lido sem precisar de usar um 'realloc'
/// 'c' � o d�gito que ainda n�o foi processado
char getDecimal(char *resp, char i, char size_max, char *c) {

    while(TRUE){                            /// fica lendo at� parar de ler d�gitos

        if(*c < '0' || *c > '9'){           /// n�o � um digito
            voltaCaracter();                /// volta o caracter no arquivo
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

    resp[i] = '.';              /// consome o caractere
    char *c = getCaracter();    /// pega o pr�ximo

    return getDecimal(resp, i+1, size_max, c);   /// l� todo o resto de n�mero
}

/// o caracter atual � 'e' ou 'E'.
/// Ent�o l� um '-' ou '+' seguido de um n�mero
char getNotacaoCientifica(char *resp, char i, char size_max, char *isFloat) {

    char sinal_numero = *getCaracter(); /// pode ser '-' ou '+' ou n�mero
    char *numero = NULL;                /// este apontar� para o n�mero

    if(sinal_numero != '-' && sinal_numero != '+') {    /// se n�o for sinal
        if(sinal_numero < '0' || sinal_numero > '9') {  /// e n�o for letra
            voltaCaracter();                            /// volta este que n�o � sinal nem n�mero
            voltaCaracter();                            /// volta o 'e'

            return i;
        }
        /// ent�o � n�mero
    } else {                                /// 'sinal_numero' realmente tem um sinal
        numero = getCaracter();             /// este DEVE ser n�mero
        if(*numero < '0' || *numero > '9') {/// se n�o for n�mero
            voltaCaracter();                /// volta este que n�o � n�mero
            voltaCaracter();                /// volta o que � sinal
            voltaCaracter();                /// volta o 'e'

            return i;
        } else {                            /// se for n�mero
            voltaCaracter();                /// volta o n�mero para depois re-ler
        }
    }

    if(i % size_max == size_max-2){     /// se precisar realocar, realoca duas posi��es
        resp = realoca(resp, &i, &size_max);
    }

    resp[i++] = 'e';            /// 'e' ou 'E'
    resp[i++] = sinal_numero;   /// numero ou sinal
    *isFloat = TRUE;            /// marca como float

    return getDecimal(resp, i, size_max, getCaracter());/// l� todo o resto de n�mero
}

/// alguns tokens podem mudar dependendo do pr�ximo caracter: >, >=
/// esta fun��o verifica se o pr�ximo caracter � igual ao 'proximoCaracter'.
/// se for, troca o tipo atual do token para o 'tipo'
char verificaAFrente(TokenRecord *token, char proximoCaracter, TokenType tipo){
    char *caracter = getCaracter();

    if(*caracter != proximoCaracter){   /// se o caracter lido for diferente do esperado
        voltaCaracter();                /// guarda o caracter n�o processado
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

    char *nextCharacter;        /// alguns tokens precisam ver o pr�ximo para saber quem s�o: '>' '>='
    int finishToken = FALSE;    /// se terminou de ler o token
    int tokenAtual;             /// o token atual de processamento
    TokenRecord *token = (TokenRecord*) malloc(sizeof(TokenRecord));/// cria o token

    while(!finishToken){
        inicial:                /// label para depois do coment�rio, voltar e recomputar o novo token

        c = getCaracter();      /// l� o caracter

        if(*c == EOFU){          /// se j� chegou no fim do arquivo
            tokenAtual = EOFU;   /// retorna um token EOFU
        }

        /// computa o token atual
        if (*c >= '0' && *c <= '9'){
            tokenAtual = NUMERO;
        } else if ( (*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= 128 && *c <= 252)){
            tokenAtual = IDENTIFICADOR;
        } else if (*c == '*' || *c == '+' || *c == '-' || *c == '/' || *c == ':' || *c == '<' || *c == '='
                || *c == '>' || *c == '[' || *c == ']' || *c == '(' || *c == ')' || *c == ',' || *c == '!'){
            tokenAtual = UNICO;
        } else if (*c == ' ' || *c == 13 || *c == 10 || *c == '\t') {
            /// espa�o, nova linha, line feed ou tabula��o
            if(*c == 13) {
                numlines ++;        /// incrementa n�mero da linha
                numcaracters = 0;   /// volta a contagem de caracteres para 0;
            } else if (*c == 10) {
                numcaracters = 0;    /// line feed - '\n' - que move o ponteiro para a pr�xima linha sem apontar para o primeiro caracter
            }

            goto inicial;   /// entao volta para o comeco do laco e le o proximo caracter
        } else if (*c == '{') {
            tokenAtual = COMENTARIO;
        } else if (*c == '&' || *c == '|'){
            tokenAtual = LOGICO;
        } else if (*c == EOFU){
            tokenAtual = EOFU;
        } else {    /// algum caracter n�o v�lido
            tokenAtual = NI;
        }

        char i;                             /// usado nos cases de tokens de n�meros e identificadores
        recomputaSwitch:                    /// label do goto para nao precisar ler caracter novamente
        switch(tokenAtual){

            case NUMERO:    /// este estado le ate o final do numero
                token->numcaracter = numcaracters;

                if(!respVal)/// se o valor for 0
                    resp = (char*) malloc(SIZE_NUM*sizeof(char)); /// aloca para guardar cada d�gito do n�mero

                char size_num = SIZE_NUM;                       /// se precisar realocar mais espaco, sera incrementado o size_num
                char isFloat = FALSE;                           /// se ira transformar em numero com o atof() ou atoi()

                i = getDecimal(resp, (char) 0, size_num, c);    /// l� todo o n�mero e retorna a �ltima posi��o do array

                if(*getCaracter() == '.'){                      /// se for igual h� um ponto, l� o ponto e o n�mero depois do ponto
                    i = getFlutuante(resp, i, size_num);        /// l� todo o restante de n�meros depois da v�rgula
                    isFloat = TRUE;
                } else {
                    voltaCaracter();                            /// se n�o for o '.', volta o caracter
                }

                c = getCaracter();
                /// n�mero com nota��o cient�fica
                if(*c == 'e' || *c == 'E'){
                    i = getNotacaoCientifica(resp, i, size_num, &isFloat);
                } else {
                    voltaCaracter();                            /// volta o caracter
                }

                finishToken = TRUE;                                 /// termina de ler
                resp[i] = '\0';                                     /// finaliza a representacao do numero no resposta

                /// transformacao do numero
                token->tokenval = isFloat ? NUM_F:NUM_I;            /// marca se � numero inteiro ou float
                if(isFloat) {
                    float *numval = (float *) malloc(sizeof(float));
                    *numval = atof(resp);
                    token->val = (void *) numval;
                } else {
                    int *numval = (int *) malloc(sizeof(int));
                    *numval = atoi(resp);                            /// recupera o valor inteiro
                    token->val = (void *) numval;                    /// guarda no string val
                }

                respVal = 1;
                break;

            case IDENTIFICADOR:
                token->numcaracter = numcaracters;

                if(!respVal)                                                    /// se o valor for 0
                    resp = (char*) malloc(SIZE_IDENT*sizeof(char));             /// aloca para guardar letras at� size_ident caracteres

                token->tokenval = ID;                                           /// diz que ele � identificador, POR�M PODE SER ALTERADO SE FOR UMA PALAVRA RESERVADA
                char size_ident = SIZE_IDENT;
                char temAcento = 0;                                             /// para indicar se o identificador tem acento

                for(i = 0; TRUE; i ++){                                         /// l� enquanto tiver caracteres para ler

                    resp[i] = *c;                                               /// adiciona o caracter na resposta
                    c = getCaracter();                                          /// le o proximo

                    if( ((*c < 'A' || *c > 'Z') && (*c < 'a' || *c > 'z')) &&   /// nao � uma letra
                        (*c < '0' || *c > '9') &&                               /// n�o � n�mero
                        (*c != '_' && (*c < 128 || *c > 252))){                 /// n�o � '_' nem acentos
                        break;                                                  /// termina este for
                    }

                    if(*c >= 128 && *c <= 252){                                 /// acento s� pode em palavras reservadas
                        token->tokenval = NAO_IDENTIFICADO;                     /// troca para token n�o identificado
                        temAcento = 1;                                          /// se n�o for reservada e tiver acento, n�o � reconhecido
                    }

                    if(i == size_ident-1){                                      /// se precisar realocar
                        resp = realoca(resp, &i, &size_ident);        /// realoca e devolve o novo ponteiro para resp
                    }
                }

                finishToken = TRUE;         /// termina de ler
                resp[i+1] = '\0';           /// finaliza a representa��o do identificador
                token->val = (void *) resp; /// guarda o ponteiro do identificador
                voltaCaracter();            /// n�o processa o caracter atual
                palavrasReservadas(token, temAcento);   /// verifica se o valor do token n�o � uma palavra reservada e troca o seu tipo

                break;

            case UNICO:     /// estes s�o: * + - / : < = > [ ] , !
                token->numcaracter = numcaracters;

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
                token->numcaracter = numcaracters;

                tokenAtual = NI;                    /// o token ainda n�o foi identificado
                char atual = *c;                    /// o caracter l�gico: | ou &
                char *nextCharacter = getCaracter();/// o pr�ximo
                if (atual != *nextCharacter){       /// n�o s�o os mesmos caracteres: '||' ou '&&'
                    *c = atual;                     /// salva para o NI ler como n�o identificado
                    voltaCaracter();                /// volta o que n�o � igual ao '|' ou '&'
                    goto recomputaSwitch;           /// cria um token de NI
                }

                finishToken = TRUE;
                token->tokenval = (*c == '&' ? E_LOGICO : OU_LOGICO);

                break;

            case COMENTARIO:
                token->numcaracter = numcaracters;

                char qtd = 1;           /// quantidade de fechas - '}' - que faltam
                while(qtd){             /// s� termina de processar o coment�rio quando tiver fechado todos os abre
                    c = getCaracter();
                    if (*c == 13)       /// contabiliza o n�mero de linhas
                        numlines ++;

                    if(*c == EOFU){      /// se n�o fechar o coment�rio
                        tokenAtual = EOFU;
                        goto recomputaSwitch;
                    }

                    if (*c == '}')
                        qtd --;
                    else if (*c == '{')
                        qtd ++;
                }

                goto inicial;           /// volta para estado inicial quando sair do while

            case EOFU:    /// EOFU
                finishToken = TRUE;
                token->tokenval = EOFU;
                free(resp);             /// n�o vai mais us�-lo
                fclose(leitorArquivo);  /// fecha o arquivo
                leitorArquivo = NULL;
                break;

            case NI:    /// NAO_IDENTIFICADO
                token->numcaracter = numcaracters;

                finishToken = TRUE;
                token->tokenval = NAO_IDENTIFICADO;
                char *b = (char *) malloc(sizeof(char));
                *b = *c;
                token->val = (void *) b;
                respVal = 0;            /// este endere�o n�o pode ser reaproveitado
                break;
        }
    }

    token->numline = numlines;  /// guarda o n�mero da linha que este token foi encontrado
    return token;
}

void printToken(TokenRecord *token, char printLine, char printCaracter){
        if (token->tokenval == ID)
            printf("(ID, %s", (char *) token->val);
        else if (token->tokenval == NUM_I)
            printf("(NUM, %d", *((int *) token->val));
        else if (token->tokenval == NUM_F)
            printf("(NUM, %f", *((float *) token->val));
        else if (token->tokenval == ATE)
            printf("(ATE");
        else if (token->tokenval == ENTAO)
            printf("(ENTAO");
        else if (token->tokenval == ESCREVA)
            printf("(ESCREVA");
        else if (token->tokenval == FIM)
            printf("(FIM");
        else if (token->tokenval == FLUTUANTE)
            printf("(FLUTUANTE");
        else if (token->tokenval == INTEIRO)
            printf("(INTEIRO");
        else if (token->tokenval == LEIA)
            printf("(LEIA");
        else if (token->tokenval == REPITA)
            printf("(REPITA");
        else if (token->tokenval == RETORNA)
            printf("(RETORNA");
        else if (token->tokenval == SE)
            printf("(SE");
        else if (token->tokenval == SENAO)
            printf("(SENAO");
        else if (token->tokenval == SOMA)
            printf("( + ");
        else if (token->tokenval == SUBTRACAO)
            printf("( - ");
        else if (token->tokenval == MULTIPLICACAO)
            printf("( * ");
        else if (token->tokenval == DIVISAO)
            printf("( / ");
        else if (token->tokenval == MAIOR)
            printf("( > ");
        else if (token->tokenval == MENOR)
            printf("( < ");
        else if (token->tokenval == MAIOR_IGUAL)
            printf("( >= ");
        else if (token->tokenval == MENOR_IGUAL)
            printf("( <= ");
        else if (token->tokenval == IGUALDADE)
            printf("( = ");
        else if (token->tokenval == ABRE_COLCHETES)
            printf("( [ ");
        else if (token->tokenval == FECHA_COLCHETES)
            printf("( ] ");
        else if (token->tokenval == ABRE_PARENTESES)
            printf("( ( ");
        else if (token->tokenval == FECHA_PARENTESES)
            printf("( ) ");
        else if (token->tokenval == VIRGULA)
            printf("( , ");
        else if (token->tokenval == DOIS_PONTOS)
            printf("( : ");
        else if (token->tokenval == ATRIBUICAO)
            printf("( := ");
        else if (token->tokenval == E_LOGICO)
            printf("( && ");
        else if (token->tokenval == OU_LOGICO)
            printf("( || ");
        else if (token->tokenval == DIFERENTE)
            printf("( <> ");
        else if (token->tokenval == NAO_IDENTIFICADO)
            printf("(NAO_IDENTIFICADO, %s", (char *) token->val);
        else if (token->tokenval == EOFU)
            return;

        if(printLine)
            printf(", %d", token->numline);
        if(printCaracter)
            printf(", %d", token->numcaracter);

        printf(")\n");
}
