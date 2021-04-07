#ifndef CONTROL_TYPES_H
#define CONTROL_TYPES_H

typedef struct {
  int ratio;
  int threshold;
  int attack;
  int release;
  int makeup;
  int gain;
} db_controls_t;

#endif // CONTROL_TYPES_H
