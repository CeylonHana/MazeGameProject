#pragma once
#ifndef MAZE_TEXTURES_H
#define MAZE_TEXTURES_H

#include "maze_config.h"

// 纹理资源结构体
typedef struct {
    Texture2D start;    // 起点纹理
    Texture2D end;      // 终点纹理
    Texture2D floor;    // 普通地面纹理
    Texture2D wall;     // 墙纹理
    Texture2D grass;    // 草地纹理
    Texture2D lava;     // 熔岩纹理
    Texture2D character;// 角色纹理
    Texture2D slime;    // 敌人纹理
} MazeTextures;

// 加载所有纹理
MazeTextures* TexturesLoad();
// 释放纹理资源
void TexturesUnload(MazeTextures* tex);

#endif // MAZE_TEXTURES_H