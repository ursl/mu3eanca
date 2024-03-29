#!/usr/bin/env node

// ----------------------------------------------------------------------
// playground 0
// ----------------------------------------------------------------------

console.log("Hello world");

// -- String interpolation: must have backticks!
const name = 'Lev', time = 'today';
console.log(`Hello ${name}, how are you ${time}?`);

// -- class declaration 
class Rectangle {
    let width = 0;
    let height = 0;
    
    constructor(width, height) {
        this.height = height;
        this.width = width;
    }
}

let a = new Rectangle(100, 20);
console.log(`original a = ${a.width} x ${a.height}`);
a.height = 25;
console.log(`modified a = ${a.width} x ${a.height}`);

let b = new Rectangle(100); // not what I expected
console.log(`original b = ${b.width} x ${b.height}`);


// -- static vs normal methods?
class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
    
    static displayName = "Point";
    static sdistance(a, b) {
        const dx = a.x - b.x;
        const dy = a.y - b.y;
        
        return Math.hypot(dx, dy);
    }

    distance(b) {
        const dx = this.x - b.x;
        const dy = this.y - b.y;
        
        return Math.hypot(dx, dy);
    }
}

const p1 = new Point(5, 5);
const p2 = new Point(10, 10);

console.log(`static distance d = ${Point.sdistance(p1, p2)}`);
console.log(`method distance d = ${p1.distance(p2)}`);

// sys.exec
// 
// process.createChildProcess("cat");
// cat.addListener("output", function(data) {
//   if (data) sys.puts(data);
// });
//
