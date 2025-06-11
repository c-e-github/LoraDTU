<template>
    <div class="row row-cols-1 row-cols-md-3 g-3">
        <div class="col">
            <div class="card">
<!--                  <div class="card-header text-bg-success">{{ $t('invertertotalinfo.BattVoltage') }}</div> 
totalausbeute durch batteriespannung ersetzt  "YieldTotal" - "BattVoltage"
-->
                <div class="text-bg-primary">{{ $t('invertertotalinfo.BattVoltage') }}</div>
                <div class="card-body card-text text-center">
                    <h2>
                        {{ $n(totalData.YieldTotal.v, 'decimal', {
                                minimumFractionDigits: totalData.YieldTotal.d,
                                maximumFractionDigits: totalData.YieldTotal.d
                            })}}
                        <small class="text-muted">{{ totalData.YieldTotal.u }}</small>
                    </h2>
                </div>
            </div>
        </div>
        <div class="col">
            <div class="card">
<!--                <div class="card-header text-bg-success">{{ $t('invertertotalinfo.TotalYieldDay') }}</div> 
-->
                <div class="text-bg-primary">{{ $t('invertertotalinfo.TotalYieldDay') }}</div>
                <div class="card-body card-text text-center" style="padding-top: 0.3em; padding-bottom: 0.3em;">
                    <h2>
                        {{ $n(totalData.YieldDay.v, 'decimal', {
                             minimumFractionDigits: totalData.YieldDay.d,
                             maximumFractionDigits: totalData.YieldDay.d
                         })}}
                        <small class="text-muted">{{ totalData.YieldDay.u }}</small>
                    </h2>

<!--   TotalYieldYesterDay hinzugefügt -->
                        (Gestern:
                            {{ $n(totalData.YieldYesterDay.v, 'decimal', {
                                minimumFractionDigits: 3,
                                maximumFractionDigits: 3
                            })}}
                         kWh)
<!--   ende -->
                        
                </div>
            </div>
        </div>
        <div class="col">
            <div class="card">
<!--                <div class="card-header text-bg-success">{{ $t('invertertotalinfo.TotalPower') }}</div> -->
                <div class="text-bg-primary">{{ $t('invertertotalinfo.TotalPower') }}</div>
                <div class="card-body card-text text-center">
                    <h2>
                        {{ $n(totalData.Power.v, 'decimal', {
                                minimumFractionDigits: totalData.Power.d,
                                maximumFractionDigits: totalData.Power.d
                            })}}
                        <small class="text-muted">{{ totalData.Power.u }}</small>
                    </h2>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import type { Total } from '@/types/LiveDataStatus';
import { defineComponent, type PropType } from 'vue';

export default defineComponent({
    props: {
        totalData: { type: Object as PropType<Total>, required: true },
    },
});
</script>