// JavaScript สำหรับ Toggle Switch
const toggleSwitch = document.getElementById('toggleSwitch');

toggleSwitch.addEventListener('change', function () {
    if (this.checked) {
        // เมื่อสลับสถานะไปเป็น On
        console.log('Switch is On');
        // ทำสิ่งที่คุณต้องการเมื่อสลับสถานะไปเป็น On ตามความต้องการ
        document.body.className = "dark-mode";
    } else {
        // เมื่อสลับสถานะไปเป็น Off
        console.log('Switch is Off');
        // ทำสิ่งที่คุณต้องการเมื่อสลับสถานะไปเป็น Off ตามความต้องการ
        document.body.className = 'body';
    }
});

// Firebase configuration
const firebaseConfig = {
    apiKey: "AIzaSyA3AvqCKyjhGPI2RKPH34pVyoT_kqKOvv0",
    authDomain: "YOUR_AUTH_DOMAIN",
    databaseURL: "https://weatherstation-a1501-default-rtdb.asia-southeast1.firebasedatabase.app/",
    projectId: "weatherstation-a1501",
    storageBucket: "YOUR_STORAGE_BUCKET",
    messagingSenderId: "YOUR_MESSAGING_SENDER_ID",
    appId: "YOUR_APP_ID"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Reference to your RTDB node
const temperatureRef = firebase.database().ref('sensor/temperature');
const humidityRef = firebase.database().ref('sensor/humidity');
const gas_resistanceRef = firebase.database().ref('sensor/gas_resistance');
const pressureRef = firebase.database().ref('sensor/pressure');
const uv_intensityRef = firebase.database().ref('sensor/uv_intensity');
const altitudeRef = firebase.database().ref('sensor/altitude');
const pm_1_0Ref = firebase.database().ref('sensor/pm_1_0');
const pm_2_5Ref = firebase.database().ref('sensor/pm_2_5');
const pm_10_0Ref = firebase.database().ref('sensor/pm_10_0');

// Listen for changes in temperature node
temperatureRef.on('value', (snapshot) => {
    const temperature = snapshot.val();
    document.getElementById('temperature').innerText = temperature;
});
// Listen for changes in humidity node
humidityRef.on('value', (snapshot) => {
    const humidity = snapshot.val();
    document.getElementById('humidity').innerText = humidity;
});
// Listen for changes in gas_resistance node
gas_resistanceRef.on('value', (snapshot) => {
    const gas_resistance = snapshot.val();
    document.getElementById('gas_resistance').innerText = gas_resistance;
});
// Listen for changes in pressure node
pressureRef.on('value', (snapshot) => {
    const pressure = snapshot.val();
    document.getElementById('pressure').innerText = pressure;
});
// Listen for changes in uv_intensity node
uv_intensityRef.on('value', (snapshot) => {
    const uv_intensity = snapshot.val();
    document.getElementById('uv_intensity').innerText = uv_intensity;
});
// Listen for changes in altitude node
altitudeRef.on('value', (snapshot) => {
    const altitude = snapshot.val();
    document.getElementById('altitude').innerText = altitude;
});
// Listen for changes in pm_1_0 node
pm_1_0Ref.on('value', (snapshot) => {
    const pm_1_0 = snapshot.val();
    document.getElementById('pm_1_0').innerText = pm_1_0;
});
// Listen for changes in pm_2_5 node
pm_2_5Ref.on('value', (snapshot) => {
    const pm_2_5 = snapshot.val();
    document.getElementById('pm_2_5').innerText = pm_2_5;
});
// Listen for changes in pm_10_0 node
pm_10_0Ref.on('value', (snapshot) => {
    const pm_10_0 = snapshot.val();
    document.getElementById('pm_10_0').innerText = pm_10_0;
});