#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

extern float tds_threshold;

void set_tds_threshold(float value);
float get_tds_threshold(void);

#endif /* SETTINGS_H */