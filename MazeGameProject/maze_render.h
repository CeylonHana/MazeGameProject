#pragma once
#ifndef MAZE_RENDER_H
#define MAZE_RENDER_H

#include "maze_config.h"
#include "maze_textures.h"

// 获取方块像素坐标
Vector2 GetCellPixelPos(Maze* maze, int x, int y);
// 绘制单个地块（纹理版）
void DrawCellWithTexture(Maze* maze, MazeTextures* tex, int x, int y, CellType type);
// 绘制迷宫网格
void DrawMazeGridWithTexture(Maze* maze, MazeTextures* tex);
// 绘制路径（叠加在纹理上）
void DrawPathOnTexture(Maze* maze, Point* path, int pathLen);

#endif // MAZE_RENDER_H