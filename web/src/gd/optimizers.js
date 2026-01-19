import { f } from "./surfaces.js";

const DEFAULTS = Object.freeze({
  divisionEpsilon: 1e-12, // matches C++ kDivisionEpsilon
  // C++ uses 1e-12; in JS this is often too small/noisy for finite differences.
  finiteDiffEpsilon: 1e-6,
  convergenceEpsilon: 1e-2, // matches C++ kConvergenceEpsilon
});

export class Point {
  constructor(x = 0, z = 0) {
    this.x = x;
    this.z = z;
  }
  clone() {
    return new Point(this.x, this.z);
  }
}

export class GradientDescentBase {
  constructor({ functionName, learningRate = 0.001 } = {}) {
    this.functionName = functionName;
    this.learning_rate = learningRate;

    this.p = new Point(0, 0);
    this.starting_p = new Point(0, 0);
    this.m_delta = new Point(0, 0);
    this.grad = new Point(0, 0);
    this.is_converged = false;

    this.divisionEpsilon = DEFAULTS.divisionEpsilon;
    this.finiteDiffEpsilon = DEFAULTS.finiteDiffEpsilon;
    this.convergenceEpsilon = DEFAULTS.convergenceEpsilon;
  }

  setFunctionName(functionName) {
    this.functionName = functionName;
  }

  setStartingPosition(x, z) {
    this.starting_p.x = x;
    this.starting_p.z = z;
  }

  position() {
    return this.p.clone();
  }

  delta() {
    return this.m_delta.clone();
  }

  gradX() {
    return this.grad.x;
  }

  gradZ() {
    return this.grad.z;
  }

  computeGradient() {
    const eps = this.finiteDiffEpsilon;
    const x = this.p.x;
    const z = this.p.z;
    this.grad.x = (f(x + eps, z, this.functionName) - f(x - eps, z, this.functionName)) / (2 * eps);
    this.grad.z = (f(x, z + eps, this.functionName) - f(x, z - eps, this.functionName)) / (2 * eps);
  }

  resetState() {
    // overridden by subclasses
  }

  resetPositionAndComputeGradient() {
    this.is_converged = false;
    this.m_delta = new Point(0, 0);
    this.resetState();
    this.setPositionAndComputeGradient(this.starting_p.x, this.starting_p.z);
  }

  setPositionAndComputeGradient(x, z) {
    this.p.x = x;
    this.p.z = z;
    this.computeGradient();
  }

  isConverged() {
    return this.is_converged;
  }

  updateGradientDelta() {
    throw new Error("updateGradientDelta not implemented");
  }

  takeGradientStep() {
    if (
      Math.abs(this.gradX()) < this.convergenceEpsilon &&
      Math.abs(this.gradZ()) < this.convergenceEpsilon
    ) {
      this.is_converged = true;
    }
    if (this.is_converged) return this.position();

    this.updateGradientDelta();
    this.setPositionAndComputeGradient(this.p.x + this.m_delta.x, this.p.z + this.m_delta.z);
    return this.position();
  }
}

export class VanillaGradientDescent extends GradientDescentBase {
  constructor(opts = {}) {
    super(opts);
    this.resetPositionAndComputeGradient();
  }
  updateGradientDelta() {
    this.m_delta.x = -this.learning_rate * this.grad.x;
    this.m_delta.z = -this.learning_rate * this.grad.z;
  }
}

export class Momentum extends GradientDescentBase {
  constructor(opts = {}) {
    super(opts);
    this.decay_rate = opts.decay_rate ?? 0.9;
    this.resetPositionAndComputeGradient();
  }
  updateGradientDelta() {
    this.m_delta.x = this.decay_rate * this.m_delta.x - this.learning_rate * this.grad.x;
    this.m_delta.z = this.decay_rate * this.m_delta.z - this.learning_rate * this.grad.z;
  }
}

export class QHM extends GradientDescentBase {
  constructor(opts = {}) {
    super(opts);
    this.decay_rate = opts.decay_rate ?? 0.99; // beta
    this.discount_factor = opts.discount_factor ?? 0.7; // v
    this.momentum = new Point(0, 0);
    this.resetPositionAndComputeGradient();
  }
  resetState() {
    this.momentum = new Point(0, 0);
  }
  updateGradientDelta() {
    this.momentum.x = this.decay_rate * this.momentum.x + (1 - this.decay_rate) * this.grad.x;
    this.momentum.z = this.decay_rate * this.momentum.z + (1 - this.decay_rate) * this.grad.z;

    // adjusted_learning_rate = learning_rate / (1 - decay_rate)
    const adjusted_lr = this.learning_rate / (1 - this.decay_rate);

    this.m_delta.x =
      -adjusted_lr * ((1 - this.discount_factor) * this.grad.x + this.discount_factor * this.momentum.x);
    this.m_delta.z =
      -adjusted_lr * ((1 - this.discount_factor) * this.grad.z + this.discount_factor * this.momentum.z);
  }
}

export class AdaGrad extends GradientDescentBase {
  constructor(opts = {}) {
    super(opts);
    this.grad_sum_of_squared = new Point(0, 0);
    this.resetPositionAndComputeGradient();
  }
  resetState() {
    this.grad_sum_of_squared = new Point(0, 0);
  }
  gradSumOfSquared() {
    return this.grad_sum_of_squared.clone();
  }
  updateGradientDelta() {
    this.grad_sum_of_squared.x += this.grad.x ** 2;
    this.grad_sum_of_squared.z += this.grad.z ** 2;
    this.m_delta.x =
      (-this.learning_rate * this.grad.x) / (Math.sqrt(this.grad_sum_of_squared.x) + this.divisionEpsilon);
    this.m_delta.z =
      (-this.learning_rate * this.grad.z) / (Math.sqrt(this.grad_sum_of_squared.z) + this.divisionEpsilon);
  }
}

export class RMSProp extends GradientDescentBase {
  constructor(opts = {}) {
    super(opts);
    this.decay_rate = opts.decay_rate ?? 0.99;
    this.decayed_grad_sum_of_squared = new Point(0, 0);
    this.resetPositionAndComputeGradient();
  }
  resetState() {
    this.decayed_grad_sum_of_squared = new Point(0, 0);
  }
  decayedGradSumOfSquared() {
    return this.decayed_grad_sum_of_squared.clone();
  }
  updateGradientDelta() {
    this.decayed_grad_sum_of_squared.x *= this.decay_rate;
    this.decayed_grad_sum_of_squared.x += (1 - this.decay_rate) * this.grad.x ** 2;
    this.decayed_grad_sum_of_squared.z *= this.decay_rate;
    this.decayed_grad_sum_of_squared.z += (1 - this.decay_rate) * this.grad.z ** 2;

    this.m_delta.x =
      (-this.learning_rate * this.grad.x) /
      (Math.sqrt(this.decayed_grad_sum_of_squared.x) + this.divisionEpsilon);
    this.m_delta.z =
      (-this.learning_rate * this.grad.z) /
      (Math.sqrt(this.decayed_grad_sum_of_squared.z) + this.divisionEpsilon);
  }
}

export class Adam extends GradientDescentBase {
  constructor(opts = {}) {
    super(opts);
    this.beta1 = opts.beta1 ?? 0.9;
    this.beta2 = opts.beta2 ?? 0.999;
    this.use_bias_correction = opts.use_bias_correction ?? true;

    this.decayed_grad_sum = new Point(0, 0);
    this.decayed_grad_sum_of_squared = new Point(0, 0);
    this.beta1_pow = this.beta1;
    this.beta2_pow = this.beta2;

    this.resetPositionAndComputeGradient();
  }

  resetState() {
    this.decayed_grad_sum_of_squared = new Point(0, 0);
    this.decayed_grad_sum = new Point(0, 0);
    this.beta1_pow = this.beta1;
    this.beta2_pow = this.beta2;
  }

  decayedGradSum() {
    return this.decayed_grad_sum.clone();
  }

  decayedGradSumOfSquared() {
    return this.decayed_grad_sum_of_squared.clone();
  }

  baseCompute() {
    // first moment
    this.decayed_grad_sum.x = this.beta1 * this.decayed_grad_sum.x + (1 - this.beta1) * this.grad.x;
    this.decayed_grad_sum.z = this.beta1 * this.decayed_grad_sum.z + (1 - this.beta1) * this.grad.z;

    // second moment
    this.decayed_grad_sum_of_squared.x =
      this.beta2 * this.decayed_grad_sum_of_squared.x + (1 - this.beta2) * this.grad.x ** 2;
    this.decayed_grad_sum_of_squared.z =
      this.beta2 * this.decayed_grad_sum_of_squared.z + (1 - this.beta2) * this.grad.z ** 2;

    const grad_sum = new Point(0, 0);
    const grad_sum_sq = new Point(0, 0);

    if (this.use_bias_correction) {
      grad_sum.x = this.decayed_grad_sum.x / (1 - this.beta1_pow);
      grad_sum.z = this.decayed_grad_sum.z / (1 - this.beta1_pow);
      grad_sum_sq.x = this.decayed_grad_sum_of_squared.x / (1 - this.beta2_pow);
      grad_sum_sq.z = this.decayed_grad_sum_of_squared.z / (1 - this.beta2_pow);

      this.beta1_pow *= this.beta1;
      this.beta2_pow *= this.beta2;
    } else {
      grad_sum.x = this.decayed_grad_sum.x;
      grad_sum.z = this.decayed_grad_sum.z;
      grad_sum_sq.x = this.decayed_grad_sum_of_squared.x;
      grad_sum_sq.z = this.decayed_grad_sum_of_squared.z;
    }

    return { grad_sum, grad_sum_sq };
  }

  updateGradientDelta() {
    const { grad_sum, grad_sum_sq } = this.baseCompute();
    this.m_delta.x =
      (-this.learning_rate * grad_sum.x) / (Math.sqrt(grad_sum_sq.x) + this.divisionEpsilon);
    this.m_delta.z =
      (-this.learning_rate * grad_sum.z) / (Math.sqrt(grad_sum_sq.z) + this.divisionEpsilon);
  }
}

export class QHAdam extends Adam {
  constructor(opts = {}) {
    super(opts);
    this.discount_factor = opts.discount_factor ?? 0.7; // v1
    this.squared_discount_factor = opts.squared_discount_factor ?? 1.0; // v2
    this.resetPositionAndComputeGradient();
  }

  updateGradientDelta() {
    const { grad_sum, grad_sum_sq } = this.baseCompute();
    this.m_delta.x =
      (-this.learning_rate *
        ((1 - this.discount_factor) * this.grad.x + this.discount_factor * grad_sum.x)) /
      (Math.sqrt((1 - this.squared_discount_factor) * this.grad.x ** 2 + this.squared_discount_factor * grad_sum_sq.x) +
        this.divisionEpsilon);
    this.m_delta.z =
      (-this.learning_rate *
        ((1 - this.discount_factor) * this.grad.z + this.discount_factor * grad_sum.z)) /
      (Math.sqrt((1 - this.squared_discount_factor) * this.grad.z ** 2 + this.squared_discount_factor * grad_sum_sq.z) +
        this.divisionEpsilon);
  }
}

