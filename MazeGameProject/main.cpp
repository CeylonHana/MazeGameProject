#include "maze_config.h"
#include "maze_textures.h"
#include "maze_data.h"
#include "maze_utils.h"
#include "maze_algorithms.h"
#include "maze_render.h"
#include "player.h"
#include "ui.h"
#include "maze_generator.h"
#include "enemy.h"

int main() {
    // 初始化窗口
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    // 加载纹理
    MazeTextures* tex = TexturesLoad();
    if (!tex) {
        TraceLog(LOG_ERROR, "纹理加载失败");
        CloseWindow();
        return 1;
    }

    // 生成20x20随机迷宫
    Maze* maze = GenerateRandomMaze(20, 20);
    if (!maze) {
        TraceLog(LOG_ERROR, "随机迷宫生成失败");
        TexturesUnload(tex);
        CloseWindow();
        return 1;
    }

    // 初始化玩家
    Player* player = PlayerInit(maze, tex);
    if (!player) {
        TraceLog(LOG_ERROR, "玩家初始化失败");
        MazeDestroy(maze);
        TexturesUnload(tex);
        CloseWindow();
        return 1;
    }

    // 初始化敌人（随机生成在空地）
    Enemy* enemy = EnemyInit(maze, tex);
    if (!enemy) {
        TraceLog(LOG_ERROR, "敌人初始化失败");
        PlayerFree(player);
        MazeDestroy(maze);
        TexturesUnload(tex);
        CloseWindow();
        return 1;
    }

    // 路径变量
    PathData* pd = NULL;
    Point path[400];
    int path_len = 0;
    int algo_type = 0;
    bool game_start = false;

    // 主循环
    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        // 游戏开始触发（空格）
        if (IsKeyPressed(KEY_SPACE) && !game_start && !player->is_dead && !player->is_win) {
            game_start = true;
        }

        // 玩家和敌人更新逻辑
        if (game_start) {
            // 更新玩家
            PlayerUpdate(player, maze, tex, delta_time);

            // 计算玩家当前格子位置
            Point current_player_grid;
            int total_grid_width = maze->cols * (CELL_SIZE + CELL_GAP);
            int total_grid_height = maze->rows * (CELL_SIZE + CELL_GAP);
            float maze_offset_x = (WINDOW_WIDTH - total_grid_width) / 2.0F;
            float maze_offset_y = (WINDOW_HEIGHT - total_grid_height) / 2.0F;
            float curr_rel_x = player->pos.x - maze_offset_x;
            float curr_rel_y = player->pos.y - maze_offset_y;

            current_player_grid.x = (int)(curr_rel_x / (CELL_SIZE + CELL_GAP));
            current_player_grid.y = (int)(curr_rel_y / (CELL_SIZE + CELL_GAP));
            // 边界保护
            current_player_grid.x = (current_player_grid.x < 0) ? 0 : current_player_grid.x;
            current_player_grid.x = (current_player_grid.x >= maze->cols) ? maze->cols - 1 : current_player_grid.x;
            current_player_grid.y = (current_player_grid.y < 0) ? 0 : current_player_grid.y;
            current_player_grid.y = (current_player_grid.y >= maze->rows) ? maze->rows - 1 : current_player_grid.y;

            EnemyUpdate(enemy, maze, tex, player, current_player_grid, delta_time);
        }

        // 算法按键逻辑
        if (IsKeyPressed(KEY_ONE)) {
            if (pd) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            path_len = (pd && DFS(maze, pd, maze->start.x, maze->start.y)) ? PathBacktrack(maze, pd, path, 400) : 0;
            algo_type = 1;
        }
        if (IsKeyPressed(KEY_TWO)) {
            if (pd) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            path_len = (pd && BFS(maze, pd)) ? PathBacktrack(maze, pd, path, 400) : 0;
            algo_type = 2;
        }
        if (IsKeyPressed(KEY_THREE)) {
            if (pd) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            path_len = (pd && Dijkstra(maze, pd)) ? PathBacktrack(maze, pd, path, 400) : 0;
            algo_type = 3;
        }
        if (IsKeyPressed(KEY_FOUR)) {
            if (pd) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            path_len = (pd && DijkstraWithOneLava(maze, pd)) ? PathBacktrack(maze, pd, path, 400) : 0;
            algo_type = 4;
        }
        if (IsKeyPressed(KEY_ZERO)) {
            if (pd) PathDataDestroy(maze, pd);
            pd = NULL;
            path_len = 0;
            algo_type = 0;
        }

        // 渲染逻辑
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 绘制迷宫
        DrawMazeGridWithTexture(maze, tex);

        // 绘制路径
        if (path_len > 0) {
            DrawPathOnTexture(maze, path, path_len);
        }

        // 绘制敌人和玩家
        EnemyDraw(enemy, tex);
        PlayerDraw(player, tex);

        // 绘制原有UI
        DrawHPUI(player, tex);
        DrawGameState(player, game_start);
        DrawAlgoHint(algo_type, path_len);

        EndDrawing();
    }

    // 资源释放
    if (pd) PathDataDestroy(maze, pd);
    EnemyFree(enemy);
    PlayerFree(player);
    MazeDestroy(maze);
    TexturesUnload(tex);
    CloseWindow();

    return 0;
}
