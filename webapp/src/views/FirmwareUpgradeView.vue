<template>
    <BasePage :title="$t('firmwareupgrade.FirmwareUpgrade')">
        <div class="position-relative" v-if="loading">
            <div class="position-absolute top-50 start-50 translate-middle">
                <div class="spinner-border" role="status">
                    <span class="visually-hidden">{{ $t('firmwareupgrade.Loading') }}</span>
                </div>
            </div>
        </div>

        <CardElement :text="$t('firmwareupgrade.OtaError')" textVariant="text-bg-danger" center-content
        v-if="!loading && !uploading && OTAError != ''">
            <p class="h1 mb-2">
                <BIconExclamationCircleFill />
            </p>
            <span style="vertical-align: middle" class="ml-2">
                {{ OTAError }}
            </span>
            <br />
            <br />
            <button class="btn btn-light" @click="clear">
                <BIconArrowLeft /> {{ $t('firmwareupgrade.Back') }}
            </button>
            <button class="btn btn-primary" @click="retryOTA">
                <BIconArrowRepeat /> {{ $t('firmwareupgrade.Retry') }}
            </button>
        </CardElement>

        <CardElement :text="$t('firmwareupgrade.OtaStatus')" textVariant="text-bg-success" center-content
        v-else-if="!loading && !uploading && OTASuccess">
            <span class="h1 mb-2">
                <BIconCheckCircle />
            </span>
            <span>&nbsp;{{ $t('firmwareupgrade.OtaSuccess') }} </span>
            <br />
            <br />
            <button class="btn btn-primary" @click="clear">
                <BIconArrowLeft /> {{ $t('firmwareupgrade.Back') }}
            </button>
        </CardElement>

<!-- ######################## CardElement mit file-input #########################################################-->
        <CardElement :text="$t('firmwareupgrade.FirmwareUpload')" textVariant="text-bg-primary" center-content
        v-else-if="!loading && !uploading">
            <div class="form-group pt-2 mt-3" style="text-align:left">
                {{$t('firmwareupgrade.FirmwareUploadHint')}}
<!--                <input class="form-control" type="file" ref="file" accept=".bin,.bin.gz" @change="uploadOTA" />-->
                <input class="form-control" type="file" ref="file" @change="uploadOTA" />
            </div>
        </CardElement>

        <CardElement :text="$t('firmwareupgrade.UploadProgress')" textVariant="text-bg-primary" center-content
        v-else-if="!loading && uploading">
            <div class="progress">
                <div class="progress-bar" role="progressbar" :style="{ width: progress + '%' }"
                    v-bind:aria-valuenow="progress" aria-valuemin="0" aria-valuemax="100">
                    {{ progress }}%
                </div>
            </div>
        </CardElement>

    </BasePage>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import CardElement from '@/components/CardElement.vue';
import { authHeader, isLoggedIn } from '@/utils/authentication';
import {
    BIconArrowLeft,
    BIconArrowRepeat,
    BIconCheckCircle,
    BIconExclamationCircleFill
} from 'bootstrap-icons-vue';
import SparkMD5 from "spark-md5";
import { defineComponent } from 'vue';
import { nextTick } from 'vue';

export default defineComponent({
    components: {
        BasePage,
        CardElement,
        BIconArrowLeft,
        BIconArrowRepeat,
        BIconCheckCircle,
        BIconExclamationCircleFill,
    },
    data() {
        return {
            loading: true,
            uploading: false,
            progress: 0,
            OTAError: "",
            OTASuccess: false,
            type: "firmware",
            file: {} as Blob,
        };
    },
    methods: {
        fileMD5(file: Blob) {
            return new Promise((resolve, reject) => {
                const blobSlice = File.prototype.slice;
                const chunkSize = 2097152; // Read in chunks of 2MB
                const chunks = Math.ceil(file.size / chunkSize);
                const spark = new SparkMD5.ArrayBuffer();
                const fileReader = new FileReader();
                let currentChunk = 0;
                fileReader.onload = () => {
                    spark.append(fileReader.result as ArrayBuffer); // Append array buffer
                    currentChunk += 1;
                    if (currentChunk < chunks) {
                        loadNext();
                    } else {
                        const md5 = spark.end();
                        resolve(md5);
                    }
                };
                fileReader.onerror = (e) => {
                    reject(e);
                };
                const loadNext = () => {
                    const start = currentChunk * chunkSize;
                    const end =
                        start + chunkSize >= file.size ? file.size : start + chunkSize;
                    fileReader.readAsArrayBuffer(blobSlice.call(file, start, end));
                };
                loadNext();
            });
        },
        uploadOTA(event: Event | null) {
            this.uploading = true;
            const formData = new FormData();
            if (event !== null) {
                const target = event.target as HTMLInputElement;
                if (target.files !== null) {
                    this.file = target.files[0];
                }
            }
            const request = new XMLHttpRequest();
            request.addEventListener("load", () => {
                // request.response will hold the response from the server
                if (request.status === 200) {
                    this.OTASuccess = true;
                } else if (request.status !== 500) {
                    this.OTAError = `[HTTP ERROR] ${request.statusText}`;
                } else {
                    this.OTAError = request.responseText;
                }
                this.uploading = false;
                this.progress = 0;
            });
            // Upload progress
            request.upload.addEventListener("progress", (e) => {
                this.progress = Math.trunc((e.loaded / e.total) * 100);
            });
            request.withCredentials = true;
            this.fileMD5(this.file)
            .then((md5) => {
               formData.append("MD5", (md5 as string));
               formData.append("firmware", this.file, "firmware");
               request.open("post", "/api/firmware/update");
               authHeader().forEach((value, key) => {
                  request.setRequestHeader(key, value);
               });
               request.send(formData);
            })
            .catch(() => {
               this.OTAError =
                  "Unknown error while upload, check the console for details.";
               this.uploading = false;
               this.progress = 0;
            });
        },
        retryOTA() {
            this.OTAError = "";
            this.OTASuccess = false;
            this.uploadOTA(null);
        },
        clear() {
            this.OTAError = "";
            this.OTASuccess = false;
        },
    },
    mounted() {
        nextTick(() => {
            const fileInput = this.$refs.file as HTMLInputElement | undefined;
            if (fileInput) {
                fileInput.setAttribute("accept", ".bin,.bin.gz"); // damit nur diese files angeboten werden
            } else {
                console.error("File input ref is not available.");
            }
        });

        if (!isLoggedIn()) {
            this.$router.push({ path: "/login", query: { returnUrl: this.$router.currentRoute.value.fullPath } });
        }
        this.loading = false;
    },
});
</script>