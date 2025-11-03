#ifndef OBSTACLE_H
#define OBSTACLE_H

struct HumanAgent; // 前方宣言でOK（型だけ必要）

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
void calculate_obstacle_force(struct HumanAgent *agent,
                              Obstacle *obstacles,
                              int num_obstacles,
                              double force[2]);

#endif
