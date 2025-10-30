// agent.c
// desired_force,social_force,wall_force と　update_agent
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
