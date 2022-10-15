{#
Copyright 2021 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
#}

function triggerGC() {
  for (let i = 0; i < 50; i++) {
    new ArrayBuffer(1024*1024);
  }
}

const typedArrays = new Array(64);
typedArrays.fill(Object);
triggerGC();
// TODO: this can be a prefilled array
const leakMe = [];
for (let i = 0; i < 554; i++) {
  leakMe[i] = 0;
}
triggerGC();
for (let i = 0; i < typedArrays.length; i++) {
  typedArrays[i] = new Uint8Array(0x20);
  triggerGC();
}
triggerGC();
