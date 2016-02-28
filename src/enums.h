/*
 * Just some handy constants used throughout vitunes.
 * Note that no typing is used at all.  These are just
 * constants.
 */
#ifndef ENUMS_H
#define ENUMS_H

typedef enum {
   UP, DOWN,
   LEFT, RIGHT,
   BACKWARDS, FORWARDS,
   SAME, OPPOSITE
} Direction;

typedef enum {
   SECONDS, MINUTES,
   NUMBER, PERCENT
} Scale;

typedef enum {
   SINGLE, HALF, WHOLE
} Amount;

typedef enum {
   TOP, MIDDLE, BOTTOM,
   BEFORE, AFTER
} Placement;

#endif
