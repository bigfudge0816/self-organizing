#include "Tree.hpp"

#include <iostream>

static constexpr float PipeModelExponent = 2.5f;
static constexpr float PipeModelLeafValue = 1.0e-8f;

Tree::Tree(Environment &environment, Point seedlingPosition) : environment(environment) {
  const auto end = seedlingPosition.translate(0.0f, Environment::MetamerLength, 0.0f);
  root = std::make_unique<Metamer>(environment, seedlingPosition, end);
}

void Tree::performGrowthIteration() {
  // 1. Calculate local environment of all tree buds.
  environment.markerSet.resetAllocations();
  allocateMarkers(root);
  // 2. Determine the fate of each bud (the extended Borchert-Honda model).
  propagateLightBasipetally(root);
  root->growthResource = Environment::BorchertHondaAlpha * root->light;
  propagateGrowthAcropetally(root);
  // 3. Append new shoots.
  performGrowthIteration(root);
  // 4. Shed branches (not implemented).
  // 5. Update internode width for all internodes.
  updateInternodeWidths(root);
}

void Tree::allocateMarkers(std::unique_ptr<Metamer> &metamer) {
  if (!metamer) {
    return;
  }
  const auto theta = Environment::PerceptionAngle;
  const auto r = Environment::PerceptionRadius;
  if (!metamer->axillary) {
    // TODO: change this point and direction somehow. This has to use the phyllotaxis thing.
    const auto direction = Vector(metamer->beginning, metamer->end);
    environment.markerSet.updateAllocatedInCone(metamer->axillaryId, metamer->end, direction, theta, r);
  } else {
    allocateMarkers(metamer->axillary);
  }
  if (!metamer->terminal) {
    const auto direction = Vector(metamer->beginning, metamer->end);
    environment.markerSet.updateAllocatedInCone(metamer->terminalId, metamer->end, direction, theta, r);
  } else {
    allocateMarkers(metamer->terminal);
  }
}

void Tree::propagateLightBasipetally(std::unique_ptr<Metamer> &metamer) {
  if (!metamer) {
    return;
  }
  propagateLightBasipetally(metamer->axillary);
  propagateLightBasipetally(metamer->terminal);
  const auto theta = Environment::PerceptionAngle;
  const auto r = Environment::PerceptionRadius;
  metamer->light = 0.0f;
  if (!metamer->axillary) {
    // TODO: change this point and direction somehow. This has to use the phyllotaxis thing.
    const auto direction = Vector(metamer->beginning, metamer->end);
    const auto budId = metamer->axillaryId;
    metamer->light += environment.markerSet.getAllocatedInCone(budId, metamer->end, direction, theta, r).q;
  } else {
    metamer->light += metamer->axillary->light;
  }
  if (!metamer->terminal) {
    const auto direction = Vector(metamer->beginning, metamer->end);
    const auto budId = metamer->terminalId;
    metamer->light += environment.markerSet.getAllocatedInCone(budId, metamer->end, direction, theta, r).q;
  } else {
    metamer->light += metamer->terminal->light;
  }
}

void Tree::propagateGrowthAcropetally(std::unique_ptr<Metamer> &metamer) {
  if (!metamer) {
    return;
  }
  if (!metamer->axillary && !metamer->terminal) {
    return;
  }
  const auto qM = metamer->terminal ? metamer->terminal->light : 0.0f;
  const auto qL = metamer->axillary ? metamer->axillary->light : 0.0f;
  // Dodge divisions by zero if these branches have not acquired any light.
  if (qM + qL == 0.0f) {
    return;
  }
  const auto v = metamer->growthResource;
  const auto lambda = Environment::BorchertHondaLambda;
  const auto denominator = lambda * qM + (1.0f - lambda * qL);
  const auto vM = v * (lambda * qM) / denominator;
  const auto vL = v * ((1.0f - lambda) * qL) / denominator;
  if (metamer->axillary) {
    metamer->axillary->growthResource = vL;
    metamer->axillaryGrowthResource = 0.0f;
  } else {
    metamer->axillaryGrowthResource = vL;
  }
  if (metamer->terminal) {
    metamer->terminal->growthResource = vM;
    metamer->terminalGrowthResource = 0.0f;
  } else {
    metamer->terminalGrowthResource = vM;
  }
  metamer->growthResource = 0.0f;
}

std::unique_ptr<Metamer> Tree::attemptGrowth(BudId budId, Point origin, Vector direction) {
  const auto theta = Environment::PerceptionAngle;
  const auto r = Environment::PerceptionRadius;
  const auto analysis = environment.markerSet.getAllocatedInCone(budId, origin, direction, theta, r);
  if (analysis.q == 1.0f) {
    const auto shootV = analysis.v.scale(Environment::MetamerLength);
    const auto shootEnd = origin.translate(shootV.x, shootV.y, shootV.z);
    environment.markerSet.removeMarkersInSphere(shootEnd, Environment::OccupancyRadius);
    return std::make_unique<Metamer>(environment, origin, shootEnd);
  }
  return nullptr;
}

void Tree::performGrowthIteration(std::unique_ptr<Metamer> &metamer) {
  if (!metamer) {
    return;
  }
  if (!metamer->axillary) {
    // TODO: change this direction somehow.
    const auto direction = Vector(metamer->beginning, metamer->end);
    metamer->axillary = attemptGrowth(metamer->axillaryId, metamer->end, direction);
  } else {
    performGrowthIteration(metamer->axillary);
  }
  if (!metamer->terminal) {
    const auto direction = Vector(metamer->beginning, metamer->end);
    metamer->terminal = attemptGrowth(metamer->terminalId, metamer->end, direction);
  } else {
    performGrowthIteration(metamer->terminal);
  }
}

void Tree::updateInternodeWidths(std::unique_ptr<Metamer> &metamer) {
  if (!metamer) {
    return;
  }
  updateInternodeWidths(metamer->axillary);
  // Assume all branches have a leaf.
  auto total = PipeModelLeafValue;
  if (metamer->axillary) {
    total += std::pow(metamer->axillary->width, PipeModelExponent);
  }
  updateInternodeWidths(metamer->terminal);
  if (metamer->terminal) {
    total += std::pow(metamer->terminal->width, PipeModelExponent);
  }
  metamer->width = std::pow(total, 1.0f / PipeModelExponent);
}

U64 Tree::countMetamers() const {
  if (!root) {
    return 0;
  }
  return root->countMetamers();
}
