# TiremoCortex — Device Provisioning (Detaylı Doküman)

Bu doküman, TiremoCortex firmware’inde **cihazın buluta güvenli şekilde kimlik kazanması** sürecini (fleet provisioning) uçtan uca anlatır.

İlgili kaynaklar:

| Dosya | Rol |
|-------|-----|
| `device provisioning.txt` | Platform / MQTT sözleşmesi (İngilizce) |
| `DEVICE_PROVISIONING_README.md` | Operatör anketi (admin’den alınacak değerler) |
| Bu dosya | Mimari, boot akışı, kod yolları, hata ayıklama |

> **Not:** Bu repoda yalnızca firmware vardır. Dashboard, AWS şablonları, tenant limitleri ve “Active” işaretleme platform tarafındadır (`iot.tiremo.ai`).

---

## 1. Provisioning nedir? Neden var?

IoT’de Wi‑Fi’ye bağlanmak yeterli değildir. Cihazın buluta “ben kimim?” diye kanıtlaması gerekir. Tiremo bunu **X.509 sertifikaları** ile yapar:

| Kavram | Havayolu benzetmesi | Tiremo karşılığı |
|--------|---------------------|------------------|
| Kimlik | Pasaport | Cihaza özel X.509 sertifika |
| Doğrulama | Göçmenlik kontrolü | TLS + sertifika zinciri |
| Yetki | Vize / izinler | MQTT topic’lere publish/subscribe hakkı |

**Neden ortak şifre / API key değil?**  
Tek bir paylaşılan anahtar sızarsa tüm filo risk altındadır. Her cihaza **kendine özel** kalıcı sertifika verilirse, bir cihaz iptal edildiğinde diğerleri etkilenmez.

### Eski model → yeni model

| Eskiden | Şimdi (fleet provisioning) |
|---------|-----------------------------|
| Her cihaza özel sertifika firmware’e gömülürdü | Üretimde **ortak bootstrap** sertifika gömülür |
| Cihaz doğrudan “kalıcı kimlik” ile bağlanırdı | İlk açılışta provisioning yapıp **kalıcı** sertifika alır |
| Kimlik sabit bir string olabilirdi | Kimlik: `{ürünKodu}_{cihazKodu}` |

Bootstrap sertifika bir “geçici vize” gibidir: yalnızca kayıt masasına girmek içindir. Asıl pasaport (kalıcı sertifika) orada basılır.

---

## 2. Mimari — kim ne yapıyor?

```
┌──────────────────────────────────────────────────────────────┐
│  Tiremo Dashboard / Admin (repo dışında)                     │
│  • Product oluşturur                                         │
│  • Bootstrap cert/key + Product Secret verir                 │
└────────────────────────────┬─────────────────────────────────┘
                             │ firmware’e gömülür
                             ▼
┌──────────────────────────────────────────────────────────────┐
│  ABOV A34G43x MCU (TmplUserApp)                              │
│  • Kimlik: mqtt_device_config.h                              │
│  • Bootstrap PEM: certificates/*.inc  →  .cert_flash         │
│  • Fleet mantığı: mqtt_cert_provision.c                      │
│  • Boot orkestrasyonu: tiremo_app_net.c                      │
│                    UART AT komutları                         │
└────────────────────────────┬─────────────────────────────────┘
                             ▼
┌──────────────────────────────────────────────────────────────┐
│  ESP32-C3 (ESP-AT)                                           │
│  • Wi‑Fi + MQTT over TLS                                     │
│  • Sertifikalar NVS’te: mqtt_ca / mqtt_cert / mqtt_key       │
│  • Provisioned işareti: ns=tiremo, key=provisioned = PROV1   │
└────────────────────────────┬─────────────────────────────────┘
                             ▼  TLS :8883
┌──────────────────────────────────────────────────────────────┐
│  Tiremo / AWS IoT  —  iot.tiremo.ai                          │
│  • Bootstrap (claim) sertifika ile auth                      │
│  • $aws/certificates/create/json                             │
│  • $aws/provisioning-templates/tiremo-default/provision/json │
│  • Kalıcı sertifika + Thing oluşturur                        │
└──────────────────────────────────────────────────────────────┘
```

**Özet:** MCU karar verir ve MQTT protokolünü yönetir; ESP32 Wi‑Fi/TLS taşıyıcısı ve sertifika deposudur; Tiremo broker kimlik basar.

Özellik bayrağı: `EMPA_ESP32_MQTT_AWS` (`Config/app_config.h`). Kapalıysa fleet kodu derlenmez (stub’lar boş döner).

---

## 3. Temel kavramlar sözlüğü

| Terim | Açıklama |
|-------|----------|
| **Bootstrap Certificate** | Ürün filoya ortak, üretime gömülen sertifika. Sadece ilk provisioning MQTT oturumu için. |
| **Bootstrap Private Key** | Bootstrap’ın private key’i. Firmware’e gömülür; log’a yazılmaz. |
| **Product Identifier** | ~12 karakter ürün kodu (ör. `hexasenshq5k`). Seri no ve topic’lerde kullanılır. |
| **Product Secret** | Ürüne bağlı gizli string. Register adımında platform doğrular. Topic’e **konmaz**. |
| **Device Identifier** | Bu fiziksel birimi ayıran alfanumerik kod (`A–Z`, `a–z`, `0–9`). Tire, alt çizgi, boşluk yok. |
| **Serial / Thing Name / Client ID** | `{productIdentifier}_{deviceIdentifier}`. Platformdaki kalıcı kimlik. |
| **Permanent Certificate** | Create adımında AWS’in verdiği cihaza özel sertifika + private key. Sonraki tüm bağlantılar bununla. |
| **Ownership Token** | Create yanıtındaki tek kullanımlık token. Register’da zorunlu. Sadece RAM’de tutulur. |
| **Provisioned Marker** | ESP32 NVS’te `PROV1`. “Bu cihaz fleet’i bitirdi” işareti. |
| **Principal Identifier** | Opsiyonel son kullanıcı / ev kimliği. Varsa telemetry topic’ine eklenir. |
| **isGateway** | `"true"` / `"false"` string. Gateway davranışını platforma bildirir. |

### Seri numarası formülü

```
serial = {productIdentifier}_{deviceIdentifier}
örnek:  hexasenshq5k_SN00123
```

Bu değer:

- MQTT **Client ID** olur (`MQTT_CLIENT_ID`)
- Platformdaki **Thing name** olur
- Register yanıtındaki `thingName` ile aynı olmalıdır

### Sık karışanlar

| Karıştırma | Doğrusu |
|------------|---------|
| “Eski cihaz sertifikamı mı vereyim?” | Hayır → **bootstrap** (ürün ortak) + provisioning ile kalıcı sertifika |
| “Device Identifier = Client ID mi?” | Hayır → Client ID = `ürün_cihaz` |
| “Secret’ı topic’e mi koyacağım?” | Hayır → sadece register JSON parametresi |
| “Her cihaza ayrı bootstrap mu?” | Hayır → bootstrap **ürün ortak**; deviceIdentifier **cihaza özel** |

---

## 4. Yaşam döngüsü (büyük resim)

```
Cihaz açılır
    │
    ▼
Zaten provisioned mi? (ESP32 NVS: PROV1)
    │
    ├─ EVET ──► Kalıcı sertifika ile MQTT bağlan ──► Telemetry
    │
    └─ HAYIR
           │
           ▼
     ESP32’deki mevcut cert ile sessiz bağlanmayı dene
           │
           ├─ BAŞARILI ──► Marker’ı yeniden yaz ──► Normal MQTT
           │                (kalıcı cert vardı, marker silinmiş olabilir)
           │
           └─ BAŞARISIZ
                  │
                  ▼
            Bootstrap PEM’i MCU flash’tan ESP32’ye yükle
                  │
                  ▼
            Bootstrap ile Quiet MQTT connect
                  │
                  ▼
            Create cert  →  Register thing  →  Kalıcı cert’i ESP32’ye yaz
                  │
                  ▼
            Marker PROV1  →  Bootstrap oturumu kapat
                  │
                  ▼
            Kalıcı cert ile normal MQTT ──► Platform “Active”
                  │
                  ▼
            Telemetry / alarm publish
```

İlk başarılı kalıcı bağlantıda platform cihazı dashboard’da **Active** yapar; firmware’in ekstra bir “aktif oldum” mesajı göndermesi gerekmez.

---

## 5. Faz A — Admin / üretim önkoşulları

Cihaz kodundan önce platform admin şunları hazırlar (dashboard):

1. **Product** kaydı
2. **Bootstrap Certificate + Private Key** (filoya ortak)
3. **Product Identifier** + **Product Secret**
4. Broker: `iot.tiremo.ai:8883`
5. Fabrika / geliştirici her birime benzersiz **deviceIdentifier** atar

Operatör checklist’i için: `DEVICE_PROVISIONING_README.md`.

**Güvenlik uyarısı:** Bootstrap key ürün filoya ortaktır. Sızarsa admin ile **rotate** edilmelidir. Secret ve private key’leri git’e / sohbete açık bırakmayın.

---

## 6. Faz B — Firmware’e ne gömülür?

### 6.1 Kimlik ve topic’ler — `mqtt_device_config.h`

Yol:

`Generation/.../TmplUserApp/Config/mqtt_device_config.h`

| Macro | Anlamı |
|-------|--------|
| `MQTT_PRODUCT_IDENTIFIER` | Ürün kodu |
| `MQTT_DEVICE_IDENTIFIER` | Cihaz kodu |
| `MQTT_PRODUCT_SECRET` | Ürün secret |
| `MQTT_IS_GATEWAY` | `"true"` / `"false"` |
| `MQTT_USE_PRINCIPAL` | `0` veya `1` |
| `MQTT_PRINCIPAL_IDENTIFIER` | Principal (opsiyonel) |
| `MQTT_CLIENT_ID` | `ürün_cihaz` (otomatik birleşim) |
| `MQTT_BROKER_HOST` | `iot.tiremo.ai` |
| `MQTT_BROKER_PORT` | TLS açıksa `8883` |

Topic örnekleri (`MQTT_USE_PRINCIPAL == 0`):

```
pub/{product}/{device}/telemetry
pub/{product}/{device}/alarm
sub/{product}/{device}/telemetry
```

Principal açıksa araya `{principal}` girer:

```
pub/{product}/{principal}/{device}/telemetry
```

Fleet topic’leri de aynı header’da tanımlıdır:

```
$aws/certificates/create/json
$aws/certificates/create/json/accepted|rejected
$aws/provisioning-templates/tiremo-default/provision/json
$aws/provisioning-templates/tiremo-default/provision/json/accepted|rejected
```

Tüm ürünler aynı şablonu kullanır: **`tiremo-default`**. Platform, gelen `productIdentifier` / secret ile tenant ve ürünü çözer.

### 6.2 Bootstrap PEM dosyaları

| Dosya | İçerik |
|-------|--------|
| `Mqtt_Library/certificates/mqtt_certificate.inc` | Bootstrap client cert (PEM) |
| `Mqtt_Library/certificates/mqtt_private.inc` | Bootstrap private key (PEM) |
| `Mqtt_Library/certificates/mqtt_rootCA.inc` | Amazon Root CA 1 (TLS güven zinciri) |

Bunlar linker’da **`.cert_flash`** bölümüne konur (sayfa hizalı, silinebilir flash bölgesi). `mqtt_certs.c` boot’ta PEM’i RAM’e yükler; ilk byte `0xFF` ise “silinmiş / yok” kabul edilir.

### 6.3 Wi‑Fi — `network_config.h`

Provisioning ve MQTT için ESP32’nin internete çıkması şarttır.

- `WIFI_SSID` / `WIFI_PASSWORD`
- `WIFI_TIMEZONE` — TLS sertifika tarih kontrolü için **doğru saat** kritiktir (yanlış saat → TLS handshake fail)

### 6.4 Ortam değişkeni yok

Bu firmware projesinde `.env` / ortam değişkeni yoktur. Her şey `#define` + `.inc` PEM’dir.

---

## 7. Faz C — İlk boot: karar ağacı (`tiremo_app_net.c`)

Giriş noktası: `TiremoAppNet_InitAndConnect()`.

### Adım adım

1. **ESP32 AT init / recovery** — modül cevap veriyor mu?
2. **`MqttFleet_IsProvisioned()`** — ESP32 NVS’te `tiremo` / `provisioned` = `PROV1` var mı?

#### Dal 1 — Marker var

- Bootstrap yüklenmez, fleet çalışmaz.
- ESP32’deki kalıcı sertifikalarla normal `MQTT_ConnectBroker()` yapılır.

#### Dal 2 — Marker yok (kritik güvenlik mantığı)

Kod **hemen bootstrap yüklemez**. Çünkü bootstrap upload, ESP32’deki mevcut cert slotlarını ezer; kalıcı sertifikalar hâlâ NVS’te olabilir (ör. marker silinmiş).

Sıra:

1. `MQTT_ConnectBrokerQuiet()` — ESP32’deki mevcut cert ile sessiz bağlan
2. Başarılıysa → `MqttFleet_MarkProvisioned()` ile marker’ı onar, fleet’i atla
3. Başarısızsa → gerçek fleet yolu:
   - `MqttFleet_UploadBootstrap()` — MCU `.cert_flash` → ESP32 NVS
   - `MQTT_ConnectBrokerQuiet()` — bootstrap ile bağlan  
     *(Claim/bootstrap oturumunda telemetry publish yasaktır; Quiet connect hello/telemetry göndermez.)*
   - `MqttFleet_Run()` — create + register + kalıcı cert upload + marker
   - Ardından `MQTT_ConnectBroker()` — kalıcı kimlikle normal oturum

Bu karar ağacı bilinçli olarak “marker yok = kesinlikle yeniden provision” demez; önce mevcut kalıcı cert’i korur.

---

## 8. Faz D — Fleet protokolü (`MqttFleet_Run`)

Dosya: `Mqtt_Library/cert_Lib/mqtt_cert_provision.c`

Önkoşul: MQTT **zaten bootstrap sertifika ile** bağlıdır.

### 8.1 Create — yeni sertifika iste

1. Subscribe:
   - `$aws/certificates/create/json/accepted`
   - `$aws/certificates/create/json/rejected`
2. Publish `{}` → `$aws/certificates/create/json`
3. Accepted JSON’dan çıkarılır:
   - `certificateOwnershipToken`
   - `certificatePem`
   - `privateKey`
   - (ayrıca `certificateId` gelir; register için token yeter)

### 8.2 Register — Thing kaydı

1. Subscribe register accepted/rejected
2. Publish (örnek, principal yokken):

```json
{
  "certificateOwnershipToken": "<token>",
  "parameters": {
    "deviceIdentifier": "<MQTT_DEVICE_IDENTIFIER>",
    "productIdentifier": "<MQTT_PRODUCT_IDENTIFIER>",
    "productSecret": "<MQTT_PRODUCT_SECRET>",
    "isGateway": "false"
  }
}
```

Topic:

`$aws/provisioning-templates/tiremo-default/provision/json`

3. Accepted’ta `thingName` beklenir → genelde `{product}_{device}`

### 8.3 Kalıcı kimliği sakla

1. `MqttCerts_SetPermanent(cert, key)` — RAM’deki client cert/key’i kalıcı PEM ile değiştir (Root CA aynı kalır)
2. `Wifi_MqttCertsUpload2()` — ESP32 NVS’e yaz:
   - `mqtt_ca` / `mqtt_cert` / `mqtt_key`
3. Marker yaz: namespace `tiremo`, key `provisioned`, value `PROV1`
4. `Wifi_MqttClean2()` — bootstrap MQTT oturumunu kapat
5. RAM’deki token ve private key `memset` ile temizlenir (kalıcı key ESP32 NVS’te kalır)

### 8.4 UART / ESP-AT dikkat noktası

Create yanıtı büyük PEM içerir; ESP-AT `+MQTTSUBRECV` frame’leri UART’tan akar.

Kritik kural (kod yorumundan):

> ESP hâlâ frame basarken `printf`/parse ile **bloklama**; aksi halde HW FIFO taşar ve token’lı son frame kaybolur.

Çözüm:

1. Önce UART’ı “slurp” et (ham buffer, ~6 KB)
2. SUBRECV görüldükten sonra ~2.5 s sessizlik → burst bitti
3. Sonra payload’ları birleştirip JSON parse et

Timeout: create/register için pratikte ~45–60 s dinleme penceresi (`FLEET_WAIT_MS` 60000).

Buffer limitleri:

| Alan | Max |
|------|-----|
| Ownership token | 512 |
| Permanent cert PEM | 1408 |
| Permanent key PEM | 1792 |

PEM’ler beklenenden büyükse parse fail olur.

---

## 9. Faz E — Normal çalışma

Kalıcı sertifika ile:

| Özellik | Değer |
|---------|--------|
| Host | `iot.tiremo.ai` |
| Port | `8883` |
| TLS | Zorunlu |
| Client ID | `MQTT_CLIENT_ID` (seri no) |
| Cert / Key | ESP32 NVS’teki kalıcı PEM’ler |

Sonraki reboot’larda marker `PROV1` ise fleet atlanır; cihaz doğrudan telemetry yayınlar (`TiremoAppNet_PublishCycle`).

Payload şeması ürüne özeldir; platform admin’in data ingestion tanımına bakılır.

---

## 10. Sertifika yaşam döngüsü (nerede durur?)

| Kimlik bilgisi | Nerede | Ömür | Amaç |
|----------------|--------|------|------|
| Bootstrap cert/key | MCU `.cert_flash` (`.inc`); ihtiyaçta ESP32 NVS’e upload | Ürün filoya ortak | Sadece fleet oturumu |
| Amazon Root CA | Aynı | Uzun ömür | TLS trust |
| Permanent cert/key | Create’den gelir → ESP32 NVS | Cihaza özel | Tüm sonraki MQTT |
| Ownership token | Sadece RAM (fleet sırasında) | Tek atış | Register |
| Marker `PROV1` | ESP32 SYSMFG NVS | Kalıcı | Fleet’i atla |
| Product secret | Compile-time `#define` | Firmware ömrü | Register parametresi |

### MCU flash silme (`MqttCerts_EraseFlash`)

Tasarım notu: Bootstrap PEM upload’tan sonra MCU flash’tan silinebilir (private key MCU’da kalmasın). Fonksiyon mevcuttur; **şu an boot akışında otomatik çağrılmıyor** — yani re-flash edilmedikçe bootstrap PEM’ler MCU flash’ta kalabilir. ESP32’ye yazıldıktan sonra asıl kullanılan kimlik ESP32 NVS’tedir.

MCU’yu IDE ile yeniden flaşlamak `.cert_flash`’ı geri getirir; ESP32 marker/certs temizlenmişse yeni bir provisioning turu tetiklenebilir.

---

## 11. Kod haritası (nereden okumalı?)

| Bileşen | Yol |
|---------|-----|
| Boot / karar ağacı | `TmplUserApp/Tiremo_Process/tiremo_app_net.c` |
| Fleet MQTT protokolü | `TmplUserApp/Mqtt_Library/cert_Lib/mqtt_cert_provision.c` |
| API bildirimi | `.../mqtt_cert_provision.h` |
| PEM yükleme / SetPermanent / EraseFlash | `.../mqtt_certs.c` |
| Bootstrap PEM | `.../Mqtt_Library/certificates/*.inc` |
| Kimlik + topic + fleet topic | `TmplUserApp/Config/mqtt_device_config.h` |
| Wi‑Fi | `TmplUserApp/Config/network_config.h` |
| Özellik bayrağı | `TmplUserApp/Config/app_config.h` → `EMPA_ESP32_MQTT_AWS` |
| Quiet vs normal connect | `Mqtt_Library/EMPA_MqttAws.c` |
| ESP-AT MQTT / SYSMFG | `Mqtt_Library/mqtt_core.c` |
| Platform sözleşmesi | `TiremoCortex/device provisioning.txt` |
| Operatör anketi | `TiremoCortex/DEVICE_PROVISIONING_README.md` |

### Public API özeti

```c
uint8_t MqttFleet_IsProvisioned(void);           // PROV1 var mı?
int     MqttFleet_MarkProvisioned(buf, size);    // marker yaz / onar
int     MqttFleet_UploadBootstrap(buf, size);    // MCU → ESP32 bootstrap
int     MqttFleet_Run(buf, size);                // create + register + kalıcı upload
int     MqttCertProv_Run(buf, size);             // eski yardımcı: provisioned değilse bootstrap upload
```

---

## 12. Platform red nedenleri

Register `.../rejected` gelirse tipik sebepler:

| Sebep | Ne yapmalı? |
|-------|-------------|
| Eksik / geçersiz `productIdentifier` | Admin ile doğrula |
| Geçersiz `productSecret` | Secret’ı kontrol et |
| Identifier ↔ secret uyuşmazlığı | Aynı ürüne ait olduklarından emin ol |
| `deviceIdentifier` özel karakter | Sadece alfanumerik |
| Geçersiz `isGateway` | Tam olarak `"true"` veya `"false"` |
| Sertifika bu ürün için kayıtlı değil | Yanlış bootstrap (başka ürünün cert’i) |
| Tenant cihaz limiti | Admin ile iletişime geç |

---

## 13. Firmware log’ları ile teşhis

UART debug satırları `[FLEET]`, `[MQTT]`, `[CERT]` ile başlar.

| Belirti | Olası neden |
|---------|-------------|
| `No bootstrap certificates in firmware` | `.cert_flash` boş / silinmiş / `.inc` yok |
| `Bootstrap upload failed` | ESP32 SYSMFG / AT hatası |
| `Bootstrap MQTT connect failed` | Wi‑Fi, NTP saati, yanlış bootstrap, broker |
| `Create timeout / no token` | Büyük PEM sırasında UART taşması veya parse fail |
| `incomplete frame need=… have=…` | Kesilmiş `+MQTTSUBRECV` |
| `Parse token/certificatePem/privateKey failed` | JSON escape veya buffer boyutu |
| `Register REJECTED` | Parametre / secret / limit |
| `WARN: provisioned marker write failed` | Fleet oldu ama marker yazılamadı; sonraki boot probe ile kurtulabilir |
| Marker yok ama connect çalışıyor | Kalıcı cert var; kod marker’ı yeniden yazar |

### Yeniden provision ne zaman tetiklenir?

| Senaryo | Sonuç |
|---------|--------|
| MCU re-flash + ESP32 NVS temiz / geçersiz | Tam fleet yolu |
| Sadece marker silindi, kalıcı cert duruyor | Quiet connect başarılı → marker onarılır, fleet yok |
| Marker yokken bootstrap zorla upload | **Kalıcı cert’leri siler** — bu yüzden kod önce probe eder |
| `EMPA_ESP32_MQTT_AWS` kapalı | Fleet stub; provisioning yok |

---

## 14. Gönderim öncesi firmware checklist

Platform dokümanına ve mevcut koda göre:

| Madde | Durum |
|-------|--------|
| Boot’ta kalıcı credential varsa provisioning atlanır | Evet (marker + quiet probe) |
| Kalıcı cert/key NVS’te saklanır | Evet (ESP32 SYSMFG) |
| Client ID = serial | Evet (`MQTT_CLIENT_ID`) |
| Principal-scoped topic | Compile-time (`MQTT_USE_PRINCIPAL`) |
| Shadow üzerinden principal yeniden atama | **Firmware’de henüz yok** |
| Private key log’lanmaz | Token/key RAM wipe var; debug PEM içeriğini basmaz |

---

## 15. Bu repoda olmayanlar

Aşağıdakiler Tiremo platform / AWS tarafındadır; bu workspace’te kodu yoktur:

- Dashboard / admin UI
- REST API’ler, veritabanı modelleri
- `tiremo-default` CloudFormation / Lambda implementasyonu
- Tenant limit yönetimi
- Device Shadow `principalIdentifier` güncellemesi için firmware aboneliği

Firmware yalnızca dokümante MQTT fleet arayüzünü tüketir.

---

## 16. Pratik senaryo — sıfırdan bir cihaz

1. Admin’den product ID, secret, bootstrap cert/key, broker onayı al.
2. Bu birim için benzersiz `deviceIdentifier` seç.
3. `mqtt_device_config.h` doldur; PEM’leri `.inc` dosyalarına koy; Wi‑Fi ayarla.
4. `EMPA_ESP32_MQTT_AWS` açıkken derle ve MCU’yu flaşla.
5. İlk açılış: Wi‑Fi → (gerekirse) fleet → kalıcı cert → MQTT Ready.
6. Debug’da `thingName` / `Client ID` satırını doğrula.
7. Dashboard’da cihazın **Active** olduğunu kontrol et.
8. Telemetry topic’ine veri geldiğini doğrula.

Sonraki açılışlarda fleet tekrarlanmaz (marker + kalıcı cert).

---

## 17. Güvenlik özeti

1. Bootstrap key filoya ortaktır → sızıntıda rotate.
2. Product secret firmware’de gömülüdür → repo / log / sohbet sızıntısına dikkat.
3. Permanent private key cihazı terk etmemeli; ESP32 NVS’te kalır, register sonrası RAM silinir.
4. Marker yokken bootstrap’ı körlemesine yüklemek kalıcı kimliği yok eder → firmware bunu bilerek engeller.
5. TLS için saat senkronu şarttır.

---

## 18. Kısa “tek bakışta” özet

**Provisioning**, ortak bootstrap sertifika ile Tiremo’ya bağlanıp AWS IoT fleet API’sinden **cihaza özel kalıcı sertifika almak** ve bunu ESP32 NVS’e yazmaktır. Kimlik `{ürün}_{cihaz}` seri numarasıdır. Boot’ta `PROV1` marker veya çalışan kalıcı cert varsa işlem atlanır; yoksa create → register → reconnect zinciri bir kez çalışır. Bundan sonra cihaz normal MQTT telemetry yayınlar.

Operatör değerleri için: `DEVICE_PROVISIONING_README.md`  
Ham MQTT sözleşmesi için: `device provisioning.txt`
