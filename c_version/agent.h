#ifndef AGENT_H
#define AGENT_H

typedef struct HumanSimulationModel HumanSimulationModel;

#include "obstacle.h"


typedef struct HumanAgent {
    int id;
    double pos[2];
    double target_pos[2];
    double radius;
    double mass;
    double velocity[2];
    double desired_speed;
    double relaxation_time;
    double A;
    double B;
    double k;
    double kappa;
} HumanAgent;

// 関数宣言
void init_agent(HumanAgent *agent, int id, double x, double y, double tx, double ty, HumanSimulationModel *model);
void calculate_desired_force(HumanAgent *agent, double force[2]);
void calculate_social_force(HumanAgent *self, HumanAgent *other, double force[2]);
void calculate_wall_force(HumanAgent *agent, double force[2],double width, double height);
void update_agent(HumanAgent *agent, HumanAgent *agents, int num_agents, double dt, HumanSimulationModel *model);

#endif
