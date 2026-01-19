export const FunctionName = Object.freeze({
  local_minimum: "local_minimum",
  global_minimum: "global_minimum",
  saddle_point: "saddle_point",
  ecliptic_bowl: "ecliptic_bowl",
  hills: "hills",
  plateau: "plateau",
});

// Ported from the Qt/C++ implementation (`src/gradient_descent.cpp`, GradientDescent::f).
export function f(x, z, functionName) {
  switch (functionName) {
    case FunctionName.local_minimum: {
      z *= 1.4;
      return (
        -2 * Math.exp(-(((x - 1) * (x - 1) + z * z) / 0.2)) -
        6 * Math.exp(-(((x + 1) * (x + 1) + z * z) / 0.2)) +
        x * x +
        z * z
      );
    }
    case FunctionName.global_minimum: {
      return x * x + z * z;
    }
    case FunctionName.saddle_point: {
      return Math.sin(x) + z * z;
    }
    case FunctionName.ecliptic_bowl: {
      x /= 2;
      z /= 2;
      return -Math.exp(-(x * x + 5 * z * z)) + x * x + 0.5 * z * z;
    }
    case FunctionName.hills: {
      z *= 1.4;
      return (
        2 * Math.exp(-(((x - 1) * (x - 1) + z * z) / 0.2)) +
        6 * Math.exp(-(((x + 1) * (x + 1) + z * z) / 0.2)) -
        2 * Math.exp(-(((x - 1) * (x - 1) + (z + 1) * (z + 1)) / 0.2)) +
        x * x +
        z * z
      );
    }
    case FunctionName.plateau: {
      x *= 10;
      z *= 10;
      const r = Math.sqrt(z * z + x * x) + 0.01;
      return -Math.sin(r) / r + 0.01 * r * r;
    }
    default:
      return 0;
  }
}

