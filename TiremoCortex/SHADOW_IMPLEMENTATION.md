# Device Shadow — Implementasyon Dokümanı

Bu dosya, TiremoCortex firmware’inde **AWS IoT Classic Device Shadow** cihaz tarafının güncel durumunu anlatır.

Platform (template / bulk / audit): `shadow.txt`  
Kullanım: `SHADOW_KULLANIM.md`

---

## 1. AWS kurallarına uyum

Evet — akış **AWS IoT Device Shadow** klasik modeline göre:

| AWS kuralı | Firmware |
|------------|----------|
| Cloud `desired` yazar | Dashboard / Device State Controller |
| AWS `desired` vs `reported` farkını hesaplar → **delta** | Bulut (cihaz hesaplamaz) |
| Cihaz `$aws/things/{thing}/shadow/update/delta` dinler | Subscribe var |
| Cihaz `shadow/get` ile bekleyen farkı ister | Connect’te `{}` publish |
| Cihaz yalnızca değişen alanları uygular | `lightState` uygula |
| Cihaz sonucu `reported` yazar | `shadow/update` → `{"state":{"reported":{...}}}` |
| Thing name = Client ID | `lightninenwh_e83dc160639c` |

**Pratik ek (ESP-AT):** Bazen `get/accepted` / `update/accepted` gelir, `delta` key’i kesilir veya gelmez. Bu durumda AWS’ye aykırı “desired vs reported karşılaştır” yok; sadece mesajdaki **`desired.lightState` fallback** uygulanır (aynı sonucu üretir: istenen state’i uygula → reported yaz).

Öncelik sırası (`mqtt_shadow.c`):

1. `state.delta` / `"delta":{...}` → **birinci tercih (AWS)**
2. Klasik `update/delta` topic: `state` doğrudan alanlar (`lightState`)
3. Fallback: `state.desired` (delta yoksa / truncated)

---

## 2. Delta ne işe yarar?

Shadow üç parçadır:

| Bölüm | Kim yazar? | Anlamı |
|------|------------|--------|
| **desired** | Uygulama / dashboard | “Işık şöyle olsun” |
| **reported** | Cihaz | “Işık şu an böyle” |
| **delta** | **AWS otomatik** | desired − reported → “Şunu değiştir” |

Örnek:

```json
"desired":  { "lightState": true }
"reported": { "lightState": false }
"delta":    { "lightState": true }   ← AWS üretir
```

**Neden önemli?**

- Cihaz tüm shadow’u tekrar okumak zorunda kalmaz; sadece **değişen alan** gelir.
- Offline iken desired yazılır; cihaz gelince delta (veya get + delta) ile yakalar.
- Cihaz “desired ile reported’ı ben kıyaslayayım” demez → sync bug’ı azalır.

Bizim kodda delta **kullanılıyor**: subscribe + parse + apply. Log’da `apply: delta object` veya `apply: update/delta state` görürsen delta yolu çalışmış demektir. `apply: desired fallback` görürsen delta key’i yoktu, desired uygulandı.

---

## 3. Dosya haritası

| Katman | Dosya | Görev |
|--------|-------|--------|
| MQTT protokol | `Mqtt_Library/mqtt_shadow.c` | Sub/GET/reported, UART continuous listen, parse |
| Uygulama | `Tiremo_Process/tiremo_shadow.c` | `lightState` → tüm LED’ler + reported |
| Config | `Config/mqtt_device_config.h` | Topic’ler, `MQTT_SHADOW_ENABLE`, dinleme süreleri |
| Entegrasyon | `tiremo_app_net.c`, `tiremo_app.c` | Connect setup + idle UART dinleme |

---

## 4. Boot / çalışma akışı (güncel)

```
TiremoShadow_Init()
Fleet (gerekirse) + MQTT_ConnectBroker()
TiremoShadow_OnConnected()
  ├─ Sub: update/delta, get/accepted, update/accepted (+ rejected)
  ├─ Pub: shadow/get {}
  ├─ ~5 sn continuous UART listen
  └─ delta/desired varsa uygula → reported

Main loop:
  Publish telemetry
  TiremoAppNet_IdleService(~2s)   ← kör DelayMs YOK; sürekli UART dinle
    └─ SUBRECV → apply → reported
```

**Önemli:** ESP-AT `+MQTTSUBRECV` asenkron gelir. Kör `DelayMs` sırasında kaybolur. Idle’da **tek uzun dinleme** kullanılır (frame bölünmez).

---

## 5. Topic’ler

Thing = `MQTT_CLIENT_ID` = `{product}_{device}`  
Örnek: `lightninenwh_e83dc160639c`

| Yön | Topic |
|-----|--------|
| Sub | `$aws/things/<serial>/shadow/update/delta` |
| Sub | `$aws/things/<serial>/shadow/get/accepted` |
| Sub | `$aws/things/<serial>/shadow/update/accepted` |
| Pub | `$aws/things/<serial>/shadow/get` → `{}` |
| Pub | `$aws/things/<serial>/shadow/update` → reported |

---

## 6. Komut şeması

### Desired / Delta

```json
{ "lightState": true }
```

veya `false` — yalnızca on/off.

### Reported (cihaz)

```json
{
  "state": {
    "reported": {
      "lightState": true
    }
  }
}
```

| Key | Tip | Etki |
|-----|-----|------|
| `lightState` | bool | `true` → tüm LED ON, `false` → tüm LED OFF |

MQTT: sürekli bağlı. GNSS telemetry: sonra.

---

## 7. Debug log’ları

| Log | Anlamı |
|-----|--------|
| `RX payload=...` | SUBRECV alındı |
| `apply: delta object` | AWS delta uygulandı |
| `apply: update/delta state` | delta topic state uygulandı |
| `apply: desired fallback` | delta key yok; desired uygulandı |
| `lightState=true → ALL LEDs ON` | Donanım güncellendi |
| `Publish reported...` | Cloud’a reported yazıldı |
| `SUBRECV seen but parse fail` | Frame yarım / parse hatası |

---

## 8. Bilinçli sınırlar

| Madde | Durum |
|-------|--------|
| Named shadow | Yok (classic shadow) |
| UART IRQ ring buffer | Yok — continuous polling listen |
| GNSS lat/lng telemetry | Sonra |
| Saatlik connect/disconnect | Yok — sürekli MQTT |
