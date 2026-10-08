#pragma once
using gpio_num_t=int;
extern int driveLevel;
inline int gpio_set_level(gpio_num_t,int level) {driveLevel=level;return 0;}
