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
    agent->radius = 0.25; 
    agent->mass = 80.0; 
    agent->velocity[0] = 0.0; 
    agent->velocity[1] = 0.0; 
    agent->desired_speed = 1.5; 
    agent->relaxation_time = 0.5;
    agent->A = 2000;
    agent->B = 0.08;
    agent->k = 1.2e5;
    agent->kappa = 2.4e5; // 2.4 * 10^5 kg m^-1 s^-1
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

// --- calculate_social_force  ---
void calculate_social_force(HumanAgent *self, HumanAgent *other, double force[2]) {
    double r_ab = self->radius + other->radius;
    double vec_ab[2] = {
        self->pos[0] - other->pos[0],
        self->pos[1] - other->pos[1]
    };
    double d_ab = sqrt(vec_ab[0]*vec_ab[0] + vec_ab[1]*vec_ab[1]);

    //距離が0の時に計算が崩壊するのを防ぐ
    if (d_ab <= 1e-8) {
        force[0] = 0.0;
        force[1] = 0.0;
        return;
    }

    //単位ベクトルの計算
    double n_ab[2] = { vec_ab[0] / d_ab, vec_ab[1] / d_ab };

    // exponent をクリップしてオーバーフロー防止
    double exponent_arg = (r_ab - d_ab) / self->B;
    if (exponent_arg > 20.0) exponent_arg = 20.0;   // exp(20) は ~4.8e8。必要に応じて調整
    if (exponent_arg < -20.0) exponent_arg = -20.0;

    double magnitude = self->A * exp(exponent_arg);

    // 接触（重なり）に対する剛体反発（線形）を追加
    double overlap = fmax(0.0, r_ab - d_ab);
    double body_force_mag = self->k * overlap; // k は接触剛性

    double friction_force[2] = {0.0, 0.0};
    if (overlap > 0.0) { 
        // 接線ベクトル t_ab の作成 (n_ab を反時計回りに90度回転: (-ny, nx))
        double t_ab[2] = { -n_ab[1], n_ab[0] }; 

        // 相対速度の計算 (v_j - v_i)
        double dvx = other->velocity[0] - self->velocity[0]; 
        double dvy = other->velocity[1] - self->velocity[1];

        // 接線方向の速度差 (Delta v_ji^t)
        double tangential_vel_diff = dvx * t_ab[0] + dvy * t_ab[1];

        // 摩擦力の大きさ = kappa * 重なり * 接線速度差
        double friction_mag = self->kappa * overlap * tangential_vel_diff;

        friction_force[0] = friction_mag * t_ab[0];
        friction_force[1] = friction_mag * t_ab[1];
    }
    force[0] = (magnitude + body_force_mag) * n_ab[0] + friction_force[0]; 
    force[1] = (magnitude + body_force_mag) * n_ab[1] + friction_force[1];
}
void calculate_wall_force(HumanAgent *agent, double force[2], double width, double height) {
    force[0] = 0.0;
    force[1] = 0.0;

    // 出口の設定（中心 width/2, 半幅 0.8m）
    double exit_center_x = width / 2.0;
    double exit_half_width = 0.8;

    double d;           // 壁からエージェント中心までの最短距離
    double overlap;     // 重なり量 (radius - d)
    double social_mag;  // 心理的斥力（常に働く）
    double body_mag;    // 物理的反発（接触時のみ）

    // --- 左壁 (x=0) ---
    d = agent->pos[0];
    overlap = fmax(0.0, agent->radius - d);
    // 心理的斥力: A * exp((ri - d)/B)
    social_mag = agent->A * exp((agent->radius - d) / agent->B);
    // 物理的斥力: k * overlap (接触時のみ)
    body_mag = (overlap > 0.0) ? (agent->k * overlap) : 0.0;
    
    force[0] += (social_mag + body_mag); // 壁から離れる方向 (+x)
    if (overlap > 0.0) {
        // 摩擦力: κ * overlap * 速度の接線成分
        force[1] -= agent->kappa * overlap * agent->velocity[1];
    }

    // --- 右壁 (x=width) ---
    d = width - agent->pos[0];
    overlap = fmax(0.0, agent->radius - d);
    social_mag = agent->A * exp((agent->radius - d) / agent->B);
    body_mag = (overlap > 0.0) ? (agent->k * overlap) : 0.0;
    
    force[0] -= (social_mag + body_mag); // 壁から離れる方向 (-x)
    if (overlap > 0.0) {
        force[1] -= agent->kappa * overlap * agent->velocity[1];
    }

    // --- 下壁 (y=0) ---
    // 出口（xの範囲が中心±0.8m）の外側のみ壁の力を計算
    if (agent->pos[0] < (exit_center_x - exit_half_width) || 
        agent->pos[0] > (exit_center_x + exit_half_width)) {
        
        d = agent->pos[1];
        overlap = fmax(0.0, agent->radius - d);
        social_mag = agent->A * exp((agent->radius - d) / agent->B);
        body_mag = (overlap > 0.0) ? (agent->k * overlap) : 0.0;
        
        force[1] += (social_mag + body_mag); // 壁から離れる方向 (+y)
        if (overlap > 0.0) {
            force[0] -= agent->kappa * overlap * agent->velocity[0];
        }
    }

    // --- 上壁 (y=height) ---
    d = height - agent->pos[1];
    overlap = fmax(0.0, agent->radius - d);
    social_mag = agent->A * exp((agent->radius - d) / agent->B);
    body_mag = (overlap > 0.0) ? (agent->k * overlap) : 0.0;
    
    force[1] -= (social_mag + body_mag); // 壁から離れる方向 (-y)
    if (overlap > 0.0) {
        force[0] -= agent->kappa * overlap * agent->velocity[0];
    }
}



void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents, double dt, HumanSimulationModel *model) {
    double total_force[2] = {0.0, 0.0};

    double desire_force[2];
    calculate_desired_force(agent, desire_force);
    total_force[0] += desire_force[0];
    total_force[1] += desire_force[1];

    // 他エージェントとの相互作用力
    for (int i = 0; i < num_agents; i++) {
        if (agents[i].id != agent->id) {
            double social_force[2];
            calculate_social_force(agent, &agents[i], social_force);
            total_force[0] += social_force[0];
            total_force[1] += social_force[1];
        }
    }

    // 壁からの力
    double wall_force[2];
    calculate_wall_force(agent, wall_force, model->width, model->height);
    total_force[0] += wall_force[0];
    total_force[1] += wall_force[1];

    //障害物からの力
    double obstacle_total_force[2];
    calculate_obstacle_force(agent, 
                             model->obstacles, 
                             model->num_obstacles, 
                             obstacle_total_force);
                             
    total_force[0] += obstacle_total_force[0];
    total_force[1] += obstacle_total_force[1];

    // 力の上限
    double max_force = 2000.0;
    double fmag = sqrt(total_force[0]*total_force[0] + total_force[1]*total_force[1]);
    if (fmag > max_force) {
        total_force[0] = total_force[0] / fmag * max_force;
        total_force[1] = total_force[1] / fmag * max_force;
    }

    // 加速度計算
    double acceleration[2] = { total_force[0] / agent->mass, total_force[1] / agent->mass };
    double max_acc = 10.0; // 現実的な人間の加速度上限
    double acc_mag = sqrt(acceleration[0]*acceleration[0] + acceleration[1]*acceleration[1]);
    if (acc_mag > max_acc) {
        acceleration[0] = acceleration[0] / acc_mag * max_acc;
        acceleration[1] = acceleration[1] / acc_mag * max_acc;
    }

    // 速度更新
    agent->velocity[0] += acceleration[0] * dt;
    agent->velocity[1] += acceleration[1] * dt;

    // 速度上限
    double max_v = 10.0; // 人間の歩行速度上限
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