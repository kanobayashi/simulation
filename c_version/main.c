#include <stdio.h>
#include <stdlib.h>  // ← 追加
#include <time.h> 
#include "model.h"

#define NUM_AGENTS 10
#define WIDTH 38.0
#define HEIGHT 25.0
#define SIMULATION_STEPS 1000

int main() {
    printf("NUM_AGENTS = %d\n", NUM_AGENTS);
    HumanSimulationModel model;
    init_model(&model, NUM_AGENTS, WIDTH, HEIGHT);

    // エージェントの初期化（1回だけ）
    for (int i = 0; i < NUM_AGENTS; i++) {
        double x = ((double)rand() / RAND_MAX) * WIDTH;
        double y = ((double)rand() / RAND_MAX) * HEIGHT;
        double tx = 18.2 + ((double)rand() / RAND_MAX) * 1.6;
        double ty = 0.0;
        init_agent(&model.agents[i], i, x, y, tx, ty);
    }

    FILE *fp = fopen("positions.csv", "w");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    fprintf(fp, "step,id,x,y\n");

    for (int step = 0; step < SIMULATION_STEPS; step++) {
        step_model(&model);
        for (int i = 0; i < model.num_agents; i++) {
            fprintf(fp, "%d,%d,%.2f,%.2f\n",
                    step,
                    model.agents[i].id,
                    model.agents[i].pos[0],
                    model.agents[i].pos[1]);
        }
    }

    fclose(fp);
    free_model(&model);
    return 0;
}

// gcc main.c model.c agent.c obstacle.c -o sim -lm