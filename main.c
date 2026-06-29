#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "questoes.h" // inclui as estruturas definidas em questoes.h

//essas linhas a seguir foram adicionadas para corrigir um erro de formatação de alguns caracteres especiais no terminal
#ifdef _WIN32
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

//estrutura que armazena o estado atual do jogo (dinheiro, acertos, etc)
typedef struct {
    long dinheiro;
    int ajudas[3];
    int universitarios[3];
    int corretos_atuais;
    int dificuldade_atual;
    int questoes_respondidas;
} EstadoJogo;

//estrutura que pega quais alternativas foram removidas pela ajuda e se a ajuda já foi usada na questão
typedef struct {
    int removidas[4];          //0=não removida, 1=removida (índices a,b,c,d)
    int ajuda_usada;           //0=não usada, 1=usada
    int universitarios_usado;  //0=não usado, 1=usado
    int indice_destaque_uni;   //guarda o índice da alternativa destacada (-1 se não usado)
} EstadoAjuda;

// VARIÁVEIS GLOBAIS
ConjuntoQuestoes conjuntos[3]; //3 conjuntos (fácil, médio, difícil)
EstadoAjuda ajuda_atual;       //estado das ajudas da questão atual

//DECLARAÇÃO DE FUNÇÕES
void exibir_menu_inicial();
void jogar();
void exibir_questao(Questao q, EstadoJogo jogo);
int validar_resposta(char resposta);
void exibir_status_jogo(EstadoJogo jogo);
int usar_ajuda(Questao *q, EstadoJogo *jogo);
void usar_universitarios(Questao *q, EstadoJogo *jogo);

//essa função a seguir foi adicionada para corrigir um erro de formatação de alguns caracteres especiais no terminal
void setup_console() {
#ifdef _WIN32
    // Enable UTF-8 no Windows
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
#endif
}

int main() {
    setup_console();    //configura caracteres especiais
    srand(time(NULL));  //gerador de números aleatórios
    carregar_questoes(&conjuntos[0], &conjuntos[1], &conjuntos[2]); //carrega as questoes de cada dificuldade
    exibir_menu_inicial(); //exibe o menu
    return 0;
}

//menu inicial que permite jogar ou sair
void exibir_menu_inicial() {
    int opcao; //pega a escolha do usuario
    while (1) { //laço continua ate o usuario escolher sair
        system("clear || cls"); //limpa o terminal
        printf("\n");
        printf("BEM-VINDO AO SHOW DO MILHÃO!\n");
        printf("Um Jogo de Perguntas e Respostas\n");
        printf("\n");
        printf("  1. Jogar\n");
        printf("  2. Sair\n");
        printf("\n");
        printf("  Escolha uma opção: ");
        scanf("%d", &opcao);
        getchar();

        //verificando a escolha do usuario
        if (opcao == 1) {
            jogar(); //iniciando de fato  o jogo
        } else if (opcao == 2) {
            printf("\nAté logo!\n");
            break; //encerra o programa
        } else {
            printf("\nOpção inválida! Pressione ENTER para tentar novamente.\n");
            getchar();
        }
    }
}

//executa a logica principal
void jogar() {
    EstadoJogo jogo;
    jogo.dinheiro = 0;  //inicio sem dinheiro acumulado

    //ajudas recebidas em cada nível
    jogo.ajudas[0] = 2;
    jogo.ajudas[1] = 2;
    jogo.ajudas[2] = 2;

    //universitários recebidos em cada nível (1 uso por nível)
    jogo.universitarios[0] = 1;
    jogo.universitarios[1] = 1;
    jogo.universitarios[2] = 1;

    jogo.corretos_atuais = 0; //inicio sem acertos
    jogo.dificuldade_atual = 0; //inicio no nivel facil
    jogo.questoes_respondidas = 0; //inicio sem questoes respondidas

    system("clear || cls");
    printf("     INICIANDO O SHOW DO MILHÃO!\n");
    printf("  Responda perguntas e fique milionário!\n");
    printf("\nPressione ENTER para começar...");
    getchar(); //espera o usuário pressionar Enter

    //laço enquanto não chega no último nível
    while (jogo.dificuldade_atual < 3) {
        system("clear || cls");

        // Reseta o estado de ajuda para a próxima questão
        for (int i = 0; i < 4; i++) {
            ajuda_atual.removidas[i] = 0; //nenhuma alternativa removida ainda
        }
        ajuda_atual.ajuda_usada = 0;
        ajuda_atual.universitarios_usado = 0; // NOVO
        ajuda_atual.indice_destaque_uni = -1; // NOVO

        //sorteia uma questão aleatoria ainda não usada do nível atual
        Questao questao_atual = obter_questao_aleatoria(&conjuntos[jogo.dificuldade_atual], jogo.dificuldade_atual);
        jogo.questoes_respondidas++;            //contador do número de questões respondidas
        exibir_questao(questao_atual, jogo);    //mostra a questão no terminal

        //menu de opções antes de responder
        int opcao_menu = 0;
        int respondida = 0;
        char resposta = 'z';

        while (!respondida) {
            printf("1. Responder\n");

            //verifica se algum recurso já foi usado na questão atual
            int pode_usar_recursos = !ajuda_atual.ajuda_usada && !ajuda_atual.universitarios_usado;

            int opcao_ajuda = 0;
            int opcao_uni = 0;
            int opcao_desistir = 2;

            //monta o menu dinamicamente
            if (pode_usar_recursos) {
                if (jogo.ajudas[jogo.dificuldade_atual] > 0) {
                    opcao_ajuda = opcao_desistir++;
                    printf("%d. Usar ajuda (Remove 2 alternativas)\n", opcao_ajuda);
                }
                if (jogo.universitarios[jogo.dificuldade_atual] > 0) {
                    opcao_uni = opcao_desistir++;
                    printf("%d. Resposta dos Universitários\n", opcao_uni);
                }
            }
            printf("%d. Desistir\n", opcao_desistir);

            //exibe o dinheiro e a quantidade de ajudas do jogador na questão atual
            printf("-------------------------------------------------------------------\n");
            printf(" Dinheiro: R$ %ld\n", jogo.dinheiro);
            printf(" Ajudas disponíveis: %d\n", jogo.ajudas[jogo.dificuldade_atual]);
            printf(" Universitários disponíveis: %d\n", jogo.universitarios[jogo.dificuldade_atual]);
            printf("-------------------------------------------------------------------\n\n");

            printf("Escolha: ");
            scanf("%d", &opcao_menu);
            getchar();

            //opção de responder
            if (opcao_menu == 1) {
                int resposta_valida = 0;

                //enquanto não receber uma resposta válida
                while (!resposta_valida) {
                    printf("Sua resposta (a/b/c/d): ");
                    scanf("%c", &resposta);
                    getchar();

                    //converte maiúsculo para minúsculo
                    if (resposta >= 'A' && resposta <= 'D') {
                        resposta = resposta - 'A' + 'a';
                    }

                    //valida a resposta considerando as alternativas respondidas
                    resposta_valida = validar_resposta(resposta);

                    //se for inválida e for uma letra válida, a mensagem já foi impressa
                    if (!resposta_valida) {
                        printf("Resposta inválida! Digite a, b, c ou d.\n");
                    }
                }
                respondida = 1;
            }

            //usar ajuda
            else if (opcao_menu == opcao_ajuda && opcao_ajuda > 0) {
                usar_ajuda(&questao_atual, &jogo);
                exibir_questao(questao_atual, jogo); //mostra a questão de novo com as alternativas removidas
            }

            //usar universitários
            else if (opcao_menu == opcao_uni && opcao_uni > 0) {
                usar_universitarios(&questao_atual, &jogo);
                exibir_questao(questao_atual, jogo); //mostra a questão de novo com o destaque
            }

            //desistir
            else if (opcao_menu == opcao_desistir) {
                printf("\nVocê desistiu! O jogo foi encerrado. Seu saldo total foi de R$ %ld\n", jogo.dinheiro);
                printf("Pressione ENTER para continuar...");
                getchar();
                return; //sai de jogar() e volta ao menu
            } else {
                printf("Opção inválida!\n");
            }
        }

        //compara a resposta do usuário com o gabarito
        if (resposta == questao_atual.gabarito) {
            printf("\nCORRETO!\n");
            jogo.corretos_atuais++; //contador de acertos no nível

            //somando dinheiro conforme o nivel de dificuldade
            if (jogo.dificuldade_atual == 0) {
                jogo.dinheiro += 10000;
            } else if (jogo.dificuldade_atual == 1) {
                jogo.dinheiro += 90000;
            } else {
                jogo.dinheiro += 150000;
            }
        }
        else {
            printf("\nERRADO! A resposta correta era: %c\n", questao_atual.gabarito);
            printf("Você perdeu todo o seu dinheiro!\n");
            jogo.dinheiro = 0;
        }

        //verifica progressão de nível, se acertar 4 questões, avança
        if (jogo.corretos_atuais >= 4) {
            jogo.corretos_atuais = 0;   //resetando acertos para o proximo nivel
            jogo.dificuldade_atual++;   //avança de nivel

            //se não for o último nível, mostra a mensagem de avanço
            if (jogo.dificuldade_atual < 3) {
                printf("\n🎉 PARABÉNS! Você avançou para o próximo nível!\n");
            }
        }

        exibir_status_jogo(jogo); //mostra o estado atual do jogador (dinheiro, nível, ajudas)

        //se o jogo não terminou, aguarda para a próxima questão
        if (jogo.dificuldade_atual < 3) {
            printf("\nPressione ENTER para a próxima questão...");
            getchar();
        }
    }

    //menu de vitória
    system("clear || cls");
    printf("FIM DO JOGO!\n");
    printf("PARABÉNS, VOCÊ É UM MILIONÁRIO!\n");
    printf("  Dinheiro total acumulado: R$ %ld\n", jogo.dinheiro);
    printf("Pressione ENTER para voltar ao menu...");
    getchar();
}

//DEFINIÇÃO DE FUNÇÕES:
//mostra a questão e suas alternativas na tela
void exibir_questao(Questao q, EstadoJogo jogo) {
    char dificuldades[3][10] = {"FÁCIL", "MÉDIO", "DIFÍCIL"};
    printf("QUESTÃO %d - NÍVEL: %s\n", jogo.questoes_respondidas, dificuldades[q.dificuldade]);
    printf("\n");
    printf("%s\n", q.pergunta);
    printf("\n");

    //exibe as alternativas, destacando a correta se os universitários foram usados
    for (int i = 0; i < 4; i++) {
        if (ajuda_atual.universitarios_usado && i == ajuda_atual.indice_destaque_uni) {
            printf("  %c) %s  <-- Resposta dos Universitários\n", 'a' + i, q.alternativas[i]);
        } else {
            printf("  %c) %s\n", 'a' + i, q.alternativas[i]);
        }
    }
    printf("\n");
}

//verifica se a resposta é uma letra válida (a/b/c/d)
int validar_resposta(char resposta) {
    return (resposta == 'a' || resposta == 'b' || resposta == 'c' || resposta == 'd');
}

//valida resposta considerando alternativas removidas
int validar_resposta_com_ajuda(char resposta) {
    if (resposta < 'a' || resposta > 'd') return 0;
    int indice = resposta - 'a';
    if (ajuda_atual.removidas[indice]) {
        printf("Essa alternativa foi removida! Escolha outra.\n");
        return 0;
    }
    return 1;
}

//mostra dinheiro, nível, acertos e ajudas
void exibir_status_jogo(EstadoJogo jogo) {
    char dificuldades[3][10] = {"FÁCIL", "MÉDIO", "DIFÍCIL"};
    printf(" Dinheiro: R$ %-30ld\n", jogo.dinheiro);
    printf(" Nível Atual: %-32s\n", dificuldades[jogo.dificuldade_atual]);
    printf(" Acertos no nível: %d/4\n", jogo.corretos_atuais);
    printf(" Ajudas disponíveis: %d\n", jogo.ajudas[jogo.dificuldade_atual]);
    printf(" Universitários disponíveis: %d\n", jogo.universitarios[jogo.dificuldade_atual]);
}

//função de usar_ajuda
int usar_ajuda(Questao *q, EstadoJogo *jogo) { //ponteiro para a questão atual, ponteiro para o estado do jogo

    //verificando se já usou a ajuda na questão e se há ajudas disponíveis
    if (ajuda_atual.ajuda_usada) return 0;
    if (jogo->ajudas[jogo->dificuldade_atual] <= 0) return 0;

    //encontrando as alternativas incorretas
    int indices_incorretos[3];
    int count = 0;

    //percorrendo as alternativas
    for (int i = 0; i < 4; i++) {

            //verificando se a alternativa não está vazia, se não é o gabarito e se ainda não foi removida anteriormente
        if (q->alternativas[i][0] != '\0' && q->gabarito != ('a' + i) && !ajuda_atual.removidas[i]) {
            indices_incorretos[count++] = i; //salva o índice da alternativa
        }
    }

    //se encontrar pelo menos duas incorretas, remove-las
    if (count >= 2) {

            //sorteando as alternativas a serem removidas, garantindo que as duas sejam diferentes
        int remove1 = indices_incorretos[rand() % count];
        int remove2;
        do {
            remove2 = indices_incorretos[rand() % count];
        } while (remove2 == remove1);

        //marca as alternativas como removidas
        ajuda_atual.removidas[remove1] = 1;
        ajuda_atual.removidas[remove2] = 1;
        strcpy(q->alternativas[remove1], "[REMOVIDA]");
        strcpy(q->alternativas[remove2], "[REMOVIDA]");
    }

    //marca que a ajuda ja foi usada e remove um uso
    ajuda_atual.ajuda_usada = 1;
    jogo->ajudas[jogo->dificuldade_atual]--;

    printf("\nAJUDA UTILIZADA! Duas alternativas foram removidas.\n");
    printf("Pressione ENTER para continuar...");
    getchar();
    return 1; //sucesso
}

//função de usar ajuda dos universitarios
void usar_universitarios(Questao *q, EstadoJogo *jogo) {
    printf("\nOs universitários responderam!\n");
    printf("Pressione ENTER para ver a resposta...");
    getchar();

    //marca como usado e salva o índice da alternativa correta para destaque
    ajuda_atual.universitarios_usado = 1;
    ajuda_atual.indice_destaque_uni = q->gabarito - 'a';

    //diminui a quantidade de usos disponíveis no nível
    jogo->universitarios[jogo->dificuldade_atual]--;
}
