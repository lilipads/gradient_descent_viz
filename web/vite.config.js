import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import tailwindcss from '@tailwindcss/vite'

// https://vite.dev/config/
export default defineConfig(() => ({
  // GitHub Pages serves your site from `/<repo>/`, not `/`.
  // In CI we set BASE_PATH to `/${{ github.event.repository.name }}/`.
  base: process.env.BASE_PATH || '/',
  plugins: [vue(), tailwindcss()],
}))
