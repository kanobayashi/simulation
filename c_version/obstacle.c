#include "agent.h"      // HumanAgent の中身を使うために先に読み込む
#include "obstacle.h"
#include <math.h>

static void calc_circle_force(HumanAgent *agent, Obstacle *obs, double force[2]) {
    double dx = agent->pos[0] - obs->pos[0];
    double dy = agent->pos[1] - obs->pos[1];
    double distance = sqrt(dx * dx + dy * dy);

    if (distance < obs->radius + agent->radius && distance > 0.0) {
        double strength = 8.0 * exp(-distance);
        force[0] += strength * (dx / distance);
        force[1] += strength * (dy / distance);
    }
}

static void calc_rect_force(HumanAgent *agent, Obstacle *obs, double force[2]) {
    // 矩形中心からの距離ベース（簡易版）
    double dx = fmax(obs->pos[0] - obs->width/2.0 - agent->pos[0], 0.0);
    dx = fmax(dx, agent->pos[0] - (obs->pos[0] + obs->width/2.0));
    double dy = fmax(obs->pos[1] - obs->height/2.0 - agent->pos[1], 0.0);
    dy = fmax(dy, agent->pos[1] - (obs->pos[1] + obs->height/2.0));

    double dist = sqrt(dx*dx + dy*dy);

    if (dist < agent->radius) {
        double strength = 10.0 * exp(-dist);
        double dirx = agent->pos[0] - obs->pos[0];
        double diry = agent->pos[1] - obs->pos[1];
        double len = sqrt(dirx*dirx + diry*diry) + 1e-9;
        force[0] += strength * (dirx / len);
        force[1] += strength * (diry / len);
    }
}

static void calc_segment_force(HumanAgent *agent, Obstacle *obs, double force[2]) {
    // ベクトル p1→p2
    double vx = obs->p2[0] - obs->p1[0];
    double vy = obs->p2[1] - obs->p1[1];
    double len2 = vx*vx + vy*vy;

    // エージェント位置 → p1 まで
    double wx = agent->pos[0] - obs->p1[0];
    double wy = agent->pos[1] - obs->p1[1];

    // t = 射影係数 (0<=t<=1 なら線分上)
    double t = (wx*vx + wy*vy) / (len2 + 1e-9);
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    // 最近傍点
    double cx = obs->p1[0] + t*vx;
    double cy = obs->p1[1] + t*vy;

    // エージェントから最近傍点へのベクトル
    double dx = agent->pos[0] - cx;
    double dy = agent->pos[1] - cy;
    double dist = sqrt(dx*dx + dy*dy);

    // 帯の厚み込みで判定
    double effective_dist = dist - obs->thickness;

    if (effective_dist < agent->radius && effective_dist > -obs->thickness) {
        double strength = 15.0 * exp(-fmax(0.0, effective_dist));
        if (dist > 1e-9) {
            force[0] += strength * (dx / dist);
            force[1] += strength * (dy / dist);
        }
    }
}

void calculate_obstacle_force(HumanAgent *agent,
                              Obstacle *obstacles,
                              int num_obstacles,
                              double force[2]) {
    force[0] = 0.0;
    force[1] = 0.0;

    for (int i = 0; i < num_obstacles; i++) {
        Obstacle *obs = &obstacles[i];
        switch (obs->type) {
            case OBSTACLE_CIRCLE:
                calc_circle_force(agent, obs, force);
                break;
            case OBSTACLE_RECTANGLE:
                calc_rect_force(agent, obs, force);
                break;
            case OBSTACLE_SEGMENT:
                calc_segment_force(agent, obs, force);
                break;
        }
    }
}

