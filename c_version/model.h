#ifndef MODEL_H
#define MODEL_H

#include "agent.h"
#include "obstacle.h"
#include "exit.h"

typedef struct {
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

void init_model(HumanSimulationModel* model, int num_agents, double width, double height); //シミュレーション全体を初期化
void step_model(HumanSimulationModel* model); //シミュレーションを1ステップ進めます
void remove_agent(HumanSimulationModel* model, int agent_id); //指定されたIDのエージェントをシミュレーションから削除
void free_model(HumanSimulationModel* model);//シミュレーションで使用したすべての動的メモリを解放

#endif
