<h1>🌤️ ESP32 Indoor Weather Dashboard</h1>

<p>
A sleek, real-time indoor weather station built with an ESP32-WROOM, BME680 sensor, and a 2.8" ILI9341 TFT display. Features a glitch-style hacker splash screen, time-of-day greetings, dynamic sensor data, and a live web dashboard.
</p>

<h2>⚡ Features</h2>
<ul>
  <li>🖥️ Glitchy purple splash screen that says <code>Made by Chr0nicHacker</code></li>
  <li>⏰ Auto-synced time and date via NTP</li>
  <li>🌡️ Temperature, 💧 Humidity, 🌪️ Air Pressure, 🫁 Air Quality (Gas resistance)</li>
  <li>🎨 Color-coded sensor data (white = normal, orange = warning, red = danger)</li>
  <li>🧠 Smart greetings like “Good Morning” based on time of day</li>
  <li>🌐 Live web dashboard with real-time updates via WebSocket</li>
  <li>📝 Environmental data logged to SPIFFS every 15 minutes</li>
  <li>🔁 Logs automatically trimmed to save space</li>
</ul>

<h2>🧰 Hardware Required</h2>
<ul>
  <li>ESP32-WROOM development board</li>
  <li>BME680 sensor (I2C)</li>
  <li>2.8" ILI9341 SPI TFT Display</li>
  <li>Breadboard + jumper wires</li>
  <li>USB cable + Wi-Fi access</li>
</ul>

<h2>📌 Wiring Guide</h2>
<table>
<thead><tr><th>Component</th><th>ESP32 GPIO</th></tr></thead>
<tbody>
<tr><td>BME680 SDA</td><td>GPIO21</td></tr>
<tr><td>BME680 SCL</td><td>GPIO22</td></tr>
<tr><td>TFT_CS</td><td>GPIO17</td></tr>
<tr><td>TFT_RST</td><td>GPIO5</td></tr>
<tr><td>TFT_DC</td><td>GPIO16</td></tr>
<tr><td>TFT_MOSI</td><td>GPIO23</td></tr>
<tr><td>TFT_SCK</td><td>GPIO18</td></tr>
<tr><td>TFT_MISO</td><td>GPIO19</td></tr>
</tbody>
</table>

<h2>📚 Libraries Required (Install via Library Manager)</h2>
<ul>
  <li>Adafruit BME680</li>
  <li>Adafruit Unified Sensor</li>
  <li>Adafruit GFX</li>
  <li>Adafruit ILI9341</li>
  <li>ESPAsyncWebServer</li>
  <li>SPIFFS (built-in for ESP32)</li>
</ul>

<h2>🛠️ Getting Started</h2>
<ol>
  <li>Clone this repo or download the <code>.ino</code> file</li>
  <li>Open <code>IndoorWeather.ino</code> in Arduino IDE</li>
  <li>Install the libraries listed above</li>
  <li>Enter your Wi-Fi credentials:
    <pre><code>const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";</code></pre></li>
  <li>Select <strong>ESP32 Dev Module</strong> from Tools > Board</li>
  <li>Choose the correct COM port</li>
  <li>Upload the sketch to your ESP32</li>
</ol>

<h2>🔍 Finding the Device’s IP Address</h2>
<ul>
  <li>Open the <strong>Serial Monitor</strong> at <code>115200 baud</code></li>
  <li>Look for output like:
    <pre><code>✅ Connected
192.168.1.42</code></pre></li>
  <li>Open a browser and go to <code>http://192.168.1.42/</code> (your IP may differ)</li>
</ul>

<h2>🖼️ Customizing the Splash Screen</h2>
<p>The splash screen appears on boot with the text:</p>
<pre>Made by Chr0nicHacker</pre>

<p>To customize it:</p>
<ul>
  <li>Open the <code>setup()</code> function</li>
  <li>Find this block:
    <pre><code>tft.setCursor(30, 120);
tft.println("Chr0nicHacker");</code></pre></li>
  <li>Change <code>"Chr0nicHacker"</code> to your name, logo, or phrase</li>
  <li>You can also change the color from <code>ILI9341_MAGENTA</code> to another color like <code>ILI9341_CYAN</code> or <code>ILI9341_WHITE</code></li>
</ul>

<h2>📱 Web Dashboard Features</h2>
<ul>
  <li>See live sensor readings in real time</li>
  <li>Log file updates every 15 minutes</li>
  <li>View/download the log or clear it from your browser</li>
</ul>

<h2>🤖 Author</h2>
<p>Made by <a href="https://github.com/Chr0nicHacker" target="_blank">Chr0nicH@ck3r</a><br>
Hacker-style home automation, one pixel at a time.</p>

<blockquote>💡 Feel free to fork, improve, or glitchify this further!</blockquote>
