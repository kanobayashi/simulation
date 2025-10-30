//agent.h
#ifndef AGENT_H
#define AGENT_H
//インクルードガード
//このヘッダファイル（AGENT_H）がコンパイル時に複数回読み込まれるのを防ぐ。これにより、構造体の多重定義エラーなどを回避する。

#include "obstacle.h"  // Obstacle型を使うために必要

typedef struct HumanAgent {
    int id;                 // 識別番号
    double pos[2];          // 現在位置
    double target_pos[2];   // 目標位置
    double radius;          // 体の半径
    double mass;            // 質量
    double velocity[2];     // 速度ベクトル
    double desired_speed;   // 希望速度
    double relaxation_time; // 速度調整の時間スケール(エージェントが希望速度に到達するまでの応答時間を示し、小さいほど早く反応)
    double A;               // 社会的反発力のパラメータ
    double B;               // 距離減衰のパラメータ(エージェント間の距離が離れると反発力がどれくらいの速さで弱まるかを示す)
    double k;               // 壁や障害物との反発力の強さを決めるパラメータ
} HumanAgent;

// 関数宣言
void init_agent(HumanAgent *agent, int id, double x, double y, double tx, double ty);
void calculate_desired_force(HumanAgent *agent, double force[2]);
void calculate_social_force(HumanAgent *self, HumanAgent *other, double force[2]);
void calculate_wall_force(HumanAgent *agent, double force[2]);
void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents, double dt);


#endif