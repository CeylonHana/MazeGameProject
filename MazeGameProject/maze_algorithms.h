#pragma once
#ifndef MAZE_ALGORITHMS_H
#define MAZE_ALGORITHMS_H

#include "maze_config.h"
#include "maze_utils.h"

// DFSÀ„∑®
bool DFS(Maze* maze, PathData* pd, int x, int y);
// BFSÀ„∑®
bool BFS(Maze* maze, PathData* pd);
// DijkstraÀ„∑®
bool Dijkstra(Maze* maze, PathData* pd);

#endif // MAZE_ALGORITHMS_H