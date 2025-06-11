<template>
  <div id="app-layout">
    <NavBar />
    <main class="container-fluid">
      <router-view />
    </main>
  </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import NavBar from "./components/NavBar.vue";

export default defineComponent({
  name: "App",
  components: {
    NavBar,
  },
});
</script>

<style>
#app-layout {
  display: grid;
  grid-template-rows: auto 1fr; /* NavBar height is auto, main takes the rest */
  height: 100vh; /* Full viewport height */
  overflow: hidden; /* Prevent scrolling of the entire layout */
}

NavBar {
  grid-row: 1; /* Place NavBar in the first row */
}

main.container-fluid {
  grid-row: 2; /* Place main content in the second row */
  overflow-y: auto; /* Allow scrolling within the main content only */
}
</style>
