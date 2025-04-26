const { Util } = require("./util.js");

const t_a = {a: {a: 'a', b: 'b'}, b: 1, c: [1, 2, 3]};
const t_b = {a: {a: 'a', c: 'c'}, b: 1};
const t_c = {a: {a: 'a', b: 'b'}, b: 2};
const t_d = {a: {a: 'a', b: 'b'}, b: 1, c: [1, 2, 5]};
const t_e = {a: {a: 'a', b: 'b', c: {c: 'c'}}, b: 1, c: [1, 2, 3, 5]};

console.log(Util.contain(t_a, t_b));
console.log("--------------------------------------");
console.log(Util.contain(t_a, t_b, false));
console.log("--------------------------------------");
console.log(Util.contain(Util.contain(t_e, t_a, false), t_a, false));
