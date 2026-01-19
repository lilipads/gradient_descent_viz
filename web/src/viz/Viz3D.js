import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";

import { f } from "../gd/surfaces.js";
import { DOMAIN } from "../app/defaults.js";

function clamp01(x) {
  return Math.max(0, Math.min(1, x));
}

function colormap(t) {
  // 0 -> dark red, 0.1 -> red, 0.3 -> amber, 1 -> dark green
  t = clamp01(t);
  const stops = [
    { t: 0.0, c: new THREE.Color("#7f1d1d") },
    { t: 0.1, c: new THREE.Color("#ef4444") },
    { t: 0.3, c: new THREE.Color("#f59e0b") },
    { t: 1.0, c: new THREE.Color("#166534") },
  ];

  for (let i = 0; i < stops.length - 1; i++) {
    const a = stops[i];
    const b = stops[i + 1];
    if (t >= a.t && t <= b.t) {
      const u = (t - a.t) / (b.t - a.t);
      return a.c.clone().lerp(b.c, u);
    }
  }
  return stops[stops.length - 1].c.clone();
}

export class Viz3D {
  constructor(hostEl) {
    this.hostEl = hostEl;

    this.functionName = null;
    this.gridSize = 51;
    this.yScale = 2;

    const w = hostEl.clientWidth || 800;
    const h = hostEl.clientHeight || 600;

    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color("#0b1220");

    this.camera = new THREE.PerspectiveCamera(50, w / h, 0.01, 200);
    // Slightly tighter framing; combined with a modest yScale this feels closer to the Qt view.
    this.camera.position.set(3.6, 2.4, 3.6);

    this.renderer = new THREE.WebGLRenderer({ antialias: true });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    this.renderer.setSize(w, h);
    hostEl.appendChild(this.renderer.domElement);

    this.controls = new OrbitControls(this.camera, this.renderer.domElement);
    this.controls.enableDamping = true;
    this.controls.target.set(0, 0.0, 0);

    this.surfaceMaterial = new THREE.MeshStandardMaterial({
      vertexColors: true,
      metalness: 0.1,
      roughness: 0.85,
      side: THREE.DoubleSide,
      wireframe: false,
    });
    this.surfaceMesh = null;

    // Lighting
    const light = new THREE.DirectionalLight(0xffffff, 1.2);
    light.position.set(4, 6, 3);
    this.scene.add(light);
    this.scene.add(new THREE.AmbientLight(0xffffff, 0.35));

    // Helpers
    const grid = new THREE.GridHelper(4.5, 20, 0x334155, 0x1f2937);
    grid.position.y = -0.001;
    this.scene.add(grid);
    const axes = new THREE.AxesHelper(1.5);
    axes.position.y = 0.01;
    this.scene.add(axes);

    // Method actors (M1 overview mode)
    // key -> { ball, pathLine, maxPoints, count, positions }
    this.methodActors = new Map();

    this._onResize = () => this.resize();
    window.addEventListener("resize", this._onResize);
  }

  dispose() {
    window.removeEventListener("resize", this._onResize);
    this.controls.dispose();
    this.renderer.dispose();

    if (this.surfaceMesh) {
      this.surfaceMesh.geometry.dispose();
      this.surfaceMesh = null;
    }
    this.surfaceMaterial.dispose();

    for (const actor of this.methodActors.values()) {
      actor.ball.geometry.dispose();
      actor.ball.material.dispose();
      actor.pathLine.geometry.dispose();
      actor.pathLine.material.dispose();
    }
    this.methodActors.clear();
  }

  resize() {
    const w = this.hostEl.clientWidth || 800;
    const h = this.hostEl.clientHeight || 600;
    this.camera.aspect = w / h;
    this.camera.updateProjectionMatrix();
    this.renderer.setSize(w, h);
  }

  setWireframe(wireframe) {
    this.surfaceMaterial.wireframe = !!wireframe;
  }

  setSurface({ functionName, gridSize, yScale }) {
    this.functionName = functionName;
    this.gridSize = gridSize;
    this.yScale = yScale;
    this._rebuildSurfaceMesh();

    // Re-project method actors onto the new surface scale.
    for (const [key, actor] of this.methodActors) {
      this._reprojectActorPath(key);
      this._reprojectActorBall(key);
    }
  }

  surfaceY(x, z) {
    return f(x, z, this.functionName) * this.yScale;
  }

  ensureMethodActor(key, colorHex) {
    if (this.methodActors.has(key)) return;

    const ball = new THREE.Mesh(
      new THREE.SphereGeometry(0.06, 20, 16),
      new THREE.MeshStandardMaterial({ color: new THREE.Color(colorHex) })
    );
    this.scene.add(ball);

    const pathLine = new THREE.Line(
      new THREE.BufferGeometry(),
      new THREE.LineBasicMaterial({ color: new THREE.Color(colorHex), transparent: true, opacity: 0.9 })
    );
    this.scene.add(pathLine);

    const maxPoints = 10_000;
    const positions = new Float32Array(maxPoints * 3);
    pathLine.geometry.setAttribute("position", new THREE.BufferAttribute(positions, 3));
    pathLine.geometry.setDrawRange(0, 0);
    pathLine.geometry.computeBoundingSphere();

    this.methodActors.set(key, { ball, pathLine, maxPoints, count: 0, positions });
  }

  setMethodEnabled(key, enabled) {
    const actor = this.methodActors.get(key);
    if (!actor) return;
    actor.ball.visible = !!enabled;
    actor.pathLine.visible = !!enabled;
  }

  resetMethodPath(key) {
    const actor = this.methodActors.get(key);
    if (!actor) return;
    actor.count = 0;
    actor.pathLine.geometry.setDrawRange(0, 0);
    actor.pathLine.geometry.attributes.position.needsUpdate = true;
  }

  appendMethodPoint(key, x, z) {
    const actor = this.methodActors.get(key);
    if (!actor) return;
    if (actor.count >= actor.maxPoints) return;
    const y = this.surfaceY(x, z);
    const i = actor.count * 3;
    actor.positions[i + 0] = x;
    actor.positions[i + 1] = y;
    actor.positions[i + 2] = z;
    actor.count += 1;

    actor.pathLine.geometry.setDrawRange(0, actor.count);
    actor.pathLine.geometry.attributes.position.needsUpdate = true;
    actor.pathLine.geometry.computeBoundingSphere();
  }

  setMethodBallPosition(key, x, z) {
    const actor = this.methodActors.get(key);
    if (!actor) return;
    actor.ball.position.set(x, this.surfaceY(x, z), z);
  }

  _reprojectActorPath(key) {
    const actor = this.methodActors.get(key);
    if (!actor) return;
    for (let p = 0; p < actor.count; p++) {
      const i = p * 3;
      const x = actor.positions[i + 0];
      const z = actor.positions[i + 2];
      actor.positions[i + 1] = this.surfaceY(x, z);
    }
    actor.pathLine.geometry.attributes.position.needsUpdate = true;
    actor.pathLine.geometry.computeBoundingSphere();
  }

  _reprojectActorBall(key) {
    const actor = this.methodActors.get(key);
    if (!actor) return;
    const x = actor.ball.position.x;
    const z = actor.ball.position.z;
    actor.ball.position.y = this.surfaceY(x, z);
  }

  render() {
    this.controls.update();
    this.renderer.render(this.scene, this.camera);
  }

  _rebuildSurfaceMesh() {
    if (this.surfaceMesh) {
      this.scene.remove(this.surfaceMesh);
      this.surfaceMesh.geometry.dispose();
      this.surfaceMesh = null;
    }

    const n = this.gridSize;
    const dx = (DOMAIN.maxX - DOMAIN.minX) / (n - 1);
    const dz = (DOMAIN.maxZ - DOMAIN.minZ) / (n - 1);

    const positions = [];
    const colors = [];
    const indices = [];

    let minY = Infinity;
    let maxY = -Infinity;
    const heights = new Array(n * n);

    for (let iz = 0; iz < n; iz++) {
      const z = DOMAIN.minZ + iz * dz;
      for (let ix = 0; ix < n; ix++) {
        const x = DOMAIN.minX + ix * dx;
        const y = f(x, z, this.functionName) * this.yScale;
        const idx = iz * n + ix;
        heights[idx] = y;
        minY = Math.min(minY, y);
        maxY = Math.max(maxY, y);
        positions.push(x, y, z);
      }
    }

    const denom = maxY - minY || 1;
    for (let i = 0; i < heights.length; i++) {
      const t = (heights[i] - minY) / denom;
      const c = colormap(t);
      colors.push(c.r, c.g, c.b);
    }

    for (let iz = 0; iz < n - 1; iz++) {
      for (let ix = 0; ix < n - 1; ix++) {
        const a = iz * n + ix;
        const b = iz * n + (ix + 1);
        const c = (iz + 1) * n + ix;
        const d = (iz + 1) * n + (ix + 1);
        indices.push(a, c, b);
        indices.push(b, c, d);
      }
    }

    const geom = new THREE.BufferGeometry();
    geom.setAttribute("position", new THREE.Float32BufferAttribute(positions, 3));
    geom.setAttribute("color", new THREE.Float32BufferAttribute(colors, 3));
    geom.setIndex(indices);
    geom.computeVertexNormals();

    this.surfaceMesh = new THREE.Mesh(geom, this.surfaceMaterial);
    this.scene.add(this.surfaceMesh);
  }
}

