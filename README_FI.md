# simppeliTCU 🍃
**Kevyt ja avoin tee-se-itse-korvaaja Nissan Leaf ZE1:n alkuperäiselle etäohjaukselle (TCU).**

*Lue tämä englanniksi: [README.md](README.md)*

Kun Nissanin alkuperäiset pilvipalvelut lakkaavat toimimasta tai muuttuvat epäluotettaviksi, `simppeliTCU` palauttaa autosi hallinnan omiin käsiisi. Tämä ESP32-pohjainen laite kytkeytyy suoraan auton IT CAN -väylään (yhteisössä usein tunnettu nimellä CAR-CAN) kuten alkuperäinen TCU ja tarjoaa helpon Wi-Fi-käyttöliittymän.

Huomio! Projekti on hahmotelmavaiheessa ja koodit on suurelta osin generoitu tekoälyn avulla.

## Ominaisuudet
* 🌡️ **Lämmityksen etäohjaus:** Ilmastoinnin/lämmityksen käynnistys ja välitön pakkosammutus.
* 🔋 **Latauksen etäohjaus:** Latausajastimen ohitus (latauksen aloitus).
* 🔐 **Ovien lukitus:** Lukitse ja avaa ovet etänä (oletuksena pois päältä, kytke päälle asetuksista).
* 🔕 **Ilmoitusruudun automaattinen kuittaus:** Kuittaa ZE1:n tietoviihdejärjestelmän aloitusilmoituksen (nag screen) automaattisesti, kun laite on kytketty AV-CAN-väylään.
* 📊 **Auton tila:** Lukee väylältä akun tarkan varausprosentin (SOC) ja sisälämpötilan.
* 📍 **Sijainnin seuranta:** AV-CAN-väylän GPS-koordinaatit näytetään web-käyttöliittymässä Google Maps -linkin kera, julkaistaan MQTT:hen ja tunnistetaan Home Assistantissa (oletuksena pois päältä, kytkettävissä `tracking`-asetuksella).
* 🌐 **Web-käyttöliittymä:** Toimii suoraan puhelimen selaimella, yksinkertainen ja kevyt.
* 🏠 **Home Assistant -integraatio:** Automaattinen laitteen ja mittaustietojen tunnistus MQTT:n kautta.
* 📡 **MQTT-tuki:** Integrointi kotiautomaatioon tai mobiilisovelluksiin salatun yhteyden yli. Huom: MQTT-palvelinta ei todenneta, ellei kunnollista varmenteen tarkistusta ole toteutettu. Katso [MQTT-asennusohje](docs/mqtt.md).

## Toiminta
* ⚙️ **Uusi komento ei keskeytä aloitettua:** Laite käsittelee CAN-komennot asynkronisesti. Jos uusi komento annetaan edellisen ollessa vielä kesken, uusi komento ohitetaan turvallisesti.

![Kuvakaappaus web-käyttöliittymästä puhelimen selaimella](docs/screenshot.jpg)

## Videot
* **Ensimmäiset testit:** [https://youtu.be/qn05-901b3Y](https://youtu.be/qn05-901b3Y)
* **Web-käyttöliittymä Wi-Fin kautta:** [https://youtube.com/shorts/XqAgwizNodE](https://youtube.com/shorts/XqAgwizNodE)
* **MQTT-käyttö IoTMQTTPanelilla ja OVMS Connectilla:** [https://youtu.be/g8Yh6OgjL-Q?si=QtElrzG9FnF5YEUm](https://youtu.be/g8Yh6OgjL-Q?si=QtElrzG9FnF5YEUm)
* **Hansikaslokeron irrotus (LHD ZE1 Leaf):** [https://youtu.be/kyx2A4U-M1M?si=YGewj1TeBEfrpOKV](https://youtu.be/kyx2A4U-M1M?si=YGewj1TeBEfrpOKV)
* **Ilmoitusruudun automaattinen kuittaus (Nag Screen Auto-Dismiss):** [https://youtu.be/FWOML1giRBo?si=TElSWKBwn389HtfV](https://youtu.be/FWOML1giRBo?si=TElSWKBwn389HtfV)

## Laitteistovaatimukset
1. **Lilygo T-2CAN** (ESP32-S3-mikrokontrolleri sisäänrakennetuilla CAN-piireillä) — *tai vaihtoehtoisesti **Waveshare ESP32-S3-RS485-CAN** (yksi CAN-väylä, sisäänrakennettu 7–36V DC -regulaattori)*.
2. **12V -> 5V autokäyttöön tarkoitettu jännitteenalennin** (Vahva suositus Lilygo T-2CAN:lle: upotettava 12V USB-autopistorasia) + **2A johtosulake**.
3. Pinnit/liittimet alkuperäisen TCU-johtosarjan sovittamiseen (esim. 16-nastainen Sumitomo NH `.025"` -liitinsarja).

**Katso [Asennusohje (ENG)](docs/INSTALLATION.md) saadaksesi vaiheittaiset asennusohjeet, kytkentäkaaviot ja tarvittavat työkalut.**

### ⚠️ TÄRKEÄ VAROITUS VIRRANSYÖTÖSTÄ
**ÄLÄ kytke auton 12V-linjaa suoraan Lilygo-levyyn!** Auton sähköjärjestelmän jännitepiikit todennäköisesti rikkovat laitteen.
* Irrota `IT-CAN`-johdot (`#6` ja `#7`) alkuperäisen TCU:n liittimestä (jätä/kierrätä `+12V` ja `GND` alkuperäiselle TCU:lle, jos haluat auton handsfree-mikrofonin toimivan edelleen).
* Ota 12V-virta TCU:n johtosarjasta **2A sulakkeen** ja autokäyttöön tarkoitetun 12V->5V USB-muuntimen läpi (tai Waveshare-kortin `DC 7–36V` -ruuviliittimeen).
* Syötä Lilygolle virta tavallisella USB-kaapelilla 5V-muuntimesta.
* Kytke `CAN-H`, `CAN-L` ja `SGNDB` (signaalimaa) Lilygon TWAI-ruuviliittimiin (**Port B**: tässä koodissa TX 7, RX 6).

## Asetukset

Laitteen asetukset (Wi-Fi, MQTT-tunnukset jne.) tallennetaan pysyväismuistiin. Voit määrittää järjestelmän USB-sarjaliitännän kautta käyttämällä pääteohjelmaa (kuten Arduino IDE:n Serial Monitor, PuTTY tai vastaava), jonka nopeudeksi on asetettu **115200 baudia** ja rivinvaihto (Newline, `\n`) tai palautus (Carriage Return, `\r`) on käytössä.

Käytettävissä olevat komennot:
* `list` - Näyttää kaikki nykyiset asetukset (mukaan lukien laiteohjelmiston version).
* `get <avain>` - Tulostaa määritetyn avaimen nykyisen arvon.
* `set <avain> <arvo>` - Päivittää määritettyyn avaimeen uuden arvon (tallentuu pysyvästi).
* `version` - Tulostaa nykyisen laiteohjelmiston version ja git-hajautustunnisteen (commit hash).
* `reboot` - Käynnistää laitteen uudelleen, jotta muutokset tulevat voimaan välittömästi.
* `factory-reset` - Tyhjentää kaikki asetukset ja käynnistää laitteen uudelleen.

**Esimerkki asetusistunnosta:**
```text
set ssid OmaWiFiVerkko
set password OmaSalasana
set ap_ssid LeafLocalAP
set ap_password LeafLocalPassword
set mqtt_server 192.168.1.100
set locking_enabled true
reboot
```

*Tuetut asetusavaimet:*
* `ssid`, `password` (Yhdistäminen olemassa olevaan Wi-Fi-verkkoon / STA-tila)
* `ap_ssid`, `ap_password` (Paikallisen Wi-Fi-tukiaseman luominen / AP-tila)
* `hostName`
* `mqtt_server`, `mqtt_port`, `mqtt_user`, `mqtt_password`, `vehicle_id`
* `mqtt_tls` (Ota MQTT:n TLS käyttöön/pois; oletus: `true`)
* `locking_enabled` (Ota ovien lukitustoiminto käyttöön; oletus: `false`)
* `hass_discovery` (Ota Home Assistantin tunnistusviestit käyttöön/pois; oletus: `true`)
* `nag_dismiss` (Ota tietoviihdejärjestelmän ilmoitusruudun automaattinen kuittaus käyttöön/pois AV-CAN-väylällä; oletus: `true`)
* `tracking` (Ota GPS-sijainnin seuranta käyttöön/pois: web-käyttöliittymä, MQTT-julkaisu ja Home Assistant -entiteetit; oletus: `false`. Kytke päälle komennolla: `set tracking true`)

**Wi-Fi-tilat:**
* **STA-tila:** Aktiivinen, kun `ssid` on määritetty. Laite yhdistää kodin/autotallin Wi-Fi-verkkoon.
* **AP-tila:** Aktiivinen, kun `ap_ssid` on määritetty. Laite luo oman Wi-Fi-verkkonsa (kätevä poissa kotoa).
* **AP + STA -tila:** Aktiivinen, kun sekä `ssid` että `ap_ssid` on määritetty. Laite yhdistää kodin Wi-Fi-verkkoon ja luo samalla oman tukiasemansa.

## Kääntäminen ja lataus

Ohjeet laiteohjelmiston kääntämiseen ja lataamiseen Arduino IDE:llä tai komentorivikäyttöliittymällä (CLI) löytyvät [Kääntämisen ja lataamisen oppaasta (ENG)](docs/compiling.md).

## Valmiiksi käännettyjen binäärien lataus

Ohjeet valmiiksi käännettyjen julkaisubinäärien lataamiseen (käyttäen graafisia työkaluja tai `esptool`-komentoa) löytyvät [Valmiiksi käännettyjen binäärien latausoppaasta (ENG)](docs/flashing.md). Katso julkaisukohtainen muutosloki tiedostosta [CHANGELOG.md](CHANGELOG.md).

## CAN-viestintä
Katso [CAN-viestinnän tiedot (ENG)](docs/can.md) -asiakirjasta tietoja CAN-väylän viesteistä, komennoista, tunnetuista ongelmista ja IT-CAN-integraatioon liittyvistä rajoituksista.

### Kahden CAN-väylän tuki (LilyGO T-2Can)
Laiteohjelmisto tukee kahta CAN-liitäntää:

1. **Ensisijainen CAN (TWAI):** ESP32:n sisäänrakennettu CAN-ohjain (Port B: TX 7, RX 6).
2. **Toissijainen CAN (MCP2515):** Ulkoinen SPI-väylään kytketty MCP2515-CAN-ohjain, joka löytyy LilyGO T-2Can -kortilta (Port A).

Kaikki lähtevät CAN-viestit lähetetään **molemmille** väylille, ja kummalta tahansa väylältä vastaanotetut viestit käsitellään samalla tavalla. Tämän ansiosta laite voidaan kytkeä kahteen erilliseen CAN-väylään samanaikaisesti, esim. IT-CAN ja AV-CAN.

Toinen CAN-liitäntä on oletuksena käytössä ja vaatii `autowp-mcp2515`-kirjaston (katso [Kääntämisen ja lataamisen opas (ENG)](docs/compiling.md)). Sen voi poistaa käytöstä asettamalla `USE_SECOND_CAN`-arvoksi `0` tiedostossa `simppeliTCU/featureFlags.h` tai käännösvalitsimella `-DUSE_SECOND_CAN=0`. Molempien CAN-liitäntöjen pinnimääritykset ovat keskitetysti tiedostossa `simppeliTCU/hardware.h`.

## Lisenssi ja vastuuvapauslauseke (MIT-lisenssi)
Tämä projekti on lisensoitu MIT-lisenssillä.

**VASTUUVAPAUSLAUSEKE:**
Tämä ohjelmisto ja laitteistomuutos on vuorovaikutuksessa ajoneuvon korkeajännitejärjestelmien kanssa. Se tarjotaan "SELLAISENAAN" (As Is) ilman minkäänlaisia takuita, nimenomaisia tai oletettuja. Käyttämällä tätä koodia otat kaikki riskit itsellesi. Tekijöitä tai osallistujia EI pidetä vastuullisina mistään ajoneuvollesi aiheutuvista vahingoista, takuiden raukeamisesta, omaisuusvahingoista tai henkilövahingoista, jotka johtuvat tämän ohjelmiston tai laitteiston käytöstä. **KÄYTÄ OMALLA VASTUULLA.**

## Kiitokset
* Inspiraationa ovat olleet [OVMS-projektin](https://github.com/openvehicles/Open-Vehicle-Monitoring-System-3) ja [Dalathegreatin](https://github.com/dalathegreat) (Dala's EV Repair) uskomaton työ Nissan Leafin CAN-väylien dokumentoinnissa.
