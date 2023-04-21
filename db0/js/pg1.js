#!/usr/bin/env node

// ----------------------------------------------------------------------
// playground 1
// ----------------------------------------------------------------------

function map(f, a) {
  const result = new Array(a.length);
  for (let i = 0; i < a.length; i++) {
    result[i] = f(a[i]);
  }
  return result;
}

// -- with function expression
const f = function (x) {
  return x * x * x;
};

const numbers = [0, 1, 2, 5, 10];
const cube = map(f, numbers);
console.log(cube);


// -- with named function definition
function f2(x) {
  return x * x * x;
};

const numbers2 = [0, 1, 2, 5, 10, 20];
const cube2 = map(f2, numbers2);
console.log("f2: " + cube2);
