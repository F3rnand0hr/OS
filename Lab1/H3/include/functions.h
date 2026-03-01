#ifndef INCLUDE_FUNCTIONS_H
#define INCLUDE_FUNCTIONS_H

/**
 * @brief Represents a point in 2D space.
 */
typedef struct {
  float x;
  float y;
} Point;

/**
 * @brief Calculates the Euclidean distance between two points.
 * @param[in] p1 The first point (x, y).
 * @param[in] p2 The second point (x, y).
 * @return The distance between p1 and p2.
 */
float CalculateDistance(Point p1, Point p2);

#endif