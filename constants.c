#include "constants.h"

const ColorCodes COLORS = {
    .green = "\033[1;32m",
    .reset = "\033[0m",
    .html_green_start = "<span style=\"color: green\">",
    .html_green_end = "</span>"
};

const DirectionVector DIRECTION_VECTORS[DIRECTIONS_COUNT] = {
    {-1,  0, "UP"},
    { 1,  0, "DOWN"},
    { 0, -1, "LEFT"},
    { 0,  1, "RIGHT"},
    {-1, -1, "UP_LEFT"},
    {-1,  1, "UP_RIGHT"},
    { 1, -1, "DOWN_LEFT"},
    { 1,  1, "DOWN_RIGHT"}
};
