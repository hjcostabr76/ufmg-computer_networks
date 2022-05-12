#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

typedef struct{
    char id[2];
    bool sensores[4];
} Equipamento;

// Inicializa o equipamento com todos os sensores inativos
Equipamento InicializaEquipamento(char* id){
    Equipamento E;
    E.id[0] = id[0];
    E.id[1] = id[1];
    E.sensores[0] = false;
    E.sensores[1] = false;
    E.sensores[2] = false;
    E.sensores[3] = false;
    return E;
}

/* Realiza a instalação do sensor, com os seguintes valores de retorno:
    0 = O sensor estava desinstalado e foi instalado com sucesso
    1 = O sensor já estava instalado
    2 = O sensor não foi instalado pois o máximo de sensores já está em uso
*/
int InstalaSensor(Equipamento *e, int id_sensor, int *alarmes_ativos){
    if (*alarmes_ativos >= 15)
        return 2;
    else if (e->sensores[id_sensor-1] == 0){
        e->sensores[id_sensor-1] = 1;
        *alarmes_ativos += 1;
        return 0;
    }
    else
        return 1;
}

/* Realiza a desinstalação do sensor, com os seguintes valores de retorno:
    0 = O sensor estava instalado e foi desinstalado com sucesso
    1 = O sensor já estava desinstalado
*/
int DesinstalaSensor(Equipamento *e, int id_sensor, int *alarmes_ativos){
    if (e->sensores[id_sensor-1] == 0)
        return 1;
    else {
        e->sensores[id_sensor-1] = 0;
        *alarmes_ativos -= 1;
        return 0;
    }
}

const char* ConsultaEquipamento(Equipamento e){
    char *retorno;
    bool tem_sensor = false;

    if (e.sensores[0] == true){
        strcat(retorno, "01 ");
        tem_sensor = true;
    }
    if (e.sensores[1] == true){
        strcat(retorno, "02 ");
        tem_sensor = true;
    }
    if (e.sensores[2] == true){
        strcat(retorno, "03 ");
        tem_sensor = true;
    }
    if (e.sensores[3] == true){
        strcat(retorno, "04");
        tem_sensor = true;
    }
    if (tem_sensor == false){
        return "none";
    }

    return retorno;
}


// Está dando erro, não consegui consertar
const char* ConsultaSensor(Equipamento e, int *id_sensor){
    char *retorno_tem, *retorno_nao_tem, *temp;
    int i, r;
    int tamanho_vetor = sizeof(id_sensor)/sizeof((id_sensor)[0]);
    bool falta_sensor = false;

    strcat(retorno_nao_tem, "sensor(s) ");

    for (i=0; i<tamanho_vetor; i++){
        if (e.sensores[id_sensor[i] - 1] == true){
            r = rand() % 100;
            sprintf(temp, "%d", r);
            strcat(retorno_tem, temp);
        }
        else {
            sprintf(temp, "%d", id_sensor[i]);
            strcat(retorno_nao_tem, " 0");
            strcat(retorno_nao_tem, temp);
            falta_sensor = true;
        }
    }

    if(falta_sensor == true){
        strcat(retorno_nao_tem, " not installed");
        return retorno_nao_tem;
    }

    return retorno_tem;
}

int main(){
    
    // Seed para números aleatórios
    srand(time(NULL));

    // Inicializando os equipamentos e o contador de sensores ativos
    int alarmes_ativos = 0;
    Equipamento Equipamentos[4];
    Equipamentos[0] = InicializaEquipamento("01");
    Equipamentos[1] = InicializaEquipamento("02");
    Equipamentos[2] = InicializaEquipamento("03");
    Equipamentos[3] = InicializaEquipamento("04");

    InstalaSensor(&Equipamentos[0], 1, &alarmes_ativos);
    printf("%s\n", Equipamentos[0].id);
    printf("%d\n", Equipamentos[0].sensores[0]);
    printf("%d\n", alarmes_ativos);

    printf("-----------------\n");

    printf("%s\n", ConsultaEquipamento(Equipamentos[0]));
    printf("%s\n", ConsultaEquipamento(Equipamentos[1]));

    printf("-----------------\n");

    int teste[2] = {1, 2};
    printf("%s\n", ConsultaSensor(Equipamentos[0], teste));
    printf("%s\n", ConsultaSensor(Equipamentos[1], teste));

    printf("-----------------\n");

    DesinstalaSensor(&Equipamentos[0], 1, &alarmes_ativos);
    printf("%s\n", Equipamentos[0].id);
    printf("%d\n", Equipamentos[0].sensores[0]);
    printf("%d\n", alarmes_ativos);

    return 0;
}