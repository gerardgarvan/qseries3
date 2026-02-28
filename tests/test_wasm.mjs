// Comprehensive Node.js test for qseries Wasm evaluate() API
// Usage: node tests/test_wasm.mjs
// Requires: build/wasm/qseries.js (built via `make wasm`)

import { fileURLToPath, pathToFileURL } from 'url';
import { dirname, join } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

let passed = 0, failed = 0;

function test(name, condition) {
    if (condition) { console.log(`PASS: ${name}`); passed++; }
    else { console.log(`FAIL: ${name}`); failed++; }
}

const wasmPath = join(__dirname, '..', 'build', 'wasm', 'qseries.js');
const createQSeries = (await import(pathToFileURL(wasmPath).href)).default;
const Module = await createQSeries();

// 1. Banner
test("get_banner() === 'qseries v2.0'",
    Module.get_banner() === "qseries v2.0");

// 2. Basic arithmetic
test("evaluate('1+1') contains '2'",
    Module.evaluate("1+1").includes("2"));
test("evaluate('3*7') contains '21'",
    Module.evaluate("3*7").includes("21"));

// 3. Series creation
{
    const r = Module.evaluate("etaq(1,20)");
    test("etaq(1,20) does not start with 'error:'",
        !r.startsWith("error:"));
    test("etaq(1,20) contains 'q'",
        r.includes("q"));
}

// 4. Error handling (critical — WASM-02)
test("etaq(0,50) starts with 'error:'",
    Module.evaluate("etaq(0,50)").startsWith("error:"));
test("1/0 starts with 'error:'",
    Module.evaluate("1/0").startsWith("error:"));
test("xyz_undefined starts with 'error:'",
    Module.evaluate("xyz_undefined").startsWith("error:"));
test("sift(etaq(1,50),0,0,50) starts with 'error:'",
    Module.evaluate("sift(etaq(1,50),0,0,50)").startsWith("error:"));

// 5. Statefulness
{
    Module.evaluate("x := etaq(1,20)");
    const r = Module.evaluate("prodmake(x,15)");
    test("prodmake(x,15) after assignment does not error",
        !r.startsWith("error:"));
}

// 6. Rogers-Ramanujan (core acceptance)
{
    const rr = Module.evaluate("rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)");
    test("Rogers-Ramanujan sum assignment — no error",
        !rr.startsWith("error:"));
    const pm = Module.evaluate("prodmake(rr,40)");
    test("prodmake(rr,40) contains '(1-q' (product form)",
        pm.includes("(1-q"));
}

// 7. Theta functions
test("theta3(20) does not start with 'error:'",
    !Module.evaluate("theta3(20)").startsWith("error:"));

// 8. Suppress output (colon)
test("evaluate('y := etaq(1,20):') returns '' (suppressed)",
    Module.evaluate("y := etaq(1,20):") === "");

// 9. set_trunc
{
    const st = Module.evaluate("set_trunc(30)");
    test("set_trunc(30) — no error",
        !st.startsWith("error:"));
    const et = Module.evaluate("etaq(1,30)");
    test("etaq(1,30) after set_trunc — no error, contains 'q'",
        !et.startsWith("error:") && et.includes("q"));
}

// 10. help()
{
    const h = Module.evaluate("help(etaq)");
    test("help(etaq) returns non-empty, no error",
        h.length > 0 && !h.startsWith("error:"));
}

// Summary
console.log(`\n${passed}/${passed + failed} tests passed`);
process.exit(failed === 0 ? 0 : 1);
