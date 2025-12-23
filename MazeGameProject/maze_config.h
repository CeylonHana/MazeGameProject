#pragma once
#ifndef MAZE_CONFIG_H
#define MAZE_CONFIG_H

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// 窗口配置
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;
#define WINDOW_TITLE  "MazeGame"

// 迷宫方块配置
constexpr int CELL_SIZE = 40;    // 每个方块的像素大小
constexpr int CELL_GAP = 1;     // 方块间距

// 颜色配置（路径叠加用）
#define COLOR_PATH    Color{255, 255, 0, 120}

// 地块类型枚举
typedef enum {
    CELL_END = -2,     // 终点
    CELL_START = -1,   // 起点
    CELL_GROUND = 0,   // 普通地面
    CELL_WALL = 1,     // 墙
    CELL_GRASS = 2,    // 草地
    CELL_LAVA = 3      // 熔岩
} CellType;

// 坐标结构体
typedef struct {
    int x;  // 列
    int y;  // 行
} Point;

// 迷宫数据结构体
typedef struct {
    int rows;          // 迷宫行数
    int cols;          // 迷宫列数
    CellType** grid;   // 迷宫网格数据
    Point start;       // 起点坐标
    Point end;         // 终点坐标
} Maze;

// 路径数据结构体
typedef struct {
    bool** visited;    // 访问标记
    Point** parent;    // 父节点（路径回溯）
    int** distance;    // 距离（Dijkstra用）
} PathData;

#endif // MAZE_CONFIG_H