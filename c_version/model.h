#ifndef MODEL_H
#define MODEL_H

#include "main.h"
#include "agent.h"
#include "obstacle.h"
#include "exit.h"

typedef struct HumanSimulationModel{
    int num_agents; //シミュレーションに参加しているエージェントの総数。
    HumanAgent* agents; //すべてのエージェントのデータを格納する配列へのポインタ。
    int num_obstacles; //シミュレーション空間内の障害物の総数。
    Obstacle* obstacles; //すべての障害物のデータを格納する配列へのポインタ。
    double width; //シミュレーション空間の幅
    double height; //シミュレーション空間の高さ
    double time_step; //シミュレーションの時間刻み
    int running; //シミュレーションが実行中であるかを示すフラグ
    int num_exits;     // 出口の数
    Exit* exits;       //出口のデータを格納する配列へのポインタ

} HumanSimulationModel;

// model.h
void init_model(HumanSimulationModel* model, int num_agents, double width, double height);
void step_model(HumanSimulationModel* model);
void remove_agent(HumanSimulationModel* model, int agent_id);
void free_model(HumanSimulationModel* model);
int run_simulation(HumanSimulationModel *model);
int check_evacuation_complete(HumanSimulationModel *model);

#endif
