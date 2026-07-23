# Device Shadow — Implementasyon (shadow.txt / AWS kuralları)

## Senin anladığın mantık — doğru

```
Cihaz shadow'u okur (GET / delta dinler)
    │
    ├─ delta VAR  → komutu uygula (LED) → reported yaz → sync ✅
    └─ delta YOK  → reported'daki son onaylı state'i yerelde geri yükle
                    (reboot'ta son komut aktif kalsın)
```

Desired ile reported’ı **cihaz kıyaslamaz**. Farkı **AWS** üretir → `delta`.

Kaynak: `shadow.txt` (Tiremo Device State Controller) + AWS IoT Device Shadow.

---

## Reboot’ta son komut aktif olmalı mı?

**Evet** — virtual twin’in amacı bu.

| Cloud durumu | Cihaz açılınca |
|--------------|----------------|
| `delta` var (örn. desired=true, reported=false) | Delta uygula → LED ON → reported=true |
| `delta` yok, reported=true | LED’leri reported’dan ON yap (yeniden reported yazmaya gerek yok) |
| `delta` yok, reported=false | LED OFF |

Böylece “led on” bir kez başarıyla reported olduysa, kapat-aç sonrası da yanık kalır.

---

## Komut

| desired / delta | Etki |
|-----------------|------|
| `{"lightState":true}` | Tüm LED’ler ON |
| `{"lightState":false}` | Tüm LED’ler OFF |

Reported cevabı:

```json
{"state":{"reported":{"lightState":true}}}
```

---

## Dosyalar

| Dosya | Rol |
|-------|-----|
| `mqtt_shadow.c` | MQTT sub/get/reported + UART dinleme |
| `tiremo_shadow.c` | Delta uygula / reported restore / LED |

---

## Beklenen log

```
[SHADOW] GET shadow...
[SHADOW] RX len=...
[SHADOW] DELTA → apply lightState
[SHADOW] LEDs ON (lightState=true)
[SHADOW] Publish reported...
[SHADOW] reported OK — delta should clear on cloud
```

veya reboot’ta sync’liyse:

```
[SHADOW] no delta — restore from reported
[SHADOW] LEDs ON (lightState=true)
```
