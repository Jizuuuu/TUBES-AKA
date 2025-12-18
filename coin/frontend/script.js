let timeChart = null;
const baseURL = "http://localhost:8080";

async function hitungSekarang() {
    const nominal = document.getElementById("nominal").value;
    const koin = document.getElementById("koin").value;
    
    if (!nominal || !koin) {
        alert("Harap isi nominal dan koin!");
        return;
    }
    
    try {
        const response = await fetch(`${baseURL}/?nominal=${nominal}&koin=${koin}`);
        const data = await response.json();
        
        // Tampilkan hasil
        document.getElementById("hasil-rekursif").innerHTML = 
            `<strong>Jumlah Cara:</strong> ${data.rekursif.hasil}`;
        document.getElementById("waktu-rekursif").innerHTML = 
            `<strong>Waktu Eksekusi:</strong> ${data.rekursif.waktu_ms} mikrodetik`;
        
        document.getElementById("hasil-iteratif").innerHTML = 
            `<strong>Jumlah Cara:</strong> ${data.iteratif.hasil}`;
        document.getElementById("waktu-iteratif").innerHTML = 
            `<strong>Waktu Eksekusi:</strong> ${data.iteratif.waktu_ms} mikrodetik`;
        
    } catch (error) {
        console.error("Error:", error);
        alert("Gagal menghubungi server. Pastikan server C++ berjalan di port 8080.");
    }
}

async function jalankanAnalisisBanyak() {
    const koin = document.getElementById("koin").value;
    const koinArray = koin.split(',').map(Number);
    const dataPoints = [];
    
    // Ambil sampel nominal dari 1 sampai 100 dengan interval
    for (let nominal = 1; nominal <= 100; nominal += 5) {
        try {
            const response = await fetch(`${baseURL}/?nominal=${nominal}&koin=${koin}`);
            const data = await response.json();
            
            dataPoints.push({
                nominal: nominal,
                rekursif: data.rekursif.waktu_ms / 1000, // konversi ke milidetik
                iteratif: data.iteratif.waktu_ms / 1000
            });
            
            console.log(`Nominal ${nominal}: Rekursif=${data.rekursif.waktu_ms}µs, Iteratif=${data.iteratif.waktu_ms}µs`);
        } catch (error) {
            console.error(`Error pada nominal ${nominal}:`, error);
        }
    }
    
    buatGrafik(dataPoints);
}

function buatGrafik(dataPoints) {
    const ctx = document.getElementById('timeChart').getContext('2d');
    
    if (timeChart) {
        timeChart.destroy();
    }
    
    timeChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: dataPoints.map(d => d.nominal),
            datasets: [
                {
                    label: 'Waktu Rekursif (ms)',
                    data: dataPoints.map(d => d.rekursif),
                    borderColor: '#ff6384',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.4,
                    fill: true
                },
                {
                    label: 'Waktu Iteratif (ms)',
                    data: dataPoints.map(d => d.iteratif),
                    borderColor: '#36a2eb',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.4,
                    fill: true
                }
            ]
        },
        options: {
            responsive: true,
            plugins: {
                title: {
                    display: true,
                    text: 'Perbandingan Waktu Eksekusi vs Nominal'
                },
                tooltip: {
                    mode: 'index',
                    intersect: false
                }
            },
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Nominal Target'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Waktu Eksekusi (milidetik)'
                    },
                    beginAtZero: true
                }
            }
        }
    });
}

// Inisialisasi awal
document.addEventListener('DOMContentLoaded', () => {
    console.log("Frontend siap. Pastikan server C++ berjalan di port 8080.");
});