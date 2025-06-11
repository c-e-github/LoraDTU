<template id="top">
    <BasePage :title="$t('about.AboutLoraDTU')">
        <div class="accordion" id="accordionExample">
            <div class="accordion-item">
                <div id="collapseOne" class="accordion-collapse collapse show" aria-labelledby="headingOne"
                    data-bs-parent="#accordionExample">
                    <div class="accordion-body">
                        <p class="fw-bold" v-html="$t('about.ProjectOriginBody0')"></p>
                        <p class="fw-bold" v-html="$t('about.ProjectOriginBody0a')"></p>
                        <p class="fw-bold" v-html="$t('about.ProjectOriginBody0b')"></p>
                        <p class="fw-bold" v-html="$t('about.ProjectOriginBody0c')"></p>
                        <br>
                        <div id="loradtublock">
                           <p class="fw-bold" v-html="$t('about.ProjectOriginBody0d')"></p>
                           <p class="fw-normal" v-html="$t('about.ProjectOriginBody1')"></p>
                           <p class="fw-normal" v-html="$t('about.ProjectOriginBody2')"></p>
                           <p class="fw-normal" v-html="$t('about.ProjectOriginBody3')"></p>
                           <p class="fw-normal" v-html="$t('about.ProjectOriginBody4')"></p>
                        </div>
                    </div>
                </div>
            </div>
        </div>

    </BasePage>

</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import { defineComponent } from 'vue';

export default defineComponent({
   components: {
      BasePage,
     },
   mounted() {
      window.scrollTo(0,0);
      let storedTheme = localStorage.getItem('theme') || 'light';
      const element = document.getElementById("loradtublock");
      if (element){
         if (storedTheme == "light" || storedTheme == "auto"){
            element.style.backgroundColor = "lavender";
         } else {
            element.style.backgroundColor = "darkslategrey";
         }
      }

     setInterval(() => {
        let currentTheme = localStorage.getItem('theme') || 'light';
        if (currentTheme !== storedTheme) {
           storedTheme = currentTheme;
           location.reload();
        }
      }, 200); // Check every 200 ms

   },
});
</script>