#include "Marker.hpp"

void Marker::resetAllocation() {
  allocationId = 0;
  distanceToAllocated = std::numeric_limits<F32>::infinity();
}
