function Time() {
// Creating object of the Date class\n
    var date = new Date();
    var year = date.getYear();
    var month = date.getMonth();
    var day = date.getDay();
// Get current hour\n
    var hour = date.getHours();
// Get current minute\n
    var minute = date.getMinutes();
// Get current second\n
    var second = date.getSeconds();
// Variable to store AM / PM\n
    var period = "";
// Assigning AM / PM according to the current hour\n
    if (hour >= 12) {
        period = \"PM\";
    } else {
        period = \"AM\";
    }
// Converting the hour in 12-hour format
    if (hour == 0) {
        hour = 12;
    } else {
        if (hour > 12) {
            hour = hour - 12;
        }
    }
// Updating hour, minute, and second
// if they are less than 10
    hour = update(hour);
    minute = update(minute);
    second = update(second);
// Adding time elements to the div
    document.getElementById(\"digital-clock\").innerText = hour + \" : \" + minute + \" : \" + second + \" \" + period;
    document.getElementById('clientOffset').value = date.getTimezoneOffset();
    document.getElementById('clientMillis').value = date.getTime();
// Set Timer to 1 sec (1000 ms)\n
    setTimeout(Time, 1000);     
}

// Function to update time elements if they are less than 10\n
// Append 0 before time elements if they are less than 10\n
function update(t) {
    if (t < 10) {
        return \"0\" + t;
    }
    else {
        return t;
    }
}

Time();