#pragma once
#ifndef MAZE_UTILS_H
#define MAZE_UTILS_H

#include "maze_config.h"

// 校验坐标是否有效
bool IsPointValid(Maze* maze, int x, int y);
// 初始化路径数据
PathData* PathDataCreate(Maze* maze);
// 释放路径数据
void PathDataDestroy(Maze* maze, PathData* pd);
// 回溯路径（返回路径长度）
int PathBacktrack(Maze* maze, PathData* pd, Point* path, int maxPathLen);

#endif // MAZE_UTILS_H