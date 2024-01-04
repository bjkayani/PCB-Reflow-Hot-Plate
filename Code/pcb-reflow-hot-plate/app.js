// Wait for the document to be ready
document.addEventListener("DOMContentLoaded", function () {
    // Get references to various elements
    const reflowBtn = document.getElementById("reflowBtn");
    const heatBtn = document.getElementById("heatBtn");
    const modeOptions = document.getElementById("modeOptions");
    const reflowOptions = document.getElementById("reflowOptions");
    const heatOptions = document.getElementById("heatOptions");
    const profilePoints = document.getElementById("profilePoints");

    // Initialize the Chart.js graph
    const ctx = document.getElementById("temperatureChart").getContext("2d");
    const temperatureChart = new Chart(ctx, {
        type: "line",
        data: {
            labels: [], // Initial labels (empty)
            datasets: [
                {
                    label: "Current Temperature",
                    borderColor: "red",
                    data: [], // Initial data (empty)
                },
            ],
        },
        options: {
            scales: {
                x: [
                    {
                        type: "linear",
                        position: "bottom",
                        ticks: {
                            min: 0,
                            max: 600, // 10 minutes in seconds
                            stepSize: 60, // 1 minute interval
                            callback: function (value) {
                                const minutes = Math.floor(value / 60);
                                const seconds = value % 60;
                                return `${minutes}:${seconds < 10 ? "0" : ""}${seconds}`;
                            },
                        },
                    },
                ],
                y: [
                    {
                        ticks: {
                            min: 20,
                            max: 260,
                        },
                    },
                ],
            },
        },
    });

    // Event listener for Reflow button click
    reflowBtn.addEventListener("click", function () {
        // Show Reflow mode options, hide Heat mode options
        modeOptions.style.display = "block";
        reflowOptions.style.display = "block";
        heatOptions.style.display = "none";

        // Generate textboxes for Set Point and Time for Reflow Profile
        generateReflowProfileOptions();

        reflowBtn.classList.add("active-button");
        heatBtn.classList.remove("active-button");
    });

    // Event listener for Heat button click
    heatBtn.addEventListener("click", function () {
        // Show Heat mode options, hide Reflow mode options
        modeOptions.style.display = "block";
        reflowOptions.style.display = "none";
        heatOptions.style.display = "block";

        heatBtn.classList.add("active-button");
        reflowBtn.classList.remove("active-button");
    });

    document.getElementById("onOffBtn").addEventListener("click", function () {
        const onOffButton = document.getElementById("onOffBtn");
        if (onOffButton.textContent === "On") {
            onOffButton.textContent = "Off";
            onOffButton.classList.remove("btn-success");
            onOffButton.classList.add("btn-danger");
        } else {
            onOffButton.textContent = "On";
            onOffButton.classList.remove("btn-danger");
            onOffButton.classList.add("btn-success");
        }
    });

    // Function to generate textboxes for Reflow Profile options
    function generateReflowProfileOptions() {
        // Clear any existing profile points
        profilePoints.innerHTML = "";

        // Create textboxes for Set Point and Time for each of the 10 points
        for (let i = 1; i <= 10; i++) {
            const setPointInput = document.createElement("input");
            setPointInput.setAttribute("type", "text");
            setPointInput.setAttribute("class", "form-control mb-2");
            setPointInput.setAttribute("placeholder", "Set Point " + i);

            const timeInput = document.createElement("input");
            timeInput.setAttribute("type", "text");
            timeInput.setAttribute("class", "form-control mb-2");
            timeInput.setAttribute("placeholder", "Time " + i);

            profilePoints.appendChild(setPointInput);
            profilePoints.appendChild(timeInput);
        }
    }

    // Function to add a single data point to the graph with a timestamp
    const addDataPointWithTimestamp = (temperature) => {
        // Generate a timestamp for the current time
        const now = new Date();
        const timestamp = now.getMinutes() + ":" + (now.getSeconds() < 10 ? "0" : "") + now.getSeconds();

        // Add the timestamp and data point to the Chart.js graph
        temperatureChart.data.labels.push(timestamp);
        temperatureChart.data.datasets[0].data.push(temperature);
        temperatureChart.update();
    };

    // Example usage: Call addDataPointWithTimestamp to add new data with a timestamp
    // Replace this with your actual data source or event listener
    const exampleDataPoints = [100, 105, 110, 115, 120, 125, 130, 135, 140, 145];

    // Simulate adding data points at intervals (for demonstration purposes)
    let currentIndex = 0;
    const dataInterval = setInterval(() => {
        if (currentIndex < exampleDataPoints.length) {
            const temperature = exampleDataPoints[currentIndex];
            addDataPointWithTimestamp(temperature);
            currentIndex++;
        } else {
            clearInterval(dataInterval);
        }
    }, 1000); // Add data point every 1 second (adjust as needed)
});
