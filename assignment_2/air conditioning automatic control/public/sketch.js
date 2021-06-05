var socket;
const FRAME_RATE = 30;

const UPPER_THRESHOLD = 24;
const LOWER_THRESHOLD = 22;
var state = false;
var now_data;

function setup() {
    createCanvas(200, 200);
    background(255);
    ellipseMode(CENTER);
    frameRate(FRAME_RATE);
    state = false;

    socket = io.connect('http://localhost:3000');

    socket.emit('read_data', 1);
    check_state(now_data);

    socket.on("data", (data)=>{
        document.getElementById("data").innerHTML = "temperature : " + data;
        now_data = data;
        check_state(now_data);
    });
}

function draw() {
    background(255);

    push();
    stroke (0);
    translate(width/2, height/2);
    ellipse(0, 0, 150, 150);
    fill(0);
    ellipse(0, 0, 30, 30);

    if (frameCount % FRAME_RATE == 0) console.log(frameCount / FRAME_RATE);

    if (frameCount % (60 * FRAME_RATE) == 0) {
        socket.emit('read_data', 1);
    }

    if (state) rotate(frameCount / FRAME_RATE * PI);
    for (let i = 0; i < 4; i++) {
        arc(0, 0, 120, 120, 0, PI / 4);
        rotate((PI / 2));
    }
    pop();
}

function check_state(data) {
    if (data > UPPER_THRESHOLD && state == false) {
        state = true;
    }
    else if (data < LOWER_THRESHOLD && state == true) {
        state = false;
    }
}
