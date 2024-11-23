var url_string = window.location.href;
var url = new URL(url_string);
var log_type = url.searchParams.get("log"); // ?log=sec

let h1_map = {
    "sec": "Temperature by seconds",
    "hour": "Mean temperature per hour",
    "day": "Mean temperature per day"
}

if (!["sec", "hour", "day"].includes(log_type)) {
    // console.log(url.hostname+"/404.http");
    window.location.replace(url.hostname + "/404.http");
}

fetch(`/${log_type}/raw`)
    .then(response => {
        if (!response.ok) {
            console.log(response);
        }
        return response.text();
    })
    .then(data => {
        // console.log(data);
        el = document.getElementsByClassName("tab")[0]
        el.innerHTML = data.trim().split("\n")
            .map(line => {
                let splt = line.split(" ")
                d = splt[0]
                t = splt[1]
                d = new Date(parseInt(d) * 1000)
                return `<tr><td>${d.toLocaleString()}</td><td>${t}</td></tr>`
            }).join("");
        // .map(line => `<tr>${line.split(" ").map(x => `<td>${x}</td>`).join("")}</tr>`).join("")

        el.insertAdjacentHTML("afterbegin", `<tr><th>Time</th><th>Temp</th></tr>`);
    })
    .catch(e => {
        console.log(e);
        el = document.getElementsByClassName("tab")[0].innerHTML = "No data gathered yet!";
    })
    .finally(() => {
        document.getElementsByTagName("h1")[0].innerHTML = h1_map[log_type]
    })

