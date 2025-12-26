// MazeGameProject.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "maze_config.h"
#include "maze_textures.h"
#include "maze_data.h"
#include "maze_utils.h"
#include "maze_algorithms.h"
#include "maze_render.h"

int main() {
    // 初始化Raylib窗口
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60); // 设置帧率

    // 加载纹理资源
    MazeTextures* tex = TexturesLoad();
    if (tex == NULL) {
        TraceLog(LOG_ERROR, "纹理加载失败，程序退出");
        CloseWindow();
        return 1;
    }

    // 加载迷宫文件（请确保maze20x20.txt在同目录）
    Maze* maze = MazeCreate();
    if (maze == NULL) {
        TraceLog(LOG_ERROR, "迷宫初始化失败，程序退出");
        TexturesUnload(tex);
        CloseWindow();
        return 1;
    }
    if (!MazeLoadFromFile(maze, "maze20x20.txt")) {
        TraceLog(LOG_ERROR, "迷宫文件加载失败，程序退出");
        TexturesUnload(tex);
        MazeDestroy(maze);
        CloseWindow();
        return 1;
    }

    // 路径相关变量
    PathData* pd = NULL;
    Point path[400]; // 存储路径（最大400个节点）
    int pathLen = 0;
    int algoType = 0; // 0=无路径, 1=DFS, 2=BFS, 3=Dijkstra

    // 主循环
    while (!WindowShouldClose()) {
        // 按键交互逻辑
        if (IsKeyPressed(KEY_ONE)) {
            TraceLog(LOG_INFO, "执行DFS算法");
            // 释放旧路径数据
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pd = PathDataCreate(maze);
            if (pd == NULL) {
                TraceLog(LOG_ERROR, "路径数据初始化失败");
                pathLen = 0;
                algoType = 0;
                continue;
            }
            // 执行DFS（从起点开始）
            if (DFS(maze, pd, maze->start.x, maze->start.y)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            }
            else {
                pathLen = 0;
                TraceLog(LOG_WARNING, "DFS未找到有效路径");
            }
            algoType = 1;
        }

        if (IsKeyPressed(KEY_TWO)) {
            TraceLog(LOG_INFO, "执行BFS算法");
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pd = PathDataCreate(maze);
            if (pd == NULL) {
                TraceLog(LOG_ERROR, "路径数据初始化失败");
                pathLen = 0;
                algoType = 0;
                continue;
            }
            // 执行BFS
            if (BFS(maze, pd)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            }
            else {
                pathLen = 0;
                TraceLog(LOG_WARNING, "BFS未找到有效路径");
            }
            algoType = 2;
        }

        if (IsKeyPressed(KEY_THREE)) {
            TraceLog(LOG_INFO, "执行Dijkstra算法");
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pd = PathDataCreate(maze);
            if (pd == NULL) {
                TraceLog(LOG_ERROR, "路径数据初始化失败");
                pathLen = 0;
                algoType = 0;
                continue;
            }
            // 执行Dijkstra
            if (Dijkstra(maze, pd)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            }
            else {
                pathLen = 0;
                TraceLog(LOG_WARNING, "Dijkstra未找到有效路径");
            }
            algoType = 3;
        }

        if (IsKeyPressed(KEY_ZERO)) {
            TraceLog(LOG_INFO, "清空路径");
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pathLen = 0;
            algoType = 0;
        }

        // 渲染逻辑
        BeginDrawing();
        ClearBackground(RAYWHITE); // 清空背景

        // 绘制纹理版迷宫
        DrawMazeGridWithTexture(maze, tex);

        // 绘制路径（如果有）
        if (pathLen > 0) {
            DrawPathOnTexture(maze, path, pathLen);
        }

        // 绘制提示文字
        DrawText("MazeGame", 20, 20, 24, BLACK);
        DrawText("ButtonInstruction-", 20, 60, 20, BLACK);
        DrawText("1 - DFS_path   2 - BFS_path   3 - Dijkstra_path   0 - ClearPath", 20, 90, 20, BLACK);

        // 显示当前算法
        const char* algoName[] = { "NONE", "DFS", "BFS", "Dijkstra" };
        DrawText(TextFormat("CurrentAlgorithms-%s | PathLength-%d", algoName[algoType], pathLen), 20, 120, 20, RED);

        EndDrawing();
    }

    // 释放所有资源
    if (pd != NULL) PathDataDestroy(maze, pd);
    MazeDestroy(maze);
    TexturesUnload(tex);
    CloseWindow();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
