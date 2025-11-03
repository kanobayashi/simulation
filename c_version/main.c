#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "model.h"

int main() {
    // 乱数初期化
    srand((unsigned int)time(NULL));
    printf("=== シミュレーション開始 ===\n");
    printf("NUM_AGENTS = %d, WIDTH = %.2f, HEIGHT = %.2f\n", NUM_AGENTS, WIDTH, HEIGHT);

    HumanSimulationModel model;
    init_model(&model, NUM_AGENTS, WIDTH, HEIGHT);

    // 初期状態の確認
    printf("=== 初期化後のエージェント位置 ===\n");
    for (int i = 0; i < model.num_agents; i++) {
        printf("Agent %d: x = %.2f, y = %.2f -> target_x = %.2f, target_y = %.2f\n",
               model.agents[i].id,
               model.agents[i].pos[0],
               model.agents[i].pos[1],
               model.agents[i].target_pos[0],
               model.agents[i].target_pos[1]);
    }

    FILE *fp = fopen("positions.csv", "w");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }
    fprintf(fp, "step,id,x,y\n");

    for (int step = 0; step < SIMULATION_STEPS; step++) {
        step_model(&model);

        // デバッグ用: 各ステップの先頭エージェントだけ表示
        if (step % 10 == 0 || step == SIMULATION_STEPS - 1) {
            printf("=== Step %d ===\n", step);
            for (int i = 0; i < model.num_agents; i++) {
                printf("Agent %d: x = %.2f, y = %.2f\n",
                       model.agents[i].id,
                       model.agents[i].pos[0],
                       model.agents[i].pos[1]);
            }
        }

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

    printf("=== シミュレーション終了 ===\n");
    printf("positions.csv に出力しました\n");

    return 0;
}
// gcc main.c model.c agent.c obstacle.c -o sim.exe -lm
//.\sim.exe