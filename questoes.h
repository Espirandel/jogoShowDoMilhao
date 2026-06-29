#ifndef QUESTOES_H
#define QUESTOES_H



// estrutura de questão que armazena uma pergunta com suas alternativas e resposta
typedef struct {
    char pergunta[500];        // vetor da pergunta (até 500 caracteres)
    char alternativas[4][200]; // 4 alternativas (a,b,c,d)
    char gabarito;             // letra da resposta correta
    int dificuldade;           // nível (0=fácil, 1=médio, 2=difícil)
} Questao;

// estrutura do conjunto de questões que gerencia todas as questões de uma dificuldade
typedef struct {
    Questao questoes[20];  // vetor que guarda as questões de cada nível
    int total;            // quantidade de questões carregadas
    int usadas[20];        // marca quais questões já foram usadas
} ConjuntoQuestoes;


// funções estão definidas em questoes.c, mas são declaradas aqui:

// carrega todas as questões de todas as dificuldades dos arquivos
void carregar_questoes(ConjuntoQuestoes *facil, ConjuntoQuestoes *medio, ConjuntoQuestoes *dificil); //ponteiros para os conjuntos separados por dificuldades

// pega uma questão aleatória não usada da dificuldade selecionada
Questao obter_questao_aleatoria(ConjuntoQuestoes *conjunto, int dificuldade); //ponteiro para o conjunto e nível de dificuldade

// reseta o marcador de questões usadas (permite repetição)
void resetar_questoes_usadas(int dificuldade, ConjuntoQuestoes *conjuntos); //nível de dificuldade e ponteiro para o conjunto

#endif // QUESTOES_H
