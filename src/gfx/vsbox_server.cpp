#include "vsbox.h"
#include "xml_support.h"

Box::Box(const Vector &corner1, const Vector &corner2) : corner_min(corner1), corner_max(corner2) {
}

void Box::ProcessDrawQueue(int) {
}
