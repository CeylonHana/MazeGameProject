#pragma once
#ifndef MAZE_DATA_H
#define MAZE_DATA_H

#include "maze_config.h"

// 初始化迷宫
Maze* MazeCreate();
// 释放迷宫内存
void MazeDestroy(Maze* maze);
// 从文件加载迷宫
bool MazeLoadFromFile(Maze* maze, const char* filename);

#endif // MAZE_DATA_H