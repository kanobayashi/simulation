#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "main.h"
#include "model.h"
#include "agent.h"
#include "obstacle.h"
#include "exit.h"



void init_model(HumanSimulationModel* model, int num_agents, double width, double height) {
    model->num_agents = num_agents;
    model->width = width;
    model->height = height;
    model->time_step = 0.01;
    model->running = 1;

    //出口の設定
    model->num_exits = 1;
    model->exits = (Exit*)malloc(sizeof(Exit) * model->num_exits);
    model->exits[0] = (Exit){ {width/2,0},1.6 , 0.8};


    // エージェント配列
    model->agents = (HumanAgent*)malloc(sizeof(HumanAgent) * num_agents);

    //障害物の初期設定
    model->num_obstacles = 2;
    model->obstacles = (Obstacle*)malloc(sizeof(Obstacle) * model->num_obstacles);
    //円柱
    // model->obstacles[0] = (Obstacle){OBSTACLE_CIRCLE, {18.1, 0.8}, 0.4514, 0.0, 0.0, {0.0,0.0}, {0.0,0.0}, 0.0};
    // model->obstacles[1] = (Obstacle){OBSTACLE_CIRCLE, {19.9, 0.8}, 0.4514, 0.0, 0.0, {0.0,0.0}, {0.0,0.0}, 0.0};
    //四角形
    model->obstacles[0] = (Obstacle){OBSTACLE_RECTANGLE,{18.1,0.8},0.0,0.9,0.9,{0.0,0.0},{0.0,0.0},0.0}; 
    model->obstacles[1] = (Obstacle){OBSTACLE_RECTANGLE,{19.9,0.8},0.0,0.9,0.9,{0.0,0.0},{0.0,0.0},0.0};

    // エージェント初期化
    for (int i = 0; i < num_agents; i++) {
        double x = ((double)rand()/(double)RAND_MAX) * width;
        double y = ((double)rand()/(double)RAND_MAX) * height;
        double tx = model->exits[0].pos[0]; 
        double ty = model->exits[0].pos[1];
        init_agent(&model->agents[i], i, x, y, tx, ty, model);
        // printf("Agent %d: x = %.2f, y = %.2f\n", i, x, y);  // 確認用
    }
}

void step_model(HumanSimulationModel* model) {
    for (int i = model->num_agents - 1; i >= 0; i--) {
        HumanAgent* agent = &model->agents[i];

        // エージェントの更新（移動・力計算）
        update_agent(agent,
                     model->agents,
                     model->num_agents,
                     model->time_step,
                     model);

        // 出口到達判定
        for (int e = 0; e < model->num_exits; e++) {
            Exit *ex = &model->exits[e];
            
            // 出口の左右端の座標
            double exit_left  = ex->pos[0] - ex->width / 2.0;
            double exit_right = ex->pos[0] + ex->width / 2.0;

            // フィードバックに基づいた条件：
            // 1. x が出口範囲内（左端+radius 〜 右端-radius）にある
            // 2. y=0 の線を越えた（中心が y <= 0 になった）
            if (agent->pos[0] >= (exit_left + agent->radius) &&
                agent->pos[0] <= (exit_right - agent->radius)) {
                
                if (agent->pos[1] <= 0.0) {
                    remove_agent(model, agent->id);
                    break; // このエージェントは消滅したので次のエージェントへ
                }
            }
        }
    }
}

void free_model(HumanSimulationModel* model) {
    if (model->agents != NULL) {
        free(model->agents);
        model->agents = NULL;
    }

    if (model->obstacles != NULL) {
        free(model->obstacles);
        model->obstacles = NULL;
    }

        if (model->exits != NULL) {
        free(model->exits);
        model->exits = NULL;
    }
}

void remove_agent(HumanSimulationModel* model, int agent_id) {
    int index = -1;

    // 対象エージェントのインデックスを探す
    for (int i = 0; i < model->num_agents; i++) {
        if (model->agents[i].id == agent_id) {
            index = i;
            break;
        }
    }

    // 見つかった場合は削除（後ろの要素を前に詰める）
    if (index != -1) {
        for (int i = index; i < model->num_agents - 1; i++) {
            model->agents[i] = model->agents[i + 1];
        }
        model->num_agents--;  // エージェント数を1減らす
    }
}
int check_evacuation_complete(HumanSimulationModel* model) {
    return (model->num_agents == 0);
}

// 避難完了までシミュレーションを回してステップ数を返す
int run_simulation(HumanSimulationModel* model) {
    int steps = 0;

    while (!check_evacuation_complete(model)) {
        step_model(model);  // 1ステップ進める
        steps++;

        if (steps > SIMULATION_STEPS) { // 安全のため上限
            break;
        }
    }

    return steps;
}