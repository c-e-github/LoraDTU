<template>
    <BasePage :title="$t('ntpinfo.NtpInformation')" :isLoading="dataLoading" :show-reload="true" @reload="getNtpInfo">
        <CardElement :text="$t('ntpinfo.ConfigurationSummary')" textVariant="text-bg-primary">
            <div class="table-responsive">
                <table class="table table-hover table-condensed">
                    <tbody>
                        <tr>
                            <th>{{ $t('ntpinfo.Server') }}</th>
                            <td>{{ ntpDataList.ntp_server }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('ntpinfo.Timezone') }}</th>
                            <td>{{ ntpDataList.ntp_timezone }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('ntpinfo.TimezoneDescription') }}</th>
                            <td>{{ ntpDataList.ntp_timezone_descr }}</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </CardElement>

        <CardElement :text="$t('ntpinfo.CurrentTime')" textVariant="text-bg-primary" add-space>
            <div class="table-responsive">
                <table class="table table-hover table-condensed">
                    <tbody>
                        <tr>
                            <th>{{ $t('ntpinfo.Status') }}</th>
                            <td>
                                <StatusBadge :status="ntpDataList.ntp_status" true_text="ntpinfo.Synced" false_text="ntpinfo.NotSynced" />
                            </td>
                        </tr>
                        <tr>
                            <th>{{ $t('ntpinfo.LocalTime') }}</th>
                            <td>
                               {{ ntpDataList.ntp_localtime }}
                               <button type="button" class=" btn btn-outline-primary"
                                  @click="getNtpInfo" v-tooltip :title="$t('base.Reload')" ><BIconArrowClockwise />
                               </button>
                            </td>
                        </tr>

                    </tbody>
                </table>
            </div>
        </CardElement>
    </BasePage>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import CardElement from '@/components/CardElement.vue';
import StatusBadge from '@/components/StatusBadge.vue';
import type { NtpStatus } from "@/types/NtpStatus";
import { authHeader, handleResponse } from '@/utils/authentication';
import { defineComponent } from 'vue';
import { BIconArrowClockwise } from 'bootstrap-icons-vue';

export default defineComponent({
    components: {
        BasePage,
        CardElement,
        StatusBadge,
        BIconArrowClockwise,
    },
    data() {
        return {
            dataLoading: true,
            ntpDataList: {} as NtpStatus,
        };
    },
    created() {
        this.getNtpInfo();
    },
    methods: {
        getNtpInfo() {
            this.dataLoading = true;
            fetch("/api/ntp/status", { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.ntpDataList = data;
                    this.dataLoading = false;
                });
        },
    },
});
</script>