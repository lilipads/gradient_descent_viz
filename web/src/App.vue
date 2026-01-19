<template>
  <div class="h-dvh w-dvw bg-slate-950 text-slate-100">
    <div class="flex h-full flex-col">
      <header class="border-b border-slate-800 bg-slate-950/80 backdrop-blur">
        <div class="mx-auto flex max-w-[1400px] items-center justify-between gap-3 px-4 py-3">
          <div class="min-w-0">
            <div class="truncate text-lg font-semibold">Gradient Descent Viz (Web) — M1</div>
            <div class="truncate text-xs text-slate-400">Overview mode: multiple optimizers + colored paths</div>
          </div>

          <div class="flex items-center gap-2">
            <button
              class="rounded-md bg-slate-800 px-3 py-2 text-sm font-medium hover:bg-slate-700"
              @click="togglePlay"
            >
              {{ isPlaying ? "Pause" : "Play" }}
            </button>
            <button
              class="rounded-md bg-slate-800 px-3 py-2 text-sm font-medium hover:bg-slate-700"
              @click="resetAll"
            >
              Reset
            </button>
            <label class="hidden select-none items-center gap-2 text-sm sm:flex">
              <input type="checkbox" v-model="wireframe" />
              <span class="text-slate-300">Wireframe</span>
            </label>
            <div class="hidden rounded-md border border-slate-700/60 bg-slate-950/50 px-2 py-1 text-xs text-slate-200 sm:block">
              Active: <span class="font-semibold">{{ enabledMethodCount }}</span>
            </div>
          </div>
        </div>
      </header>

      <!--
        Responsive layout:
        - Mobile/tablet: 2 rows (viz on top with a guaranteed height, controls below scrollable)
        - Desktop (lg+): 2 columns (viz left, controls right)
      -->
      <main
        class="mx-auto grid min-h-0 w-full max-w-[1400px] flex-1 grid-cols-1 grid-rows-[minmax(320px,45vh)_minmax(0,1fr)] gap-4 px-4 py-4 lg:grid-cols-[1fr_420px] lg:grid-rows-1"
      >
        <section class="relative min-h-0 overflow-hidden rounded-xl border border-slate-800 bg-slate-900">
          <div ref="threeHost" class="absolute inset-0"></div>

          <div class="pointer-events-none absolute left-3 top-3 flex flex-col gap-1">
            <div class="rounded-md bg-slate-950/70 px-2 py-1 text-xs text-slate-200">
              Steps: <span class="font-semibold">{{ steps }}</span>
            </div>
            <div class="rounded-md bg-slate-950/70 px-2 py-1 text-xs text-slate-200">
              Surface: <span class="font-semibold">{{ surfaceLabel }}</span>
            </div>
          </div>
        </section>

        <aside class="min-h-0 overflow-auto rounded-xl border border-slate-800 bg-slate-900 p-4">
          <div class="space-y-4">
            <div>
              <div class="text-sm font-semibold text-slate-200">Surface</div>
              <select
                class="mt-2 w-full rounded-md border border-slate-700 bg-slate-950 px-3 py-2 text-sm"
                v-model="surfaceName"
                @change="onChangeSurface"
              >
                <option value="local_minimum">Local Minimum</option>
                <option value="global_minimum">Global Minimum</option>
                <option value="saddle_point">Saddle Point</option>
                <option value="ecliptic_bowl">Ecliptic Bowl</option>
                <option value="hills">Hills</option>
                <option value="plateau">Plateau</option>
              </select>
            </div>

            <div class="rounded-lg border border-slate-800 bg-slate-950 p-3">
              <div class="text-sm font-semibold text-slate-200">Overview</div>
              <div class="mt-1 text-xs text-slate-400">
                Toggle methods on/off. Each method runs independently and draws its own path.
              </div>
            </div>

            <div class="space-y-3">
              <div
                v-for="m in methods"
                :key="m.key"
                class="rounded-xl border border-slate-800 bg-slate-950 p-3"
              >
                <div class="flex items-center justify-between gap-3">
                  <label class="flex min-w-0 items-center gap-2">
                    <input type="checkbox" v-model="m.enabled" @change="onToggleMethod(m.key)" />
                    <span class="truncate text-sm font-semibold" :style="{ color: m.colorHex }">
                      {{ m.label }}
                    </span>
                  </label>
                  <div class="text-xs text-slate-400">LR: 1e{{ m.lrExp }}</div>
                </div>

                <div class="mt-3 grid grid-cols-2 gap-3">
                  <label class="text-sm">
                    <div class="text-xs text-slate-400">Learning rate exponent</div>
                    <input
                      class="mt-1 w-full rounded-md border border-slate-700 bg-slate-900 px-2 py-1 text-sm"
                      type="number"
                      min="-10"
                      max="10"
                      step="1"
                      v-model.number="m.lrExp"
                      @change="onUpdateMethodParams(m.key)"
                    />
                  </label>

                  <template v-if="m.params && m.params.length">
                    <label
                      v-for="p in m.params"
                      :key="p.key"
                      class="text-sm"
                    >
                      <div class="text-xs text-slate-400">{{ p.label }}</div>
                      <input
                        class="mt-1 w-full rounded-md border border-slate-700 bg-slate-900 px-2 py-1 text-sm"
                        type="number"
                        :step="p.step"
                        :min="p.min"
                        :max="p.max"
                        v-model.number="p.value"
                        @change="onUpdateMethodParams(m.key)"
                      />
                    </label>
                  </template>
                </div>
              </div>
            </div>
          </div>
        </aside>
      </main>
    </div>
  </div>
</template>

<script>
import { markRaw } from "vue";
import { DEFAULT_START, DEFAULT_SURFACE } from "./app/defaults.js";
import { Viz3D } from "./viz/Viz3D.js";
import { FunctionName } from "./gd/surfaces.js";
import {
  VanillaGradientDescent,
  Momentum,
  QHM,
  AdaGrad,
  RMSProp,
  Adam,
  QHAdam,
} from "./gd/optimizers.js";

function log(...args) {
  // Centralized logger so we can easily turn it off later.
  console.log("[M1]", ...args);
}

function buildOptimizer(methodKey, opts) {
  switch (methodKey) {
    case "gd":
      return new VanillaGradientDescent(opts);
    case "momentum":
      return new Momentum(opts);
    case "qhm":
      return new QHM(opts);
    case "adagrad":
      return new AdaGrad(opts);
    case "rmsprop":
      return new RMSProp(opts);
    case "adam":
      return new Adam(opts);
    case "qhadam":
      return new QHAdam(opts);
    default:
      return new VanillaGradientDescent(opts);
  }
}

export default {
  name: "App",
  data() {
    return {
      isPlaying: true,
      wireframe: false,
      steps: 0,

      gridSize: 51,
      // The raw surface is already fairly tall; keep default Y scale modest so it doesn't look like a skyscraper.
      yScale: 0.5,
      surfaceName: DEFAULT_SURFACE,

      viz: null,
      optimizers: null, // Map<string, Optimizer>
      rafId: null,
      frameCounter: 0,

      // Debug controls
      debug: false,
      tickLogs: 0,

      methods: [
        { key: "gd", label: "Gradient Descent", enabled: true, colorHex: "#22d3ee", lrExp: -3, params: [] },
        {
          key: "momentum",
          label: "Momentum",
          enabled: true,
          colorHex: "#a855f7",
          lrExp: -3,
          params: [{ key: "decay_rate", label: "Decay rate", value: 0.9, min: 0, max: 2, step: 0.001 }],
        },
        {
          key: "qhm",
          label: "QHM",
          enabled: true,
          colorHex: "#ef4444",
          lrExp: -3,
          params: [
            { key: "decay_rate", label: "Decay rate (β)", value: 0.99, min: 0, max: 2, step: 0.001 },
            { key: "discount_factor", label: "Discount factor (v)", value: 0.7, min: 0, max: 2, step: 0.001 },
          ],
        },
        { key: "adagrad", label: "Adagrad", enabled: true, colorHex: "#94a3b8", lrExp: -3, params: [] },
        {
          key: "rmsprop",
          label: "RMSprop",
          enabled: true,
          colorHex: "#22c55e",
          lrExp: -3,
          params: [{ key: "decay_rate", label: "Decay rate", value: 0.99, min: 0, max: 2, step: 0.001 }],
        },
        {
          key: "adam",
          label: "Adam",
          enabled: true,
          colorHex: "#3b82f6",
          lrExp: -3,
          params: [
            { key: "beta1", label: "Beta1", value: 0.9, min: 0, max: 2, step: 0.001 },
            { key: "beta2", label: "Beta2", value: 0.999, min: 0, max: 2, step: 0.001 },
          ],
        },
        {
          key: "qhadam",
          label: "QHAdam",
          enabled: true,
          colorHex: "#06b6d4",
          lrExp: -3,
          params: [
            { key: "beta1", label: "Beta1", value: 0.9, min: 0, max: 2, step: 0.001 },
            { key: "beta2", label: "Beta2", value: 0.999, min: 0, max: 2, step: 0.001 },
            { key: "discount_factor", label: "Discount factor (v1)", value: 0.7, min: 0, max: 2, step: 0.001 },
            { key: "squared_discount_factor", label: "Sq discount (v2)", value: 1.0, min: 0, max: 2, step: 0.001 },
          ],
        },
      ],
    };
  },
  computed: {
    enabledMethodCount() {
      return this.methods.filter((m) => m.enabled).length;
    },
    surfaceLabel() {
      const map = {
        [FunctionName.local_minimum]: "Local Minimum",
        [FunctionName.global_minimum]: "Global Minimum",
        [FunctionName.saddle_point]: "Saddle Point",
        [FunctionName.ecliptic_bowl]: "Ecliptic Bowl",
        [FunctionName.hills]: "Hills",
        [FunctionName.plateau]: "Plateau",
      };
      return map[this.surfaceName] ?? this.surfaceName;
    },
    allEnabledConverged() {
      if (!this.optimizers) return false;
      const enabled = this.methods.filter((m) => m.enabled);
      if (!enabled.length) return false;
      return enabled.every((m) => this.optimizers.get(m.key)?.isConverged?.());
    },
  },
  watch: {
    wireframe(next) {
      if (this.debug) log("wireframe changed:", next);
    },
    isPlaying(next) {
      if (this.debug) log("isPlaying changed:", next);
    },
  },
  methods: {
    _methodByKey(key) {
      return this.methods.find((m) => m.key === key);
    },
    _buildOptimizerForMethod(m) {
      const learningRate = 10 ** m.lrExp;
      const opts = { functionName: this.surfaceName, learningRate };
      if (m.params) {
        for (const p of m.params) opts[p.key] = p.value;
      }
      const optimizer = buildOptimizer(m.key, opts);
      optimizer.setStartingPosition(DEFAULT_START.x, DEFAULT_START.z);
      optimizer.resetPositionAndComputeGradient();
      return markRaw(optimizer);
    },
    _rebuildAllOptimizers() {
      this.optimizers = markRaw(new Map());
      for (const m of this.methods) {
        this.optimizers.set(m.key, this._buildOptimizerForMethod(m));
      }
    },
    _syncVizActorsFromState({ resetPaths = false } = {}) {
      for (const m of this.methods) {
        this.viz.ensureMethodActor(m.key, m.colorHex);
        this.viz.setMethodEnabled(m.key, m.enabled);
        if (resetPaths) this.viz.resetMethodPath(m.key);

        const opt = this.optimizers.get(m.key);
        if (!opt) continue;
        const p = opt.position();
        if (resetPaths) this.viz.appendMethodPoint(m.key, p.x, p.z);
        this.viz.setMethodBallPosition(m.key, p.x, p.z);
      }
    },
    togglePlay() {
      if (this.debug) log("togglePlay clicked");
      this.isPlaying = !this.isPlaying;
    },
    resetAll() {
      if (this.debug) log("resetAll clicked");
      this.steps = 0;
      this._rebuildAllOptimizers();
      this._syncVizActorsFromState({ resetPaths: true });
    },
    onToggleMethod(key) {
      const m = this._methodByKey(key);
      if (!m) return;
      this.viz.setMethodEnabled(key, m.enabled);
    },
    onUpdateMethodParams(key) {
      const m = this._methodByKey(key);
      if (!m) return;
      this.optimizers.set(key, this._buildOptimizerForMethod(m));
      this.viz.resetMethodPath(key);
      const p = this.optimizers.get(key).position();
      this.viz.appendMethodPoint(key, p.x, p.z);
      this.viz.setMethodBallPosition(key, p.x, p.z);
    },
    onChangeSurface() {
      this.steps = 0;
      this.viz.setSurface({ functionName: this.surfaceName, gridSize: this.gridSize, yScale: this.yScale });
      this._rebuildAllOptimizers();
      this._syncVizActorsFromState({ resetPaths: true });
    },
    tick() {
      try {
        if (this.isPlaying && this.allEnabledConverged) this.isPlaying = false;

        // M1: step all enabled methods in overview mode.
        const shouldStep = this.isPlaying && this.frameCounter % 2 === 0;
        if (shouldStep) {
          for (const m of this.methods) {
            if (!m.enabled) continue;
            const opt = this.optimizers.get(m.key);
            if (!opt || opt.isConverged()) continue;
            const p = opt.takeGradientStep();
            this.viz.appendMethodPoint(m.key, p.x, p.z);
            this.viz.setMethodBallPosition(m.key, p.x, p.z);
          }
          this.steps += 1;
        }

        // Periodic heartbeat (every ~2s at 60fps)
        if (this.debug && this.frameCounter % 120 === 0) {
          log("heartbeat", {
            isPlaying: this.isPlaying,
            steps: this.steps,
            allEnabledConverged: this.allEnabledConverged,
          });
        }

        this.frameCounter = (this.frameCounter + 1) % 1_000_000;
        this.viz.setWireframe(this.wireframe);
        this.viz.render();
      } catch (err) {
        console.error("[M1] tick error:", err);
      } finally {
        this.rafId = requestAnimationFrame(this.tick);
      }
    },
  },
  mounted() {
    // Enable debug logs with `?debug=1`
    try {
      const params = new URLSearchParams(window.location.search);
      this.debug = params.get("debug") === "1";
    } catch {
      this.debug = false;
    }
    if (this.debug) log("mounted (debug enabled)");
    // IMPORTANT: Three.js objects must NOT be made reactive (Vue Proxy breaks WebGLRenderer internals).
    this.viz = markRaw(new Viz3D(this.$refs.threeHost));
    this.viz.setSurface({
      functionName: this.surfaceName,
      gridSize: this.gridSize,
      yScale: this.yScale,
    });

    // Ensure the renderer picks up the final layout size (important for flex/grid layouts).
    this.$nextTick(() => {
      if (this.viz) {
        this.viz.resize();
        const host = this.$refs.threeHost;
        if (this.debug) log("after layout:", { hostW: host?.clientWidth, hostH: host?.clientHeight });
      }
    });

    this._rebuildAllOptimizers();
    this._syncVizActorsFromState({ resetPaths: true });

    this.tick = this.tick.bind(this);
    this.rafId = requestAnimationFrame(this.tick);
  },
  beforeUnmount() {
    if (this.rafId) cancelAnimationFrame(this.rafId);
    if (this.viz) this.viz.dispose();
  },
};
</script>
