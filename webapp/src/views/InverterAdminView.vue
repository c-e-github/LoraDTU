<template>
    <BasePage :title="$t('inverteradmin.InverterSettings')" :isLoading="dataLoading">
        <BootstrapAlert v-model="alert.show" dismissible :variant="alert.type">
            {{ alert.message }}
        </BootstrapAlert>

        <CardElement v-if="inverters.length < 1" :text="$t('inverteradmin.AddInverter')" textVariant="text-bg-primary">
<!-- v-if="inverters.length < 1" damit eingabe-cardelement angezeigt wird, wenn noch kein wechserichter angelegt ist-->
            <form class="form-inline" v-on:submit.prevent="onSubmit">
                <div class="form-group">
                    <label>{{ $t('inverteradmin.Serial') }}</label>
                    <input v-model="newInverterData.serial" type="number" class="form-control ml-sm-2 mr-sm-4 my-2"
                        required />
                </div>
                <div class="form-group">
                    <label>{{ $t('inverteradmin.Name') }}</label>
                    <input v-model="newInverterData.name" type="text" class="form-control ml-sm-2 mr-sm-4 my-2"
                        maxlength="31" required />
                </div>
                <div class="ml-auto text-right">
                    <button type="submit" class="btn btn-primary my-2">{{ $t('inverteradmin.Add') }}</button>
                </div>
                <div class="alert alert-secondary" role="alert" v-html="$t('inverteradmin.AddHint')"></div>
            </form>
        </CardElement>

        <CardElement :text="$t('inverteradmin.InverterList')" textVariant="text-bg-primary" add-space>
            <div class="table-responsive">
                <table class="table">
                    <thead>
                        <tr>
                            <th>#</th>
                            <th scope="col">{{ $t('inverteradmin.Status') }}</th>
                            <th>{{ $t('inverteradmin.Serial') }}</th>
                            <th>{{ $t('inverteradmin.Name') }}</th>
                            <th>{{ $t('inverteradmin.Type') }}</th>
                            <th>{{ $t('inverteradmin.Action') }}</th>
                        </tr>
                    </thead>
                    <tbody ref="invList">
                        <tr v-for="inverter in inverters" v-bind:key="inverter.id" :data-id="inverter.id">
                            <td>1</td>
                            <td>
                                <span class="badge" :title="$t('inverteradmin.Receive')" :class="{
                                    'text-bg-warning': !inverter.poll_enable_night,
                                    'text-bg-dark': inverter.poll_enable_night,}"
                                    ><BIconArrowDown v-if="inverter.poll_enable"  /></span>

                                <span class="badge" :title="$t('inverteradmin.Send')" :class="{
                                    'text-bg-warning': !inverter.command_enable_night,
                                    'text-bg-dark': inverter.command_enable_night,}"
                                    ><BIconArrowUp v-if="inverter.command_enable" /></span>
                            </td>
                            <td>{{ inverter.serial }}</td>
                            <td>{{ inverter.name }}</td>
                            <td>{{ inverter.type }}</td>
                            <td>
                                <a href="#" class="icon text-danger" :title="$t('inverteradmin.DeleteInverter')">
                                    <BIconTrash v-on:click="onOpenModal(modalDelete, inverter)" />
                                </a>&nbsp;
                                <a href="#" class="icon" :title="$t('inverteradmin.EditInverter')">
                                    <BIconPencil v-on:click="onOpenModal(modal, inverter)" />
                                </a>
                            </td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </CardElement>
    </BasePage>

    <div class="modal" id="inverterEdit" tabindex="-1">
        <div class="modal-dialog modal-lg">
            <div class="modal-content">
                <div class="modal-header">
                    <h5 class="modal-title">{{ $t('inverteradmin.EditInverter') }}</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                </div>
                <div class="modal-body">
                    <form>
                        <nav>
                            <div class="nav nav-tabs" id="nav-tab" role="tablist">
                                <button class="nav-link active" id="nav-general-tab" data-bs-toggle="tab" data-bs-target="#nav-general"
                                    type="button" role="tab" aria-controls="nav-general" aria-selected="true">{{
                                        $t('inverteradmin.General')
                                    }}</button>
                                <button class="nav-link" id="nav-string-tab" data-bs-toggle="tab" data-bs-target="#nav-string"
                                    type="button" role="tab" aria-controls="nav-string">{{ $t('inverteradmin.String') }}</button>
                                <button class="nav-link" id="nav-advanced-tab" data-bs-toggle="tab" data-bs-target="#nav-advanced"
                                    type="button" role="tab" aria-controls="nav-advanced">{{ $t('inverteradmin.Advanced') }}</button>
                            </div>
                        </nav>
                        <div class="tab-content" id="nav-tabContent">
                            <div class="tab-pane fade show active" id="nav-general" role="tabpanel" aria-labelledby="nav-general-tab" tabindex="0">
                                <div class="mb-3">
                                    <label for="inverter-serial" class="col-form-label">
                                        {{ $t('inverteradmin.InverterSerial') }}
                                    </label>
                                    <input v-model="selectedInverterData.serial" type="number" id="inverter-serial"
                                        class="form-control" />
                                    <label for="inverter-name" class="col-form-label">{{ $t('inverteradmin.InverterName') }}
                                        <BIconInfoCircle v-tooltip :title="$t('inverteradmin.InverterNameHint')" />
                                    </label>
                                    <input v-model="selectedInverterData.name" type="text" id="inverter-name"
                                        class="form-control" maxlength="31" />
<!-- heraus genommen, da für batteriebetrieb nicht relevant
                                    <CardElement :text="$t('inverteradmin.InverterStatus')" addSpace>
                                        <InputElement :label="$t('inverteradmin.PollEnable')"
                                            v-model="selectedInverterData.poll_enable"
                                            type="checkbox" wide />
                                        <InputElement :label="$t('inverteradmin.PollEnableNight')"
                                            v-model="selectedInverterData.poll_enable_night"
                                            type="checkbox" wide/>
                                        <InputElement :label="$t('inverteradmin.CommandEnable')"
                                            v-model="selectedInverterData.command_enable"
                                            type="checkbox" wide/>
                                        <InputElement :label="$t('inverteradmin.CommandEnableNight')"
                                            v-model="selectedInverterData.command_enable_night"
                                            type="checkbox" wide/>
                                    </CardElement>
-->
                                        <div class="alert alert-secondary mt-3" role="alert" v-html="$t('inverteradmin.StatusHint')"></div>
                                </div>
                            </div>

                            <div class="tab-pane fade show" id="nav-string" role="tabpanel" aria-labelledby="nav-string-tab" tabindex="0">
                                <div v-for="(ch, index) in selectedInverterData.channel" :key="`${index}`">
                                    <div class="row g-2">
                                        <div class="col-md">
                                            <label :for="`inverter-name_${index}`" class="col-form-label">
                                                {{ $t('inverteradmin.StringName', { num: index + 1 }) }}
                                                <BIconInfoCircle v-tooltip :title="$t('inverteradmin.StringNameHint')" />
                                            </label>
                                            <div class="d-flex mb-2">
                                                <div class="input-group">
                                                    <input type="text" class="form-control" :id="`inverter-name_${index}`"
                                                        maxlength="31" v-model="ch.name" />
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                    <div class="row g-2">
                                        <div class="col">
                                            <label :for="`inverter-max_${index}`" class="col-form-label">
                                                {{ $t('inverteradmin.StringMaxPower', { num: index + 1 }) }}
                                                <BIconInfoCircle v-tooltip :title="$t('inverteradmin.StringMaxPowerHint')" />
                                            </label>
                                            <div class="d-flex mb-2">
                                                <div class="input-group">
                                                    <input type="number" class="form-control" :id="`inverter-max_${index}`"
                                                        min="0" v-model="ch.max_power"
                                                        :aria-describedby="`inverter-maxDescription_${index} inverter-customizer`" />
                                                    <span class="input-group-text"
                                                        :id="`inverter-maxDescription_${index}`">W<sub>p</sub><sup>*</sup></span>
                                                </div>
                                            </div>
                                        </div>
                                        <div class="col">
                                            <label :for="`inverter-ytoffset_${index}`" class="col-form-label">
                                                {{ $t('inverteradmin.StringYtOffset', { num: index + 1 }) }}
                                                <BIconInfoCircle v-tooltip :title="$t('inverteradmin.StringYtOffsetHint')" />
                                            </label>
                                            <div class="d-flex mb-2">
                                                <div class="input-group">
                                                    <input type="number" class="form-control" :id="`inverter-ytoffset_${index}`"
                                                        min="0" v-model="ch.yield_total_offset"
                                                        :aria-describedby="`inverter-ytoffsetDescription_${index} inverter-customizer`" />
                                                    <span class="input-group-text"
                                                        :id="`inverter-ytoffsetDescription_${index}`">kWh</span>
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                                <div :id="`inverter-customizer`" class="form-text" v-html="$t('inverteradmin.InverterHint')">
                                </div>
                            </div>

                            <div class="tab-pane fade show" id="nav-advanced" role="tabpanel" aria-labelledby="nav-advanced-tab" tabindex="0">
                                <InputElement :label="$t('inverteradmin.ReachableThreshold')"
                                    v-model="selectedInverterData.reachable_threshold"
                                    type="number" min="1" max="100"
                                    :tooltip="$t('inverteradmin.ReachableThresholdHint')" wide />
<!--
                                <InputElement :label="$t('inverteradmin.ZeroRuntime')"
                                    v-model="selectedInverterData.zero_runtime"
                                    type="checkbox"
                                    :tooltip="$t('inverteradmin.ZeroRuntimeHint')" wide/>
                                <InputElement :label="$t('inverteradmin.ZeroDay')"
                                    v-model="selectedInverterData.zero_day"
                                    type="checkbox"
                                    :tooltip="$t('inverteradmin.ZeroDayHint')" wide/>

                                <InputElement :label="$t('inverteradmin.YieldDayCorrection')"
                                    v-model="selectedInverterData.yieldday_correction"
                                    type="checkbox"
                                    :tooltip="$t('inverteradmin.YieldDayCorrectionHint')" wide/>
-->

                            </div>
                        </div>
                    </form>

                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" @click="onCloseModal(modal)"
                        data-bs-dismiss="modal">{{ $t('inverteradmin.Cancel') }}</button>
                    <button type="button" class="btn btn-primary" @click="onEditSubmit">
                        {{ $t('inverteradmin.Save') }}</button>
                </div>
            </div>
        </div>
    </div>

    <div class="modal" id="inverterDelete" tabindex="-1">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <h5 class="modal-title">{{ $t('inverteradmin.DeleteInverter') }}</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                </div>
                <div class="modal-body">
                    {{ $t('inverteradmin.DeleteMsg', {
                            name: selectedInverterData.name,
                            serial: selectedInverterData.serial
                        })
                    }}
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" @click="onCloseModal(modalDelete)"
                        data-bs-dismiss="modal">{{ $t('inverteradmin.Cancel') }}</button>
                    <button type="button" class="btn btn-danger" @click="onDelete">
                        {{ $t('inverteradmin.Delete') }}</button>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import BootstrapAlert from "@/components/BootstrapAlert.vue";
import CardElement from '@/components/CardElement.vue';
import InputElement from '@/components/InputElement.vue';
import Sortable from 'sortablejs';
import { authHeader, handleResponse } from '@/utils/authentication';
import * as bootstrap from 'bootstrap';
import {
    BIconInfoCircle,
    BIconPencil,
    BIconTrash,
    BIconArrowDown,
    BIconArrowUp,
} from 'bootstrap-icons-vue';
import { defineComponent } from 'vue';

declare interface Channel {
    name: string;
    max_power: number;
    yield_total_offset: number;
}

declare interface Inverter {
    id: string;
    serial: number;
    name: string;
    type: string;
    order: number;
    poll_enable: boolean;
    poll_enable_night: boolean;
    command_enable: boolean;
    command_enable_night: boolean;
    reachable_threshold: number;
    zero_runtime: boolean;
    zero_day: boolean;
    yieldday_correction: boolean;
    channel: Array<Channel>;
}

declare interface AlertResponse {
    message: string;
    type: string;
    code: number;
    show: boolean;
}

export default defineComponent({
    components: {
        BasePage,
        BootstrapAlert,
        CardElement,
        InputElement,
        BIconInfoCircle,
        BIconPencil,
        BIconTrash,
        BIconArrowDown,
        BIconArrowUp,
    },
    data() {
        return {
            modal: {} as bootstrap.Modal,
            modalDelete: {} as bootstrap.Modal,
            newInverterData: {} as Inverter,
            selectedInverterData: {} as Inverter,
            inverters: [] as Inverter[],
            dataLoading: true,
            alert: {} as AlertResponse,
            sortable: {} as Sortable,
        };
    },
    mounted() {
        this.modal = new bootstrap.Modal('#inverterEdit');
        this.modalDelete = new bootstrap.Modal('#inverterDelete');
    },
    created() {
        this.getInverters();
    },
    methods: {
        getInverters() {
            this.dataLoading = true;
            fetch("/api/inverter/list", { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.inverters = data.inverter.slice().sort((a : Inverter, b: Inverter) => {
                        return a.order - b.order;
                    });
                    this.dataLoading = false;

                    this.$nextTick(() => {
                        const table = this.$refs.invList as HTMLElement;

                        this.sortable = Sortable.create(table, {
                            sort: true,
                            handle: '.drag-handle',
                            animation: 150,
                            draggable: 'tr',
                        });
                    });
                });
        },
        callInverterApiEndpoint(endpoint: string, jsonData: string) {
            const formData = new FormData();
            formData.append("data", jsonData);

            fetch("/api/inverter/" + endpoint, {
                method: "POST",
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.getInverters();
                    this.alert = data;
                    this.alert.message = this.$t('apiresponse.' + data.code, data.param);
                    this.alert.show = true;
                });
        },
        onSubmit() {
            this.callInverterApiEndpoint("add", JSON.stringify(this.newInverterData));
            this.newInverterData = {} as Inverter;
        },
        onDelete() {
            this.callInverterApiEndpoint("del", JSON.stringify({ id: this.selectedInverterData.id }));
            this.onCloseModal(this.modalDelete);
        },
        onEditSubmit() {
            this.callInverterApiEndpoint("edit", JSON.stringify(this.selectedInverterData));
            this.onCloseModal(this.modal);
        },
        onOpenModal(modal: bootstrap.Modal, inverter: Inverter) {
            // deep copy inverter object for editing/deleting
            this.selectedInverterData = JSON.parse(JSON.stringify(inverter)) as Inverter;
            modal.show();
        },
        onCloseModal(modal: bootstrap.Modal) {
            modal.hide();
        },
        onSaveOrder() {
            this.callInverterApiEndpoint("order", JSON.stringify({ order: this.sortable.toArray() }));
        },
    },
});
</script>

<style>
.drag-handle {
    cursor: grab;
}
</style>