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
const f1 = function (x) {
  return x * x * x;
};

const numbers = [0, 1, 2, 5, 10];
const cube = map(f1, numbers);
console.log(cube);


// -- with named function definition
function f2(x) {
  return x * x * x;
};

const numbers2 = [0, 1, 2, 5, 10, 20];
const cube2 = map(f2, numbers2);
console.log("f2: " + cube2);

// A nested function also forms a closure. A closure is an expression (most commonly, a function)
// that can have free variables together with an environment that binds those variables
// (that "closes" the expression). 
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Functions

// -- capturing the variable "by reference"
// https://en.wikipedia.org/wiki/Closure_(computer_programming)
var f, g;
function foo() {
    var x;
    f = function() { return ++x; };
    g = function() { return --x; };
    x = 1;
    console.log('inside foo, call to f(): ' + f());
}
foo();  // 2
console.log('call to g(): ' + g());  // 1 (--x)
console.log('call to g(): ' + g());  // 0 (--x)
console.log('call to f(): ' + f());  // 1 (++x)
console.log('call to f(): ' + f());  // 2 (++x)


// "preservation of variables"?
function outside(x) {
    function inside(y) {
        return x + y;  // how is y passed into this function?
    }
    return inside;
    // nothing below works
    // function inside2(y, z) {
    //     return x + y + z;  
    // }
    //    return inside2;
    //    return inside(y)+x;  // This does not work. y unknown. True. 
}

const fnInside = outside(3); // Think of it like: give me a function that adds 3 to whatever you give it
console.log(fnInside(5));    // returns 8
console.log(outside(3)(5));  // returns 8

function outside2(x) {
    function inside2(y) {
        function inside3(z) {
            return x + y + z;
        }
        return inside3; // essential!
    }
    return inside2;     // essential!
}

console.log(outside2(3)(5)(7));



function getThis() {
  return this;
}

const obj1 = { name: "obj1" };
const obj2 = { name: "obj2" };

obj1.getThis = getThis;
obj2.getThis = getThis;

console.log(obj1.getThis()); // { name: 'obj1', getThis: [Function: getThis] }
console.log(obj2.getThis()); // { name: 'obj2', getThis: [Function: getThis] }

console.log("x in obj1:");
for (x in obj1) {
    console.log(`obj1.${x} = ${obj1[x]}`);
}
