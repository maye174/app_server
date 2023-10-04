import { func } from "./func";
import { p_suffix } from "ffi";

export function hello() {
    console.log("Hello World");
}

hello();

var i = 0;
for (; ;) {
    ++i;
    if (i === 100) break;
}


var f = func();
f();

async function f1() {
    console.log("async begin");
    const m = await import("./dynamic_test" + p_suffix);
    console.log("fib(20)=", m.fib(20));
}

f1();
console.log(p_suffix);