#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "questoes.h" // inclui as estruturas definidas em questoes.h
//esse arquivo trata a lógica

//remove o caractere '\n' (quebra de linha) do final de uma string
void remover_newline(char *str) { //ponteiro para a string a ser processada
    size_t len = strlen(str); //retorna o comprimento da string

    //se a string não está vazia e se o último caractere é '\n'
    if (len > 0 && str[len - 1] == '\n') {

        //removendo o '\n' substituindo por pelo caractere de fim de string '\0'
        str[len - 1] = '\0';
    }
}

//le um arquivo .txt e extrai a pergunta, alternativas e gabarito
int carregar_questao_de_arquivo(const char *caminho, Questao *q, int dificuldade) { //endereço do arquivo, ponteiro para a estrutura Questao a ser preenchida, nível de dificuldade
    //abrir o arquivo no modo leitura
    FILE *arquivo = fopen(caminho, "r");

    //se fopen retorna NULL, o arquivo não foi encntrontado
    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo: %s\n", caminho);
        return 0; //retorna 0 como falha
    }

    // fgets lê até 500 caracteres ou até encontrar '\n'
    if (fgets(q->pergunta, sizeof(q->pergunta), arquivo) == NULL) {
        fclose(arquivo); //fecha antes de retornar
        return 0; //retorna 0 como falha
    }
    remover_newline(q->pergunta); // remove o '\n' do final

    //lendo as 4 alternativas
    for (int i = 0; i < 4; i++) {
        if (fgets(q->alternativas[i], sizeof(q->alternativas[i]), arquivo) == NULL) {
            fclose(arquivo);
            return 0;
        }
        remover_newline(q->alternativas[i]);
    }

    //string temporária para guardar a resposta
    char gabarito_str[10];
    if (fgets(gabarito_str, sizeof(gabarito_str), arquivo) == NULL) {
        fclose(arquivo);
        return 0;
    }

    //pegando apenas o primeiro caractere da string para ser o gabarito
    q->gabarito = gabarito_str[0];

    //guarda o nível de dificuldade
    q->dificuldade = dificuldade;

    //fecha o arquivo e libera os recursos
    fclose(arquivo);
    return 1; //retorna 1 como sucesso
}


//carrega 12 questões (4 de cada dificuldade) dos arquivos
void carregar_questoes(ConjuntoQuestoes *facil, ConjuntoQuestoes *medio, ConjuntoQuestoes *dificil) { //ponteiros para os 3 conjuntos de questões
    //define que 0 questões de cada dificudldade foram carregadas
    facil->total = 0;
    medio->total = 0;
    dificil->total = 0;

    //reseta o marcador de questões usadas
    for (int i = 0; i < 20; i++) {
        facil->usadas[i] = 0;
        medio->usadas[i] = 0;
        dificil->usadas[i] = 0;
    }

    //vetor temporário para guardar o caminho do arquivo
    char caminho[256];

    //carrega questões fáceis
    for (int i = 1; i <= 20; i++) {
        //snprintf cria uma string formatada
        snprintf(caminho, sizeof(caminho), "questions/facil/q%d.txt", i);
        if (carregar_questao_de_arquivo(caminho, &facil->questoes[i-1], 0)) { //endereço da questão (i-1 porque começa no 0), 0: dificuldade = fácil
            facil->total++; //contador de questões carregadas
        } else {
            printf("Aviso: Não foi possível carregar questions/facil/q%d.txt\n", i);
        }
    }

    //a seguir é a mesma lógica do carregamento de questões fáceis para questões médias e difíceis também
    for (int i = 1; i <= 20; i++) {
        snprintf(caminho, sizeof(caminho), "questions/medio/q%d.txt", i);
        if (carregar_questao_de_arquivo(caminho, &medio->questoes[i-1], 1)) {
            medio->total++;
        } else {
            printf("Aviso: Não foi possível carregar questions/medio/q%d.txt\n", i);
        }
    }

    for (int i = 1; i <= 20; i++) {
        snprintf(caminho, sizeof(caminho), "questions/dificil/q%d.txt", i);
        if (carregar_questao_de_arquivo(caminho, &dificil->questoes[i-1], 2)) {
            dificil->total++;
        } else {
            printf("Aviso: Não foi possível carregar questions/dificil/q%d.txt\n", i);
        }
    }

    //se algum conjunto não carregou, mostra erro
    if (facil->total == 0 || medio->total == 0 || dificil->total == 0) {
        printf("\nErro: Nem todas as questões foram carregadas!\n");
        printf("Certifique-se de que a pasta 'questions' está no diretório do programa.\n");
    }
}

//retorna uma questão aleatória não usada
Questao obter_questao_aleatoria(ConjuntoQuestoes *conjunto, int dificuldade) { //ponteiro para o ConjuntoQuestoes, nível de dificuldade
    int indice; //indice sorteado
    int tentativas = 0;
    int max_tentativas = 50;

    // seleciona um índice aleatório entre 0 e o total - 1
    do {
        indice = rand() % conjunto->total;  //aleatoriedade
        tentativas++; //contador de tentativas

    //se a questão já foi usada (conjunto->usadas[indice] == 1) e ainda não tentou todas as vezes
    } while (conjunto->usadas[indice] && tentativas < max_tentativas);


    // se todas as questões foram usadas, reseta para serem usadas novamente
    if (conjunto->usadas[indice]) {
            //se chegou aqui, tentou muitas vezes
        for (int i = 0; i < 20; i++) {
            conjunto->usadas[i] = 0; //marcando todas como nao usadas
        }
        indice = 0;//pega a primeira questao
    }

    // marca como usada para não repetir na mesma rodada
    conjunto->usadas[indice] = 1;

    return conjunto->questoes[indice]; //retorna a questão sorteada
}

//reseta o marcador de questoes usadas
void resetar_questoes_usadas(int dificuldade, ConjuntoQuestoes *conjuntos) { //nivel de dificuldade, ponteiro com os 3 conjuntos
    for (int i = 0; i < 20; i++) {
        conjuntos[dificuldade].usadas[i] = 0;
    }
}
