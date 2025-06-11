<template>
    <div :class="{'container-xxl': !isWideScreen,
    'container-fluid': isWideScreen}" role="main">
        <div class="page-header">
            <div class="row">
                <div class="col-sm-11">
                    <table>
                       <tr>
                          <td>
                             <h2>{{ title }}</h2>
                          </td>
                          <td width="100px">
                          </td>
                          <td v-if="totalData?.Stromzaehler?.v !== undefined">
                             <span id="Szaehler" style="font-size:1.25rem; padding:0.3rem 0.3rem 0.3rem 0rem;">
                                {{ $t('base.Szaehler') }}: {{ totalData?.Stromzaehler?.v ?? 'No Data' }}
                                <span v-if="totalData?.Stromzaehler?.v !== undefined && totalData?.Stromzaehler?.v >= 0">
                                   W
                                </span>
                                <span v-if="totalData?.Stromzaehler?.v !== undefined && totalData?.Stromzaehler?.v < 0">
                                   W ({{ $t('base.Einspeisung') }})
                                </span> 
                                <span v-else>
                                </span>
                             </span>
                          </td>
                       </tr>
                    </table>
                </div>
                <div class="col-sm-1" v-if="showReload">
                    <button type="button" class="float-end btn btn-outline-primary"
                        @click="$emit('reload')" v-tooltip :title="$t('base.Reload')" ><BIconArrowClockwise /></button>
                </div>
            </div>
        </div>

        <div class="text-center" v-if="isLoading">
            <div class="spinner-border" role="status">
                <span class="visually-hidden">{{ $t('base.Loading') }}</span>
            </div>
        </div>

        <template v-if="!isLoading">
            <slot />
        </template>
    </div>
</template>


<script lang="ts">
import { defineComponent, type PropType  } from 'vue';
import { BIconArrowClockwise } from 'bootstrap-icons-vue';
import type { Total } from '@/types/LiveDataStatus';

export default defineComponent({
   components: {
      BIconArrowClockwise,
   },
   props: {
      title: { type: String, required: true },
      isLoading: { type: Boolean, required: false, default: false },
      isWideScreen: { type: Boolean, required: false, default: false },
      showReload: { type: Boolean, required: false, default: false },
      totalData: { type: Object as PropType<Total>, required: true, default: () => ({}), },
   },
   name: 'BasePage',

   watch: {
//      'totalData.Stromzaehler.v': {
      'totalData.StromzaehlerZ': { // StromzaehlerZ = wert mit angehängtem zähler --> aufblinken bei jedem lora-wert
         handler(newValue, oldValue) {
            if (newValue !== oldValue) {
               const SZele = document.getElementById("Szaehler");
               if (SZele) {
                  SZele.style.transition = "none";
                  let bgVfarbe = SZele.style.color;
                  let bgHfarbe = SZele.style.backgroundColor;
                  SZele.style.backgroundColor = "lightgreen";
                  SZele.style.color = "black";
                  requestAnimationFrame(() => {
                     setTimeout(() => {
                        SZele.style.transition = "background-color 1s linear";
                        SZele.style.backgroundColor = bgHfarbe;
                        SZele.style.color = bgVfarbe;
                     }, 0); // Use a 0ms delay to ensure immediate render update
                  });
               }
            }
         },
         immediate: true, deep: true
      },
   },

   methods: {
      reset_bg(){
         let storedTheme = localStorage.getItem('theme') || 'light';
         const SZele = document.getElementById("Szaehler");
         if (SZele) {
            let bgVfarbe = "black";
            let bgHfarbe = "white";
            if (storedTheme == "light" || storedTheme == "auto"){
               bgVfarbe = "black";
               bgHfarbe = "white";
            } else {
               bgVfarbe = "white";
               bgHfarbe = "#212529";
            }
            SZele.style.backgroundColor = bgHfarbe; // nach einiger zeit auf jeden fall hintergrundfarbe wiederherstellen
            SZele.style.color = bgVfarbe;
         }
      }
   },

   mounted() {
      setInterval(() => this.reset_bg(), 2000); // Execute reset_bg every 2 seconds
   }
});
</script>