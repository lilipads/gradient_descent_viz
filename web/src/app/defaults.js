import { FunctionName } from "../gd/surfaces.js";

export const DOMAIN = Object.freeze({
  minX: -2,
  maxX: 2,
  minZ: -2,
  maxZ: 2,
});

// Matches the Qt app’s default “within view” starting point in `PlotArea::initializeSurface`.
export const DEFAULT_START = Object.freeze({
  x: (7 * DOMAIN.maxX + DOMAIN.minX) / 8,
  z: (7 * DOMAIN.maxZ + DOMAIN.minZ) / 8,
});

export const DEFAULT_SURFACE = FunctionName.local_minimum;

