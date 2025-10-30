// agent.c
// desire_force,social_force,wall_force と　update_agent
#include "agent.h"
#include <math.h>
#include <string.h>
#include "obstacle.h"
#include <stdio.h>

void init_agent(HumanAgent *agent, int id, double x, double y, double tx, double ty) {
    agent->id = id;
    agent->pos[0] = x;
    agent->pos[1] = y;
    agent->target_pos[0] = tx;
    agent->target_pos[1] = ty;
    agent->radius = 0.3; //m
    agent->mass = 70.0; //kg
    agent->velocity[0] = 0.0; //初期速度　停止しているため0
    agent->velocity[1] = 0.0; 
    agent->desired_speed = 1.0; //m/s
    agent->relaxation_time = 0.5;  
    agent->A = 2000.0; //N
    agent->B = 0.08; //m
    agent->k = 1.2; //N/m
}
#include <float.h> // for DBL_MAX if needed
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

// --- calculate_wall_force の修正版 ---
void calculate_wall_force(HumanAgent *agent, double force[2]) {
    // 距離は「どれだけはみ出してるか」または「半径との差」
    double distance_to_wall = fmax(0.0, agent->radius - agent->pos[1]);
    if (distance_to_wall <= 0.0) {
        force[0] = 0.0;
        force[1] = 0.0;
        return;
    }

    // exponent をクリップ
    double exponent_arg = distance_to_wall / agent->B;
    if (exponent_arg > 20.0) exponent_arg = 20.0;

    double repulsive = agent->k * exp(exponent_arg); // もしくは agent->A * exp(exponent_arg) にする
    // さらに接触に対する線形反発を加える（オーバーラップがある場合）
    double contact = agent->k * distance_to_wall;

    force[0] = 0.0;
    force[1] = repulsive + contact;
}

// --- update_agent の修正版（速度上限の追加など） ---
void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents,double dt) {
    double total_force[2] = {0.0, 0.0};

    double desire_force[2];
    calculate_desired_force(agent, desire_force);
    total_force[0] += desire_force[0];
    total_force[1] += desire_force[1];

    for (int i = 0; i < num_agents; i++) {
        if (agents[i].id != agent->id) {
            double social_force[2];
            calculate_social_force(agent, &agents[i], social_force);
            total_force[0] += social_force[0];
            total_force[1] += social_force[1];
        }
    }

    double wall_force[2];
    calculate_wall_force(agent, wall_force);
    total_force[0] += wall_force[0];
    total_force[1] += wall_force[1];

    // 加速度を計算（上限を設ける）
    double acceleration[2] = {
        total_force[0] / agent->mass,
        total_force[1] / agent->mass
    };

    // 加速度クリップ（例: vmax_acc = 50 m/s^2）
    double max_acc = 50.0;
    double acc_mag = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1]);
    if (acc_mag > max_acc) {
        acceleration[0] = acceleration[0] / acc_mag * max_acc;
        acceleration[1] = acceleration[1] / acc_mag * max_acc;
    }

    // 速度更新
    agent->velocity[0] += acceleration[0] * dt;
    agent->velocity[1] += acceleration[1] * dt;

    // 速度上限（例: 5 m/s）
    double max_v = 5.0;
    double vmag = sqrt(agent->velocity[0]*agent->velocity[0] + agent->velocity[1]*agent->velocity[1]);
    if (vmag > max_v) {
        agent->velocity[0] = agent->velocity[0] / vmag * max_v;
        agent->velocity[1] = agent->velocity[1] / vmag * max_v;
    }

    // 位置更新
    agent->pos[0] += agent->velocity[0] * dt;
    agent->pos[1] += agent->velocity[1] * dt;

    // Y 範囲外抑制（床より下に行かないようにする）
    if (agent->pos[1] < 0.0) agent->pos[1] = 0.0;
}


/*

void calculate_social_force(HumanAgent *self, HumanAgent *other, double force[2]) {
    double r_ab = self->radius + other->radius;
    double vec_ab[2] = {
        self->pos[0] - other->pos[0],
        self->pos[1] - other->pos[1]
    };
    double d_ab = sqrt(vec_ab[0]*vec_ab[0] + vec_ab[1]*vec_ab[1]);

    if (d_ab == 0.0) {
        force[0] = 0.0;
        force[1] = 0.0;
        return;
    }

    double n_ab[2] = { vec_ab[0] / d_ab, vec_ab[1] / d_ab };
    double magnitude = self->A * exp((r_ab - d_ab) / self->B);

    force[0] = magnitude * n_ab[0];
    force[1] = magnitude * n_ab[1];
}

//壁がy=0の下の辺のみになっている　変更
void calculate_wall_force(HumanAgent *agent, double force[2]) {
    if (agent->pos[1] < agent->radius) {
        double distance_to_wall = fmax(0.0, agent->radius - agent->pos[1]);
        force[0] = 0.0;
        force[1] = agent->k * exp(distance_to_wall / agent->B);
    } else {
        force[0] = 0.0;
        force[1] = 0.0;
    }
}



// Obstacle *obstacles, int num_obstacles,  で一旦障害物無視ver
void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents,double dt) {
    double total_force[2] = {0.0, 0.0};

    // 1. Desire Force
    double desire_force[2];
    calculate_desired_force(agent, desire_force);
    total_force[0] += desire_force[0];
    total_force[1] += desire_force[1];

    // 2. Social Force
    for (int i = 0; i < num_agents; i++) {
        if (agents[i].id != agent->id) {
            double social_force[2];
            calculate_social_force(agent, &agents[i], social_force);
            total_force[0] += social_force[0];
            total_force[1] += social_force[1];
        }
    }

    // 3. Wall Force（y=0の壁）
    double wall_force[2];
    calculate_wall_force(agent, wall_force);
    total_force[0] += wall_force[0];
    total_force[1] += wall_force[1];

    // 加速度を計算
    double acceleration[2] = {
        total_force[0] / agent->mass,
        total_force[1] / agent->mass
    };

    // 速度を更新
    agent->velocity[0] += acceleration[0] * dt;
    agent->velocity[1] += acceleration[1] * dt;

    // 位置を更新
    agent->pos[0] += agent->velocity[0] * dt;
    agent->pos[1] += agent->velocity[1] * dt;

}
*/