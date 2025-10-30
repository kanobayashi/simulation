#ifndef OBSTACLE_H
#define OBSTACLE_H
//#include "agent.h"  // HumanAgent を使うために必要


typedef struct HumanAgent HumanAgent;

typedef enum {
    OBSTACLE_CIRCLE,
    OBSTACLE_RECTANGLE,
    OBSTACLE_SEGMENT
} ObstacleType;

typedef struct {
    ObstacleType type;
    double pos[2];
    double radius;
    double width;
    double height;
    double p1[2];
    double p2[2];
    double thickness;
} Obstacle;

#include "agent.h"  // HumanAgent を使うために必要

void calculate_obstacle_force(HumanAgent *agent, Obstacle *obstacles, int num_obstacles, double force[2]);

#endif