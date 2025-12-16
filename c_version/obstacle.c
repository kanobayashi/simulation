// obstacle.c
#include "obstacle.h"
#include "agent.h" 
#include <math.h>


static void calc_circle_force(HumanAgent *agent, Obstacle *obs, double force[2]) {
    double dx = agent->pos[0] - obs->pos[0];
    double dy = agent->pos[1] - obs->pos[1];
    double distance = sqrt(dx * dx + dy * dy);

    // ゼロ除算を防止しつつ、相互作用範囲内か判定
    if (distance > 1e-8) {
        // 半径の合計と重なりの計算
        double r_iw = obs->radius + agent->radius;
        double overlap = fmax(0.0, r_iw - distance); 

        // 法線ベクトル (n_iw)
        double nx = dx / distance;
        double ny = dy / distance;

        // 1. 心理的斥力の計算 (A * exp) 
        // 論文設定値 A=2000, B=0.08 を想定 [cite: 462]
        double social_mag = agent->A * exp((agent->radius - distance) / agent->B);

        // 2. 物理的な力の計算 (接触時 overlap > 0 のみ) 
        double body_force_mag = 0.0;
        double friction_force[2] = {0.0, 0.0};

        if (overlap > 0.0) {
            // A. 法線方向の反発力 (k * g) 
            body_force_mag = agent->k * overlap;

            // B. 接線方向の摩擦力 (kappa * g * (vi * tiw) * tiw) 
            // 接線ベクトル t_iw (法線を90度回転)
            double tx = -ny;
            double ty = nx;

            // 歩行者の速度と接線ベクトルの内積 (接線方向の速度成分)
            double v_t = agent->velocity[0] * tx + agent->velocity[1] * ty;

            // 摩擦力強度の計算 (論文の kappa 項) 
            double friction_mag = agent->kappa * overlap * v_t;

            // 摩擦力は運動を妨げる方向に作用
            friction_force[0] = -friction_mag * tx;
            friction_force[1] = -friction_mag * ty;
        }

        // 合計力を成分(x, y)ごとに加算
        force[0] += (social_mag + body_force_mag) * nx + friction_force[0];
        force[1] += (social_mag + body_force_mag) * ny + friction_force[1];
    }
}

static void calc_rect_force(HumanAgent *agent, Obstacle *obs, double force[2]) {
    // 矩形の境界を計算 (中心座標 obs->pos と サイズ obs->width, obs->height から)
    double x_min = obs->pos[0] - obs->width / 2.0;
    double x_max = obs->pos[0] + obs->width / 2.0;
    double y_min = obs->pos[1] - obs->height / 2.0;
    double y_max = obs->pos[1] + obs->height / 2.0;

    // 矩形上の最近接点 (px, py) を求める
    double px = fmax(x_min, fmin(agent->pos[0], x_max));
    double py = fmax(y_min, fmin(agent->pos[1], y_max));

    // エージェント中心から最近接点へのベクトルと距離
    double dx = agent->pos[0] - px;
    double dy = agent->pos[1] - py;
    double distance = sqrt(dx * dx + dy * dy);

    // 距離が極めて近い場合のゼロ除算防止
    if (distance > 1e-8) {
        // 重なり量 g(ri - diw) の計算 
        double overlap = fmax(0.0, agent->radius - distance);

        // 法線ベクトル (n_iw): 障害物からエージェントへ向かう向き
        double nx = dx / distance;
        double ny = dy / distance;

        // 1. 心理的斥力の計算 (A * exp) 
        // 距離 diw に基づいて計算
        double social_mag = agent->A * exp((agent->radius - distance) / agent->B);

        // 2. 物理的な力の計算 (接触時 overlap > 0 のみ)
        double body_force_mag = 0.0;
        double friction_force[2] = {0.0, 0.0};

        if (overlap > 0.0) {
            // A. 法線方向の反発力 (k * overlap) 
            body_force_mag = agent->k * overlap;

            // B. 接線方向の摩擦力 (kappa * overlap * vt) 
            // 接線ベクトル t_iw の作成 (法線を90度回転)
            double tx = -ny;
            double ty = nx;

            // 接線方向の速度成分 (vi * tiw)
            double v_t = agent->velocity[0] * tx + agent->velocity[1] * ty;

            // 摩擦力の大きさ計算 
            double friction_mag = agent->kappa * overlap * v_t;

            // 摩擦力は速度と逆方向に作用
            friction_force[0] = -friction_mag * tx;
            friction_force[1] = -friction_mag * ty;
        }

        // 合計力を成分ごとに加算 [cite: 412, 457]
        // (心理斥力 + 物理反発) * 法線ベクトル + 摩擦力ベクトル
        force[0] += (social_mag + body_force_mag) * nx + friction_force[0];
        force[1] += (social_mag + body_force_mag) * ny + friction_force[1];
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

