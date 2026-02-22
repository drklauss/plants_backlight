#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "config.h"

ESP8266WebServer server(80);

// Внешние переменные
extern float currentLux;
extern bool isBacklightOn;
extern int8_t START_HOUR;
extern int8_t END_HOUR;
extern float LUX_THRESHOLD;
extern uint32_t systemUptime;
extern uint8_t currentLogLevel;

// HTML страница в PROGMEM (Flash память)
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP Light Controller</title>
    
    <!-- Bootstrap 5 CSS и иконки -->
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.0/font/bootstrap-icons.css">
    
    <style>
        /* Плавный переход между темами */
        body {
            transition: background-color 0.3s ease, color 0.3s ease;
        }
        
        /* Анимация переключателя */
        .theme-toggle {
            cursor: pointer;
            padding: 0.5rem;
            border-radius: 50%;
            transition: background-color 0.3s ease;
        }
        
        .theme-toggle:hover {
            background-color: rgba(128, 128, 128, 0.1);
        }
        
        /* Стили для карточек при смене темы */
        .card {
            transition: background-color 0.3s ease, border-color 0.3s ease;
        }
        
        /* Анимация загрузки */
        .spinner {
            display: inline-block;
            width: 1rem;
            height: 1rem;
            border: 2px solid currentColor;
            border-right-color: transparent;
            border-radius: 50%;
            animation: spinner 0.75s linear infinite;
        }
        
        @keyframes spinner {
            to { transform: rotate(360deg); }
        }
    </style>
</head>
<body data-bs-theme="light">

    <!-- Навигационная панель с переключателем темы -->
    <nav class="navbar navbar-expand-lg bg-body-tertiary mb-4">
        <div class="container">
            <a class="navbar-brand" href="#">
                <i class="bi bi-sun-fill me-2"></i>
                ESP Light Controller
            </a>
            
            <div class="d-flex align-items-center">
                <!-- Переключатель темы -->
                <div class="theme-toggle me-3" onclick="toggleTheme()" title="Сменить тему">
                    <i class="bi bi-sun-fill" id="themeIcon"></i>
                </div>
                
                <span class="navbar-text">
                    <small class="text-body-secondary">v{FIRMWARE_VERSION}</small>
                </span>
            </div>
        </div>
    </nav>

    <div class="container">
        <!-- Карточки со статистикой -->
        <div class="row g-4 mb-5">
            <!-- Освещенность -->
            <div class="col-md-4">
                <div class="card shadow-sm h-100">
                    <div class="card-body text-center p-4">
                        <div class="display-4 text-primary mb-3">
                            <i class="bi bi-brightness-high-fill"></i>
                        </div>
                        <h6 class="text-body-secondary mb-2">ОСВЕЩЕННОСТЬ</h6>
                        <h2 class="display-5 fw-bold mb-0" id="lux">---</h2>
                        <p class="text-body-secondary">люкс</p>
                    </div>
                </div>
            </div>

            <!-- Статус подсветки -->
            <div class="col-md-4">
                <div class="card shadow-sm h-100">
                    <div class="card-body text-center p-4">
                        <div class="display-4 text-primary mb-3">
                            <i class="bi bi-lightbulb-fill"></i>
                        </div>
                        <h6 class="text-body-secondary mb-2">ПОДСВЕТКА</h6>
                        <div id="enabled">
                            <h2 class="display-5 fw-bold mb-0 text-secondary">---</h2>
                        </div>
                        <p class="text-body-secondary">статус</p>
                    </div>
                </div>
            </div>

            <!-- Качество сигнала -->
            <div class="col-md-4">
                <div class="card shadow-sm h-100">
                    <div class="card-body text-center p-4">
                        <div class="display-4 text-primary mb-3">
                            <i class="bi bi-wifi"></i>
                        </div>
                        <h6 class="text-body-secondary mb-2">СИГНАЛ</h6>
                        <h2 class="display-5 fw-bold mb-0" id="rssi">---</h2>
                        <p class="text-body-secondary">dBm</p>
                    </div>
                </div>
            </div>
        </div>

        <!-- Основная карточка с табами -->
        <div class="card shadow-sm border-0 mb-5">
            <div class="card-body p-4">
                <!-- Навигация по табам -->
                <ul class="nav nav-tabs nav-fill mb-4" role="tablist">
                    <li class="nav-item" role="presentation">
                        <button class="nav-link active" id="light-tab" data-bs-toggle="tab" data-bs-target="#light" type="button" role="tab">
                            <i class="bi bi-sun me-2"></i>Подсветка
                        </button>
                    </li>
                    <li class="nav-item" role="presentation">
                        <button class="nav-link" id="wifi-tab" data-bs-toggle="tab" data-bs-target="#wifi" type="button" role="tab">
                            <i class="bi bi-wifi me-2"></i>WiFi
                        </button>
                    </li>
                    <li class="nav-item" role="presentation">
                        <button class="nav-link" id="system-tab" data-bs-toggle="tab" data-bs-target="#system" type="button" role="tab">
                            <i class="bi bi-hdd-stack me-2"></i>Система
                        </button>
                    </li>
                </ul>

                <!-- Содержимое табов -->
                <div class="tab-content">
                    <!-- ТАБ 1: Настройки подсветки -->
                    <div class="tab-pane fade show active" id="light" role="tabpanel">
                        <form id="lightForm">
                            <div class="row g-4">
                                <div class="col-md-4">
                                    <label class="form-label text-body-secondary">
                                        <i class="bi bi-sunrise me-2"></i>Включение
                                    </label>
                                    <div class="input-group">
                                        <input type="number" class="form-control" name="start_h" min="0" max="23" value="{START_HOUR}">
                                        <span class="input-group-text">:00</span>
                                    </div>
                                </div>
                                <div class="col-md-4">
                                    <label class="form-label text-body-secondary">
                                        <i class="bi bi-sunset me-2"></i>Выключение
                                    </label>
                                    <div class="input-group">
                                        <input type="number" class="form-control" name="end_h" min="0" max="23" value="{END_HOUR}">
                                        <span class="input-group-text">:00</span>
                                    </div>
                                </div>
                                <div class="col-md-4">
                                    <label class="form-label text-body-secondary">
                                        <i class="bi bi-sliders2 me-2"></i>Порог
                                    </label>
                                    <div class="input-group">
                                        <input type="number" class="form-control" name="threshold" min="0" max="10000" value="{THRESHOLD}">
                                        <span class="input-group-text">lux</span>
                                    </div>
                                </div>
                            </div>
                            <div class="text-end mt-4">
                                <button type="submit" class="btn btn-primary px-5">
                                    <i class="bi bi-check-lg me-2"></i>Сохранить
                                </button>
                            </div>
                        </form>
                    </div>

                    <!-- ТАБ 2: Настройки WiFi -->
                    <div class="tab-pane fade" id="wifi" role="tabpanel">
                        <div class="row g-4">
                            <div class="col-md-6">
                                <h5 class="mb-3"><i class="bi bi-search me-2"></i>Доступные сети</h5>
                                <div class="list-group mb-3" id="networks">
                                    <div class="list-group-item text-center text-body-secondary">
                                        <i class="bi bi-info-circle me-2"></i>Нажмите "Сканировать сети"
                                    </div>
                                </div>
                                <button class="btn btn-outline-primary" onclick="scanWiFi()">
                                    <i class="bi bi-arrow-repeat me-2"></i>Сканировать сети
                                </button>
                            </div>
                            <div class="col-md-6">
                                <h5 class="mb-3"><i class="bi bi-key me-2"></i>Подключение</h5>
                                <form id="wifiForm">
                                    <div class="mb-3">
                                        <label class="form-label">Выбранная сеть</label>
                                        <input type="text" class="form-control" name="ssid" id="ssid" readonly>
                                    </div>
                                    <div class="mb-3">
                                        <label class="form-label">Пароль</label>
                                        <div class="input-group">
                                            <input type="password" class="form-control" name="password" id="password">
                                            <button class="btn btn-outline-secondary" type="button" onclick="togglePassword()">
                                                <i class="bi bi-eye" id="toggleIcon"></i>
                                            </button>
                                        </div>
                                    </div>
                                    <button type="submit" class="btn btn-primary">
                                        <i class="bi bi-wifi me-2"></i>Подключиться
                                    </button>
                                </form>
                            </div>
                        </div>
                    </div>

                  <!-- ТАБ 3: Системная информация -->
                    <div class="tab-pane fade" id="system" role="tabpanel">
                        <div class="row g-4">
                            <div class="col-md-6">
                                <h5 class="mb-3"><i class="bi bi-info-circle me-2"></i>Информация</h5>
                                <div class="list-group">
                                    <div class="list-group-item d-flex justify-content-between align-items-center">
                                        <span><i class="bi bi-clock me-2"></i>Время работы</span>
                                        <span class="badge bg-primary rounded-pill" id="uptime">---</span>
                                    </div>
                                    <div class="list-group-item d-flex justify-content-between align-items-center">
                                        <span><i class="bi bi-hdd-network me-2"></i>IP адрес</span>
                                        <span class="badge bg-secondary rounded-pill" id="ip">---</span>
                                    </div>
                                    <div class="list-group-item d-flex justify-content-between align-items-center">
                                        <span><i class="bi bi-memory me-2"></i>Свободно памяти</span>
                                        <span class="badge bg-info rounded-pill" id="freeHeap">---</span>
                                    </div>
                                    <div class="list-group-item d-flex justify-content-between align-items-center">
                                        <span><i class="bi bi-tag me-2"></i>Версия</span>
                                        <span class="badge bg-secondary rounded-pill">{FIRMWARE_VERSION}</span>
                                    </div>
                                    <!-- НОВОЕ: Отображение текущего уровня логирования -->
                                    <div class="list-group-item d-flex justify-content-between align-items-center">
                                        <span><i class="bi bi-terminal me-2"></i>Уровень логов</span>
                                        <span class="badge bg-secondary rounded-pill" id="currentLogLevelDisplay">---</span>
                                    </div>
                                </div>
                            </div>
                            
                            <div class="col-md-6">
                                <h5 class="mb-3"><i class="bi bi-gear me-2"></i>Управление</h5>
                                <div class="list-group">
                                    <div class="list-group-item">
                                        <label class="form-label fw-bold">Изменить уровень логирования</label>
                                        <select class="form-select" id="logLevelSelector" onchange="changeLogLevel(this.value)">
                                            <option value="0">🔇 Выключено</option>
                                            <option value="1">🔴 Ошибки</option>
                                            <option value="2" selected>🟢 Информация</option>
                                            <option value="3">🔵 Отладка</option>
                                        </select>
                                        <small class="text-body-secondary">Требуется перезагрузка</small>
                                    </div>
                                    
                                    <div class="list-group-item">
                                        <button class="btn btn-outline-danger w-100" onclick="restartESP()">
                                            <i class="bi bi-arrow-repeat me-2"></i>Перезагрузить устройство
                                        </button>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <!-- Футер -->
        <footer class="text-center text-body-secondary mb-4">
            <small>ESP8266 Plant Light Controller • Данные обновляются каждые 5 секунд</small>
        </footer>
    </div>

    <!-- Bootstrap JS -->
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
    
    <script>
        // Переключение темы
        function toggleTheme() {
            const html = document.body;
            const currentTheme = html.getAttribute('data-bs-theme');
            const newTheme = currentTheme === 'light' ? 'dark' : 'light';
            
            html.setAttribute('data-bs-theme', newTheme);
            
            // Меняем иконку
            const themeIcon = document.getElementById('themeIcon');
            themeIcon.className = newTheme === 'light' ? 'bi bi-sun-fill' : 'bi bi-moon-stars-fill';
            
            // Сохраняем выбор в localStorage
            localStorage.setItem('theme', newTheme);
        }
        
        // Загрузка сохраненной темы
        function loadTheme() {
            const savedTheme = localStorage.getItem('theme') || 'light';
            document.body.setAttribute('data-bs-theme', savedTheme);
            
            const themeIcon = document.getElementById('themeIcon');
            themeIcon.className = savedTheme === 'light' ? 'bi bi-sun-fill' : 'bi bi-moon-stars-fill';
        }
        
        // Обновление статистики с сервера
        function updateStats() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('lux').textContent = data.lux;
                    document.getElementById('rssi').textContent = data.rssi;
                    
                    // Статус подсветки
                    const enabledDiv = document.getElementById('enabled');
                    if (data.enabled) {
                        enabledDiv.innerHTML = '<h2 class="display-5 fw-bold mb-0 text-success">ВКЛ</h2>';
                    } else {
                        enabledDiv.innerHTML = '<h2 class="display-5 fw-bold mb-0 text-danger">ВЫКЛ</h2>';
                    }
                    
                    // Uptime
                    let uptime = '';
                    const days = Math.floor(data.uptime / 86400);
                    const hours = Math.floor((data.uptime % 86400) / 3600);
                    const mins = Math.floor((data.uptime % 3600) / 60);
                    if (days > 0) uptime += days + 'д ';
                    uptime += hours + 'ч ' + mins + 'м';
                    document.getElementById('uptime').textContent = uptime;
                    
                    // IP
                    document.getElementById('ip').textContent = data.ip;
                    
                    // Свободная память
                    document.getElementById('freeHeap').textContent = data.freeHeap + ' KB';
                    
                    // НОВОЕ: Отображение текущего уровня логирования
                    const logLevelDisplay = document.getElementById('currentLogLevelDisplay');
                    const logLevelSelector = document.getElementById('logLevelSelector');
                    
                    // Устанавливаем текст в информационной колонке
                    switch(data.logLevel) {
                        case 0:
                            logLevelDisplay.textContent = '🔇 Выключено';
                            break;
                        case 1:
                            logLevelDisplay.textContent = '🔴 Ошибки';
                            break;
                        case 2:
                            logLevelDisplay.textContent = '🟢 Информация';
                            break;
                        case 3:
                            logLevelDisplay.textContent = '🔵 Отладка';
                            break;
                        default:
                            logLevelDisplay.textContent = 'Неизвестно';
                    }
                    
                    // Синхронизируем селектор
                    logLevelSelector.value = data.logLevel;
                })
                .catch(error => {
                    console.log('Error fetching status:', error);
                });
        }

        // Изменение уровня логирования
        function changeLogLevel(level) {
            fetch('/setLogLevel', {
                method: 'POST',
                body: new URLSearchParams({level: level})
            })
            .then(r => r.text())
            .then(msg => {
                alert('✅ Уровень логирования изменен. Перезагрузите устройство для применения.');
            })
            .catch(error => {
                alert('❌ Ошибка при изменении уровня логирования');
            });
        }
        
        // Функции WiFi
        function selectSSID(ssid) {
            document.getElementById('ssid').value = ssid;
        }
        
        function togglePassword() {
            const pwd = document.getElementById('password');
            const icon = document.getElementById('toggleIcon');
            
            if (pwd.type === 'password') {
                pwd.type = 'text';
                icon.classList.remove('bi-eye');
                icon.classList.add('bi-eye-slash');
            } else {
                pwd.type = 'password';
                icon.classList.remove('bi-eye-slash');
                icon.classList.add('bi-eye');
            }
        }
        
        function scanWiFi() {
            const networksDiv = document.getElementById('networks');
            networksDiv.innerHTML = '<div class="list-group-item text-center"><span class="spinner me-2"></span>Сканирование...</div>';
            
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    if (data.status === 'scanning') {
                        // Сканирование еще идет, пробуем снова через секунду
                        setTimeout(scanWiFi, 1000);
                        return;
                    }
                    
                    let html = '';
                    if (data.networks && data.networks.length > 0) {
                        data.networks.forEach(net => {
                            html += `<div class="list-group-item list-group-item-action" onclick="selectSSID('${net.ssid}')">
                                <div class="d-flex justify-content-between align-items-center">
                                    <div>
                                        <i class="bi bi-wifi me-2"></i>
                                        <strong>${net.ssid}</strong>
                                    </div>
                                    <div>
                                        <span class="badge bg-secondary me-2">${net.rssi} dBm</span>
                                        ${net.encrypted ? '<i class="bi bi-lock-fill text-secondary"></i>' : '<i class="bi bi-unlock-fill text-success"></i>'}
                                    </div>
                                </div>
                            </div>`;
                        });
                    } else {
                        html = '<div class="list-group-item text-center text-body-secondary">Сети не найдены</div>';
                    }
                    networksDiv.innerHTML = html;
                })
                .catch(error => {
                    networksDiv.innerHTML = '<div class="list-group-item text-center text-danger">Ошибка сканирования</div>';
                    console.log('Scan error:', error);
                });
        }
        
        // Обработчики форм
        document.getElementById('lightForm').onsubmit = function(e) {
            e.preventDefault();
            
            const formData = new FormData(this);
            const data = new URLSearchParams(formData);
            
            fetch('/save', {
                method: 'POST',
                body: data
            })
            .then(response => response.text())
            .then(result => {
                if (result === 'OK') {
                    alert('✅ Настройки сохранены');
                    updateStats();
                }
            })
            .catch(error => {
                alert('❌ Ошибка сохранения');
                console.log('Save error:', error);
            });
        };
        
        document.getElementById('wifiForm').onsubmit = function(e) {
            e.preventDefault();
            
            const ssid = document.getElementById('ssid').value;
            if (!ssid) {
                alert('❌ Выберите WiFi сеть');
                return;
            }
            
            const formData = new FormData(this);
            const data = new URLSearchParams(formData);
            
            fetch('/connect', {
                method: 'POST',
                body: data
            })
            .then(response => response.text())
            .then(message => {
                alert('✅ ' + message);
                setTimeout(() => {
                    window.location.reload();
                }, 5000);
            })
            .catch(error => {
                alert('❌ Ошибка подключения');
                console.log('Connect error:', error);
            });
        };
        
        // Перезагрузка ESP
        function restartESP() {
            if (confirm('Перезагрузить устройство?')) {
                fetch('/restart')
                    .then(() => {
                        alert('🔄 Устройство перезагружается...');
                        setTimeout(() => {
                            window.location.reload();
                        }, 5000);
                    });
            }
        }
        
        // Инициализация
        loadTheme();
        updateStats();
        setInterval(updateStats, 5000);
    </script>
</body>
</html>
)rawliteral";

// Обработчики веб-сервера
void handleRoot()
{
    String html = FPSTR(INDEX_HTML);  // Загружаем шаблон из PROGMEM
    
    // Заменяем плейсхолдеры на реальные значения
    html.replace("{START_HOUR}", String(START_HOUR));
    html.replace("{END_HOUR}", String(END_HOUR));
    html.replace("{THRESHOLD}", String((int)LUX_THRESHOLD));
    html.replace("{FIRMWARE_VERSION}", FIRMWARE_VERSION);
    
    server.send(200, "text/html", html);
}

void handleStatus() {
    String json = "{";
    json += "\"lux\":" + String((int)currentLux);
    json += ",\"enabled\":" + String(isBacklightOn ? "true" : "false");
    json += ",\"rssi\":" + String(WiFi.RSSI());
    json += ",\"uptime\":" + String(systemUptime);
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += ",\"freeHeap\":" + String(ESP.getFreeHeap() / 1024);
    json += ",\"logLevel\":" + String(currentLogLevel);
    json += "}";
    server.send(200, "application/json", json);
}

void handleSave()
{
    bool changed = false;

    if (server.hasArg("start_h"))
    {
        int newStart = server.arg("start_h").toInt();
        if (newStart >= 0 && newStart <= 23 && newStart != START_HOUR)
        {
            START_HOUR = newStart;
            changed = true;
        }
    }

    if (server.hasArg("end_h"))
    {
        int newEnd = server.arg("end_h").toInt();
        if (newEnd >= 0 && newEnd <= 23 && newEnd != END_HOUR)
        {
            END_HOUR = newEnd;
            changed = true;
        }
    }

    if (server.hasArg("threshold"))
    {
        float newThreshold = server.arg("threshold").toFloat();
        if (newThreshold >= 0 && newThreshold <= 10000 && newThreshold != LUX_THRESHOLD)
        {
            LUX_THRESHOLD = newThreshold;
            changed = true;
        }
    }

    if (changed)
    {
        saveSettings();
    }

    server.send(200, "text/plain", "OK");
}

// Обработчик сканирования WiFi
void handleWiFiScan()
{
    static bool scanning = false;

    if (!scanning)
    {
        // Запускаем асинхронное сканирование
        WiFi.scanNetworks(true);
        scanning = true;
        server.send(202, "application/json", "{\"status\":\"scanning\"}");
        return;
    }

    int n = WiFi.scanComplete();

    if (n == -2)
    {
        // Сканирование не запущено
        scanning = false;
        WiFi.scanNetworks(true);
        server.send(202, "application/json", "{\"status\":\"scanning\"}");
        return;
    }

    if (n == -1)
    {
        // Сканирование еще идет
        server.send(202, "application/json", "{\"status\":\"scanning\"}");
        return;
    }

    // Сканирование завершено
    String json = "{\"networks\":[";

    for (int i = 0; i < n; i++)
    {
        if (i > 0)
            json += ",";

        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encrypted\":" + String(WiFi.encryptionType(i) != ENC_TYPE_NONE ? "true" : "false");
        json += "}";
    }

    json += "]}";

    WiFi.scanDelete();
    scanning = false;

    server.send(200, "application/json", json);
}

// Обработчик подключения к WiFi
void handleWiFiConnect()
{
    if (!server.hasArg("ssid") || !server.hasArg("password"))
    {
        server.send(400, "text/plain", "Missing ssid or password");
        return;
    }

    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() == 0)
    {
        server.send(400, "text/plain", "SSID cannot be empty");
        return;
    }

    LOG_D("WiFi", "📡 Connecting to new WiFi: %s\n", ssid.c_str());

    // Сохраняем новые credentials в EEPROM
    saveWiFiCredentials(ssid, password);

    // Пытаемся подключиться
    WiFi.begin(ssid.c_str(), password.c_str());

    server.send(200, "text/plain", "Connecting to " + ssid + "...");

    // Перезагрузка через 3 секунды для применения новых настроек
    delay(3000);
    ESP.restart();
}

void handleSetLogLevel() {
    if (server.hasArg("level")) {
        uint8_t newLevel = server.arg("level").toInt();
        if (newLevel <= 3) {
            currentLogLevel = newLevel;
            saveSettings();
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Invalid level");
        }
    }
}

void handleRestart()
{
    server.send(200, "text/plain", "Restarting...");
    delay(100);
    ESP.restart();
}

void initWebServer()
{
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/scan", handleWiFiScan);
    server.on("/connect", HTTP_POST, handleWiFiConnect);
    server.on("/setLogLevel", HTTP_POST, handleSetLogLevel);
    server.on("/restart", handleRestart);

    server.begin();

    // Запускаем mDNS
    if (MDNS.begin("esp-light"))
    {
        MDNS.addService("http", "tcp", 80);
        LOG_D("WebServer", "✅ mDNS: http://esp-light.local");
    }

    LOG_D("WebServer", "✅ Web server started");
    LOG_D("WebServer", "   http://%s\n", WiFi.localIP().toString().c_str());
}

void handleWebServer()
{
    server.handleClient();
    MDNS.update();
}

#endif