# Device Shadow — Kullanım Kılavuzu

Kavram: `shadow.txt`  
Kod detayı: `SHADOW_IMPLEMENTATION.md`  
Provisioning: `PROVISIONING_DETAYLI_DOKUMAN.md`

---

## 1. Önkoşullar

1. Cihaz provisioned (kalıcı sertifika)
2. Broker: `iot.tiremo.ai:8883`
3. Thing name = **`lightninenwh_e83dc160639c`** (IoT ile aynı)
4. Policy: `$aws/things/<serial>/shadow/*`
5. `MQTT_SHADOW_ENABLE = 1`, `EMPA_ESP32_MQTT_AWS` açık

---

## 2. Test (lightState)

Dashboard / Device State Controller desired:

```json
{ "lightState": true }
```

```json
{ "lightState": false }
```

Beklenen UART:

```
[SHADOW] RX payload=...
[SHADOW] apply: ...
[SHADOW] lightState=true → ALL LEDs ON
[SHADOW] Publish reported...
```

- Tüm LED’ler yanar / söner
- Shadow reported: `lightState` aynı değer
- Delta temizlenir (desired ≈ reported)

---

## 3. Delta ne demek? (kısa)

- **desired**: bulutun istediği
- **reported**: cihazın bildirdiği
- **delta**: AWS’nin hesapladığı fark (“bunu uygula”)

Cihaz delta’yı (veya yoksa desired fallback) uygular → reported yazar.  
Desired ile reported’ı **kendisi kıyaslamaz**.

---

## 4. Offline senaryo

1. Cihaz kapalıyken desired güncelle
2. Cihaz açılınca `shadow/get` + dinleme
3. Pending fark uygulanır → reported

---

## 5. Checklist

- [ ] `[SHADOW] OnConnected complete`
- [ ] `lightState: true/false` → tüm LED’ler
- [ ] Reported güncelleniyor
- [ ] Offline desired → online uygulanıyor
- [ ] Telemetry hâlâ yayınlanıyor (sürekli MQTT)

---

## 6. Sorun giderme

| Belirti | Kontrol |
|---------|---------|
| Hiç `RX payload` yok | Firmware güncel mi? Idle listen var mı? |
| `parse fail` | Büyük JSON / UART — tekrar dene |
| `apply` var LED yok | Fiziksel LED / `AllOn` yolu |
| Delta gelmiyor | desired == reported ise AWS delta üretmez; değeri tersine çevirip dene |
| Sub FAILED | Policy / thing name |

---

## 7. Shadow kapatma

```c
#define MQTT_SHADOW_ENABLE  0
```

Rebuild. Sadece telemetry kalır.

---

## 8. Notlar

- GNSS lat/lng telemetry **şimdilik yok**
- MQTT **sürekli** bağlı
- Kaynak yolu: `C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/...` (Eclipse `subdir.mk`)
