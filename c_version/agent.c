// agent.c
#include "agent.h"
#include "model.h"
#include "obstacle.h"
#include "main.h"
#include <math.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

void init_agent(HumanAgent *agent, int id, double x, double y, double tx, double ty, HumanSimulationModel *model) {
    agent->id = id;
    agent->pos[0] = fmin(fmax(x, 0.0), model->width);   
    agent->pos[1] = fmin(fmax(y, 0.0), model->height);
    agent->target_pos[0] = tx;
    agent->target_pos[1] = ty;
    agent->radius = 0.25; //m
    agent->mass = 80.0; //kg
    agent->velocity[0] = 0.0; //初期速度　停止しているため0
    agent->velocity[1] = 0.0; 
    agent->desired_speed = 1.5; //m/s
    agent->relaxation_time = 0.5;//s  
    agent->A = 2000; //N
    agent->B = 0.1; //m
    agent->k = 1e5; //kg/m
}


void calculate_desired_force(HumanAgent *agent, double force[2]) {
    double dx = agent->target_pos[0] - agent->pos[0];
    double dy = agent->target_pos[1] - agent->pos[1];
    double dist = sqrt(dx * dx + dy * dy); //距離計算

    if (dist < agent->radius) {
        force[0] = 0.0; //ゴール到達判定
        force[1] = 0.0;
        return;
    }

    double dir_x = dx / dist;
    double dir_y = dy / dist;

    double desired_vx = agent->desired_speed * dir_x;
    double desired_vy = agent->desired_speed * dir_y;

    force[0] = (desired_vx - agent->velocity[0]) * agent->mass / agent->relaxation_time;
    force[1] = (desired_vy - agent->velocity[1]) * agent->mass / agent->relaxation_time;
}

// --- calculate_social_force の修正版 ---
void calculate_social_force(HumanAgent *self, HumanAgent *other, double force[2]) {
    double r_ab = self->radius + other->radius;
    double vec_ab[2] = {
        self->pos[0] - other->pos[0],
        self->pos[1] - other->pos[1]
    };
    double d_ab = sqrt(vec_ab[0]*vec_ab[0] + vec_ab[1]*vec_ab[1]);

    if (d_ab <= 1e-8) { // 同一座標ならランダムな微小ベクトルで回避
        force[0] = 0.0;
        force[1] = 0.0;
        return;
    }

    double n_ab[2] = { vec_ab[0] / d_ab, vec_ab[1] / d_ab };

    // exponent をクリップしてオーバーフロー防止
    double exponent_arg = (r_ab - d_ab) / self->B;
    if (exponent_arg > 20.0) exponent_arg = 20.0;   // exp(20) は ~4.8e8。必要に応じて調整
    if (exponent_arg < -20.0) exponent_arg = -20.0;

    double magnitude = self->A * exp(exponent_arg);

    // 接触（重なり）に対する剛体反発（線形）を追加
    double overlap = fmax(0.0, r_ab - d_ab);
    double body_force_mag = self->k * overlap; // k は接触剛性

    force[0] = magnitude * n_ab[0] + body_force_mag * n_ab[0];
    force[1] = magnitude * n_ab[1] + body_force_mag * n_ab[1];
}

void calculate_wall_force(HumanAgent *agent, double force[2], double width, double height) {
    // 初期化
    force[0] = 0.0;
    force[1] = 0.0;

    double overlap;
    double repulsive;

    // 左壁 x=0
    overlap = fmax(0.0, agent->radius - agent->pos[0]);
    repulsive = agent->k * exp(overlap / agent->B) + agent->k * overlap;
    force[0] += repulsive;

    // 右壁 x=width
    overlap = fmax(0.0, agent->pos[0] + agent->radius - width);
    repulsive = - (agent->k * exp(overlap / agent->B) + agent->k * overlap);
    force[0] += repulsive;

    // 下壁 y=0
    overlap = fmax(0.0, agent->radius - agent->pos[1]);
    repulsive = agent->k * exp(overlap / agent->B) + agent->k * overlap;
    force[1] += repulsive;

    // 上壁 y=height
    overlap = fmax(0.0, agent->pos[1] + agent->radius - height);
    repulsive = - (agent->k * exp(overlap / agent->B) + agent->k * overlap);
    force[1] += repulsive;
}


    // // 下壁 (y = 0)
    // double d_bottom = fmax(0.0, agent->radius - agent->pos[1]);
    // if (d_bottom > 0.0) {
    //     double exp_arg = fmin(d_bottom / agent->B, 20.0);
    //     double repulsive = agent->A * exp(exp_arg);
    //     double contact = agent->k * d_bottom;
    //     force[1] += repulsive + contact;
    // }

    // // 上壁 (y = height)
    // double d_top = fmax(0.0, agent->pos[1] + agent->radius - height);
    // if (d_top > 0.0) {
    //     double exp_arg = fmin(d_top / agent->B, 20.0);
    //     double repulsive = agent->A * exp(exp_arg);
    //     double contact = agent->k * d_top;
    //     force[1] -= repulsive + contact; // 上向きなので負
    // }

    // // 左壁 (x = 0)
    // double d_left = fmax(0.0, agent->radius - agent->pos[0]);
    // if (d_left > 0.0) {
    //     double exp_arg = fmin(d_left / agent->B, 20.0);
    //     double repulsive = agent->A * exp(exp_arg);
    //     double contact = agent->k * d_left;
    //     force[0] += repulsive + contact;
    // }

    // // 右壁 (x = width)
    // double d_right = fmax(0.0, agent->pos[0] + agent->radius - width);
    // if (d_right > 0.0) {
    //     double exp_arg = fmin(d_right / agent->B, 20.0);
    //     double repulsive = agent->A * exp(exp_arg);
    //     double contact = agent->k * d_right;
    //     force[0] -= repulsive + contact; // 左向きにするので負
    // }
//}

// --- update_agent の修正版（速度上限の追加など） ---





// void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents,double dt,  HumanSimulationModel *model) {
//     double total_force[2] = {0.0, 0.0};

//     double desire_force[2];
//     calculate_desired_force(agent, desire_force);
//     total_force[0] += desire_force[0];
//     total_force[1] += desire_force[1];

//     for (int i = 0; i < num_agents; i++) {
//         if (agents[i].id != agent->id) {
//             double social_force[2];
//             calculate_social_force(agent, &agents[i], social_force);
//             total_force[0] += social_force[0];
//             total_force[1] += social_force[1];
//         }
//     }

//     double wall_force[2];
//     calculate_wall_force(agent, wall_force, model->width, model->height);
//     total_force[0] += wall_force[0];
//     total_force[1] += wall_force[1];

//     double max_force = 1000.0;
//     double force_mag = sqrt(total_force[0]*total_force[0] + total_force[1]*total_force[1]);
//     if (force_mag > max_force) {
//         total_force[0] = total_force[0] / force_mag * max_force;
//         total_force[1] = total_force[1] / force_mag * max_force;
//     }



//     // 加速度を計算（上限を設ける）
//     double acceleration[2] = {
//         total_force[0] / agent->mass,
//         total_force[1] / agent->mass
//     };

//     // 加速度クリップ（例: vmax_acc = 50 m/s^2）
//     double max_acc = 50.0;
//     double acc_mag = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1]);
//     if (acc_mag > max_acc) {
//         acceleration[0] = acceleration[0] / acc_mag * max_acc;
//         acceleration[1] = acceleration[1] / acc_mag * max_acc;
//     }

//     // 速度更新
//     agent->velocity[0] += acceleration[0] * dt;
//     agent->velocity[1] += acceleration[1] * dt;

//     // 速度上限（例: 5 m/s）
//     double max_v = 5.0;
//     double vmag = sqrt(agent->velocity[0]*agent->velocity[0] + agent->velocity[1]*agent->velocity[1]);
//     if (vmag > max_v) {
//         agent->velocity[0] = agent->velocity[0] / vmag * max_v;
//         agent->velocity[1] = agent->velocity[1] / vmag * max_v;
//     }

//     // 位置更新
//     agent->pos[0] += agent->velocity[0] * dt;
//     agent->pos[1] += agent->velocity[1] * dt;

//     // Y 範囲外抑制（床より下に行かないようにする）
//     if (agent->pos[1] < 0.0) agent->pos[1] = 0.0;
// }

void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents, double dt, HumanSimulationModel *model) {
    double total_force[2] = {0.0, 0.0};

    double desire_force[2];
    calculate_desired_force(agent, desire_force);
    total_force[0] += desire_force[0];
    total_force[1] += desire_force[1];

    // 社会的相互作用
    for (int i = 0; i < num_agents; i++) {
        if (agents[i].id != agent->id) {
            double social_force[2];
            calculate_social_force(agent, &agents[i], social_force);
            total_force[0] += social_force[0];
            total_force[1] += social_force[1];
        }
    }

    // 壁との力
    double wall_force[2];
    calculate_wall_force(agent, wall_force, model->width, model->height);
    total_force[0] += wall_force[0];
    total_force[1] += wall_force[1];

    // 力の上限
    double max_force = 1000.0;
    double fmag = sqrt(total_force[0]*total_force[0] + total_force[1]*total_force[1]);
    if (fmag > max_force) {
        total_force[0] = total_force[0] / fmag * max_force;
        total_force[1] = total_force[1] / fmag * max_force;
    }

    // 加速度計算
    double acceleration[2] = { total_force[0] / agent->mass, total_force[1] / agent->mass };
    double max_acc = 5.0; // 現実的な人間の加速度上限
    double acc_mag = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1]);
    if (acc_mag > max_acc) {
        acceleration[0] = acceleration[0] / acc_mag * max_acc;
        acceleration[1] = acceleration[1] / acc_mag * max_acc;
    }

    // 速度更新
    agent->velocity[0] += acceleration[0] * dt;
    agent->velocity[1] += acceleration[1] * dt;

    // 速度上限
    double max_v = 2.0; // 人間の歩行速度上限
    double vmag = sqrt(agent->velocity[0]*agent->velocity[0] + agent->velocity[1]*agent->velocity[1]);
    if (vmag > max_v) {
        agent->velocity[0] = agent->velocity[0] / vmag * max_v;
        agent->velocity[1] = agent->velocity[1] / vmag * max_v;
    }

    // 位置更新
    agent->pos[0] += agent->velocity[0] * dt;
    agent->pos[1] += agent->velocity[1] * dt;

    // 位置クリップ（壁の内側に留める）
    if (agent->pos[0] < 0.0) agent->pos[0] = 0.0;
    if (agent->pos[0] > model->width) agent->pos[0] = model->width;
    if (agent->pos[1] < 0.0) agent->pos[1] = 0.0;
    if (agent->pos[1] > model->height) agent->pos[1] = model->height;
}