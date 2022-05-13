#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

typedef struct{
    char id[2];
    bool sensors[4];
} Equipment;

/**
 * @brief Initialize new equipment with all sensors deactivated.
 * 
 * @param id  TODO...
 * @return Equipment 
 */
Equipment initEquipment(char* id){
    Equipment E;
    E.id[0] = id[0];
    E.id[1] = id[1];
    E.sensors[0] = false;
    E.sensors[1] = false;
    E.sensors[2] = false;
    E.sensors[3] = false;
    return E;
}

/**
 * @brief Activate na equipment sensor.
 * 
 * Returns:
 * 0 = Success;
 * 1 = Error: Sensor already installed;
 * 2 = Error: Max sensors achieved; TODO: check if we're really gonna do it this way
 * 
 * @param e   TODO...
 * @param id   TODO...
 * @param activeSensorsCount   TODO...
 * @return int 
 */
int addSensor(Equipment *equipment, int id, int *activeSensorsCount){
    if (*activeSensorsCount >= 15)
        return 2;
    else if (equipment->sensors[id-1] == 0){
        equipment->sensors[id-1] = 1;
        *activeSensorsCount += 1;
        return 0;
    }
    else
        return 1;
}

/**
 * @brief Remove one sensor from an equipment.
 * 
 * Returns:
 * 0 = Success;
 * 1 = Error: Sensor already installed; TODO: check if this is necessary...
 * 
 * @param e 
 * @param id 
 * @param activeSensorsCount ; TODO: check if we're really gonna do it this way
 * @return int 
 */
int removeSensor(Equipment *equipment, int id, int *activeSensorsCount){
    if (equipment->sensors[id-1] == 0)
        return 1;
    else {
        equipment->sensors[id-1] = 0;
        *activeSensorsCount -= 1;
        return 0;
    }
}

/**
 * @brief TODO...
 * 
 * @param e 
 * @return const char* 
 */
const char* listSensors(Equipment equipment){
    char *resultList;
    bool hasSensor = false;

    if (equipment.sensors[0] == true){
        strcat(resultList, "01 ");
        hasSensor = true;
    }
    if (equipment.sensors[1] == true){
        strcat(resultList, "02 ");
        hasSensor = true;
    }
    if (equipment.sensors[2] == true){
        strcat(resultList, "03 ");
        hasSensor = true;
    }
    if (equipment.sensors[3] == true){
        strcat(resultList, "04");
        hasSensor = true;
    }
    if (hasSensor == false){
        return "none";
    }

    return resultList;
}


/**
 * TODO: 2022-05-13 - Checar isso...
 * TODO: 2022-05-13 - ADD Descricao
 */
const char* readSensor(Equipment equipment, int *id){
    
    char *retorno_tem, *retorno_nao_tem, *temp;
    int i, r;
    int tamanho_vetor = sizeof(id)/sizeof((id)[0]);
    bool falta_sensor = false;

    strcat(retorno_nao_tem, "sensor(s) ");

    for (i=0; i<tamanho_vetor; i++){
        if (equipment.sensors[id[i] - 1] == true){
            r = rand() % 100;
            sprintf(temp, "%d", r);
            strcat(retorno_tem, temp);
        }
        else {
            sprintf(temp, "%d", id[i]);
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

    // Inicializando os equipamentos e o contador de sensors ativos
    int alarmes_ativos = 0;
    Equipment Equipamentos[4];
    Equipamentos[0] = initEquipment("01");
    Equipamentos[1] = initEquipment("02");
    Equipamentos[2] = initEquipment("03");
    Equipamentos[3] = initEquipment("04");

    addSensor(&Equipamentos[0], 1, &alarmes_ativos);
    printf("%s\n", Equipamentos[0].id);
    printf("%d\n", Equipamentos[0].sensors[0]);
    printf("%d\n", alarmes_ativos);

    printf("-----------------\n");

    printf("%s\n", listSensors(Equipamentos[0]));
    printf("%s\n", listSensors(Equipamentos[1]));

    printf("-----------------\n");

    int teste[2] = {1, 2};
    printf("%s\n", readSensor(Equipamentos[0], teste));
    printf("%s\n", readSensor(Equipamentos[1], teste));

    printf("-----------------\n");

    removeSensor(&Equipamentos[0], 1, &alarmes_ativos);
    printf("%s\n", Equipamentos[0].id);
    printf("%d\n", Equipamentos[0].sensors[0]);
    printf("%d\n", alarmes_ativos);

    return 0;
}