# Device Provisioning — Senin Yapman Gerekenler

Bu dosya **senin için**. Kod yazmana gerek yok.  
Platform admin’den / Tiremo dashboard’dan bilgileri alıp aşağıdaki boşlukları doldur.  
Hepsi dolunca bana bu dosyayı (veya cevaplarını) gönder; firmware’e fleet provisioning’i o zaman bağlarız.

---

## Ne değişiyor? (kısa)

| Eskiden | Bundan sonra |
|---------|----------------|
| Her cihaza özel sertifika firmware’e gömülüyordu | Üretimde **ortak bootstrap** sertifika gömülür |
| Cihaz doğrudan buluta bağlanırdı | İlk açılışta cihaz **provisioning** yapar, **kalıcı** sertifika alır |
| Kimlik: `hungarywp4qj_hun20` gibi sabit | Kimlik: `{ürünKodu}_{cihazKodu}` (örn. `hexasenshq5k_SN00123`) |

Senin işin: aşağıdaki **anketi** doldurmak.  
Benim işim: doldurduğun değerlerle firmware’i güncellemek.

---

## Anket — Adım adım doldur

Her soruyu sırayla cevapla. Bilmiyorsan Tiremo / platform **admin**’e sor.

---

### Soru 1 — Product Identifier (ürün kodu)

**Ne?** Dashboard’da ürün için tanımlı 12 karakterlik kod.  
**Nerede kullanılır?** Cihaz seri numarası ve MQTT topic’lerde.  
**Örnek:** `hexasenshq5k`

```
Cevabım: lightninenwh
```

---

### Soru 2 — Product Secret (ürün gizli anahtarı)

**Ne?** Ürüne bağlı gizli string. Provisioning sırasında platform bunu doğrular.  
**Dikkat:** Şifre gibi sakla; git’e / sohbete açık paylaşma istersen sonra sileriz.  
**Örnek:** (admin verir; tahmin edilmez)

```
Cevabım: 0cebd880-8964-4b0a-b218-5a66fe5d632e
```

---

### Soru 3 — Device Identifier (bu kartın / cihazın kodu)

**Ne?** Bu fiziksel cihazı diğerlerinden ayıran kod.  
**Kurallar:**
- Sadece harf ve rakam (`A-Z`, `a-z`, `0-9`)
- Boşluk, tire `-`, alt çizgi `_`, nokta **yok**
- Aynı ürün içinde **benzersiz** olmalı

**Örnek:** `SN00123` veya `hun20` veya `DEV0001`

```
Cevabım: e83dc160639c
```

**Seri numaran şöyle olacak (otomatik):**

```
{productIdentifier}_{deviceIdentifier}
Örnek: hexasenshq5k_SN00123
```

Senin seri numaran (Soru 1 + Soru 3):

```
_________________________________________
```

---

### Soru 4 — Bu cihaz gateway mi?

**Ne?** Gateway cihazlar farklı davranır; normal sensör kartı için genelde hayır.

```
[X] Hayır  →  isGateway = "false"   (çoğu cihaz)
[ ] Evet   →  isGateway = "true"
```

---

### Soru 5 — Principal Identifier var mı? (opsiyonel)

**Ne?** Son kullanıcı / “ev / müşteri” kimliği. Varsa telemetry topic’i değişir.

| Durum | Topic örneği |
|-------|----------------|
| Principal **yok** | `pub/{ürün}/{cihaz}/telemetry` |
| Principal **var** | `pub/{ürün}/{principal}/{cihaz}/telemetry` |

```
[X] Yok  (boş bırak — ilk sürüm için genelde bu)
[ ] Var  → değer: ________________________________
```

---

### Soru 6 — Bootstrap Certificate (PEM)

**Ne?** Üretimde tüm aynı ürün cihazlarına gömülen **ortak** sertifika.  
Kalıcı kimlik değil; sadece ilk provisioning bağlantısı için.

**Nasıl alırsın?** Platform admin / dashboard → Product → Bootstrap credentials.

**Ne yapacaksın?** Aşağıya **tüm PEM metnini** yapıştır (başlık ve bitiş satırları dahil):

```
-----BEGIN CERTIFICATE-----
...
-----END CERTIFICATE-----
```

```
Cevabım (tüm PEM):

-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUfpi2vQeUzTQmEdIa8RBA5pZuOqkwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDcxNzEyMjIx
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN+f9/S4Dz9Z2zqy/t9G
sgxJyPbPWMz4OO3SlCm48tJZkSrr0+bhcozxPTiIWey/vawEUK04g0VGhuiKdVnq
s9ByopJ0vrC978/inPDOfLHFv6zIrqJGwT3Pl3znC9u7wkNKCxGNIaUDgX3iQ1dq
iRT88q8qUNW7dJN9pxEe98BYziy0GYgBfKPB67Xgtar1jmnadCu74Zc0SyxKVLrl
TnkvqHqkbS3eRv6e+HTI812JkmYEoxcgyijpOjvy6+6G4sEldDolagAnLD92ikJ2
Gani929Wvjt6dQ/Hl1IF5m19RZ49OdsVT8+ETqgoJ94O15W8i2F7T4DkA0EoZvXD
kPECAwEAAaNgMF4wHwYDVR0jBBgwFoAUZTc4GqAuxbjpu6E5oi/AHkaFxscwHQYD
VR0OBBYEFNRAhd+b9BLWkjjUP9v7+ZNgD7WgMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQB+yNc04EC9FKTAIsgZ0LweGgJY
B3dCUOQm+/YHwOKOOeOEcmKlNUqaoOKnm1XmQBFQs/DBa9Q12UUo7yeR1w109+zt
Jj06OhIKvoi/1fiqJx2YqjGTLH9TC66B55cHt7GZvVV7mxedf40Zx/DUsfgr7UXJ
hjcWcGBe6Xaqm2lhFg2EW1KgaIHNBqCKoBV93rAMLZY5vA65KpybIZBdgqa50HKQ
T3u6BtsFq2cfzlyjvDEBSHl99LWvzMZWxA4Widi3NGQEQoTEyBNO1Z8h1hZ4h4uw
aR43X0BTZUclNdUe4O16l3aBYxGZtqWhEHu/CPZIB3F7qidzGxnHx+0KEqO8
-----END CERTIFICATE-----

(veya ayrı dosya: bootstrap_cert.pem — projeye koy / bana ver)
```

---

### Soru 7 — Bootstrap Private Key (PEM)

**Ne?** Bootstrap sertifikanın private key’i.  
**Dikkat:** Çok gizli. Sadece firmware’e gömülür; asla log’a yazılmaz.

```
-----BEGIN RSA PRIVATE KEY-----   (veya BEGIN PRIVATE KEY)
...
-----END RSA PRIVATE KEY-----
```

```
Cevabım (tüm PEM):

-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA35/39LgPP1nbOrL+30ayDEnI9s9YzPg47dKUKbjy0lmRKuvT
5uFyjPE9OIhZ7L+9rARQrTiDRUaG6Ip1Weqz0HKiknS+sL3vz+Kc8M58scW/rMiu
okbBPc+XfOcL27vCQ0oLEY0hpQOBfeJDV2qJFPzyrypQ1bt0k32nER73wFjOLLQZ
iAF8o8HrteC1qvWOadp0K7vhlzRLLEpUuuVOeS+oeqRtLd5G/p74dMjzXYmSZgSj
FyDKKOk6O/Lr7obiwSV0OiVqACcsP3aKQnYZqeL3b1a+O3p1D8eXUgXmbX1Fnj05
2xVPz4ROqCgn3g7XlbyLYXtPgOQDQShm9cOQ8QIDAQABAoIBAQCOGOxKiJrHuqVf
y1XBMv74Z/pXOrJGrIQgOusDCA7kMx3XlDr2PtO3U6N/RMU/dydjIurQ51QitQoM
wc6H8GeKUQ2U2rJBRLAZ/PmU2uDJZcrCVFMDssogKMUuDPKwEcMDVw1pfbCfVnb2
Msxvw7aPq5vaSp1K8u7EuddrPa5YfeV0XPsqnXkOUg8BE2TysA44mtoY3vfg3X0O
+Mr0gyGzI8URtb3nPDVYwM3cLkrNZNhUsS8J5SBveVUnr0eFI3KhSiyXr6PZLcNz
rc8bzemti6UUSWWT3HWMx2ERB3zc+r/C3OcKFB9mUXvV8CuLdKN0FT2wwkrmiFjr
o1BADPABAoGBAPXjox4UTeT+KjvPgO55aR4fThcToYQBM4DywM+OpY0LIRCNES3n
Epu0RQo/rHeFl2IH8uJpl+ZCYQa002RfnxgnPTVxku9Vw0Y3NXlnrkaZPQQfoagk
y9k9aKO8Bo0HqqlEZu7oBiczNGTto1hVAR/08+agDV8/h0LBrkkUX8PBAoGBAOjR
9wsX4YC0y1EQYfRAya9od4tfgQWITGP0Z7KuBNm10wlmHp9EeKCQRS+Cw3n6B3K5
iFMD0V2KXDT/Nwq3I9cQQD0yxbOiVu+jSE11Hy5xRr+y3HFCHnUDGfE/V1GAbkgS
vr/BWDqrNThkwEI49wDgGN4o0+/0hm8uVKSooVkxAoGBAJc0tcEwgToZJgWDs+hy
FrckTxQXyVHwnyhjnzfDo4BZYKrZ5L+SgjnnoDEOONOC/jWVZ8HdZ0B8f0fteLUX
rDDhKF2uCspMtfl+x85xeJUHdKMNhI5umkBr5+YQIQMmvMa8PoQZgHcooMaT78fI
9hkdu8KNl8uDuHzPt4pPEhjBAoGAZuQb3JAyPXIpBGtMDuju+REe+746Q9qnf+kM
Rv/27swXNuklkVduQ+9eVA/jGa3wyrfJ0n42cbNq6pT7m0WyliH9cc5VZvzQlNlB
Y9Hl9N1k2eVO3NLOqQG03lQS39b1Ze660/27Yzo0q/aCrQGFz6I5+zxp5XrYSpxc
FeTcYZECgYBlWpTCh1Oy/BwzjgknqXt4ouDGf4/7oTeKKrmUy6rDiFohR/PCE7Z7
7gWNEKhfcxFUTeZvsZN8O1dqiaOdhVc2WBlE/5xSBotpJe0E6eTGYAn70ydujOYk
HvTiY56zLNkNJqCxGTi23/1LlclzNTxupaS74HY5KrUINJ9JGY9/fA==
-----END RSA PRIVATE KEY-----

(veya ayrı dosya: bootstrap_key.pem — projeye koy / bana ver)
```

---

### Soru 8 — Root CA değişti mi?

Projede şu an **Amazon Root CA 1** var (`mqtt_rootCA.inc`).  
Tiremo hâlâ aynı CA’yı kullanıyorsa dokunma.

```
[ ] Aynı kaldı  →  bir şey yapmama gerek yok
[x] Değişti     →  yeni Root CA PEM’ini ekle / bana ver
```
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----


---

### Soru 9 — WiFi (cihazın bağlanacağı ağ)

Provisioning ve MQTT için ESP32’nin internete çıkması lazım.

Dosya (ileride doldurulacak): `Config/network_config.h`

```
SSID:     FiberHGW_ZTG46D5G
Password: TexxeP9DaUTK
```

---

### Soru 10 — Broker adresi

Dokümana göre varsayılan:

```
Host: iot.tiremo.ai
Port: 8883
```

```
[x] Bu doğru, değiştirme
[ ] Farklı  →  Host: _______________  Port: _______
```

---

## Kontrol listesi (kendine tik at)

Admin’den / dashboard’dan aldın mı?

- [ ] Product Identifier
- [ ] Product Secret
- [ ] Bu cihaz için Device Identifier seçtim (alfanumerik, benzersiz)
- [ ] Gateway mi değil mi karar verdim
- [ ] Principal varsa yazdım / yoksa “yok” işaretledim
- [ ] Bootstrap Certificate PEM
- [ ] Bootstrap Private Key PEM
- [ ] Root CA durumu (aynı / yeni)
- [ ] Test WiFi SSID + şifre
- [ ] Broker (iot.tiremo.ai / 8883) onaylandı

Hepsi doluysa → bu dosyayı kaydet / bana gönder → “tamam, implemente et” de.

---

## Sen kod yazmayacaksın — akış şöyle

```
1. Sen anketi doldurursun
2. Ben firmware’i güncellerim:
   - bootstrap cert/key gömülür
   - product / device / secret config’e yazılır
   - ilk boot: provisioning → kalıcı sertifika → yeniden bağlan
3. Sen derleyip flaşlarsın
4. İlk açılışta cihaz bir kez provision olur
5. Sonraki açılışlarda kalıcı sertifika ile normal MQTT
```

---

## Sık karışanlar

| Karıştırma | Doğrusu |
|------------|---------|
| “Eski cihaz sertifikamı mı vereyim?” | Hayır. Artık **bootstrap** (ürün ortak) + provisioning ile **kalıcı** sertifika. |
| “Device Identifier = Client ID mi?” | Hayır. Client ID = `ürün_cihaz` (seri numarası). |
| “Secret’ı topic’e mi koyacağım?” | Hayır. Sadece provisioning kaydında kullanılır. |
| “Her cihaza ayrı bootstrap mu?” | Hayır. Aynı ürün için bootstrap **ortak**; deviceIdentifier **cihaza özel**. |

---

## Bana nasıl ileteceksin?

En kolayı:

1. Bu dosyadaki boşlukları doldur **veya**
2. Şu paketle cevap yaz:

```
productIdentifier: 
productSecret: 
deviceIdentifier: 
isGateway: false
principalIdentifier: (yok / değer)
bootstrap cert: (dosya veya yapıştır)
bootstrap key:  (dosya veya yapıştır)
Root CA: aynı / yeni
WiFi SSID:
WiFi password:
Broker: iot.tiremo.ai:8883
```

Hazır olunca yaz: **“anket doldu, başla”**
